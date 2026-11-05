#pragma once
namespace qualifier::resource {
namespace program_detail {
inline bool finite_nonnegative(double x) { return std::isfinite(x) && x >= 0; }
struct State {
    Actor actor{};
    std::array<std::uint32_t, max_abilities> cooldown{};
    std::array<std::uint32_t, max_goals> available{};
    std::array<std::uint8_t, max_goals> receipts{};
    std::uint32_t elapsed = 0, node = 0, submitted = 0, win_turn = 0;
    std::uint32_t fund_receipts = 0, shop_visits = 0;
    std::uint64_t earned = 0;
    CurrentStage first{};
    PurchaseIntent milestone{};
    std::array<double, 14> passives{};
    bool win = false, observed_purchase = false, forecast_evaluated = false;
};
struct Search {
    const Input& in;
    BudgetCheck check;
    void* opaque;
    std::uint32_t operations = 0, expanded = 0;
    bool exhausted = false, unsupported = false, evidence = false, space_capped = false;
    std::uint32_t combat_deficit = 0; // 1=known HP failure, 2=known zero damage.
    std::uint32_t evaluated_goals_mask = 0;
    State best{};
    bool have_best = false;
    bool tick(std::uint32_t amount = 1) {
        if (exhausted) return false;
        if (amount > in.operation_limit || operations > in.operation_limit - amount) { exhausted = true; return false; }
        operations += amount;
        if ((operations & 63u) == 0 && check && !check(opaque)) exhausted = true;
        return !exhausted;
    }
};
inline void first_stage(const Input& in, State& s, StageKind kind, std::size_t g,
                        std::uint64_t token, bool witness) {
    if (s.first.kind != StageKind::None) return;
    s.first.kind = kind; s.first.stamp = in.stamp;
    s.first.endpoint_token = token; s.first.witness_known = witness;
    if (g < in.goal_count) {
        s.first.goal = static_cast<std::uint32_t>(g);
        s.first.group_id = in.goals[g].group_id; s.first.epoch = in.goals[g].epoch;
    }
}
inline bool advance(Search& q, State& s, std::uint32_t turns) {
    if (!q.tick() || turns > q.in.horizon_turns || s.elapsed > q.in.horizon_turns - turns) return false;
    s.elapsed += turns;
    for (auto& cd : s.cooldown) cd = turns >= cd ? 0 : cd - turns;
    if (turns > s.actor.shield_rounds) { s.actor.shield = 0; s.actor.shield_rounds = 0; }
    else s.actor.shield_rounds -= turns;
    return true;
}
inline bool route(Search& q, State& s, std::uint32_t destination, StageKind kind, std::size_t g) {
    if (destination >= max_nodes || s.node >= max_nodes) return false;
    if (s.node == destination) return true;
    if (destination == 1 && s.shop_visits >= 4) { q.space_capped = true; return false; }
    const auto& r = q.in.routes[s.node][destination];
    if (!r.known || !finite_nonnegative(r.hp_cost_upper) || !(s.actor.hp > r.hp_cost_upper)
        || r.gold_cost > s.actor.wallet || r.crystals_used > s.actor.crystals) return false;
    // A zero-cost zero-time edge can alias the actual origin to the chosen
    // contact/shop. It is not a movement action and must not hide Work/Buy.
    if (r.turns == 0) {
        if (!r.same_endpoint_alias || r.gold_cost != 0 || r.crystals_used != 0 || r.hp_cost_upper != 0) return false;
        s.node = destination; if (destination == 1) ++s.shop_visits; return true;
    }
    first_stage(q.in, s, kind, g, r.endpoint_token, r.current_witness && s.elapsed == 0);
    if (!advance(q, s, r.turns)) return false;
    s.actor.wallet -= r.gold_cost; s.actor.crystals -= r.crystals_used; s.node = destination;
    s.actor.hp -= r.hp_cost_upper;
    if (destination == 1) ++s.shop_visits;
    if (destination == 1 && r.includes_destination_owner_end && q.in.shop.full_recovery_on_owner_end)
        s.actor.hp = s.actor.max_hp;
    return true;
}
inline bool recover(Search& q, State& s) {
    if (s.node != 1 || !q.in.shop.known) return false;
    const double missing = s.actor.max_hp - s.actor.hp;
    if (!(missing > 1e-8)) return true;
    const auto rate = q.in.shop.recovery_per_turn;
    if (!q.in.shop.full_recovery_on_owner_end && !(std::isfinite(rate) && rate > 0)) return true; // No invented recovery.
    const double needed = q.in.shop.full_recovery_on_owner_end ? 1 : std::ceil(missing / rate);
    if (!std::isfinite(needed) || needed > q.in.horizon_turns) return false;
    first_stage(q.in, s, StageKind::Recover, max_goals, q.in.shop.token, q.in.actor.at_spawn);
    if (!advance(q, s, static_cast<std::uint32_t>(needed))) return false;
    s.actor.hp = s.actor.max_hp;
    return true;
}
inline bool refresh_passives(Search& q, State& s) {
    if (!q.tick(static_cast<std::uint32_t>(max_items))) return false;
    s.passives.fill(0);
    for (std::size_t i = 0; i < max_items; ++i) {
        const auto& m = q.in.items[i];
        const auto p = static_cast<std::size_t>(m.passive);
        if (s.actor.owned[i] && m.known && m.future_effect_known && p < s.passives.size()) {
            if (!finite_nonnegative(m.passive_magnitude)) return false;
            s.passives[p] = std::max(s.passives[p], m.passive_magnitude);
        }
    }
    return true;
}
inline double passive(const Input&, const State& s, Passive wanted) { return s.passives[static_cast<std::size_t>(wanted)]; }
inline bool incoming(Search& q, State& s, const Goal& goal,
                     const std::array<double, max_members>& hp,
                     std::array<std::uint32_t, max_members>& stun) {
    double damage = 0;
    // Choose only the strongest documented reduction, conservatively omitting
    // the other rather than inventing a product/stacking rule.
    const double reduction = goal.incoming_pre_item_mitigation
        ? std::max(passive(q.in, s, Passive::Jungle), passive(q.in, s, Passive::Durability)) : 0;
    if (!finite_nonnegative(reduction) || reduction > 1) return false;
    for (std::size_t m = 0; m < goal.member_count; ++m) {
        if (hp[m] > 0 && stun[m] == 0) damage += goal.members[m].incoming_per_phase;
        if (stun[m]) --stun[m]; // NPC timers decrement at NPC phase end.
    }
    damage *= 1 - reduction;
    if (!finite_nonnegative(damage)) return false;
    const auto absorbed = std::min(s.actor.shield, damage);
    s.actor.shield -= absorbed; damage -= absorbed;
    // Zero shields expire at champion start, AFTER this round's NPC phase.
    if (s.actor.shield_rounds == 0) s.actor.shield = 0;
    if (!(s.actor.hp > damage)) { q.combat_deficit = 1; return false; }
    s.actor.hp -= damage;
    return true;
}
inline bool fight(Search& q, State& s, std::size_t gi) {
    if (!q.tick()) return false;
    q.evaluated_goals_mask |= std::uint32_t{1} << gi;
    q.combat_deficit = 0;
    const auto& goal = q.in.goals[gi];
    const bool current_cleared_capture = s.elapsed == 0 && goal.current_capture_witness && s.receipts[gi] == 0;
    if (current_cleared_capture && goal.composition_known && goal.member_count > 0 && goal.member_count <= max_members) {
        for (std::size_t m = 0; m < goal.member_count; ++m)
            if (!goal.members[m].known || goal.members[m].hp != 0) { q.unsupported = true; return false; }
        return true; // Exact cleared-capture witness needs no invented fight.
    }
    if (!goal.composition_known || !goal.future_geometry_known || !goal.solo_independent
        || !s.actor.independent_health_known || !s.actor.projected_stats_known || goal.member_count == 0 || goal.member_count > max_members) {
        q.unsupported = true; return false;
    }
    std::array<double, max_members> hp{};
    std::array<std::uint32_t, max_members> stun{};
    for (std::size_t a = 0; a < max_abilities; ++a) {
        const auto& ability = q.in.abilities[a];
        if (!ability.multi_target && !ability.retarget_single && ability.kind != AbilityKind::AreaDamage
            && (ability.kind == AbilityKind::Damage || ability.kind == AbilityKind::Control)) {
            std::size_t hits = 0;
            for (std::size_t m = 0; m < goal.member_count; ++m) hits += goal.members[m].cast_hit[a] ? 1 : 0;
            if (hits > 1) { q.unsupported = true; return false; }
        }
    }
    for (std::size_t m = 0; m < goal.member_count; ++m) {
        const auto& x = goal.members[m];
        if (!x.known || !finite_nonnegative(x.hp) || !finite_nonnegative(x.incoming_per_phase)
            || !finite_nonnegative(x.basic_factor)) { q.unsupported = true; return false; }
        const bool current_observed_work = s.elapsed == 0 && goal.current_work_witness && s.receipts[gi] == 0;
        if (!current_observed_work && (!x.respawn_hp_known || !finite_nonnegative(x.respawn_hp))) { q.unsupported = true; return false; }
        // Never carry wounded observed camp HP through an unobserved funding,
        // purchase, recovery or route interval. No chip persistence is assumed.
        hp[m] = current_observed_work ? x.hp : x.respawn_hp;
    }
    const double damage_bonus = goal.damage_pre_item_bonuses ? passive(q.in, s, Passive::Monster) : 0;
    if (!finite_nonnegative(damage_bonus)) return false;
    for (std::uint32_t turn = 0; turn < q.in.combat_turn_limit; ++turn) {
        if (!q.tick()) return false;
        bool alive = false;
        for (std::size_t m = 0; m < goal.member_count; ++m) alive = alive || hp[m] > 0;
        if (!alive) return true;
        first_stage(q.in, s, StageKind::Work, gi, goal.contact_token,
                    goal.current_work_witness && s.elapsed == 0 && s.actor.current_allowances_known && !s.actor.currently_stunned);
        // Only an actual current work witness may act before the next NPC
        // phase. Every remote/future arrival pays an NPC phase first.
        const bool current_prefix = turn == 0 && s.elapsed == 0 && goal.current_work_witness;
        if (!current_prefix && !incoming(q, s, goal, hp, stun)) {
            if (!goal.chip_recover_evidence) q.evidence = true;
            return false;
        }
        int target = -1;
        double basic = 0;
        for (std::size_t m = 0; m < goal.member_count; ++m) {
            const auto& x = goal.members[m];
            const auto value = s.actor.strength * x.basic_factor * (1 + damage_bonus);
            if (!finite_nonnegative(value)) return false;
            if (hp[m] > 0 && x.basic_reachable && value > basic) { basic = value; target = static_cast<int>(m); }
        }
        int cast = -1;
        std::array<int, max_abilities> single_target{}; single_target.fill(-1);
        if (current_prefix && !s.actor.can_attack) { target = -1; basic = 0; }
        double best_score = basic;
        for (std::size_t a = 0; a < max_abilities; ++a) {
            const auto& ab = q.in.abilities[a];
            if (!ab.metadata_known || s.cooldown[a] || !finite_nonnegative(ab.focus_modifier)
                || (current_prefix && (!s.actor.can_cast || !goal.current_cast_witness[a]))) continue;
            const double effect = s.actor.focus * ab.focus_modifier;
            if (!finite_nonnegative(effect)) return false;
            double score = 0;
            if (ab.kind == AbilityKind::Heal && ab.self_target_allowed)
                score = std::min(effect, s.actor.max_hp - s.actor.hp);
            else if (ab.kind == AbilityKind::Shield && ab.self_target_allowed && ab.duration > 0)
                score = std::max(0.0, effect - s.actor.shield);
            else if (ab.kind == AbilityKind::Damage || ab.kind == AbilityKind::AreaDamage || ab.kind == AbilityKind::Control) {
                for (std::size_t m = 0; m < goal.member_count; ++m) if (hp[m] > 0 && goal.members[m].cast_hit[a]) {
                    if (ab.retarget_single && current_prefix && goal.members[m].id != goal.cast_target_ids[a]) continue;
                    const auto factor = goal.members[m].cast_factor[a];
                    if (!finite_nonnegative(factor)) { q.unsupported = true; return false; }
                    double member_score = std::min(hp[m], effect * factor * (1 + damage_bonus));
                    if (ab.kind == AbilityKind::Control && ab.duration > 0)
                        member_score += goal.members[m].incoming_per_phase;
                    if (ab.retarget_single) {
                        if (single_target[a] < 0 || member_score > score) { single_target[a] = static_cast<int>(m); score = member_score; }
                    } else score += member_score;
                }
            }
            // Resets are safely omitted under the one cast OR basic schedule.
            if (score > best_score + 1e-8) { cast = static_cast<int>(a); best_score = score; }
        }
        if (cast >= 0) {
            const auto& ab = q.in.abilities[static_cast<std::size_t>(cast)];
            const double effect = s.actor.focus * ab.focus_modifier;
            if (ab.kind == AbilityKind::Heal) s.actor.hp = std::min(s.actor.max_hp, s.actor.hp + effect);
            else if (ab.kind == AbilityKind::Shield) { s.actor.shield = effect; s.actor.shield_rounds = ab.duration; }
            else for (std::size_t m = 0; m < goal.member_count; ++m) if (hp[m] > 0 && goal.members[m].cast_hit[static_cast<std::size_t>(cast)]) {
                if (ab.retarget_single && static_cast<int>(m) != single_target[static_cast<std::size_t>(cast)]) continue;
                hp[m] = std::max(0.0, hp[m] - effect * goal.members[m].cast_factor[static_cast<std::size_t>(cast)] * (1 + damage_bonus));
                if (ab.kind == AbilityKind::Control && ab.duration > 0) stun[m] = ab.duration;
            }
            s.cooldown[static_cast<std::size_t>(cast)] = ab.max_cooldown;
            if (current_prefix && s.first.kind == StageKind::Work) {
                s.first.action = ActionKind::Cast; s.first.cast_slot = cast;
                s.first.cast_token = goal.cast_tokens[static_cast<std::size_t>(cast)];
                s.first.target_member_id = goal.cast_target_ids[static_cast<std::size_t>(cast)];
            }
        } else if (target >= 0 && basic > 0) {
            hp[static_cast<std::size_t>(target)] = std::max(0.0, hp[static_cast<std::size_t>(target)] - basic);
            if (current_prefix && s.first.kind == StageKind::Work) {
                s.first.action = ActionKind::Basic;
                s.first.target_member_id = goal.members[static_cast<std::size_t>(target)].id;
            }
        }
        else {
            bool future_damage_cast = false;
            for (std::size_t a = 0; a < max_abilities; ++a) {
                const auto& ability = q.in.abilities[a];
                const bool damaging = ability.kind == AbilityKind::Damage || ability.kind == AbilityKind::AreaDamage || ability.kind == AbilityKind::Control;
                if (!ability.metadata_known || !damaging || !(s.actor.focus * ability.focus_modifier > 0)
                    || (s.cooldown[a] == 0 && !(current_prefix && !s.actor.can_cast))) continue;
                for (std::size_t m = 0; m < goal.member_count; ++m)
                    future_damage_cast = future_damage_cast || (hp[m] > 0 && goal.members[m].cast_hit[a]
                        && goal.members[m].cast_factor[a] > 0);
            }
            // A finite known damaging cooldown is real future progress. This
            // idle still pays every NPC phase and timer tick in the same loop.
            // Current ActionNone owns only a freshly qualified stationary wait.
            if (!future_damage_cast) { q.combat_deficit = 2; q.unsupported = true; return false; }
        }
        if (!advance(q, s, 1)) return false;
        alive = false;
        for (std::size_t m = 0; m < goal.member_count; ++m) alive = alive || hp[m] > 0;
        if (!alive) return true;
    }
    q.space_capped = true;
    return false;
}
inline bool receive(Search& q, State& s, std::size_t gi, bool funding) {
    const auto& goal = q.in.goals[gi];
    if (funding && s.fund_receipts >= 32) { q.space_capped = true; return false; }
    if (s.win || !goal.receipt_known || s.actor.carried_gems != 0) return false;
    if (s.receipts[gi] && (!goal.repeat_known || goal.departure_turns == 0 || goal.respawn_turns == 0)) return false;
    if (s.receipts[gi]) {
        // Never remain on an occupied spawn and silently manufacture a respawn.
        if (s.node == 2 + gi || s.node == 10 + gi) {
            const auto departure = goal.contact_outside_spawn ? static_cast<std::uint32_t>(2 + gi) : 1u;
            if (!route(q, s, departure, departure == 1 ? StageKind::ReturnShop : StageKind::Approach, gi)) return false;
        }
    }
    if (s.elapsed < s.available[gi] && goal.contact_outside_spawn) {
        // Travel to the qualified outside-spawn waiting contact first. A
        // future wait must not steal a legal current Approach witness.
        if (!route(q, s, static_cast<std::uint32_t>(2 + gi), StageKind::Approach, gi)) return false;
    }
    if (s.elapsed < s.available[gi]) {
        first_stage(q.in, s, StageKind::WaitRespawn, gi, goal.contact_token,
                    s.elapsed == 0 && goal.current_wait_witness && goal.contact_outside_spawn && s.node == 2 + gi);
        if (!advance(q, s, s.available[gi] - s.elapsed)) return false;
    }
    if (!route(q, s, static_cast<std::uint32_t>(2 + gi), StageKind::Approach, gi)) return false;
    if (!fight(q, s, gi)) return false;
    // A stationary ranged contact is not automatically a capture spawn tile.
    if (!route(q, s, static_cast<std::uint32_t>(10 + gi), StageKind::Approach, gi)) return false;
    first_stage(q.in, s, StageKind::Capture, gi, q.in.routes[2 + gi][10 + gi].endpoint_token,
                goal.current_capture_witness && s.elapsed == 0);
    auto capture = goal.capture_turns;
    if (passive(q.in, s, Passive::Capture) > 0) capture = std::min<std::uint32_t>(capture, 3);
    if (!advance(q, s, capture)) return false;
    std::uint64_t gold = goal.nominal_gold, gems = goal.nominal_gems;
    if (goal.personal_reward_model_known) {
        // Solo base shares are supplied before personal bonuses. Reward bonuses
        // are not applied to an already bonus-inflated observed receipt.
        const auto prospector = passive(q.in, s, Passive::Prospector);
        const auto bonus = q.in.actor.intrinsic_gold_bonus + passive(q.in, s, Passive::Treasure);
        if (!finite_nonnegative(prospector) || !finite_nonnegative(bonus)) return false;
        const double rewarded_gold = std::floor(std::floor(static_cast<double>(gold) * (1 + prospector)) * (1 + bonus));
        const double rewarded_gems = std::floor(static_cast<double>(gems) * (1 + prospector));
        if (!finite_nonnegative(rewarded_gold) || !finite_nonnegative(rewarded_gems)
            || rewarded_gold > std::numeric_limits<std::uint32_t>::max()
            || rewarded_gems > std::numeric_limits<std::uint32_t>::max()) return false;
        gold = static_cast<std::uint64_t>(rewarded_gold); gems = static_cast<std::uint64_t>(rewarded_gems);
    }
    if (gold > std::numeric_limits<std::uint32_t>::max() - s.actor.wallet
        || gems > std::numeric_limits<std::uint32_t>::max() - s.actor.carried_gems
        || gold > std::numeric_limits<std::uint64_t>::max() - s.earned) return false;
    s.actor.wallet += static_cast<std::uint32_t>(gold); s.earned += gold;
    s.actor.carried_gems += static_cast<std::uint32_t>(gems);
    ++s.receipts[gi]; if (funding) ++s.fund_receipts;
    const auto cooldown = std::uint64_t(s.elapsed) + goal.respawn_turns + goal.departure_turns;
    s.available[gi] = cooldown > q.in.horizon_turns ? q.in.horizon_turns : static_cast<std::uint32_t>(cooldown);
    return true;
}
inline bool deliver(Search& q, State& s, std::size_t gi) {
    if (s.actor.carried_gems == 0) return true;
    const auto& d = q.in.goals[gi].delivery;
    const auto capacity = d.max_supported_gems ? d.max_supported_gems : q.in.goals[gi].nominal_gems;
    if (s.actor.carried_gems > capacity) { q.unsupported = true; return false; }
    if (!d.known || !finite_nonnegative(d.hp_cost_upper) || !(s.actor.hp > d.hp_cost_upper)
        || d.terminal_node >= max_nodes || d.gold_cost > s.actor.wallet || d.crystals_used > s.actor.crystals) return false;
    first_stage(q.in, s, StageKind::Deliver, gi, d.endpoint_token, d.current_witness && s.elapsed == 0);
    if (!advance(q, s, d.turns)) return false;
    s.actor.hp -= d.hp_cost_upper;
    s.actor.wallet -= d.gold_cost; s.actor.crystals -= d.crystals_used;
    if (s.actor.carried_gems > std::numeric_limits<std::uint32_t>::max() - s.submitted) return false;
    const auto tribute_gold = std::uint64_t(s.actor.carried_gems) * q.in.actor.tribute_gold_per_gem;
    if (tribute_gold > std::numeric_limits<std::uint32_t>::max() - s.actor.wallet
        || tribute_gold > std::numeric_limits<std::uint64_t>::max() - s.earned) return false;
    s.actor.wallet += static_cast<std::uint32_t>(tribute_gold); s.earned += tribute_gold;
    s.submitted += s.actor.carried_gems; s.actor.carried_gems = 0;
    s.node = d.terminal_node;
    if (!s.win && q.in.win_target > 0 && std::uint64_t(q.in.known_team_submitted) + s.submitted >= q.in.win_target) {
        s.win = true; s.win_turn = s.elapsed;
    }
    return true;
}
inline bool better(const State& a, const State& b) {
    if (a.win != b.win) return a.win;
    if (a.win && a.win_turn != b.win_turn) return a.win_turn < b.win_turn;
    if (a.submitted != b.submitted) return a.submitted > b.submitted;
    if (a.earned != b.earned) return a.earned > b.earned;
    if (a.elapsed != b.elapsed) return a.elapsed < b.elapsed;
    return a.actor.hp > b.actor.hp;
}
inline bool stage_qualified(const State& s) { return s.first.kind != StageKind::None && s.first.witness_known; }
inline void consider(Search& q, const State& s) {
    if (stage_qualified(s) && (!q.have_best || better(s, q.best))) { q.best = s; q.have_best = true; }
}
inline bool fund_and_shop(Search& q, State& s, std::uint32_t cost) {
    // Several small receipts can close one real milestone. Each receipt pays
    // its own contact, NPC phases, capture, deposit, and return/recovery.
    while (true) {
        if (s.win) return false; // No shopping or farming after a winning bank.
        State at_shop = s;
        if (s.actor.wallet >= cost && route(q, at_shop, 1, StageKind::ReturnShop, max_goals)) {
            // Buy an affordable Health milestone before paying to recover the
            // very HP that the actual spawn purchase will supply.
            if (at_shop.actor.wallet >= cost) { s = at_shop; return true; }
        }
        if (q.exhausted || s.fund_receipts >= 32) { if (s.fund_receipts >= 32) q.space_capped = true; return false; }
        bool found = false; State chosen{};
        for (std::size_t g = 0; g < q.in.goal_count; ++g) {
            if (!q.tick() || (q.in.goals[g].nominal_gold == 0
                && (q.in.actor.tribute_gold_per_gem == 0 || q.in.goals[g].nominal_gems == 0))) continue;
            State next = s;
            const auto before = next.actor.wallet, begin = next.elapsed;
            if (!receive(q, next, g, true) || !deliver(q, next, g)) continue;
            consider(q, next); // A deposit can win before its intended buy.
            if (next.actor.wallet <= before) continue;
            // Greedy funding is an explicit bounded search heuristic, never a
            // proof that an unselected alternative cannot provide access.
            const std::uint64_t gain = next.actor.wallet - before;
            const auto turns = std::max<std::uint32_t>(1, next.elapsed - begin);
            const auto best_gain = found ? chosen.actor.wallet - before : 0;
            const auto best_turns = found ? std::max<std::uint32_t>(1, chosen.elapsed - begin) : 1;
            if (!found || gain * best_turns > std::uint64_t(best_gain) * turns) { chosen = next; found = true; }
        }
        if (!found) {
            // Recover only when no safe next receipt is modelled. Do not force
            // a shop visit after every small receipt or cap funding at3farms.
            State recovery = s;
            const auto old_hp = recovery.actor.hp;
            const auto old_node = recovery.node;
            if (!route(q, recovery, 1, StageKind::ReturnShop, max_goals) || !recover(q, recovery)
                || (!(recovery.actor.hp > old_hp) && recovery.node == old_node)) return false;
            s = recovery; continue;
        }
        s = chosen;
    }
}
inline bool buy(Search& q, State& s, std::uint32_t item, std::uint32_t count) {
    if (item >= max_items || count == 0 || !q.in.shop.known) return false;
    const auto& m = q.in.items[item];
    const bool crystal = item == q.in.crystal_item;
    if (crystal && !q.in.shop.consumables_legal) return false;
    if (!m.known || !m.purchasable || m.cost == 0) return false;
    if (crystal ? (count != 1 || s.actor.crystals != 0)
                : (m.combine_count == 0 || m.result >= max_items)) return false;
    const auto loose = s.actor.owned[item];
    if (!crystal && (loose >= m.combine_count || count > m.combine_count - loose)) return false;
    const auto cost = std::uint64_t(count) * m.cost;
    if (cost > std::numeric_limits<std::uint32_t>::max() || !fund_and_shop(q, s, static_cast<std::uint32_t>(cost))) return false;
    PurchaseIntent intent;
    intent.kind = crystal ? PurchaseKind::Crystal : PurchaseKind::Item;
    intent.known = true; intent.item = item; intent.count = count; intent.recipe_result = m.result;
    intent.unit_cost = m.cost; intent.total_cost = static_cast<std::uint32_t>(cost);
    intent.per_visit_spend_cap = intent.total_cost; intent.reserved_route_cash = s.actor.wallet - intent.total_cost;
    intent.old_loose = loose; intent.combine_count = m.combine_count; intent.shop_token = q.in.shop.token;
    intent.milestone_complete = crystal || loose + count == m.combine_count;
    std::int64_t h = std::int64_t(count) * m.health_points;
    std::int64_t st = std::int64_t(count) * m.strength_points;
    std::int64_t f = std::int64_t(count) * m.focus_points;
    if (crystal) { h = st = f = 0; intent.requires_observation = !m.future_effect_known; }
    else if (intent.milestone_complete) {
        const auto& result = q.in.items[m.result];
        if (!result.known) { q.unsupported = true; return false; }
        h = std::int64_t(result.health_points) - std::int64_t(loose) * m.health_points;
        st = std::int64_t(result.strength_points) - std::int64_t(loose) * m.strength_points;
        f = std::int64_t(result.focus_points) - std::int64_t(loose) * m.focus_points;
        intent.requires_observation = !result.future_effect_known || result.passive == Passive::Unsupported;
    } else intent.requires_observation = !m.future_effect_known || m.passive == Passive::Unsupported;
    const double dh = static_cast<double>(h) * s.actor.health_growth;
    const double ds = static_cast<double>(st) * s.actor.strength_growth + dh * s.actor.ryth_health_strength_ratio;
    const double df = static_cast<double>(f) * s.actor.focus_growth;
    if (!finite_nonnegative(dh) || !finite_nonnegative(ds) || !finite_nonnegative(df)) { q.unsupported = true; return false; }
    intent.requires_observation = intent.requires_observation || !s.actor.projected_stats_known;
    if (intent.requires_observation) {
        // A spending observation belongs to a named, currently relevant gem
        // capability deficit. Catalog order or an empty primary list cannot
        // justify buying arbitrary unsupported equipment.
        bool associated = false;
        for (std::size_t g = 0; g < q.in.goal_count && !q.exhausted; ++g) {
            const auto& goal = q.in.goals[g];
            if (goal.nominal_gems == 0) continue;
            State probe = s;
            if (!route(q, probe, static_cast<std::uint32_t>(2 + g), StageKind::Approach, g)) continue;
            if (probe.elapsed == 0 && !advance(q, probe, 1)) continue; // Capability, not spent current allowances.
            if (fight(q, probe, g) || q.combat_deficit == 0) continue;
            std::uint32_t reason = 0;
            bool basic_gain = false, focus_gain = false;
            for (std::size_t member = 0; member < goal.member_count; ++member)
                basic_gain = basic_gain || (goal.members[member].basic_reachable && goal.members[member].basic_factor > 0);
            for (std::size_t a = 0; a < max_abilities; ++a) {
                const auto& ability = q.in.abilities[a];
                bool affected = false;
                for (std::size_t member = 0; member < goal.member_count; ++member) affected = affected || goal.members[member].cast_hit[a];
                const bool self_effect = ability.self_target_allowed && (ability.kind == AbilityKind::Heal || ability.kind == AbilityKind::Shield);
                const bool target_effect = affected && (ability.kind == AbilityKind::Damage || ability.kind == AbilityKind::AreaDamage || ability.kind == AbilityKind::Control);
                focus_gain = focus_gain || (ability.metadata_known && ability.focus_modifier > 0 && (self_effect || target_effect));
            }
            if (dh > 0 && (q.combat_deficit & 1u)) reason |= 1u;
            if (ds > 0 && basic_gain) reason |= 2u;
            if (df > 0 && focus_gain) reason |= 4u;
            if (reason == 0) continue;
            intent.primary_group_id = goal.group_id; intent.primary_epoch = goal.epoch;
            intent.capability_reason_bits = reason; associated = true; break;
        }
        if (!associated) return false;
    }
    const bool owns_first_purchase = s.first.kind == StageKind::None;
    first_stage(q.in, s, StageKind::Purchase, max_goals, q.in.shop.token, q.in.shop.current_purchase_witness && s.elapsed == 0);
    if (owns_first_purchase && s.first.kind == StageKind::Purchase) s.first.purchase = intent;
    if (!advance(q, s, q.in.shop.purchase_turns)) return false;
    s.actor.wallet -= intent.total_cost;
    s.actor.max_hp += dh; s.actor.strength += ds; s.actor.focus += df;
    if (!finite_nonnegative(s.actor.max_hp) || !finite_nonnegative(s.actor.strength) || !finite_nonnegative(s.actor.focus)) return false;
    if (q.in.shop.heals_health_delta) s.actor.hp = std::min(s.actor.max_hp, s.actor.hp + dh);
    if (q.in.shop.full_recovery_on_owner_end && q.in.shop.purchase_turns > 0) s.actor.hp = s.actor.max_hp;
    if (crystal) ++s.actor.crystals;
    else if (intent.milestone_complete) {
        if (s.actor.owned[m.result] == std::numeric_limits<std::uint32_t>::max()) return false;
        s.actor.owned[item] = 0; ++s.actor.owned[m.result];
    }
    else s.actor.owned[item] += count;
    if (!refresh_passives(q, s)) return false;
    s.milestone = intent; s.observed_purchase = intent.requires_observation;
    s.forecast_evaluated = false;
    return recover(q, s);
}
inline bool dominates(const State& a, const State& b) {
    if (a.node != b.node || a.observed_purchase != b.observed_purchase || a.shop_visits > b.shop_visits
        || a.fund_receipts > b.fund_receipts || a.elapsed > b.elapsed || a.submitted < b.submitted || a.earned < b.earned
        || a.actor.wallet < b.actor.wallet || a.actor.hp < b.actor.hp || a.actor.max_hp < b.actor.max_hp
        || a.actor.strength < b.actor.strength || a.actor.focus < b.actor.focus || a.actor.crystals < b.actor.crystals
        || a.actor.shield < b.actor.shield || a.actor.shield_rounds < b.actor.shield_rounds
        || (b.first.witness_known && !a.first.witness_known)) return false;
    // Different family inventories and cooldown/respawn futures are not merged.
    if (a.actor.owned != b.actor.owned || a.available != b.available || a.cooldown != b.cooldown) return false;
    return true;
}
inline double capability(const State& s) {
    // Used only to bound a heuristic frontier, never to compare gold and gems.
    return s.actor.max_hp + s.actor.strength * 8 + s.actor.focus * 4;
}
inline bool insert(std::array<State, max_states>& states, std::size_t& n, const State& candidate) {
    for (std::size_t i = 0; i < n; ++i) if (dominates(states[i], candidate)) return false;
    for (std::size_t i = 0; i < n;) {
        if (dominates(candidate, states[i])) states[i] = states[--n]; else ++i;
    }
    if (n < max_states) { states[n++] = candidate; return false; }
    // Width truncation is separately reported as SearchBudgetExceeded.
    std::size_t worst = 0;
    for (std::size_t i = 1; i < n; ++i) if (capability(states[i]) < capability(states[worst])) worst = i;
    if (capability(candidate) > capability(states[worst])) states[worst] = candidate;
    return true;
}
inline Forecast forecast(const State& s) {
    Forecast f;
    f.objective = s.win ? Objective::Win : s.submitted ? Objective::Gems : s.earned ? Objective::Gold : Objective::Information;
    f.timely_win = s.win; f.win_turn = s.win_turn; f.elapsed = s.elapsed;
    f.submitted_gems = s.submitted; f.lifetime_gold_earned = s.earned;
    f.wallet = s.actor.wallet; f.carried_gems = s.actor.carried_gems; f.remaining_hp = s.actor.hp;
    return f;
}
inline void evaluate_cycles(Search& q, State& s) {
    if (s.forecast_evaluated || s.observed_purchase) return;
    for (std::size_t g = 0; g < q.in.goal_count && !q.exhausted; ++g) {
        State cycle = s;
        for (std::uint32_t repeat = 0; repeat < 8 && !q.exhausted; ++repeat) {
            State completed = cycle;
            if (!receive(q, completed, g, q.in.goals[g].nominal_gems == 0) || !deliver(q, completed, g)) {
                if (q.exhausted) break;
                completed = cycle;
                const auto old_hp = completed.actor.hp;
                if (!route(q, completed, 1, StageKind::ReturnShop, g) || !recover(q, completed)
                    || !(completed.actor.hp > old_hp)
                    || !receive(q, completed, g, q.in.goals[g].nominal_gems == 0) || !deliver(q, completed, g)) break;
            }
            cycle = completed;
            consider(q, cycle);
            if (cycle.win) break;
            if (repeat == 7 && q.in.goals[g].repeat_known && cycle.elapsed < q.in.horizon_turns) q.space_capped = true;
        }
    }
    s.forecast_evaluated = true;
}
} // namespace program_detail

inline Result select(const Input& in, BudgetCheck check, void* opaque) {
    using namespace program_detail;
    Result out; out.primary_review_due = in.primary_review_due;
    if (in.goal_count > max_goals || in.horizon_turns == 0 || in.horizon_turns > 2000
        || in.operation_limit == 0 || in.operation_limit > 100000 || in.combat_turn_limit == 0 || in.combat_turn_limit > 512
        || !finite_nonnegative(in.actor.hp) || !(in.actor.hp > 0)
        || !finite_nonnegative(in.actor.max_hp) || in.actor.hp > in.actor.max_hp
        || !finite_nonnegative(in.actor.strength) || !finite_nonnegative(in.actor.focus)
        || !finite_nonnegative(in.actor.health_growth) || !finite_nonnegative(in.actor.strength_growth)
        || !finite_nonnegative(in.actor.focus_growth) || !finite_nonnegative(in.actor.ryth_health_strength_ratio)) {
        out.status = Status::InvalidInput; return out;
    }
    if (!finite_nonnegative(in.actor.shield) || in.actor.crystals > 1) { out.status = Status::InvalidInput; return out; }
    // Actual cargo belongs to the parent's freshly qualified banking fallback.
    if (in.actor.carried_gems != 0) return out;
    Search q{in, check, opaque};
    State base; base.actor = in.actor;
    if (!refresh_passives(q, base)) { out.status = Status::SearchBudgetExceeded; return out; }
    for (std::size_t a = 0; a < max_abilities; ++a) base.cooldown[a] = in.abilities[a].cooldown;
    for (std::size_t g = 0; g < in.goal_count; ++g) base.available[g] = in.goals[g].available_in;
    std::array<State, max_states> frontier{}; frontier[0] = base;
    std::size_t count = 1;
    bool width_capped = false;
    State observation{}; bool have_observation = false;
    for (std::uint32_t visit = 0; visit <= 4 && count && !q.exhausted; ++visit) {
        std::array<State, max_states> next{}; std::size_t next_count = 0;
        for (std::size_t si = 0; si < count && !q.exhausted; ++si) {
            State s = frontier[si]; ++q.expanded;
            if (s.observed_purchase) {
                if (stage_qualified(s) && (!have_observation || s.elapsed < observation.elapsed)) { observation = s; have_observation = true; }
                q.unsupported = true; continue; // No projected feasibility beyond this capability change.
            }
            // Evaluate after each meaningful purchase, before enumerating all
            // later catalog items, so a fuel cap cannot postpone every unlock
            // until a second breadth layer that is never reached.
            evaluate_cycles(q, s);
            if (visit == 4) {
                // Purchase depth is a separate heuristic limit from the four
                // physical shop visits. Several batches can share one visit.
                for (std::size_t item = 0; item < max_items; ++item) {
                    const auto& m = in.items[item];
                    if (m.known && m.purchasable && m.cost > 0 && ((item == in.crystal_item && s.actor.crystals == 0)
                        || (m.combine_count > s.actor.owned[item] && m.result < max_items))) { q.space_capped = true; break; }
                }
                continue;
            }
            for (std::size_t item = 0; item < max_items && !q.exhausted; ++item) {
                if (!q.tick()) break;
                const auto& m = in.items[item];
                const bool crystal = item == in.crystal_item;
                if (!m.known || !m.purchasable || m.cost == 0 || (crystal ? s.actor.crystals != 0 :
                    (m.combine_count == 0 || m.result >= max_items || s.actor.owned[item] >= m.combine_count))) continue;
                const auto remaining = crystal ? 1 : m.combine_count - s.actor.owned[item];
                const std::array<std::uint32_t, 2> batches{{remaining, 1}};
                for (std::size_t b = 0; b < batches.size() && !q.exhausted; ++b) {
                    if (b && remaining == 1) continue;
                    State bought = s;
                    if (!buy(q, bought, static_cast<std::uint32_t>(item), batches[b])) continue;
                    if (bought.observed_purchase) {
                        if (stage_qualified(bought) && (!have_observation || bought.elapsed < observation.elapsed)) {
                            observation = bought; have_observation = true;
                        }
                        q.unsupported = true;
                    } else evaluate_cycles(q, bought);
                    width_capped = insert(next, next_count, bought) || width_capped;
                }
            }
        }
        frontier = next; count = next_count;
    }
    out.operations = q.operations; out.expanded_states = q.expanded;
    out.evaluated_goals_mask = q.evaluated_goals_mask;
    out.unsupported_primary = q.unsupported || q.evidence;
    out.search_space_capped = q.exhausted || q.space_capped || width_capped;
    // The adapter's actual progress clock owns when review becomes due. A
    // repeat search/unsupported forecast must not re-arm it immediately after
    // a successful real observation, or Inspect itself can starve all work.
    out.primary_review_due = in.primary_review_due;
    const bool choose_observation = have_observation && out.primary_review_due
        && (!q.have_best || (!q.best.win && q.best.submitted == 0));
    if (choose_observation) {
        out.current_stage = observation.first; out.forecast = forecast(observation);
        out.intended_milestone = observation.milestone;
        out.fund_receipts = observation.fund_receipts; out.shop_visits = observation.shop_visits;
        out.status = Status::NeedsObserve;
    } else if (q.have_best) {
        out.current_stage = q.best.first; out.forecast = forecast(q.best); out.intended_milestone = q.best.milestone;
        out.fund_receipts = q.best.fund_receipts; out.shop_visits = q.best.shop_visits;
        out.status = out.search_space_capped ? Status::SearchBudgetExceeded : Status::Program;
    } else if (have_observation) {
        out.current_stage = observation.first; out.forecast = forecast(observation);
        out.intended_milestone = observation.milestone;
        out.fund_receipts = observation.fund_receipts; out.shop_visits = observation.shop_visits;
        out.status = Status::NeedsObserve;
    } else out.status = out.search_space_capped ? Status::SearchBudgetExceeded
                        : q.evidence ? Status::NeedsMechanicEvidence
                        : q.unsupported ? Status::NeedsObserve : Status::NoQualifiedCurrentStage;
    // A due safe observation is selected before another speculative gold loop.
    // Adapter memory owns the finite cadence; choosing this stage never clears it.
    if (!choose_observation && out.primary_review_due && out.forecast.objective <= Objective::Gold) {
        for (std::size_t g = 0; g < in.goal_count; ++g) if (in.goals[g].nominal_gems && in.goals[g].current_inspect_witness) {
            State inspect = base;
            first_stage(in, inspect, StageKind::Inspect, g, in.goals[g].contact_token, true);
            out.current_stage = inspect.first; out.forecast = forecast(inspect);
            out.intended_milestone = {}; out.fund_receipts = 0; out.shop_visits = 0;
            out.status = Status::NeedsObserve; break;
        }
    }
    return out;
}
} // namespace qualifier::resource
