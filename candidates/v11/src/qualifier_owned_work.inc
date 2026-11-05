// Accepted physical camp intent is history, never a reusable contact Plan or
// evidence of damage, receipt, helper work or survival. Each physical actor has
// its own bounded entry, including the two linked C&I bodies.
struct QualifierOwnWork {
    enum class Phase { None, Working, Cleared, OwnCapture };
    std::uint32_t actor = 0, group = 0;
    bool used = false;
    int entry_round = -1, accepted_round = -1, observed_round = -1;
    int observed_state = -1;
    unsigned accepted_kind = 0;
    Phase phase = Phase::None;
};
std::array<QualifierOwnWork, 16> qualifier_own_work_entries{};
QualifierOwnWork* qualifier_own_work = nullptr;

void qualifier_own_work_retire() {
    if (qualifier_own_work) {
        qualifier_own_work->phase = QualifierOwnWork::Phase::None;
        qualifier_own_work->accepted_round = qualifier_own_work->observed_round = -1;
        qualifier_own_work->observed_state = -1;
        qualifier_own_work->accepted_kind = 0;
    }
}

void qualifier_own_work_begin() {
    const auto actor = static_cast<std::uint32_t>(static_cast<handle_champion_t>(self()));
    qualifier_own_work = nullptr;
    for (auto& entry : qualifier_own_work_entries)
        if (entry.used && entry.actor == actor) { qualifier_own_work = &entry; break; }
    if (!qualifier_own_work) {
        auto* entry = &qualifier_own_work_entries.front();
        for (auto& candidate : qualifier_own_work_entries) {
            if (!candidate.used) { entry = &candidate; break; }
            if (candidate.entry_round < entry->entry_round) entry = &candidate;
        }
        *entry = {};
        entry->used = true; entry->actor = actor;
        qualifier_own_work = entry;
    }
    // Stamp this entry before any fallible observation. A duplicate retry or
    // skipped input cannot repair a failed first sample into a consecutive one.
    if (qualifier_own_work->entry_round != S->round - 1) qualifier_own_work_retire();
    qualifier_own_work->entry_round = S->round;
}

int qualifier_own_work_group() {
    if (!qualifier_current() || !qualifier_own_work
        || qualifier_own_work->phase == QualifierOwnWork::Phase::None
        || qualifier_own_work->entry_round != S->round
        || qualifier_own_work->actor != static_cast<std::uint32_t>(static_cast<handle_champion_t>(self()))
        || S->groups.size() > 512) return -1;
    int found = -1;
    for (int i = 0; i < static_cast<int>(S->groups.size()); ++i)
        if (static_cast<std::uint32_t>(static_cast<handle_camp_group_t>(S->groups[i].h))
            == qualifier_own_work->group) {
            if (found >= 0) return -1;
            found = i;
        }
    return found;
}

void qualifier_note_group_work(const Group& group, int kind) {
    // Called immediately after the host action returns success, before later
    // observation/current-position checks can fail. S->round is the historical
    // attempted action stamp, not a promise that the input remains current.
    if (!S || !qualifier_own_work || kind <= 0
        || qualifier_own_work->entry_round != S->round
        || qualifier_own_work->actor != static_cast<std::uint32_t>(static_cast<handle_champion_t>(self()))) return;
    const auto full = static_cast<std::uint32_t>(static_cast<handle_camp_group_t>(group.h));
    if (qualifier_own_work->phase == QualifierOwnWork::Phase::None || qualifier_own_work->group != full) {
        qualifier_own_work_retire();
        qualifier_own_work->group = full;
    }
    qualifier_own_work->phase = QualifierOwnWork::Phase::Working;
    qualifier_own_work->accepted_round = S->round;
    qualifier_own_work->accepted_kind |= static_cast<unsigned>(kind);
    // The pre-action current lifecycle is the only observation being saved.
    qualifier_own_work->observed_round = group.last_observed_round;
    qualifier_own_work->observed_state = group.observed_state;
}

void qualifier_note_camp_work(const Camp& camp, int kind) {
    if (!S || S->target < 0 || S->target >= static_cast<int>(S->groups.size())) return;
    const auto& group = S->groups[S->target];
    const auto full = static_cast<std::uint32_t>(static_cast<handle_camp_t>(camp.h));
    // A target assignment alone is insufficient. Bind immutable membership of
    // the actual attacked camp before assigning the group's work history.
    for (const auto& member : group.camps)
        if (static_cast<std::uint32_t>(static_cast<handle_camp_t>(member.h)) == full) {
            qualifier_note_group_work(group, kind); return;
        }
}

void qualifier_own_work_observe() {
    const int index = qualifier_own_work_group();
    if (index < 0) { qualifier_own_work_retire(); return; }
    auto& entry = *qualifier_own_work;
    const auto& group = S->groups[index];
    if (!self().is_alive() || group.last_observed_round != S->round
        || group.observed_state < 0 || group.observed_state > 1
        || entry.observed_round != S->round - 1
        || (entry.observed_state == 1 && group.observed_state == 0)) {
        qualifier_own_work_retire(); return;
    }
    if (group.observed_state == 0) {
        // No lifecycle epoch API exists. Missing current member facts or an
        // unobserved interval is not certified as the same respawn cycle.
        if (entry.phase != QualifierOwnWork::Phase::Working || group.camps.size() > 16) {
            qualifier_own_work_retire(); return;
        }
        for (const auto& member : group.camps)
            if (!member.health_observation_valid || member.last_observed_round != S->round) {
                qualifier_own_work_retire(); return;
            }
    } else {
        entry.phase = qualifier_tiles(group).is_set(self().position())
            ? QualifierOwnWork::Phase::OwnCapture : QualifierOwnWork::Phase::Cleared;
    }
    entry.observed_round = S->round;
    entry.observed_state = group.observed_state;
}
