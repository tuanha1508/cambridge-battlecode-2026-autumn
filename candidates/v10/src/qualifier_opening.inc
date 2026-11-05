// Draft design preferences are advisory only. Live feasibility and resources
// remain owned by the ordinary planner, contact model and delivery adapter.
struct QualifierOpeningRuntime {
    bool read = false, valid = false, retired = false, active = false;
    int round = -1;
    std::uint64_t revision = 0;
    std::vector<int> configured_owner, current_owner;
    std::vector<std::uint32_t> group_ids;
    std::vector<bool> group_retired;
};
QualifierOpeningRuntime qualifier_opening_runtime;

bool qualifier_opening_full_health(const champion& unit) {
    const double hp = unit.health(), maximum = unit.max_health();
    return std::isfinite(hp) && std::isfinite(maximum) && maximum > 0 && hp >= maximum;
}

void qualifier_opening_retire() noexcept {
    qualifier_opening_runtime.retired = true;
    qualifier_opening_runtime.active = false;
}

void qualifier_opening_load() {
    auto& opening = qualifier_opening_runtime;
    const auto& rt = qualifier_runtime;
    if (opening.read || opening.retired || !S || !qualifier_current()
        || rt.own_party < 0 || rt.parties.size() < 3 || rt.parties.size() > 5
        || S->groups.size() > 512 || fuel_remaining() < 20000) return;
    opening.read = true;
    try {
        if (cambc::draft_memory.size() > 1024) return;
        std::vector<std::uint8_t> bytes;
        bytes.reserve(cambc::draft_memory.size());
        for (const auto value : cambc::draft_memory)
            bytes.push_back(std::to_integer<std::uint8_t>(value));
        const auto plan = qualifier_opening_plan::decode(std::span<const std::uint8_t>{bytes});
        if (!plan || plan->slots != rt.parties.size() || plan->accepted != plan->slots) return;
        std::vector<bool> party_used(rt.parties.size(), false);
        std::vector<int> owners(S->groups.size(), -1);
        std::vector<std::uint32_t> ids;
        ids.reserve(S->groups.size());
        for (const auto& group : S->groups)
            ids.push_back(static_cast<std::uint32_t>(static_cast<handle_camp_group_t>(group.h)));
        for (std::size_t slot = 0; slot < plan->slots; ++slot) {
            const auto& entry = plan->entries[slot];
            int owner = -1;
            const bool pair = entry.pick == cambc::champion_draft::cassi_and_issac;
            for (std::size_t p = 0; p < rt.parties.size(); ++p) {
                const auto& party = rt.parties[p];
                if (party.members.size() != (pair ? 2u : 1u)) continue;
                bool compatible = true, selected_body_present = false;
                for (const auto& member : party.members) {
                    const auto type = member.champion_name();
                    compatible = compatible && qualifier_opening_plan::body_allowed(entry.pick, type);
                    selected_body_present = selected_body_present || type == entry.body;
                }
                if (compatible && selected_body_present) {
                    if (owner >= 0) return;
                    owner = static_cast<int>(p);
                }
            }
            if (owner < 0 || party_used[owner]) return;
            party_used[owner] = true;
            if (!entry.assigned) continue;
            int group_index = -1;
            for (std::size_t gi = 0; gi < ids.size(); ++gi) if (ids[gi] == entry.group) {
                if (group_index >= 0) return;
                group_index = static_cast<int>(gi);
            }
            if (group_index < 0 || owners[group_index] >= 0
                || (S->groups[group_index].gems == 0 && S->groups[group_index].gold == 0)) return;
            owners[group_index] = owner;
        }
        if (!qualifier_current()) return;
        opening.configured_owner = std::move(owners);
        opening.current_owner.assign(S->groups.size(), -1);
        opening.group_ids = std::move(ids);
        opening.group_retired.assign(S->groups.size(), false);
        opening.valid = true;
    } catch (const std::exception&) {
        opening.valid = false;
        opening.configured_owner.clear(); opening.current_owner.clear();
    }
}

void qualifier_opening_refresh() {
    auto& opening = qualifier_opening_runtime;
    opening.active = false;
    if (opening.retired || !S || !qualifier_current()) return;
    try {
        const auto& me = self();
        const auto spawn = map::spawn_region(team::ally);
        if (!me.is_alive() || me.gem_count() > 0 || !qualifier_opening_full_health(me)
            || !spawn.is_set(me.position()) || me.is_recalling() || me.is_submitting_gems()
            || me.is_teleporting() || S->target >= 0 || S->qualifier_committed_target >= 0
            || S->mode == Mode::recall) { qualifier_opening_retire(); return; }
        // Accepted actions and visible interference receive the live policy's
        // ordinary valuation without a cached opening preference.
        if (S->moved || S->cast || !qualifier_runtime.enemies.empty()) return;
        qualifier_opening_load();
        if (!opening.valid || opening.configured_owner.size() != S->groups.size()
            || opening.group_ids.size() != S->groups.size()) return;
        std::vector<bool> eligible(qualifier_runtime.parties.size(), true);
        for (std::size_t p = 0; p < eligible.size(); ++p)
            for (const auto& member : qualifier_runtime.parties[p].members) {
                if (!member.is_visible() || !member.is_alive() || member.gem_count() > 0
                    || !qualifier_opening_full_health(member) || !spawn.is_set(member.position())
                    || member.is_recalling() || member.is_submitting_gems() || member.is_teleporting()) {
                    eligible[p] = false; break;
                }
            }
        opening.current_owner.assign(S->groups.size(), -1);
        for (std::size_t gi = 0; gi < S->groups.size(); ++gi) {
            const auto& group = S->groups[gi];
            if (opening.group_ids[gi] != static_cast<std::uint32_t>(static_cast<handle_camp_group_t>(group.h))) {
                qualifier_opening_retire(); return;
            }
            if (group.last_observed_round == S->round && group.observed_state != 0)
                opening.group_retired[gi] = true;
            for (const auto& camp : group.camps)
                if (camp.health_observation_valid && camp.last_observed_round == S->round
                    && camp.hp < camp.maxhp) opening.group_retired[gi] = true;
            const int owner = opening.configured_owner[gi];
            if (!opening.group_retired[gi] && owner >= 0
                && owner < static_cast<int>(eligible.size()) && eligible[owner])
                opening.current_owner[gi] = owner;
        }
        if (!qualifier_current()) return;
        opening.round = S->round; opening.revision = qualifier_runtime.observation_revision;
        opening.active = true;
    } catch (const std::exception&) {
        opening.active = false;
    }
}

std::optional<int> qualifier_opening_owner(int group) {
    const auto& opening = qualifier_opening_runtime;
    // Pure lookup in the planning loop. Refresh and the caller's final
    // actor/round gate own freshness; do not add one SDK clock query per job.
    if (!opening.active || opening.retired || !S
        || opening.round != S->round || opening.revision != qualifier_runtime.observation_revision
        || group < 0 || group >= static_cast<int>(opening.current_owner.size())
        || group >= static_cast<int>(S->groups.size())
        || group >= static_cast<int>(opening.group_ids.size())
        || opening.group_ids[group] != static_cast<std::uint32_t>(
            static_cast<handle_camp_group_t>(S->groups[group].h))
        || opening.current_owner[group] < 0) return {};
    return opening.current_owner[group];
}
