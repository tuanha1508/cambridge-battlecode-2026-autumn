// Owning live SDK -> pure resource programme collection. No actions here.
// Included inside the backend namespace after contact/owned-work helpers.
struct QualifierResourceFrame {
    std::unique_ptr<qualifier::resource::Input> input;
    std::array<int, 8> groups{};
    std::array<vec2, qualifier::resource::max_nodes> nodes{};
    std::array<bool, qualifier::resource::max_nodes> nodes_known{};
    std::array<std::optional<vec2>, qualifier::resource::max_nodes> current_steps{};
    std::array<std::array<QualifierContactCast, 2>, 8> casts{};
    std::array<QualifierDeliveryPlan, 8> deliveries{};
    vec2 origin{}, shop{};
    bool cargo_preserving_stats_known = false, collection_capped = false;
    std::array<int, 8> lifecycle_round{}, lifecycle_state{};
    std::optional<qualifier::resource::Delivery> cargo_bank_from_shop;
    QualifierDeliveryPlan cargo_bank_quote{};
    int origin_clear_group = -1;
    std::uint32_t origin_clear_id = 0;
};

namespace qualifier_resource_collect_detail {
namespace model = qualifier::resource;
bool nonnegative(double value) { return std::isfinite(value) && value >= 0; }
std::uint64_t token(vec2 p) { return qualifier_contact_detail::position_token(p); }
bool inside(vec2 p) { return p.x >= 0 && p.y >= 0 && p.x < S->W && p.y < S->H; }
std::uint32_t epoch(const Group& g) {
    // Same-input provenance token, NOT an engine respawn epoch. Freshness also
    // binds round/revision/full group and exact cached lifecycle separately.
    return (static_cast<std::uint32_t>(g.last_observed_round + 1) << 2)
        ^ static_cast<std::uint32_t>(g.observed_state + 1);
}
model::Passive passive(item_passive p) {
    using P = model::Passive;
    switch (p) {
    case item_passive::monster: return P::Monster;
    case item_passive::durability: return P::Durability;
    case item_passive::treasure: return P::Treasure;
    case item_passive::jungle: return P::Jungle;
    case item_passive::home: return P::Home;
    case item_passive::tribute: return P::Tribute;
    case item_passive::capture: return P::Capture;
    case item_passive::prospector: return P::Prospector;
    case item_passive::hunter: return P::Hunter;
    case item_passive::skirmish: return P::Skirmish;
    case item_passive::collectors: return P::Collectors;
    }
    return P::Unsupported;
}
struct Npc { vec2 pos{}; double strength = 0; int range = 0, group = -1; bool known = false; };
struct AbilityGeometry {
    double range = 0, radius = 0;
    bool area = false, exclusive = false, targets_camps = false, targets_ground = false;
};
bool passive_magnitude(model::Passive passive, double raw, double& normalized) {
    // Released autumn item table: percentages are normalized once here. SDK
    // magnitude units are accepted only when they agree with that exact rule
    // value as either a fraction or a percentage; unknown values stay unknown.
    using P = model::Passive;
    double expected = 0;
    switch (passive) {
    case P::Monster: expected = .15; break;
    case P::Durability: expected = .10; break;
    case P::Treasure: expected = .10; break;
    case P::Jungle: expected = .30; break;
    case P::Home: expected = .50; break;
    case P::Tribute: expected = .33; break;
    case P::Prospector: expected = .25; break;
    case P::Skirmish: expected = .05; break;
    case P::Collectors: expected = .01; break;
    case P::None: normalized = 0; return true;
    case P::Capture: normalized = 3; return nonnegative(raw);
    case P::Hunter: normalized = 50; return nonnegative(raw);
    default: return false;
    }
    if (!nonnegative(raw) || (std::abs(raw - expected) > 1e-5 && std::abs(raw / 100.0 - expected) > 1e-5)) return false;
    normalized = expected; return true;
}
bool fuel() { return qualifier_current() && fuel_remaining() >= 20000; }
bool npc_geometry(std::vector<Npc>& npcs, bitboard& spawn_tiles) {
    if (S->groups.size() > 512) return false;
    for (int g = 0; g < static_cast<int>(S->groups.size()); ++g) for (const auto& c : S->groups[g].camps) {
        if (npcs.size() == 4096 || (npcs.size() % 32 == 0 && !fuel())) return false;
        Npc n; n.pos = c.pos; n.group = g;
        n.known = c.design_known && nonnegative(c.str) && c.range >= 0 && c.range <= 8;
        if (!n.known) return false; // Never invent an unknown NPC's range bound.
        n.strength = c.str; n.range = c.range;
        if (c.combat_observed_round == S->round) {
            if (!nonnegative(c.observed_strength) || c.observed_attack_range < 0 || c.observed_attack_range > 8) n.known = false;
            else { n.strength = std::max(n.strength, double(c.observed_strength)); n.range = std::max(n.range, c.observed_attack_range); }
        }
        if (!n.known) return false;
        if (c.state_observed_round == S->round && !(c.health_observation_valid && c.last_observed_round == S->round && c.hp <= 0)
            && c.h.buffs().has_enhanced_attack()) return false;
        npcs.push_back(n); spawn_tiles.set(c.pos);
    }
    return true;
}
bool exposure(vec2 p, const std::vector<Npc>& npcs, double& value, int cleared = -1) {
    value = 0;
    for (const auto& n : npcs) {
        if (n.group == cleared || cheb(p, n.pos) > n.range) continue;
        if (!n.known) return false;
        value += n.strength;
    }
    return nonnegative(value);
}
bool path_bound(const std::vector<vec2>& path, const std::vector<Npc>& npcs,
                double turns, double& hp, int cleared = -1) {
    if (path.empty() || path.size() > 1025 || !nonnegative(turns)) return false;
    double upper = 0;
    for (std::size_t i = 0; i < path.size(); ++i) {
        if (!inside(path[i]) || S->terrain.is_set(path[i]) || (i && cheb(path[i - 1], path[i]) != 1)) return false;
        double at = 0; if (!exposure(path[i], npcs, at, cleared)) return false;
        upper = std::max(upper, at);
    }
    hp = upper * turns;
    return nonnegative(hp);
}
bool post_receipt_bound(const std::vector<vec2>& path, const std::vector<Npc>& npcs,
                       double turns, double& hp, int source, double dead_window) {
    if (!path_bound(path, npcs, turns, hp, source)) return false;
    double source_upper = 0;
    for (const auto p : path) {
        double at = 0;
        for (const auto& n : npcs) if (n.group == source && cheb(p, n.pos) <= n.range) {
            if (!n.known) return false;
            at += n.strength;
        }
        source_upper = std::max(source_upper, at);
    }
    // At the exact possible respawn phase, include one phase conservatively.
    const double active = turns >= dead_window ? turns - dead_window + 1 : 0;
    hp += source_upper * active;
    return nonnegative(hp);
}
bool trace(const distance_field& field, vec2 from, vec2 to, std::vector<vec2>& path, std::uint32_t& remaining) {
    if (!inside(from) || !inside(to) || field.get(from) == UNREACH) return false;
    path.push_back(from);
    while (from != to) {
        if (remaining == 0 || path.size() >= 1025 || !fuel()) return false;
        --remaining;
        const int old = field.get(from); vec2 best = from;
        for (int dy = -1; dy <= 1; ++dy) for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) continue;
            const vec2 next{from.x + dx, from.y + dy};
            if (inside(next) && field.get(next) != UNREACH && field.get(next) < old
                && (best == from || next.y < best.y || (next.y == best.y && next.x < best.x))) best = next;
        }
        if (best == from) return false;
        from = best; path.push_back(from);
    }
    return true;
}
int node_group(const QualifierResourceFrame& f, std::size_t node) {
    if (node == 0) return f.origin_clear_group;
    if (node >= 2 && node < 10) return f.groups[node - 2];
    if (node >= 10 && node < 18) return f.groups[node - 10];
    return -1;
}
bool camp_tile_allowed(vec2 p, const std::vector<Npc>& npcs, int from_cleared, int to_cleared) {
    for (const auto& n : npcs) if (n.pos == p && n.group != from_cleared && n.group != to_cleared) return false;
    return true;
}
} // namespace qualifier_resource_collect_detail

std::unique_ptr<QualifierResourceFrame> qualifier_collect_resource(bool primary_review_due);
bool qualifier_resource_frame_matches(const QualifierResourceFrame&);
bool qualifier_resource_goal_matches(const QualifierResourceFrame&, std::size_t);

std::unique_ptr<QualifierResourceFrame> qualifier_collect_resource(bool primary_review_due) {
    namespace d = qualifier_resource_collect_detail;
    namespace m = qualifier::resource;
    try {
        if (!qualifier_current() || fuel_remaining() < 45000 || S->groups.size() > 512) return {};
        auto& me = self(); const auto flags = me.state();
        if (!flags.contains(champion_state::alive) || me.is_recalling() || me.is_submitting_gems() || me.is_teleporting()) return {};
        auto f = std::make_unique<QualifierResourceFrame>(); f->groups.fill(-1);
        f->input = std::make_unique<m::Input>(); auto& in = *f->input;
        f->origin = me.position(); in.stamp = {static_cast<std::uint32_t>(static_cast<handle_champion_t>(me)),
            static_cast<std::uint32_t>(S->round), qualifier_runtime.observation_revision, d::token(f->origin)};
        in.primary_review_due = primary_review_due;
        in.horizon_turns = constants::max_rounds > static_cast<std::uint32_t>(S->round) ? constants::max_rounds - S->round : 0;
        in.horizon_turns = std::min<std::uint32_t>(2000, in.horizon_turns);
        in.operation_limit = 12000;
        in.known_team_submitted = global_submitted_gems(team::ally); in.win_target = constants::winning_gems;
        auto& a = in.actor; const auto type = me.champion_name(); const auto design = me.base_stats();
        a.hp = me.health(); a.max_hp = me.max_health(); a.strength = me.strength(); a.focus = me.focus();
        a.health_growth = design.health_growth; a.strength_growth = design.strength_growth; a.focus_growth = design.focus_growth;
        a.wallet = me.gold_count(); a.carried_gems = me.gem_count(); a.crystals = me.consumables()[consumable_type::tp_crystal].count();
        a.at_spawn = map::spawn_region(team::ally).is_set(f->origin); a.field_shop = type == champion_type::tavi;
        a.independent_health_known = type != champion_type::cassi && type != champion_type::issac;
        a.current_allowances_known = true;
        a.can_attack = flags.contains(champion_state::can_attack);
        a.can_cast = flags.contains(champion_state::can_cast) && !S->cast;
        a.can_move = flags.contains(champion_state::can_move);
        const auto buffs = me.buffs(); a.currently_stunned = buffs.has_stun();
        if (buffs.has_shield()) { const auto s = buffs.get<buff_shield>(); if (s.duration > 0 && d::nonnegative(s.value)) { a.shield = s.value; a.shield_rounds = s.duration; } }
        f->cargo_preserving_stats_known = !buffs.has_enhanced_attack() && !buffs.has_stun() && d::nonnegative(design.camp_damage_multiplier)
            && d::nonnegative(a.hp) && d::nonnegative(a.max_hp) && d::nonnegative(a.strength) && d::nonnegative(a.focus);
        a.projected_stats_known = f->cargo_preserving_stats_known && !(type == champion_type::prism && a.carried_gems > 0);
        if (type == champion_type::ryth) a.ryth_health_strength_ratio = constants::passive_abilities::ryth_health_strength_ratio;
        if (type == champion_type::vesper_vale) a.intrinsic_gold_bonus = constants::passive_abilities::vesper_vale_bonus_gold_percent / 100.0;
        if (type == champion_type::marcella) a.tribute_gold_per_gem = constants::passive_abilities::marcella_tribute_gold_per_gem;
        const auto owned = me.item_count();
        const auto item_count = static_cast<std::size_t>(item_type::count);
        if (item_count >= m::max_items) return {};
        for (std::size_t i = 0; i < item_count; ++i) {
            const auto item = static_cast<item_type>(i); const auto info = item.info(); auto& out = in.items[i];
            a.owned[i] = owned[item]; out.known = true; out.purchasable = info.purchasable;
            out.cost = info.cost; out.combine_count = info.combine_count;
            out.result = info.result ? static_cast<std::uint32_t>(*info.result) : m::no_item;
            out.health_points = info.health_points; out.strength_points = info.strength_points; out.focus_points = info.focus_points;
            out.passive = info.passive ? d::passive(*info.passive) : m::Passive::None;
            out.future_effect_known = d::passive_magnitude(out.passive, info.passive_magnitude, out.passive_magnitude);
        }
        in.crystal_item = m::max_items - 1;
        auto& crystal = in.items[in.crystal_item]; crystal.known = crystal.purchasable = crystal.future_effect_known = true;
        crystal.cost = consumable_type::tp_crystal.cost();
        // One fixed spawn endpoint throughout this programme, not a mixture of
        // separately optimistic nearest-spawn distances.
        int shop_distance = UNREACH;
        for (const auto p : map::spawn_region(team::ally).positions()) {
            const auto dist = S->df_me.get(p);
            if (dist != UNREACH && (shop_distance == UNREACH || dist < shop_distance
                || (dist == shop_distance && d::token(p) < d::token(f->shop)))) { shop_distance = dist; f->shop = p; }
        }
        if (shop_distance == UNREACH) return {};
        if (a.at_spawn) f->shop = f->origin;
        in.shop.known = in.shop.heals_health_delta = in.shop.consumables_legal = true;
        in.shop.token = d::token(f->shop); in.shop.current_purchase_witness = a.at_spawn && !a.currently_stunned;
        in.shop.full_recovery_on_owner_end = constants::spawn_healing >= 1.0f;
        in.shop.recovery_per_turn = d::nonnegative(constants::spawn_healing) ? a.max_hp * constants::spawn_healing : 0;
        f->nodes[0] = f->origin; f->nodes[1] = f->shop; f->nodes_known[0] = f->nodes_known[1] = true;
        const auto abilities = me.abilities(); std::array<d::AbilityGeometry, 2> ability_geometry{};
        for (std::size_t slot = 0; slot < 2; ++slot) {
            const auto& ab = abilities[slot]; auto& out = in.abilities[slot]; auto& geom = ability_geometry[slot];
            if (!ab.info()) continue;
            const auto common = std::visit([](const auto& v) { return static_cast<ability_properties>(v); }, *ab.info());
            const int cooldown = ab.cooldown(); if (cooldown < 0 || common.max_cooldown < 0) continue;
            out.max_cooldown = common.max_cooldown; out.cooldown = cooldown; geom.exclusive = common.exclusive_with_movement;
            out.self_target_allowed = ab.can_target_self(); geom.targets_camps = ab.can_target_camps(); geom.targets_ground = ab.can_target_ground();
            if (ab.is_damage()) { const auto x = ab.get<ability_damage>(); out.kind = m::AbilityKind::Damage; out.retarget_single = true; geom.range = x.range; out.focus_modifier = x.modifier; }
            else if (ab.is_cc()) { const auto x = ab.get<ability_cc>(); out.kind = m::AbilityKind::Control; out.retarget_single = true; geom.range = x.range; out.focus_modifier = x.modifier; out.duration = std::max(0, x.duration); }
            else if (ab.is_aoe_damage()) {
                const auto x = ab.get<ability_aoe_damage>();
                const bool immediate = (type == champion_type::ryth && slot == 0) || (type == champion_type::prism && slot == 1) || (type == champion_type::stellar && slot == 1);
                if (!immediate) continue;
                out.kind = type == champion_type::ryth ? m::AbilityKind::Control : m::AbilityKind::AreaDamage;
                out.multi_target = geom.area = true; geom.range = x.range; geom.radius = x.radius; out.focus_modifier = x.modifier;
                if (type == champion_type::ryth && x.duration > 0) out.duration = 1;
            } else if (ab.is_heal() && ab.can_target_self()) {
                const auto x = ab.get<ability_heal>(); out.kind = m::AbilityKind::Heal; out.focus_modifier = x.modifier;
                const auto amount = ab.heal_amount(); if (!d::nonnegative(amount) || amount + 1e-5 < a.focus * x.modifier) continue;
            } else if (ab.is_shield() && ab.can_target_self() && (type == champion_type::varg || type == champion_type::heart_of_ruin)) {
                const auto x = ab.get<ability_shield>(); out.kind = m::AbilityKind::Shield; out.focus_modifier = x.modifier; out.duration = std::max(0, x.duration);
                const auto amount = ab.shield_amount(); if (!d::nonnegative(amount) || amount + 1e-5 < a.focus * x.modifier) continue;
            } else continue;
            out.metadata_known = d::nonnegative(out.focus_modifier) && d::nonnegative(geom.range) && d::nonnegative(geom.radius)
                && geom.range <= 8 && geom.radius <= 8;
        }
        struct Candidate { int group = -1, distance = UNREACH; double work = 0; bool gem = false; };
        std::vector<Candidate> primary, funds;
        for (int gi = 0; gi < static_cast<int>(S->groups.size()); ++gi) {
            if (!d::fuel()) { f->collection_capped = true; break; }
            const auto& group = S->groups[gi]; if (group.camps.empty() || group.camps.size() > 16 || (group.gems <= 0 && group.gold <= 0)) continue;
            Candidate candidate; candidate.group = gi; candidate.gem = group.gems > 0;
            bool known = true;
            for (const auto& c : group.camps) { known = known && c.design_known && d::nonnegative(c.maxhp); candidate.work += c.maxhp; candidate.distance = std::min(candidate.distance, int(S->df_me.get(c.pos))); }
            if (!known || candidate.distance == UNREACH) continue;
            (candidate.gem ? primary : funds).push_back(candidate);
        }
        auto order = [](const Candidate& a, const Candidate& b) {
            const double ka = a.distance + std::sqrt(a.work), kb = b.distance + std::sqrt(b.work);
            return ka != kb ? ka < kb : a.group < b.group;
        };
        std::sort(primary.begin(), primary.end(), order); std::sort(funds.begin(), funds.end(), order);
        std::vector<Candidate> selected;
        const auto add = [&](const Candidate& candidate) {
            if (selected.size() == 8) return;
            for (const auto& old : selected) if (old.group == candidate.group) return;
            selected.push_back(candidate);
        };
        for (const auto& c : primary) if (c.group == S->qualifier_committed_target) { add(c); break; }
        if (!primary.empty()) {
            const auto largest = std::max_element(primary.begin(), primary.end(), [](const auto& x, const auto& y) {
                const auto gx = S->groups[x.group].gems, gy = S->groups[y.group].gems;
                return gx != gy ? gx < gy : x.distance > y.distance;
            });
            add(*largest); // Known hard access objectives cannot all lose to sqrt(HP).
        }
        for (const auto& c : primary) { if (selected.size() >= 4) break; add(c); }
        const auto primary_count = selected.size();
        for (const auto& c : funds) if (c.group == S->qualifier_committed_target) { add(c); break; }
        for (const auto& c : funds) { if (selected.size() >= primary_count + 4) break; add(c); }
        for (const auto& c : primary) { if (selected.size() >= 8) break; add(c); }
        for (const auto& c : funds) { if (selected.size() >= 8) break; add(c); }
        f->collection_capped = f->collection_capped || primary.size() + funds.size() > selected.size();
        std::vector<d::Npc> npcs; bitboard camp_tiles;
        if (!d::npc_geometry(npcs, camp_tiles)) { f->collection_capped = true; return f; }
        for (int gi = 0; gi < static_cast<int>(S->groups.size()); ++gi) {
            const auto& group = S->groups[gi];
            if (group.last_observed_round != S->round || group.observed_state == 0 || !qualifier_tiles(group).is_set(f->origin)) continue;
            if (f->origin_clear_group >= 0) return {};
            f->origin_clear_group = gi; f->origin_clear_id = static_cast<std::uint32_t>(static_cast<handle_camp_group_t>(group.h));
        }
        bitboard enemy_danger;
        for (const auto& e : qualifier_runtime.enemies) enemy_danger.draw_chebyshev(e.pos, float(e.range + 1));
        for (const auto& c : selected) {
            if (!d::fuel()) { f->collection_capped = true; break; }
            const auto index = in.goal_count++; f->groups[index] = c.group;
            auto& goal = in.goals[index]; const auto& group = S->groups[c.group];
            goal.group_id = static_cast<std::uint32_t>(static_cast<handle_camp_group_t>(group.h)); goal.epoch = d::epoch(group);
            f->lifecycle_round[index] = group.last_observed_round; f->lifecycle_state[index] = group.observed_state;
            goal.member_count = group.camps.size(); goal.composition_known = true; goal.solo_independent = a.independent_health_known;
            goal.receipt_known = group.gold >= 0 && group.gems >= 0; goal.nominal_gold = std::max(0, group.gold); goal.nominal_gems = std::max(0, group.gems);
            bool adjusted_share = false;
            if (group.last_observed_round == S->round && group.observed_state == 1) {
                const bool status_known = group.qualifier_capture_round == S->round
                    && (group.qualifier_capture_status == static_cast<int>(map::camp_group_capture_type::ally_capturing)
                        || group.qualifier_capture_status == static_cast<int>(map::camp_group_capture_type::capture_available));
                goal.receipt_known = goal.receipt_known && status_known;
                std::uint32_t recipients = 1;
                for (const auto& ally : vision::champions(team::ally))
                    if (static_cast<handle_champion_t>(ally) != static_cast<handle_champion_t>(me) && ally.is_alive()
                        && qualifier_tiles(group).is_set(ally.position())) ++recipients;
                goal.nominal_gold /= recipients; goal.nominal_gems /= recipients;
                adjusted_share = recipients > 1;
            }
            goal.capture_turns = std::max(0, constants::capture_time) + 1;
            goal.respawn_turns = std::max(0, group.respawn); goal.repeat_known = goal.respawn_turns > 0; goal.departure_turns = 1;
            if (group.last_observed_round == S->round && group.observed_state == 2) goal.available_in = std::max(0, group.respawn_round - S->round);
            bitboard forbidden = S->terrain | camp_tiles | enemy_danger;
            for (const auto& npc : npcs) if (npc.group != c.group) forbidden.draw_chebyshev(npc.pos, float(npc.range));
            bitboard contacts; const int attack_range = me.attack_range();
            double contact_range = std::max(1, attack_range);
            for (std::size_t slot = 0; slot < 2; ++slot) if (in.abilities[slot].metadata_known) contact_range = std::max(contact_range, ability_geometry[slot].range);
            for (const auto& member : group.camps) contacts.draw_chebyshev(member.pos, float(std::min(8.0, contact_range)));
            contacts &= ~forbidden;
            vec2 contact{}; bool contact_known = false; int best_hits = -1, best_distance = UNREACH; std::size_t scanned = 0;
            for (const auto p : contacts.positions()) {
                if (++scanned > 1024 || !d::fuel()) { f->collection_capped = true; break; }
                int hits = 0; for (const auto& member : group.camps) if (cheb(p, member.pos) <= attack_range) ++hits;
                const int distance = S->df_me.get(p); if (distance == UNREACH) continue;
                if (!contact_known || p == f->origin || (contact != f->origin && (hits > best_hits || (hits == best_hits && distance < best_distance)))) {
                    contact = p; contact_known = true; best_hits = hits; best_distance = distance;
                }
            }
            vec2 receipt{}; bool receipt_known = false; int receipt_distance = UNREACH;
            bitboard receipt_forbidden = S->terrain | enemy_danger;
            for (const auto& npc : npcs) if (npc.group != c.group) receipt_forbidden.draw_chebyshev(npc.pos, float(npc.range));
            for (const auto p : qualifier_tiles(group).positions()) {
                if (receipt_forbidden.is_set(p)) continue;
                const auto distance = S->df_altar.get(p);
                if (!receipt_known || p == f->origin || (receipt != f->origin && distance < receipt_distance)) { receipt = p; receipt_known = true; receipt_distance = distance; }
            }
            if (!contact_known && group.last_observed_round == S->round && group.observed_state == 1 && receipt_known) { contact = receipt; contact_known = true; }
            if (!contact_known || !receipt_known) { goal.future_geometry_known = false; continue; }
            goal.contact_token = d::token(contact); goal.contact_outside_spawn = !qualifier_tiles(group).is_set(contact);
            f->nodes[2 + index] = contact; f->nodes[10 + index] = receipt; f->nodes_known[2 + index] = f->nodes_known[10 + index] = true;
            goal.future_geometry_known = d::nonnegative(design.camp_damage_multiplier);
            goal.damage_pre_item_bonuses = goal.incoming_pre_item_mitigation = true;
            bool all_current = group.last_observed_round == S->round, all_dead = true, clusters = true;
            for (std::size_t j = 0; j < group.camps.size(); ++j) {
                const auto& camp = group.camps[j]; auto& member = goal.members[j];
                member.id = static_cast<std::uint32_t>(static_cast<handle_camp_t>(camp.h)); member.known = camp.design_known;
                const auto stats = camp.h.base_stats();
                member.respawn_hp_known = camp.design_known && d::nonnegative(camp.maxhp) && stats.health_growth == 0 && stats.strength_growth == 0 && stats.base_move_speed == 0;
                member.respawn_hp = camp.maxhp; member.hp = member.respawn_hp;
                if (camp.health_observation_valid && camp.last_observed_round == S->round) member.hp = camp.hp; else all_current = false;
                all_dead = all_dead && member.hp == 0;
                member.basic_factor = design.camp_damage_multiplier; member.basic_reachable = cheb(contact, camp.pos) <= attack_range;
                member.incoming_per_phase = cheb(contact, camp.pos) <= camp.range ? camp.str : 0;
                if (camp.combat_observed_round == S->round && cheb(contact, camp.pos) <= camp.observed_attack_range)
                    member.incoming_per_phase = std::max(member.incoming_per_phase, double(camp.observed_strength));
                goal.future_geometry_known = goal.future_geometry_known && member.known && member.respawn_hp_known && d::nonnegative(member.incoming_per_phase);
                clusters = clusters && (camp.h.is_gold_cluster() || camp.h.is_gem_cluster());
                if (camp.last_observed_round == S->round) { const auto b = camp.h.buffs(); if (b.has_shield() || b.has_enhanced_attack()) goal.future_geometry_known = false; }
            }
            goal.personal_reward_model_known = clusters && !adjusted_share;
            // Varg basic splash is omitted as damage, but collateral wake must
            // still be excluded rather than silently granting a solo envelope.
            if (type == champion_type::varg) for (const auto& npc : npcs) if (npc.group != c.group)
                for (const auto& member : group.camps) if (cheb(member.pos, npc.pos) <= 1) goal.solo_independent = false;
            goal.current_work_witness = all_current && contact == f->origin && goal.future_geometry_known && !a.currently_stunned
                && qualifier_runtime.enemies.empty();
            goal.current_capture_witness = all_current && all_dead && contact == f->origin
                && group.observed_state == 1 && goal.receipt_known && !enemy_danger.is_set(f->origin) && qualifier_runtime.enemies.empty();
            double current_exposure = 0;
            goal.current_inspect_witness = a.can_move && !a.currently_stunned && qualifier_runtime.recent_damage <= 0
                && qualifier_runtime.enemies.empty() && !qualifier_runtime.unresolved_awake_npc
                && d::exposure(f->origin, npcs, current_exposure) && current_exposure == 0;
            goal.current_wait_witness = contact == f->origin && goal.contact_outside_spawn && goal.available_in > 0
                && !enemy_danger.is_set(f->origin) && qualifier_runtime.enemies.empty();
            if (goal.current_capture_witness && group.qualifier_capture_round == S->round && group.qualifier_capture_timer >= 0
                && group.qualifier_capture_status == static_cast<int>(map::camp_group_capture_type::ally_capturing)) goal.capture_turns = group.qualifier_capture_timer;
            for (std::size_t slot = 0; slot < 2; ++slot) {
                const auto& ability = in.abilities[slot]; const auto& geom = ability_geometry[slot]; auto& cast = f->casts[index][slot];
                cast.slot = slot; cast.contact_position = contact;
                if (!ability.metadata_known) continue;
                if (ability.kind == m::AbilityKind::Heal || ability.kind == m::AbilityKind::Shield) {
                    cast.target = QualifierContactCast::Target::Self; cast.tile = contact;
                    goal.cast_tokens[slot] = in.stamp.actor;
                } else {
                    int selected_member = -1; std::size_t best_count = 0;
                    for (std::size_t j = 0; j < group.camps.size(); ++j) if (cheb(contact, group.camps[j].pos) <= geom.range) {
                        if (!geom.area && goal.current_work_witness && goal.members[j].hp <= 0) continue;
                        std::size_t hits = 1;
                        if (geom.area) {
                            hits = 0;
                            for (std::size_t k = 0; k < group.camps.size(); ++k)
                                if ((!goal.current_work_witness || goal.members[k].hp > 0)
                                    && cheb(group.camps[j].pos, group.camps[k].pos) <= geom.radius) ++hits;
                            if (goal.current_work_witness && hits == 0) continue;
                        }
                        if (selected_member < 0 || hits > best_count) { selected_member = j; best_count = hits; }
                    }
                    if (selected_member < 0) continue;
                    cast.member = geom.area ? -1 : selected_member; cast.tile = group.camps[selected_member].pos;
                    cast.target = geom.area ? QualifierContactCast::Target::Ground : QualifierContactCast::Target::Camp;
                    for (std::size_t j = 0; j < group.camps.size(); ++j) {
                        goal.members[j].cast_hit[slot] = geom.area ? cheb(cast.tile, group.camps[j].pos) <= geom.radius
                            : (geom.targets_camps && cheb(contact, group.camps[j].pos) <= geom.range);
                        goal.members[j].cast_factor[slot] = design.camp_damage_multiplier;
                    }
                    bool collateral = false;
                    if (geom.area) for (const auto& npc : npcs) if (npc.group != c.group && cheb(cast.tile, npc.pos) <= geom.radius) collateral = true;
                    if (collateral || (geom.area ? !geom.targets_ground : !geom.targets_camps)) { for (auto& member : goal.members) member.cast_hit[slot] = false; continue; }
                    goal.cast_tokens[slot] = geom.area ? d::token(cast.tile) : goal.members[selected_member].id;
                    goal.cast_target_ids[slot] = geom.area ? 0 : goal.members[selected_member].id;
                }
                goal.current_cast_witness[slot] = goal.current_work_witness && a.can_cast && ability.cooldown == 0 && !(S->moved && geom.exclusive);
            }
            if (goal.nominal_gems > 0 && d::fuel()) {
                const std::uint64_t upper_bag = clusters ? (std::uint64_t(goal.nominal_gems) * 125 + 99) / 100 : goal.nominal_gems;
                if (upper_bag <= std::numeric_limits<std::uint32_t>::max()) {
                    auto quote = qualifier_delivery_resource_quote(receipt, static_cast<double>(upper_bag));
                    if (quote.found() && quote.hypothetical_resource_quote && quote.resource_geometry_complete && quote.terminal_stance_known
                        && quote.origin == receipt && std::isfinite(quote.eta_upper) && quote.eta_upper >= 0 && quote.eta_upper <= in.horizon_turns) {
                        double before_hp = 0, after_hp = 0;
                        bool path_ok = d::post_receipt_bound(quote.resource_walk_path_before, npcs, quote.eta_upper, before_hp, c.group, goal.respawn_turns);
                        if (!quote.resource_walk_path_after.empty()) path_ok = path_ok && d::post_receipt_bound(quote.resource_walk_path_after, npcs, quote.eta_upper, after_hp, c.group, goal.respawn_turns);
                        for (const auto& path : {quote.resource_walk_path_before, quote.resource_walk_path_after}) for (const auto p : path)
                            if (!d::camp_tile_allowed(p, npcs, c.group, -1)) path_ok = false;
                        const auto occupied_after_respawn = [&](const std::vector<vec2>& path, std::size_t offset) {
                            for (std::size_t p = 0; p < path.size(); ++p) {
                                if (offset + p == 0 || offset + p < goal.respawn_turns) continue;
                                for (const auto& npc : npcs) if (npc.group == c.group && npc.pos == path[p]) return true;
                            }
                            return false;
                        };
                        const auto after_offset = quote.resource_walk_path_before.empty() ? 0 : quote.resource_walk_path_before.size() - 1
                            + static_cast<std::size_t>(std::max(0, constants::tp_crystal_time));
                        if (occupied_after_respawn(quote.resource_walk_path_before, 0)
                            || occupied_after_respawn(quote.resource_walk_path_after, after_offset)) path_ok = false;
                        if (path_ok) {
                            auto& delivery = goal.delivery; delivery.known = true; delivery.turns = static_cast<std::uint32_t>(std::ceil(quote.eta_upper));
                            delivery.crystals_used = quote.crystals_required; delivery.hp_cost_upper = before_hp + after_hp;
                            delivery.max_supported_gems = static_cast<std::uint32_t>(upper_bag); delivery.terminal_node = 18 + index;
                            delivery.endpoint_token = d::token(quote.terminal_stance);
                            f->nodes[18 + index] = quote.terminal_stance; f->nodes_known[18 + index] = true; f->deliveries[index] = std::move(quote);
                        }
                    }
                }
            }
        }
        // Static routes exclude every unselected NPC envelope. The selected
        // finite goal set is allowed only with a charged raw worst exposure.
        bitboard blocked = S->terrain | camp_tiles | enemy_danger;
        for (const auto& npc : npcs) {
            bool selected_group = false; for (std::size_t g = 0; g < in.goal_count; ++g) selected_group = selected_group || f->groups[g] == npc.group;
            if (!selected_group) blocked.draw_chebyshev(npc.pos, float(npc.range));
        }
        for (std::size_t n = 0; n < m::max_nodes; ++n) if (f->nodes_known[n] && !S->terrain.is_set(f->nodes[n])) blocked.set(f->nodes[n], false);
        std::uint32_t path_steps = 4096, fields = 0;
        const std::array<std::size_t, 17> destinations{{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17}};
        for (const auto to : destinations) {
            if (!f->nodes_known[to]) continue;
            if (!d::fuel() || fields++ >= 17 || std::uint64_t(S->W) * S->H > 16384) { f->collection_capped = true; break; }
            bitboard roots, visited; roots.set(f->nodes[to]); distance_field field; field.compute(blocked, roots, visited);
            for (std::size_t from = 0; from < m::max_nodes; ++from) {
                if (!f->nodes_known[from]) continue;
                auto& route = in.routes[from][to]; route.endpoint_token = d::token(f->nodes[to]);
                if (f->nodes[from] == f->nodes[to]) { route.known = route.same_endpoint_alias = true; continue; }
                std::vector<vec2> path;
                if (!d::trace(field, f->nodes[from], f->nodes[to], path, path_steps)) { if (!path_steps || !d::fuel()) f->collection_capped = true; continue; }
                const auto distance = path.size() - 1; double turns = static_cast<double>(distance);
                if (design.base_move_speed <= 0) continue;
                if (!d::nonnegative(turns) || turns > in.horizon_turns) continue;
                const int source_group = d::node_group(*f, from);
                const int cleared = from >= 2 && from < 10 && to == from + 8 ? source_group : -1;
                bool occupied_ok = true;
                const int destination_cleared = to >= 10 && to < 18 && from + 8 == to ? d::node_group(*f, to) : -1;
                for (const auto p : path) if (!d::camp_tile_allowed(p, npcs, source_group, destination_cleared)) occupied_ok = false;
                double hp_cost = 0;
                bool hp_known = false;
                if (cleared >= 0) hp_known = d::path_bound(path, npcs, turns, hp_cost, cleared);
                else if (source_group >= 0 && (from == 0 || (from >= 10 && from < 18))) {
                    const auto& source = S->groups[source_group];
                    const double window = from == 0 && source.observed_state == 2 ? std::max(0, source.respawn_round - S->round) : std::max(0, source.respawn);
                    hp_known = d::post_receipt_bound(path, npcs, turns, hp_cost, source_group, window);
                    for (std::size_t p = 1; p < path.size(); ++p) if (p >= window)
                        for (const auto& npc : npcs) if (npc.group == source_group && npc.pos == path[p]) occupied_ok = false;
                } else hp_known = d::path_bound(path, npcs, turns, hp_cost);
                if (!occupied_ok || !hp_known) continue;
                route.known = true; route.turns = static_cast<std::uint32_t>(std::ceil(turns)); route.hp_cost_upper = hp_cost;
                route.includes_destination_owner_end = false; // Explicit recovery follows the exact endpoint.
                const bool actual_origin = f->nodes[from] == f->origin;
                const bool actual_clear = cleared < 0 || (S->groups[cleared].last_observed_round == S->round
                    && S->groups[cleared].observed_state != 0);
                route.current_witness = actual_origin && actual_clear && a.can_move && !a.currently_stunned && qualifier_runtime.enemies.empty()
                    && path.size() > 1 && !(vision::champion_tiles() | vision::camp_tiles() | vision::teleport_tiles()).is_set(path[1]);
                if (route.current_witness) {
                    f->current_steps[to] = path[1];
                    // An exact currently cleared contact may alias node0. The
                    // executor consumes only this same actual-origin step.
                    if (from != 0 && (!in.routes[0][to].known || !in.routes[0][to].current_witness)) in.routes[0][to] = route;
                }
            }
        }
        if (a.carried_gems > 0 && d::fuel()) {
            auto quote = qualifier_delivery_resource_quote(f->shop, a.carried_gems);
            if (quote.found() && quote.hypothetical_resource_quote && quote.resource_geometry_complete
                && quote.terminal_stance_known && quote.origin == f->shop && d::nonnegative(quote.eta_upper)
                && quote.eta_upper <= in.horizon_turns) {
                double before_hp = 0, after_hp = 0;
                bool ok = d::path_bound(quote.resource_walk_path_before, npcs, quote.eta_upper, before_hp);
                if (!quote.resource_walk_path_after.empty()) ok = ok && d::path_bound(quote.resource_walk_path_after, npcs, quote.eta_upper, after_hp);
                for (const auto& path : {quote.resource_walk_path_before, quote.resource_walk_path_after}) for (const auto p : path)
                    if (!d::camp_tile_allowed(p, npcs, -1, -1)) ok = false;
                if (ok) {
                    m::Delivery bank; bank.known = true; bank.turns = static_cast<std::uint32_t>(std::ceil(quote.eta_upper));
                    bank.crystals_used = quote.crystals_required; bank.hp_cost_upper = before_hp + after_hp;
                    bank.max_supported_gems = a.carried_gems; bank.endpoint_token = d::token(quote.terminal_stance);
                    // Terminal node is unused: this separate actual-bag exit
                    // forecast ends at the completed deposit, never continues.
                    bank.terminal_node = 0; f->cargo_bank_from_shop = bank; f->cargo_bank_quote = std::move(quote);
                }
            }
        }
        if (!qualifier_resource_frame_matches(*f)) return {};
        return f;
    } catch (const std::exception&) { return {}; }
}

bool qualifier_resource_frame_matches(const QualifierResourceFrame& f) {
    try {
        if (!f.input || !qualifier_current()) return false;
        const auto& in = *f.input; const auto& a = in.actor; auto& me = self();
        if (in.stamp.actor != static_cast<std::uint32_t>(static_cast<handle_champion_t>(me))
            || in.stamp.round != static_cast<std::uint32_t>(S->round) || in.stamp.revision != qualifier_runtime.observation_revision
            || me.position() != f.origin || in.stamp.origin_token != qualifier_contact_detail::position_token(f.origin)
            || me.health() != a.hp || me.max_health() != a.max_hp || me.strength() != a.strength || me.focus() != a.focus
            || me.gold_count() != a.wallet || me.gem_count() != a.carried_gems
            || me.consumables()[consumable_type::tp_crystal].count() != a.crystals) return false;
        const auto inventory = me.item_count();
        for (std::size_t i = 0; i < static_cast<std::size_t>(item_type::count); ++i)
            if (inventory[static_cast<item_type>(i)] != a.owned[i]) return false;
        const auto flags = me.state();
        const bool same_flags = flags.contains(champion_state::alive) && flags.contains(champion_state::can_attack) == a.can_attack
            && (flags.contains(champion_state::can_cast) && !S->cast) == a.can_cast
            && flags.contains(champion_state::can_move) == a.can_move;
        if (!same_flags) return false;
        if (f.origin_clear_group >= 0) {
            if (f.origin_clear_group >= static_cast<int>(S->groups.size())) return false;
            const auto& group = S->groups[f.origin_clear_group];
            if (f.origin_clear_id != static_cast<std::uint32_t>(static_cast<handle_camp_group_t>(group.h))) return false;
            std::uint32_t state{}; detail::check(detail::sdk_camp_group_state(f.origin_clear_id, &state));
            if (state != 2 && state != 3) return false;
        }
        return in.stamp.actor == static_cast<std::uint32_t>(static_cast<handle_champion_t>(me))
            && in.stamp.round == static_cast<std::uint32_t>(round_number())
            && in.stamp.revision == qualifier_runtime.observation_revision && me.position() == f.origin && qualifier_current();
    } catch (const std::exception&) { return false; }
}

bool qualifier_resource_goal_matches(const QualifierResourceFrame& f, std::size_t index) {
    try {
        if (!qualifier_resource_frame_matches(f) || index >= f.input->goal_count || index >= f.groups.size()) return false;
        const int gi = f.groups[index]; if (gi < 0 || gi >= static_cast<int>(S->groups.size())) return false;
        const auto& group = S->groups[gi]; const auto& goal = f.input->goals[index];
        if (goal.group_id != static_cast<std::uint32_t>(static_cast<handle_camp_group_t>(group.h))
            || goal.epoch != qualifier_resource_collect_detail::epoch(group)
            || f.lifecycle_round[index] != group.last_observed_round || f.lifecycle_state[index] != group.observed_state
            || goal.member_count != group.camps.size()) return false;
        for (std::size_t j = 0; j < group.camps.size(); ++j) {
            if (goal.members[j].id != static_cast<std::uint32_t>(static_cast<handle_camp_t>(group.camps[j].h))) return false;
            if (goal.current_work_witness || goal.current_capture_witness) {
                const auto state = group.camps[j].h.state(); const auto hp = state.is_dead() ? 0.0 : double(group.camps[j].h.health());
                if (hp != goal.members[j].hp) return false;
            }
        }
        return qualifier_current();
    } catch (const std::exception&) { return false; }
}
