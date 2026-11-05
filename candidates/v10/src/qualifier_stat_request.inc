// Informational first-Monster capability request, never an executable Plan.
enum class QualifierStatPurpose { IndependentWork, Support };
enum class QualifierStatEvidence { Unknown, PublicDesignPrior, CurrentGeometry };
enum class QualifierMonsterStat { Strength, Focus, Health };
enum class QualifierMonsterFamily { Unstarted, Partial, Complete, CompleteMultiple, MixedPartial };
enum class QualifierStatReason { Unknown, FixedFallback, WorkCapabilityMismatch, PartialLocked, FamilyComplete, MixedInventory };
struct QualifierMonsterVariant { item_type shard, emblem; };
QualifierMonsterVariant qualifier_monster_variant(QualifierMonsterStat stat) {
    using it = item_type;
    if (stat == QualifierMonsterStat::Focus) return {it::monster_shard_of_focus, it::monster_emblem_of_focus};
    if (stat == QualifierMonsterStat::Health) return {it::monster_shard_of_health, it::monster_emblem_of_health};
    return {it::monster_shard_of_strength, it::monster_emblem_of_strength};
}
QualifierMonsterStat qualifier_stat_fallback(champion_type type) {
    return type == champion_type::stellar || type == champion_type::prism || type == champion_type::cassi
        || type == champion_type::tavi || type == champion_type::willow
        ? QualifierMonsterStat::Focus : QualifierMonsterStat::Strength;
}
struct QualifierMonsterInventory {
    QualifierMonsterFamily family = QualifierMonsterFamily::Unstarted;
    QualifierMonsterStat stat = QualifierMonsterStat::Strength;
    std::array<std::uint32_t, 3> loose{}, emblems{};
};
QualifierMonsterInventory qualifier_monster_inventory(const enum_map<item_type>& owned) {
    QualifierMonsterInventory result;
    constexpr std::array<QualifierMonsterStat, 3> stats{{QualifierMonsterStat::Strength, QualifierMonsterStat::Focus, QualifierMonsterStat::Health}};
    int partials = 0, completed = 0;
    for (std::size_t i = 0; i < stats.size(); ++i) {
        const auto variant = qualifier_monster_variant(stats[i]);
        result.loose[i] = owned[variant.shard]; result.emblems[i] = owned[variant.emblem];
        if (result.loose[i] > 0) { ++partials; result.stat = stats[i]; }
    }
    for (std::size_t i = 0; i < stats.size(); ++i) if (result.emblems[i] > 0) {
        if (completed++ == 0) result.stat = stats[i];
    }
    // Completed family takes precedence over leftover loose shards: no new
    // Monster spend or new passive is credited, including mixed leftovers.
    if (completed) result.family = completed == 1 ? QualifierMonsterFamily::Complete : QualifierMonsterFamily::CompleteMultiple;
    else if (partials > 1) result.family = QualifierMonsterFamily::MixedPartial;
    else if (partials == 1) result.family = QualifierMonsterFamily::Partial;
    return result;
}
struct QualifierStatContribution {
    bool capable = false, currently_ready = false, geometry_known = false;
    bool timing_known = false;
    int slot = -1, observed_targets = 0;
    double nominal_per_effect = 0, current_amount = 0;
    std::optional<std::uint32_t> observed_ally;
};
struct QualifierStatRequest {
    bool valid = false, channels_known = false;
    std::uint32_t actor = 0, full_group = 0;
    champion_type type = champion_type::ryth;
    int round = -1, group_index = -1;
    std::uint64_t revision = 0;
    vec2 position{};
    QualifierStatPurpose purpose = QualifierStatPurpose::IndependentWork;
    QualifierStatEvidence evidence = QualifierStatEvidence::Unknown;
    QualifierStatReason reason = QualifierStatReason::Unknown;
    QualifierMonsterInventory inventory;
    QualifierMonsterStat preferred = QualifierMonsterStat::Strength;
    QualifierStatContribution basic, one_cast, shield, ally_support;
    bool reset_capable = false, focus_camp_possible = false;
    bool self_protection_possible = false, support_observation_unknown = false;
    bool future_support_unknown = false, focus_passive_possible = false;
    double strength_growth = 0, focus_growth = 0;
};
QualifierStatRequest qualifier_stat_request;
void qualifier_stat_request_clear() noexcept { qualifier_stat_request = {}; }

namespace qualifier_stat_detail {
bool nonnegative(double value) { return std::isfinite(value) && value >= 0; }
bool immediate_area(champion_type type, int slot) {
    return (type == champion_type::ryth && slot == 0) || (type == champion_type::prism && slot == 1)
        || (type == champion_type::stellar && slot == 1);
}
} // namespace qualifier_stat_detail

void qualifier_prepare_stat_request(int group, QualifierStatPurpose purpose = QualifierStatPurpose::IndependentWork) noexcept {
    namespace d = qualifier_stat_detail;
    qualifier_stat_request_clear();
    QualifierStatRequest request;
    request.purpose = purpose;
    try {
        if (!S || !qualifier_current()) return;
        auto& me = self();
        request.actor = static_cast<std::uint32_t>(static_cast<handle_champion_t>(me));
        request.type = me.champion_name(); request.round = S->round;
        request.revision = qualifier_runtime.observation_revision; request.position = me.position();
        request.preferred = qualifier_stat_fallback(request.type);
        request.inventory = qualifier_monster_inventory(me.item_count());
        request.reason = QualifierStatReason::FixedFallback;
        if (group >= 0) {
            if (group >= static_cast<int>(S->groups.size())) return;
            request.group_index = group;
            request.full_group = static_cast<std::uint32_t>(static_cast<handle_camp_group_t>(S->groups[group].h));
        }
        request.evidence = QualifierStatEvidence::PublicDesignPrior;
        // No-job IndependentWork is an explicit policy prior, not work proof.
        if (fuel_remaining() < 12000) { qualifier_stat_request = request; return; }
        const auto design = me.base_stats();
        request.strength_growth = design.strength_growth; request.focus_growth = design.focus_growth;
        if (!d::nonnegative(request.strength_growth) || !d::nonnegative(request.focus_growth)) return;
        const auto strength_item = qualifier_monster_variant(QualifierMonsterStat::Strength).shard.info();
        const auto focus_item = qualifier_monster_variant(QualifierMonsterStat::Focus).shard.info();
        // Only the released pure-stat variant relation can justify this first
        // capability override. Unknown hybrid metadata retains fallback.
        if (strength_item.strength_points <= 0 || strength_item.health_points != 0 || strength_item.focus_points != 0
            || focus_item.focus_points <= 0 || focus_item.health_points != 0 || focus_item.strength_points != 0) return;
        const double ds = request.strength_growth * strength_item.strength_points;
        const double df = request.focus_growth * focus_item.focus_points;
        request.basic.capable = ds > 0;
        request.basic.nominal_per_effect = ds;
        request.basic.currently_ready = me.can_attack();
        request.basic.timing_known = true;
        std::array<vec2, 16> positions{};
        int members = 0;
        bool geometry = group >= 0 && !S->groups[group].camps.empty() && S->groups[group].camps.size() <= positions.size();
        if (geometry) for (const auto& camp : S->groups[group].camps) {
            try {
                if (camp.h.state().is_dead()) continue;
                positions[members++] = camp.h.position();
            } catch (const cambc::error&) { geometry = false; break; }
        }
        if (geometry) {
            request.evidence = QualifierStatEvidence::CurrentGeometry;
            request.basic.geometry_known = true;
            for (int i = 0; i < members; ++i)
                if (cheb(request.position, positions[i]) <= me.attack_range()) ++request.basic.observed_targets;
        }
        const bool can_cast = !S->cast && me.can_cast();
        request.focus_passive_possible = me.passive() == passive_type::retaliation && df > 0;
        const auto abilities = me.abilities();
        for (int slot = 0; slot < 2; ++slot) {
            const auto& ability = abilities[slot];
            const auto& info = ability.info();
            if (!info) continue; // Legitimately unused slot.
            const auto common = std::visit([](const auto& value) { return static_cast<ability_properties>(value); }, *info);
            const bool ready = can_cast && ability.cooldown() == 0 && (!S->moved || !common.exclusive_with_movement);
            if (ability.is_attack_reset()) { request.reset_capable = true; continue; }
            double coefficient = 0, range = 0, radius = 0;
            bool damaging = false, area = false, timing = true;
            if (ability.is_damage() && ability.can_target_camps()) {
                const auto metadata = ability.get<ability_damage>(); coefficient = metadata.modifier; range = metadata.range; damaging = true;
            } else if (ability.is_cc() && ability.can_target_camps()) {
                const auto metadata = ability.get<ability_cc>(); coefficient = metadata.modifier; range = metadata.range; damaging = true;
            } else if (ability.is_aoe_damage() && ability.can_target_ground()) {
                const auto metadata = ability.get<ability_aoe_damage>(); coefficient = metadata.modifier;
                range = metadata.range; radius = metadata.radius; damaging = area = true;
                timing = d::immediate_area(request.type, slot);
            } else if (ability.is_dash() && ability.can_target_ground()) {
                const auto metadata = ability.get<ability_dash>(); coefficient = metadata.modifier;
                range = metadata.range; radius = metadata.radius; damaging = true; timing = false; // No unvalidated landing.
            }
            if (!d::nonnegative(coefficient) || !d::nonnegative(range) || !d::nonnegative(radius)) return;
            if (damaging && coefficient > 0 && df > 0) {
                request.focus_camp_possible = true;
                QualifierStatContribution contribution;
                contribution.capable = true; contribution.slot = slot; contribution.currently_ready = ready;
                contribution.nominal_per_effect = coefficient * df; contribution.timing_known = timing;
                contribution.geometry_known = geometry && timing;
                if (contribution.geometry_known) {
                    for (int i = 0; i < members; ++i) if (cheb(request.position, positions[i]) <= range) {
                        int hits = 1;
                        if (area) {
                            hits = 0;
                            for (int j = 0; j < members; ++j) if (cheb(positions[i], positions[j]) <= radius) ++hits;
                        }
                        contribution.observed_targets = std::max(contribution.observed_targets, hits);
                        if (!area || i >= 1) break; // At most two existing member centers.
                    }
                }
                if (!request.one_cast.capable || contribution.nominal_per_effect > request.one_cast.nominal_per_effect)
                    request.one_cast = contribution; // One slot summary, never sum both slots.
            }
            if (ability.is_shield() && ability.can_target_self()) {
                const auto metadata = ability.get<ability_shield>();
                if (!d::nonnegative(metadata.modifier)) return;
                if (metadata.modifier > 0 && df > 0) {
                    request.self_protection_possible = true;
                    request.shield = {true, ready, true, metadata.duration > 0, slot, 1, metadata.modifier * df, 0, {}};
                }
            }
            if (ability.is_heal()) {
                const auto metadata = ability.get<ability_heal>();
                if (!d::nonnegative(metadata.modifier) || !d::nonnegative(metadata.range)) return;
                if (metadata.modifier > 0 && df > 0 && ability.can_target_self()) request.self_protection_possible = true;
                if (metadata.modifier > 0 && df > 0 && ability.can_target_allies()) {
                    request.ally_support.capable = true; request.future_support_unknown = true;
                    request.ally_support.nominal_per_effect = metadata.modifier * df;
                    request.ally_support.slot = slot; request.ally_support.currently_ready = ready;
                    // A visible actual need remains conditionally useful even
                    // after this input's cast was spent. It is not a future cast.
                    const auto allies = vision::champions(team::ally);
                    if (allies.size() > 6) request.support_observation_unknown = true;
                    else for (const auto& ally : allies) {
                        try {
                            if (static_cast<handle_champion_t>(ally) == static_cast<handle_champion_t>(me)
                                || !ally.is_visible() || !ally.is_alive()
                                || cheb(request.position, ally.position()) > metadata.range) continue;
                            const double amount = ability.heal_amount(ally);
                            if (!d::nonnegative(amount)) { request.support_observation_unknown = true; continue; }
                            if (amount > 0) {
                                request.ally_support.geometry_known = request.ally_support.timing_known = true;
                                ++request.ally_support.observed_targets;
                                if (amount > request.ally_support.current_amount) {
                                    request.ally_support.current_amount = amount;
                                    request.ally_support.observed_ally = static_cast<std::uint32_t>(static_cast<handle_champion_t>(ally));
                                }
                            }
                        } catch (const cambc::error&) { request.support_observation_unknown = true; }
                    }
                }
            }
        }
        request.channels_known = true;
        const auto family = request.inventory.family;
        if (family == QualifierMonsterFamily::Partial) {
            request.preferred = request.inventory.stat; request.reason = QualifierStatReason::PartialLocked;
        } else if (family == QualifierMonsterFamily::Complete || family == QualifierMonsterFamily::CompleteMultiple) {
            request.preferred = request.inventory.stat; request.reason = QualifierStatReason::FamilyComplete;
        } else if (family == QualifierMonsterFamily::MixedPartial) request.reason = QualifierStatReason::MixedInventory;
        else if (purpose == QualifierStatPurpose::IndependentWork
            && request.preferred == QualifierMonsterStat::Focus && request.basic.capable
            && !request.focus_camp_possible && !request.self_protection_possible && !request.focus_passive_possible
            && !request.support_observation_unknown && request.ally_support.observed_targets == 0) {
            request.preferred = QualifierMonsterStat::Strength;
            request.reason = QualifierStatReason::WorkCapabilityMismatch;
        }
        if (!qualifier_current() || me.position() != request.position
            || qualifier_runtime.observation_revision != request.revision) return;
        request.valid = true;
        qualifier_stat_request = request;
    } catch (...) { /* Unknown request; no action and no phantom support/effects. */ }
}

bool qualifier_stat_request_matches() noexcept {
    const auto& request = qualifier_stat_request;
    try {
        if (!request.valid || !S || !qualifier_current() || request.round != S->round
            || request.revision != qualifier_runtime.observation_revision
            || request.actor != static_cast<std::uint32_t>(static_cast<handle_champion_t>(self()))
            || request.type != self().champion_name() || request.position != self().position()) return false;
        return request.group_index < 0 || (request.group_index < static_cast<int>(S->groups.size())
            && request.full_group == static_cast<std::uint32_t>(static_cast<handle_camp_group_t>(S->groups[request.group_index].h)));
    } catch (...) { return false; }
}
