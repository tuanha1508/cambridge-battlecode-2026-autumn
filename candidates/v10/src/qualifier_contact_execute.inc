// Collector types precede this include. No old fight selector may preempt a
// contact plan whose predicted work was credited by the economic selector.
std::vector<std::unique_ptr<QualifierContactRecord>> qualifier_contact_records;
std::vector<int> qualifier_contact_modelled_groups, qualifier_contact_required_groups;

bool qualifier_contact_requires_model(int group) {
    return std::find(qualifier_contact_required_groups.begin(), qualifier_contact_required_groups.end(), group)
        != qualifier_contact_required_groups.end();
}

QualifierContactRecord* qualifier_contact_record(int group) {
    for (auto& record : qualifier_contact_records)
        if (record->group_index == group) return record.get();
    return nullptr;
}

bool qualifier_contact_expansion_ok(const QualifierContactRecord& record) {
    const auto& plan = record.plan;
    return record.input && record.neutral_multiplier && record.capture_coverage_known
        && plan.selected_option >= 0 && plan.selected_option < static_cast<int>(record.input->option_count)
        && record.option_capture_coverage[plan.selected_option]
        && plan.classification == qualifier::contact::Class::ModeledSolo
        && plan.prefix_executable && plan.tail_complete
        && qualifier::contact::matches(*record.input, plan)
        && qualifier_runtime.enemies.empty();
}

// Retain the original two-group economic/current shortlist. An actual fixed
// next-buy gap may nominate at most two extra pure-gold groups for physical
// qualification. Neither list admits economic utility: original feasibility
// changes only after a current executable ModelledSolo contact is returned.
void qualifier_contact_refine_tasks(std::vector<qualifier::Task>& tasks, qualifier::Context context,
    const std::optional<QualifierCooperationNomination>& nomination) {
    qualifier_contact_records.clear();
    qualifier_contact_modelled_groups.clear(); qualifier_contact_required_groups.clear();
    if (!qualifier_current() || fuel_remaining() < 45000) return;
    auto candidates = tasks;
    for (auto& task : candidates) {
        const auto& group = S->groups[task.id];
        task.feasible = group.last_observed_round == S->round && group.observed_state == 0
            && !group.camps.empty() && group.camps.size() <= qualifier::contact::max_members;
    }
    context.bank_turns = -1; context.current_task = -1;
    std::array<int, 2> selected{-1, -1};
    const int current = S->qualifier_committed_target >= 0
        ? S->qualifier_committed_target : qualifier_inspection_target();
    for (const auto& candidate : candidates)
        if (candidate.id == current && candidate.feasible) selected[0] = current;
    const bool retained_current = selected[0] >= 0;
    for (std::size_t slot = 0; slot < selected.size(); ++slot) {
        // Retain one economic/current evaluation. A message may nominate the
        // second only when there is no current job to retain; the collector
        // still requires fresh own observations and a qualified solo prefix.
        if (slot == 1 && !retained_current && nomination) {
            for (const auto& candidate : candidates)
                if (candidate.feasible && candidate.id == nomination->group_index
                    && nomination->full_group == static_cast<std::uint32_t>(
                        static_cast<handle_camp_group_t>(S->groups[candidate.id].h)))
                    selected[slot] = candidate.id;
        }
        if (selected[slot] < 0) selected[slot] = qualifier::choose(context, candidates).task;
        if (selected[slot] < 0) break;
        for (auto& candidate : candidates) if (candidate.id == selected[slot]) candidate.feasible = false;
    }
    std::vector<int> collected(selected.begin(), selected.end());
    bool has_gem_nominee = false;
    for (const auto& task : tasks)
        if (task.reward_gems > 0
            && std::find(selected.begin(), selected.end(), task.id) != selected.end()) has_gem_nominee = true;
    // Optional expansion only. This threshold is unmeasured and does not
    // certify the fuel remaining inside a collector or after its host calls.
    if (has_gem_nominee && fuel_remaining() >= 80000 && qualifier_upgrade_funding_needed()) {
        struct FundingNominee { int group; double rank; std::uint32_t full; };
        std::vector<FundingNominee> funding;
        int retained_gold = 0;
        for (const auto& task : candidates) {
            const auto& job = S->groups[task.id];
            const double duration = task.travel_turns + task.work_turns + task.capture_turns;
            if (job.gems != 0 || task.reward_gems != 0 || !std::isfinite(task.reward_gold)
                || task.reward_gold <= 0 || !std::isfinite(duration) || duration < 0
                || job.last_observed_round != S->round || job.observed_state != 0
                || job.camps.empty() || job.camps.size() > qualifier::contact::max_members) continue;
            if (std::find(selected.begin(), selected.end(), task.id) != selected.end()) {
                ++retained_gold; continue;
            }
            funding.push_back({task.id, task.reward_gold / std::max(1.0, duration),
                static_cast<std::uint32_t>(static_cast<handle_camp_group_t>(job.h))});
        }
        std::sort(funding.begin(), funding.end(), [](const auto& a, const auto& b) {
            return a.rank != b.rank ? a.rank > b.rank : a.full < b.full;
        });
        for (const auto& nominee : funding) {
            if (retained_gold >= 2) break;
            collected.push_back(nominee.group); ++retained_gold;
        }
    }
    for (const int group : collected) {
        if (group < 0 || !qualifier_current() || fuel_remaining() < 45000) break;
        auto record = qualifier_collect_contact(group, qualifier_runtime.observation_revision);
        if (!record || !record->input || !qualifier_current()) continue;
        record->plan = qualifier::contact::choose(*record->input);
        if (qualifier_current() && fuel_remaining() >= 18000 && qualifier_contact_expansion_ok(*record)) {
            for (auto& task : tasks) if (task.id == group) {
                qualifier_contact_modelled_groups.push_back(group);
                if (!task.feasible) qualifier_contact_required_groups.push_back(group);
                // Phase0 is this input's prefix. It occupies the current turn;
                // later clear phases are additional stationary basic work.
                task.work_turns = 1.0 + record->plan.clear_phases;
                task.feasible = true;
            }
        }
        qualifier_contact_records.push_back(std::move(record));
    }
}

bool qualifier_contact_refresh_world() { return qualifier_refresh_world(); }

enum class QualifierContactExecution { NoPlan, Acted, Invalidated };

bool qualifier_contact_group_cleared(int group) {
    if (!qualifier_current()) return false;
    try {
#if QUALIFIER_LEGACY_BACKEND
        const auto state = S->groups[group].h.state();
#else
        if (!S->groups[group].qualifier_public_group) return false;
        const auto state = S->groups[group].qualifier_public_group->state();
#endif
        return std::holds_alternative<map::camp_group_reward_available>(state);
    } catch (const cambc::error&) { return false; }
}

QualifierContactExecution qualifier_execute_contact(QualifierContactRecord& record, bool model_required, int replans = 1) {
    auto& me = self();
    if (!record.input) return QualifierContactExecution::NoPlan;
    const auto& input = *record.input;
    const auto& plan = record.plan;
    const auto actor = static_cast<std::uint32_t>(static_cast<handle_champion_t>(me));
    if (!qualifier_current() || !plan.prefix_executable
        || !qualifier::contact::matches(input, plan)) return QualifierContactExecution::NoPlan;
    if (input.stamp.actor != actor || input.stamp.round != static_cast<std::uint32_t>(S->round)
        || input.stamp.revision != qualifier_runtime.observation_revision || me.position() != record.origin
        || record.group_index < 0 || record.group_index >= static_cast<int>(S->groups.size())
        || input.stamp.group != static_cast<std::uint32_t>(static_cast<handle_camp_group_t>(S->groups[record.group_index].h)))
        return QualifierContactExecution::Invalidated;
    bool acted = false;
    auto invalidate = [&]() {
        ++qualifier_runtime.observation_revision;
        // A false cast/move can reveal a hidden blocker. It is not permission
        // to fall through to the old independent fight selector.
        if (qualifier_current() && fuel_remaining() >= 18000) qualifier_contact_refresh_world();
        if (acted && qualifier_contact_group_cleared(record.group_index)) return QualifierContactExecution::Acted;
        if (replans > 0 && qualifier_current() && fuel_remaining() >= 45000) {
            auto fresh = qualifier_collect_contact(record.group_index, qualifier_runtime.observation_revision);
            if (fresh && fresh->input) {
                fresh->plan = qualifier::contact::choose(*fresh->input);
                if (fresh->plan.prefix_executable && (!model_required || qualifier_contact_expansion_ok(*fresh)))
                    {
                        if (qualifier_contact_expansion_ok(*fresh)) {
                            qualifier_runtime.contact_capture_group = fresh->group_index;
                            qualifier_runtime.contact_capture_region = fresh->capture_region;
                        }
                        const auto next = qualifier_execute_contact(*fresh, model_required, replans - 1);
                        return acted && next == QualifierContactExecution::NoPlan
                            ? QualifierContactExecution::Invalidated : next;
                    }
            }
        }
        return QualifierContactExecution::Invalidated;
    };
    auto basic = [&](int index) {
        if (index < 0) return true;
        if (!qualifier_current() || index >= static_cast<int>(input.member_count) || !me.can_attack()) return false;
        auto* camp = record.members[index];
        if (!camp || static_cast<std::uint32_t>(static_cast<handle_camp_t>(camp->h)) != plan.member_ids[index]) return false;
        camp->qualifier_refresh_observation(S->round);
        if (!camp->health_observation_valid || camp->last_observed_round != S->round || camp->hp <= 0
            || !camp->h.is_visible() || cheb(me.position(), camp->pos) > me.attack_range() || !qualifier_current()) return false;
        try { me.attack(camp->h); qualifier_note_group_work(S->groups[record.group_index], 1); ++S->attacks; acted = true; }
        catch (const cambc::error&) { return false; }
        ++qualifier_runtime.observation_revision;
        return qualifier_current();
    };
    if (!basic(plan.pre_basic)) return invalidate();
    if (acted && qualifier_contact_group_cleared(record.group_index)) {
        if (qualifier_current() && fuel_remaining() >= 18000) qualifier_contact_refresh_world();
        return QualifierContactExecution::Acted;
    }
    if (plan.cast_option >= 0) {
        if (!qualifier_current() || plan.cast_option >= static_cast<int>(input.option_count)
            || S->cast || !me.can_cast()) return invalidate();
        const auto& cast = record.casts[plan.cast_option];
        if (cast.slot != plan.cast_slot || cast.slot < 0 || cast.slot > 1) return invalidate();
        const auto ability = me.abilities()[cast.slot];
        if (!ability.is_ready()) return invalidate();
        const bool accepted_control = ability.is_cc() && ability.get<ability_cc>().duration > 0;
        const auto cell_token = [](vec2 p) {
            return (std::uint64_t(static_cast<std::uint32_t>(p.x)) << 32) | static_cast<std::uint32_t>(p.y);
        };
        std::uint64_t token = 0;
        if (cast.target == QualifierContactCast::Target::Self) token = actor;
        else if (cast.target == QualifierContactCast::Target::Camp && cast.member >= 0
            && cast.member < static_cast<int>(input.member_count)) token = input.members[cast.member].id;
        else if (cast.target == QualifierContactCast::Target::Ground) token = cell_token(cast.tile);
        if (token != plan.cast_token || cell_token(cast.contact_position) != plan.contact_token) return invalidate();
        try {
            bool success = false;
            if (cast.target == QualifierContactCast::Target::Self) {
                if (!ability.can_target_self()) return invalidate();
                success = ability.use();
            } else if (cast.target == QualifierContactCast::Target::Camp) {
                const int index = cast.member;
                if (index < 0 || index >= static_cast<int>(input.member_count) || index != plan.cast_target_member) return invalidate();
                auto* camp = record.members[index];
                if (!camp || !camp->h.is_visible() || camp->h.is_dead() || !ability.can_target_camps()
                    || static_cast<std::uint32_t>(static_cast<handle_camp_t>(camp->h)) != plan.member_ids[index]) return invalidate();
                success = ability.use(camp->h);
            } else if (cast.target == QualifierContactCast::Target::Ground) {
                if (!ability.can_target_ground()) return invalidate();
                success = ability.use(cast.tile);
            }
            if (!success) return invalidate();
            const auto& accepted_option = input.options[plan.cast_option];
            bool affects_camp = false;
            for (std::size_t i = 0; i < input.member_count; ++i)
                if (input.members[i].hp > 0 &&
                    ((std::isfinite(accepted_option.cast_damage[i]) && accepted_option.cast_damage[i] > 0)
                     || (accepted_control && accepted_option.kind == qualifier::contact::Kind::CrowdControl
                         && cast.target == QualifierContactCast::Target::Camp
                         && cast.member == static_cast<int>(i) && accepted_option.control_qualified
                         && accepted_option.initial_suppressed_phases[i] > 0)))
                    affects_camp = true;
            if (affects_camp && cast.target != QualifierContactCast::Target::Self)
                qualifier_note_group_work(S->groups[record.group_index], 2);
            S->cast = true; ++S->casts; acted = true;
            if (input.options[plan.cast_option].kind == qualifier::contact::Kind::Dash) S->moved = true;
        } catch (const cambc::error&) { return invalidate(); }
        ++qualifier_runtime.observation_revision;
        if (!qualifier_current() || me.position() != cast.contact_position) return invalidate();
        if (qualifier_contact_group_cleared(record.group_index)) {
            if (qualifier_current() && fuel_remaining() >= 18000) qualifier_contact_refresh_world();
            return QualifierContactExecution::Acted;
        }
    }
    // Post-prefix targets and allowances are host facts again. The planned
    // target is never silently replaced with a different member.
    if (!basic(plan.post_basic)) return invalidate();
    if (acted && qualifier_current() && fuel_remaining() >= 18000) qualifier_contact_refresh_world();
    return acted ? QualifierContactExecution::Acted : QualifierContactExecution::NoPlan;
}

bool qualifier_contact_capture_valid(int group) {
    auto& rt = qualifier_runtime;
    if (rt.contact_capture_group != group || !rt.contact_capture_region) return true;
    if (!qualifier_contact_refresh_capture_region(group, *rt.contact_capture_region)
        || !rt.contact_capture_region->is_set(self().position())) {
        rt.contact_capture_group = -1; rt.contact_capture_region.reset();
        return false;
    }
    bitboard roots = qualifier_receipt_goals(group) & *rt.contact_capture_region;
    distance_field route;
    bitboard visited;
    if (!roots.is_empty()) route.compute(~*rt.contact_capture_region, roots, visited);
    if (roots.is_empty() || !qualifier_current() || route.get(self().position()) == UNREACH) {
        rt.contact_capture_group = -1; rt.contact_capture_region.reset();
        return false;
    }
    return true;
}

bool qualifier_contact_capture_step(int group) {
    auto& rt = qualifier_runtime;
    if (rt.contact_capture_group != group || !rt.contact_capture_region) return false;
    if (!qualifier_contact_capture_valid(group)) { qualifier_recover(); return true; }
    struct Restore {
        bitboard original;
        ~Restore() { S->terrain = original; }
    } restore{S->terrain};
    S->terrain |= ~*rt.contact_capture_region;
    S->goals = qualifier_receipt_goals(group);
    qualifier_step();
    return true;
}
