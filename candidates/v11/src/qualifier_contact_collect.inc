// SDK observations for qualifier::contact, included inside the backend's
// anonymous namespace after qualifier_contest(). No selection or action here.
// A modeled stationary fight and a currently checked local capture leg are
// separate facts. Neither certifies future enemies, receipt or altar delivery.
struct QualifierContactCast {
    enum class Target { None, Self, Camp, Ground };
    Target target = Target::None;
    int slot = -1, member = -1;
    vec2 tile{}, contact_position{};
};

struct QualifierContactRecord {
    int group_index = -1;
    std::unique_ptr<qualifier::contact::Input> input;
    qualifier::contact::Plan plan;
    std::array<Camp*, 16> members{};
    std::array<QualifierContactCast, 12> casts{};
    vec2 origin{};
    bool neutral_multiplier = false, capture_coverage_known = false;
    std::array<bool, 12> option_capture_coverage{};
    bitboard capture_region;
};

namespace qualifier_contact_detail {
using namespace qualifier::contact;

bool nonnegative(double value) { return std::isfinite(value) && value >= 0; }
std::uint64_t position_token(vec2 p) {
    return (std::uint64_t(static_cast<std::uint32_t>(p.x)) << 32)
        | static_cast<std::uint32_t>(p.y);
}

struct External {
    Camp* camp = nullptr;
    bool awake = false, sleeping = false, dead = false;
    bool combat_known = false;
    double strength = 0;
    int range = 0;
};

// Read cached observation provenance, never a predicted Group::st or last HP
// as current vision. A failed/unknown design range cannot bound an envelope.
bool external_facts(int gi, const bitboard& region, std::vector<External>& out) {
    if (!qualifier_current() || S->groups.size() > 512 || region.is_empty()) return false;
    const auto tiles = region.positions();
    if (tiles.size() > 1024) return false;
    int minx = S->W, miny = S->H, maxx = -1, maxy = -1;
    for (const auto p : tiles) {
        minx = std::min(minx, p.x); miny = std::min(miny, p.y);
        maxx = std::max(maxx, p.x); maxy = std::max(maxy, p.y);
    }
    std::size_t count = 0;
    for (int g = 0; g < static_cast<int>(S->groups.size()); ++g) {
        if (g == gi) continue;
        auto& group = S->groups[g];
        const bool group_dead = group.last_observed_round == S->round && group.observed_state != 0;
        for (auto& camp : group.camps) {
            if (++count > 4096 || (count % 64 == 0 && fuel_remaining() < 16000)) return false;
            External e; e.camp = &camp;
            e.dead = group_dead || (camp.health_observation_valid
                && camp.last_observed_round == S->round && camp.hp <= 0);
            const bool state_current = camp.state_observed_round == S->round;
            e.awake = !e.dead && state_current && camp.observed_awake;
            e.sleeping = !e.dead && state_current && !camp.observed_awake;
            e.combat_known = e.awake && camp.combat_observed_round == S->round
                && nonnegative(camp.observed_strength) && camp.observed_attack_range >= 0;
            if (e.combat_known) { e.strength = camp.observed_strength; e.range = camp.observed_attack_range; }
            else {
                if (!camp.design_known || !nonnegative(camp.str) || camp.range < 0) return false;
                e.strength = camp.str; e.range = camp.range;
            }
            const int dx = std::max({minx - camp.pos.x, 0, camp.pos.x - maxx});
            const int dy = std::max({miny - camp.pos.y, 0, camp.pos.y - maxy});
            // Also retain nearby sleepers/dead camps for collateral wake checks.
            if (std::max(dx, dy) > std::max(e.range, 8)) continue;
            if (out.size() == 128) return false;
            out.push_back(e);
        }
    }
    return qualifier_current();
}

bool trim_region(bitboard& region, const std::vector<External>& external) {
    bitboard danger;
    for (const auto& e : external) {
        // The selected group is assumed cleared for this static capture leg;
        // other not-confirmed-dead camps still occupy their public tiles.
        if (!e.dead) region.set(e.camp->pos, false);
        if (e.dead || e.sleeping) continue;
        // Both confirmed awake and unobserved potentially awake envelopes are
        // excluded. A sleeping camp is not labeled current incoming damage.
        danger.draw_chebyshev(e.camp->pos, static_cast<float>(e.range));
    }
    for (const auto& enemy : qualifier_runtime.enemies)
        danger.draw_chebyshev(enemy.pos, static_cast<float>(enemy.range + 1));
    region &= ~danger;
    region &= ~S->terrain;
    return qualifier_current() && !region.is_empty();
}

bool capture_routes(QualifierContactRecord& record) {
    bitboard roots = qualifier_receipt_goals(record.group_index) & record.capture_region;
    if (roots.is_empty()) return false;
    distance_field route;
    bitboard visited;
    route.compute(~record.capture_region, roots, visited);
    bool any = false;
    for (std::size_t o = 0; o < record.input->option_count; ++o) {
        const auto p = record.casts[o].contact_position;
        record.option_capture_coverage[o] = record.capture_region.is_set(p) && route.get(p) != UNREACH;
        any = any || record.option_capture_coverage[o];
    }
    return qualifier_current() && any;
}
} // namespace qualifier_contact_detail

// Only narrows a stored local mask. Failure invalidates it rather than leaving
// an older certificate usable. Root must refresh observation first each input.
bool qualifier_contact_refresh_capture_region(int gi, bitboard& region) {
    try {
        if (!qualifier_current() || gi < 0 || gi >= static_cast<int>(S->groups.size())
            || fuel_remaining() < 20000) { region.clear(); return false; }
        std::vector<qualifier_contact_detail::External> external;
        if (!qualifier_contact_detail::external_facts(gi, region, external)
            || !qualifier_contact_detail::trim_region(region, external)) {
            region.clear(); return false;
        }
        return true;
    } catch (const std::exception&) { region.clear(); return false; }
}

std::unique_ptr<QualifierContactRecord> qualifier_collect_contact(int gi, std::uint64_t revision) {
    using namespace qualifier::contact;
    namespace detail = qualifier_contact_detail;
    try {
        if (!qualifier_current() || fuel_remaining() < 45000 || gi < 0
            || gi >= static_cast<int>(S->groups.size())) return {};
        auto& s = *S;
        auto& me = self();
        auto& group = s.groups[gi];
        if (group.camps.empty() || group.camps.size() > max_members) return {};
        const auto flags = me.state(); // Actual post-sustain permissions.
        if (!flags.contains(champion_state::alive) || flags.contains(champion_state::recalling)
            || flags.contains(champion_state::submitting_gems) || flags.contains(champion_state::teleporting)) return {};
        auto record = std::make_unique<QualifierContactRecord>();
        record->group_index = gi;
        record->origin = me.position();
        record->input = std::make_unique<Input>();
        auto& in = *record->input;
        in.stamp = { static_cast<std::uint32_t>(static_cast<handle_champion_t>(me)),
                     static_cast<std::uint32_t>(static_cast<handle_camp_group_t>(group.h)),
                     static_cast<std::uint32_t>(s.round), revision };
        in.self_hp = me.health();
        in.self_hp_known = detail::nonnegative(in.self_hp) && in.self_hp > 0;
        in.can_attack = flags.contains(champion_state::can_attack);
        in.can_cast = flags.contains(champion_state::can_cast) && !s.cast;
        in.moved = s.moved; // Accepted-move tracker, not an inference from can_move.
        in.max_future_phases = static_cast<std::uint32_t>(std::max(0.0,
            static_cast<double>(constants::max_rounds) - s.round));
        in.member_count = group.camps.size();
        const double strength = me.strength();
        const int attack_range = me.attack_range();
        if (!detail::nonnegative(strength) || attack_range < 0) return {};
        const auto type = me.champion_name();
        // Their HP is separate, but a remote sibling death also kills self.
        // This local model has no qualified remote lethal-risk budget.
        const bool independent_health = type != champion_type::cassi && type != champion_type::issac;
        const double multiplier = me.base_stats().camp_damage_multiplier;
        record->neutral_multiplier = std::isfinite(multiplier) && multiplier == 1.0;
        std::array<vec2, max_members> positions{};
        Values incoming{};
        std::array<int, max_members> ranges{};
        std::array<std::uint32_t, max_members> observed_stun{};
        std::array<bool, max_members> incoming_known{};
        for (std::size_t i = 0; i < in.member_count; ++i) {
            auto& camp = group.camps[i];
            record->members[i] = &camp;
            auto& member = in.members[i];
            member.id = static_cast<std::uint32_t>(static_cast<handle_camp_t>(camp.h));
            positions[i] = camp.pos;
            member.hp = detail::nonnegative(camp.maxhp) ? camp.maxhp : 0;
            try {
                const auto state = camp.h.state();
                if (state.is_dead()) {
                    member.hp = 0; member.hp_known = member.effects_known = true;
                    incoming_known[i] = true;
                } else {
                    positions[i] = camp.h.position();
                    member.hp = camp.h.health();
                    member.hp_known = detail::nonnegative(member.hp);
                    const auto buffs = camp.h.buffs();
                    member.effects_known = !buffs.has_shield() && !buffs.has_enhanced_attack();
                    if (buffs.has_stun()) observed_stun[i] = static_cast<std::uint32_t>(std::max(0, buffs.get<buff_stun>().duration));
                    incoming[i] = camp.h.strength();
                    ranges[i] = camp.h.attack_range();
                    incoming_known[i] = detail::nonnegative(incoming[i]) && ranges[i] >= 0;
                }
            } catch (const std::exception&) { member.hp_known = member.effects_known = false; }
            record->capture_region.draw_chebyshev(camp.h.spawn_position(), 8.0f);
        }
        // Smith-style stationary basic-work ordering, not an optimum for splash:
        // known damaging members first by estimated hits / incoming Strength.
        // max(Strength,1) affects this ordering only, never actual damage credit.
        std::array<std::size_t, max_members> order{};
        for (std::size_t i = 0; i < in.member_count; ++i) order[i] = i;
        std::sort(order.begin(), order.begin() + in.member_count, [&](std::size_t a, std::size_t b) {
            if (in.members[a].hp_known != in.members[b].hp_known) return in.members[a].hp_known;
            const bool a_damaging = incoming_known[a] && incoming[a] > 0;
            const bool b_damaging = incoming_known[b] && incoming[b] > 0;
            if (a_damaging != b_damaging) return a_damaging;
            if (a_damaging && in.members[a].hp_known && in.members[b].hp_known) {
                const double a_key = std::ceil(in.members[a].hp / std::max(strength, 1.0)) / incoming[a];
                const double b_key = std::ceil(in.members[b].hp / std::max(strength, 1.0)) / incoming[b];
                if (a_key != b_key) return a_key < b_key;
            }
            return in.members[a].id < in.members[b].id;
        });
        for (std::size_t rank = 0; rank < in.member_count; ++rank)
            in.members[order[rank]].priority = static_cast<std::uint32_t>(rank);
        record->capture_region.draw_chebyshev(record->origin, 8.0f);
        record->capture_region &= ~s.terrain;
        std::vector<detail::External> external;
        const bool external_complete = detail::external_facts(gi, record->capture_region, external);
        const bool region_trimmed = external_complete && detail::trim_region(record->capture_region, external);
        ShieldCredit existing_shield;
        bool own_buffs_known = true, existing_pool = false;
        try {
            const auto buffs = me.buffs();
            existing_pool = buffs.has_shield();
            if (existing_pool) {
                const auto shield = buffs.get<buff_shield>();
                if (detail::nonnegative(shield.value) && shield.duration > 0)
                    existing_shield = { ShieldSource::Existing, shield.value,
                        static_cast<std::uint32_t>(shield.duration), true };
                else if (!detail::nonnegative(shield.value)) own_buffs_known = false;
            }
        } catch (const std::exception&) { own_buffs_known = false; }

        auto matrix = [&](vec2 contact, Reach& reach, DamageMatrix& damage) {
            for (std::size_t i = 0; i < in.member_count; ++i) {
                reach[i] = in.members[i].hp_known && in.members[i].hp > 0
                    && cheb(contact, positions[i]) <= attack_range;
                // Nominal Strength omits positive passive/item bonuses. It is
                // never used as qualified camp work for a nonneutral field.
                damage[i][i] = strength;
                if (type == champion_type::varg)
                    for (std::size_t j = 0; j < in.member_count; ++j)
                        if (i != j && cheb(positions[i], positions[j]) <= 1) damage[i][j] = strength;
            }
        };
        auto base_option = [&](vec2 contact) {
            Option option;
            option.metadata_known = option.prefix_geometry_known = true;
            option.prefix_legal = true;
            option.prefix_effects_known = own_buffs_known;
            option.damage_estimates_known = record->neutral_multiplier;
            option.exposure_known = independent_health && external_complete && qualifier_runtime.enemies.empty();
            option.future_wake_known = external_complete;
            option.tail_supported = true;
            option.control_qualified = true;
            option.contact_token = detail::position_token(contact);
            option.shield = existing_shield;
            matrix(record->origin, option.before_reach, option.before_basic);
            matrix(contact, option.contact_reach, option.contact_basic);
            for (std::size_t i = 0; i < in.member_count; ++i) {
                option.exposure_known = option.exposure_known && incoming_known[i];
                // Count every potentially awakened selected member in range,
                // including currently sleeping members, with no helper credit.
                if (cheb(contact, positions[i]) <= ranges[i]) option.incoming_per_phase[i] = incoming[i];
                option.initial_suppressed_phases[i] = observed_stun[i];
            }
            return option;
        };
        auto collateral = [&](Option& option, vec2 contact, vec2 center, double radius, bool area) {
            for (const auto& e : external) {
                bool hit = area && cheb(center, e.camp->pos) <= radius;
                if (type == champion_type::varg)
                    for (std::size_t i = 0; i < in.member_count; ++i)
                        hit = hit || (in.members[i].hp > 0 && cheb(positions[i], e.camp->pos) <= 1);
                // Damage wakes a whole external group. A harmless struck
                // member can wake a damaging sibling, so strength==0 must not
                // skip this gate. This local envelope does not qualify every
                // member of the touched group. Varg's later splash can also
                // hit a respawn at a currently dead member's public position.
                if (hit && (!e.dead || type == champion_type::varg))
                    option.future_wake_known = false;
                if (e.strength == 0) continue; // No direct damage after wake gate.
                if (e.dead || (e.sleeping && !hit) || cheb(contact, e.camp->pos) > e.range) continue;
                option.outside_per_phase += e.strength;
                if (!e.combat_known) option.exposure_known = false;
            }
            for (const auto& enemy : qualifier_runtime.enemies) {
                bool hit = area && cheb(center, enemy.pos) <= radius;
                if (type == champion_type::varg)
                    for (std::size_t i = 0; i < in.member_count; ++i)
                        hit = hit || (in.members[i].hp > 0 && cheb(positions[i], enemy.pos) <= 1);
                if (hit) option.prefix_effects_known = false; // Unmodeled retaliation.
            }
        };
        auto append = [&](Option option, QualifierContactCast cast) {
            if (in.option_count >= 9 || in.option_count >= max_options) return;
            const std::size_t index = in.option_count++;
            switch (cast.target) {
            case QualifierContactCast::Target::None: option.cast_token = 0; break;
            case QualifierContactCast::Target::Self: option.cast_token = in.stamp.actor; break;
            case QualifierContactCast::Target::Camp: option.cast_token = in.members[cast.member].id; break;
            case QualifierContactCast::Target::Ground: option.cast_token = detail::position_token(cast.tile); break;
            }
            record->casts[index] = cast;
            in.options[index] = std::move(option);
        };
        auto basic = base_option(record->origin);
        basic.kind = Kind::BasicOnly; basic.basics = Basics::After;
        basic.prefix_legal = in.can_attack;
        basic.post_basic_permitted = in.can_attack;
        collateral(basic, record->origin, record->origin, 0, false);
        append(std::move(basic), { QualifierContactCast::Target::None, -1, -1, {}, record->origin });

        const auto abilities = me.abilities();
        for (int slot = 0; slot < 2 && in.can_cast; ++slot) {
            if (!qualifier_current() || fuel_remaining() < 18000) break;
            const auto& ability = abilities[slot];
            if (!ability.info() || ability.cooldown() != 0) continue;
            const auto metadata = std::visit([](const auto& value) { return static_cast<ability_properties>(value); }, *ability.info());
            if (in.moved && metadata.exclusive_with_movement) continue;
            auto emit = [&](Option option, QualifierContactCast cast, bool reset = false) {
                option.cast_slot = slot; option.cast_ready = true;
                option.exclusive_with_movement = metadata.exclusive_with_movement;
                cast.slot = slot;
                option.basics = Basics::None;
                append(option, cast); // Cast-only also works outside basic reach.
                if (reset || in.can_attack) {
                    option.basics = reset && in.can_attack ? Basics::BeforeAndAfter : Basics::After;
                    option.post_basic_permitted = true;
                    option.reset_restores_attack = reset;
                    append(std::move(option), cast);
                }
            };
            if (ability.is_attack_reset() && ability.can_target_self()) {
                auto option = base_option(record->origin);
                option.kind = Kind::AttackReset;
                collateral(option, record->origin, record->origin, 0, false);
                // Marcella's enhanced-target stun is intentionally uncredited.
                emit(std::move(option), { QualifierContactCast::Target::Self, slot, -1, record->origin, record->origin }, true);
                continue;
            }
            if (ability.is_shield() && ability.can_target_self()) {
                auto option = base_option(record->origin);
                option.kind = Kind::Shield;
                option.shield = {}; // A selected shield cannot borrow an old pool.
                const auto shield = ability.get<ability_shield>();
                if (!existing_pool && own_buffs_known && shield.duration > 0
                    && (type == champion_type::varg || type == champion_type::heart_of_ruin)) {
                    const double amount = ability.shield_amount(me);
                    if (detail::nonnegative(amount)) option.shield = { ShieldSource::SelectedCast,
                        amount, static_cast<std::uint32_t>(shield.duration), true };
                }
                collateral(option, record->origin, record->origin, 0, false);
                emit(std::move(option), { QualifierContactCast::Target::Self, slot, -1, record->origin, record->origin });
                continue;
            }
            if (ability.is_heal()) continue; // Sustain already ran; no allied credit.
            struct Candidate { vec2 tile; int member; double score; };
            std::array<Candidate, 2> candidates{};
            int candidate_count = 0;
            auto candidate = [&](vec2 tile, int member, double score) {
                for (int i = 0; i < candidate_count; ++i) if (candidates[i].tile == tile) return;
                Candidate value{tile, member, score};
                int at = candidate_count;
                if (at == 2) {
                    if (score < candidates[1].score || (score == candidates[1].score
                        && detail::position_token(tile) >= detail::position_token(candidates[1].tile))) return;
                    at = 1;
                } else ++candidate_count;
                candidates[at] = value;
                while (at > 0 && (candidates[at].score > candidates[at - 1].score
                    || (candidates[at].score == candidates[at - 1].score
                        && detail::position_token(candidates[at].tile) < detail::position_token(candidates[at - 1].tile)))) {
                    std::swap(candidates[at], candidates[at - 1]); --at;
                }
            };
            double range = 0, radius = 0, damage = 0;
            std::uint32_t stun = 0;
            bool area = false, dash = false, damage_known = true;
            Kind kind = Kind::DirectDamage;
            if (ability.is_damage()) { range = ability.get<ability_damage>().range; damage = ability.damage_amount(); }
            else if (ability.is_cc()) {
                const auto cc = ability.get<ability_cc>(); range = cc.range;
                stun = static_cast<std::uint32_t>(std::max(0, cc.duration));
                damage = ability.damage_amount(); kind = Kind::CrowdControl;
            } else if (ability.is_aoe_damage()) {
                const auto aoe = ability.get<ability_aoe_damage>(); range = aoe.range; radius = aoe.radius;
                area = true; damage = ability.damage_amount(); kind = Kind::AreaDamage;
                // These current documented areas have one immediate hit. Never
                // multiply damage by payload duration or invent persistent ticks.
                damage_known = (type == champion_type::ryth && slot == 0)
                    || (type == champion_type::prism && slot == 1)
                    || (type == champion_type::stellar && slot == 1);
                if (type == champion_type::ryth && slot == 0 && aoe.duration > 0) stun = 1;
            } else if (ability.is_dash()) {
                const auto move = ability.get<ability_dash>(); range = move.range; radius = move.radius;
                dash = area = true; kind = Kind::Dash;
                damage = move.modifier > 0 ? ability.damage_amount() : 0;
                damage_known = move.modifier == 0 || type == champion_type::heart_of_ruin;
                if (type == champion_type::heart_of_ruin && move.duration > 0) stun = 1;
            } else continue;
            if (!detail::nonnegative(range) || !detail::nonnegative(radius)
                || !detail::nonnegative(damage) || range > 8 || radius > 8) continue;
            if (!damage_known) damage = 0;
            if (dash) {
                if (!ability.can_target_ground()) continue;
                // A walkable-path subset of legal dash paths, deliberately
                // omitting hole-crossing paths. The host still owns hidden blocks.
                auto blocked = s.terrain | vision::champion_tiles() | vision::camp_tiles() | vision::teleport_tiles();
                blocked.set(record->origin, false);
                const auto landings = blocked.reachable_tiles(record->origin, static_cast<std::uint16_t>(range)).positions();
                for (const auto p : landings) {
                    if (p == record->origin || cheb(p, record->origin) > range) continue;
                    double score = 0;
                    for (std::size_t i = 0; i < in.member_count; ++i) if (in.members[i].hp_known && in.members[i].hp > 0) {
                        if (cheb(p, positions[i]) <= radius) score += std::min(damage, in.members[i].hp);
                        if (cheb(p, positions[i]) <= attack_range) score += std::min(strength, in.members[i].hp);
                    }
                    if (score > 0) candidate(p, -1, score);
                }
            } else {
                if (area ? !ability.can_target_ground() : !ability.can_target_camps()) continue;
                for (std::size_t i = 0; i < in.member_count; ++i) {
                    if (!in.members[i].hp_known || in.members[i].hp <= 0 || cheb(record->origin, positions[i]) > range) continue;
                    double score = std::min(damage, in.members[i].hp) + (stun ? incoming[i] : 0);
                    if (area) for (std::size_t j = 0; j < in.member_count; ++j)
                        if (j != i && in.members[j].hp_known && cheb(positions[i], positions[j]) <= radius)
                            score += std::min(damage, in.members[j].hp) + (stun ? incoming[j] : 0);
                    candidate(positions[i], area ? -1 : static_cast<int>(i), score);
                }
                if (area && fuel_remaining() >= 20000) {
                    // Add currently unoccupied, walkable offset centers as well
                    // as member centers. At current range4 this scans <=81 tiles.
                    // Larger metadata still uses only this bounded local subset.
                    bitboard centers;
                    centers.draw_chebyshev(record->origin, static_cast<float>(std::min(range, 4.0)));
                    centers &= ~(s.terrain | vision::champion_tiles() | vision::camp_tiles() | vision::teleport_tiles());
                    for (const auto p : centers.positions()) {
                        double score = 0;
                        for (std::size_t i = 0; i < in.member_count; ++i)
                            if (in.members[i].hp_known && in.members[i].hp > 0 && cheb(p, positions[i]) <= radius)
                                score += std::min(damage, in.members[i].hp) + (stun ? incoming[i] : 0);
                        if (score > 0) candidate(p, -1, score);
                    }
                }
            }
            for (int c = 0; c < candidate_count; ++c) {
                const auto chosen = candidates[c];
                const vec2 contact = dash ? chosen.tile : record->origin;
                auto option = base_option(contact);
                option.kind = kind;
                option.cast_target_member = chosen.member;
                option.relocation_qualified = dash;
                option.damage_estimates_known = option.damage_estimates_known && damage_known;
                for (std::size_t i = 0; i < in.member_count; ++i) {
                    const bool hit = area ? cheb(chosen.tile, positions[i]) <= radius : chosen.member == static_cast<int>(i);
                    if (!hit || !in.members[i].hp_known || in.members[i].hp <= 0) continue;
                    option.cast_damage[i] = damage;
                    if (stun > 0) option.initial_suppressed_phases[i] = observed_stun[i] > 0
                        ? std::min(observed_stun[i], stun) : stun; // No additive control stacking.
                }
                collateral(option, contact, chosen.tile, radius, area);
                emit(std::move(option), { area ? QualifierContactCast::Target::Ground : QualifierContactCast::Target::Camp,
                    slot, chosen.member, chosen.tile, contact });
            }
        }
        record->capture_coverage_known = region_trimmed && detail::capture_routes(*record);
        if (!record->capture_coverage_known) record->capture_region.clear();
        if (!qualifier_current() || me.position() != record->origin) return {};
        // Root calls choose(), binds that exact option and demotes any nominal
        // nonneutral record. No reward/capture/delivery credit is assigned here.
        return record;
    } catch (const std::exception&) { return {}; }
}
