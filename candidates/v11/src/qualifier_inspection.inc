// Optional information gathering when the economic selector admits no job.
// No capture, damage, ally support or delivered-reward credit is granted here.
struct QualifierInspection {
    int group = -1, started = -1;
    std::vector<int> retry_after;
};
QualifierInspection qualifier_inspection;

void qualifier_inspection_cancel(bool defer) {
    auto& info = qualifier_inspection;
    if (defer && S && info.group >= 0 && info.group < static_cast<int>(info.retry_after.size()))
        info.retry_after[info.group] = S->round + 24;
    info.group = info.started = -1;
}

int qualifier_inspection_target() {
    const auto& info = qualifier_inspection;
    // These are uncalibrated loop limits, not engine rules or a work ETA.
    return qualifier_current() && info.started >= 0 && S->round >= info.started
        && S->round - info.started < 8 ? info.group : -1;
}

// False means no physical action was attempted. True owns the rest of this
// input, including a hidden blocker or exception from the one attempted step.
bool qualifier_inspect(const std::vector<qualifier::Task>& tasks, qualifier::Context context,
    int named_goal = -1) {
    if (!qualifier_current() || fuel_remaining() < 45000) return false;
    auto& me = self();
    auto& info = qualifier_inspection;
    if (!me.is_alive() || me.gem_count() != 0 || !me.can_move()
        || me.is_recalling() || me.is_submitting_gems() || me.is_teleporting()
        || qualifier_runtime.recent_damage > 0 || !qualifier_runtime.enemies.empty()
        || qualifier_runtime.unresolved_awake_npc || S->groups.size() > 512) return false;
    if (info.retry_after.size() != S->groups.size()) {
        info.retry_after.assign(S->groups.size(), -1);
        qualifier_inspection_cancel();
    }
    if (info.group >= 0 && qualifier_inspection_target() < 0) qualifier_inspection_cancel(true);
    const auto origin = me.position();
    bitboard excluded;
    std::size_t count = 0;
    // Exclude even sleepers: an observation trip has no reason to enter a
    // possible camp attack envelope. Unknown designs make this inconclusive.
    for (const auto& group : S->groups) {
        const bool cleared = group.last_observed_round == S->round && group.observed_state != 0;
        // Scouting must not join/dilute another payout or delay its respawn.
        if (cleared) excluded |= qualifier_tiles(group);
        for (const auto& camp : group.camps) {
            if (++count > 4096 || (count % 64 == 0 && fuel_remaining() < 35000)) return false;
            if (cleared || (camp.health_observation_valid && camp.last_observed_round == S->round && camp.hp <= 0)) continue;
            if (!camp.design_known || !std::isfinite(camp.str) || camp.str < 0 || camp.range < 0) return false;
            int range = camp.range;
            if (camp.combat_observed_round == S->round) {
                if (!std::isfinite(camp.observed_strength) || camp.observed_strength < 0
                    || camp.observed_attack_range < 0) return false;
                range = std::max(range, camp.observed_attack_range);
            }
            excluded.set(camp.pos);
            if (camp.str > 0 || (camp.combat_observed_round == S->round && camp.observed_strength > 0))
                excluded.draw_chebyshev(camp.pos, static_cast<float>(range));
        }
    }
    // This bound describes the public/current NPC model, not fog or spell
    // safety. Only adjacent steps are used, with no assumed custom host path.
    if (excluded.is_set(origin) || !qualifier_current()) return false;
    double reach = me.attack_range();
    for (const auto& ability : me.abilities()) {
        if (!ability.is_ready()) continue;
        if (ability.is_damage() && ability.can_target_camps())
            reach = std::max(reach, static_cast<double>(ability.get<ability_damage>().range));
        else if (ability.is_cc() && ability.can_target_camps())
            reach = std::max(reach, static_cast<double>(ability.get<ability_cc>().range));
        else if (ability.is_aoe_damage() && ability.can_target_ground())
            reach = std::max(reach, static_cast<double>(ability.get<ability_aoe_damage>().range));
    }
    if (!std::isfinite(reach) || reach <= 0 || reach > 8) return false;
    auto candidates = tasks;
    for (auto& task : candidates) {
        if (task.id < 0 || task.id >= static_cast<int>(S->groups.size())) { task.feasible = false; continue; }
        const auto& group = S->groups[task.id];
        // Named capability observation is an explicit information request, even
        // if an old economic estimate admitted this task. It grants no work.
        task.feasible = (named_goal >= 0 ? task.id == named_goal : !task.feasible)
            && task.id < static_cast<int>(info.retry_after.size())
            && S->round >= info.retry_after[task.id] && !group.camps.empty()
            && group.camps.size() <= qualifier::contact::max_members
            && !(group.last_observed_round == S->round && group.observed_state != 0)
            && !(group.observed_state == 2 && group.respawn_round > S->round);
        task.delivery_available = true; // Information shortlist only, never final economic admission.
    }
    context.bank_turns = -1;
    context.current_task = -1;
    std::array<int, 2> selected{-1, -1};
    for (const auto& task : candidates)
        if (task.id == qualifier_inspection_target() && task.feasible) selected[0] = task.id;
    for (auto& id : selected) {
        if (id < 0) id = qualifier::choose(context, candidates).task;
        for (auto& task : candidates) if (task.id == id) task.feasible = false;
    }
    refresh_occ();
    vision::teleport_tiles_into(S->tmp);
    auto blocked = S->occ | S->tmp | excluded;
    blocked.set(origin, false);
    for (const int group : selected) {
        if (group < 0 || !qualifier_current() || fuel_remaining() < 30000) break;
        bitboard goals;
        for (const auto& camp : S->groups[group].camps)
            goals.draw_chebyshev(camp.pos, static_cast<float>(reach));
        goals &= ~blocked;
        if (goals.is_empty()) continue;
        if (goals.is_set(origin)) {
            // We reached a vantage point and the fresh model still admitted
            // no work. Hidden/unsupported cases stay inconclusive, then defer.
            info.group = group;
            qualifier_inspection_cancel(true);
            continue;
        }
        distance_field field;
        bitboard visited, root;
        root.set(origin);
        field.compute_to(blocked, goals, root, visited);
        if (!qualifier_current()) return false;
        const auto current = field.get(origin);
        if (current == UNREACH) continue;
        const auto legal = blocked.reachable_tiles(origin, 1);
        vec2 destination{};
        int best = current;
        bool found = false;
        for (const auto point : legal.positions()) {
            const auto distance = field.get(point);
            if (point == origin || distance == UNREACH || distance >= current) continue;
            if (!found || distance < best || (distance == best
                && (point.y < destination.y || (point.y == destination.y && point.x < destination.x)))) {
                found = true; destination = point; best = distance;
            }
        }
        if (!found || !qualifier_current() || !me.can_move() || me.position() != origin) continue;
        if (info.group != group) { info.group = group; info.started = S->round; }
        // This is an information task, not committed Work or a helper claim.
        qualifier_own_work_retire();
        S->target = S->qualifier_committed_target = -1;
        S->mode = Mode::farm;
        try {
            if (me.move_absolute(destination)) { S->moved = true; ++S->moves; }
        } catch (const cambc::error&) {}
        ++qualifier_runtime.observation_revision;
        return true;
    }
    return false;
}
