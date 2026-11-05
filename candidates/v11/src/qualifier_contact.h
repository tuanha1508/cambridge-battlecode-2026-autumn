#pragma once

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>

// Pure, bounded estimates supplied by the caller, not SDK observations or a
// survival guarantee. No allied DPS, future casts, capture or receipt credit.
namespace qualifier::contact {

inline constexpr std::size_t max_members = 16;
inline constexpr std::size_t max_options = 12;
using Values = std::array<double, max_members>;
using Reach = std::array<bool, max_members>;
using DamageMatrix = std::array<Values, max_members>; // [primary][affected]

enum class Class { ModeledSolo, Conditional, Unknown };
enum class Kind { BasicOnly, DirectDamage, AreaDamage, AttackReset, Shield, Dash, CrowdControl };
enum class Basics { None, Before, After, BeforeAndAfter };
enum class ShieldSource { None, Existing, SelectedCast };
enum class Reason {
    None, InvalidInput, MissingKnowledge, IllegalPrefix, NoCurrentAction,
    DeadCastTarget, TailUnsupported, TailUnreachable, TailDamageUnknown,
    HorizonExceeded, HealthBudgetExceeded, NumericOverflow
};

struct Stamp {
    std::uint32_t actor = 0, group = 0, round = 0;
    std::uint64_t revision = 0;
    friend constexpr bool operator==(const Stamp&, const Stamp&) = default;
};

struct Member {
    std::uint32_t id = 0, priority = 0;
    // Remaining effective work in HP units. Known target shields/modifiers
    // must be represented in this work or in the supplied damage treatment.
    double hp = 0;
    bool hp_known = false, effects_known = false;
};

struct ShieldCredit {
    ShieldSource source = ShieldSource::None;
    double value = 0;
    // Caller converts qualified expiry timing to covered future camp phases.
    // Never infer stacking: supply one qualified nonadditive post-prefix pool,
    // net of any prefix absorption and qualified after a planned shield cast.
    std::uint32_t future_phases = 0;
    bool qualified = false;
};

struct Option {
    Kind kind = Kind::BasicOnly;
    Basics basics = Basics::After;
    int cast_slot = -1, cast_target_member = -1;
    // Opaque exact cast target/destination and resulting stationary position.
    std::uint64_t cast_token = 0, contact_token = 0;
    bool metadata_known = false, prefix_geometry_known = false;
    bool relocation_qualified = false; // Required for a dash's after-position facts.
    bool prefix_legal = false, cast_ready = false;
    bool exclusive_with_movement = false;
    bool post_basic_permitted = false, reset_restores_attack = false;
    bool damage_estimates_known = false, tail_supported = false;
    bool exposure_known = false, future_wake_known = false;
    Reach before_reach{}, contact_reach{};
    // One actual basic hit per row. A primary is not counted twice. Secondary
    // splash requires explicit geometry/damage; zero is conservative omission.
    DamageMatrix before_basic{}, contact_basic{};
    // Immediate damage from the single selected cast. No unqualified DoT ticks.
    Values cast_damage{}, incoming_per_phase{};
    // A qualified initial contiguous budget of skipped future camp phases,
    // from observed remaining stun or this exact selected control prefix.
    // No future casts, immunity or old-plus-new control stacking is inferred.
    std::array<std::uint32_t, max_members> initial_suppressed_phases{};
    bool control_qualified = false;
    // Supplied net self-HP cost during the current prefix, not guessed enemy
    // retaliation. If unqualified, prefix_effects_known must remain false.
    double prefix_hp_cost = 0;
    bool prefix_effects_known = false;
    double outside_per_phase = 0;
    ShieldCredit shield{};
};

struct Input {
    Stamp stamp{};
    double self_hp = 0;
    bool self_hp_known = false;
    // Actual post-sustain allowances. A reset can restore a spent basic only
    // when that option separately qualifies reset_restores_attack.
    bool can_attack = false, can_cast = false, moved = false;
    std::uint32_t max_future_phases = 0;
    std::size_t member_count = 0, option_count = 0;
    std::array<Member, max_members> members{};
    std::array<Option, max_options> options{};
};

struct Plan {
    Class classification = Class::Unknown;
    Reason reason = Reason::MissingKnowledge;
    Stamp stamp{};
    std::size_t member_count = 0;
    std::array<std::uint32_t, max_members> member_ids{};
    int selected_option = -1;
    Kind kind = Kind::BasicOnly;
    int pre_basic = -1, cast_option = -1, post_basic = -1;
    int cast_slot = -1, cast_target_member = -1;
    std::uint64_t cast_token = 0, contact_token = 0;
    bool prefix_executable = false, tail_complete = false;
    // Predicted work after the current prefix, not a new observation.
    Values remaining_hp{};
    // Kill at current input is phase0; kill at own future stepT still incurs
    // the intervening camp phaseT. No loop over those future turns is used.
    std::array<std::uint32_t, max_members> kill_phase{};
    std::array<int, max_members> tail_order{};
    std::size_t tail_order_count = 0;
    std::uint32_t clear_phases = 0;
    double hp_cost = std::numeric_limits<double>::infinity();
    double prefix_work_removed = 0;
};

namespace detail {
inline bool nonnegative(double v) { return std::isfinite(v) && v >= 0; }

inline bool numeric_option(const Option& o, std::size_t n) {
    if (!nonnegative(o.prefix_hp_cost) || !nonnegative(o.outside_per_phase) || !nonnegative(o.shield.value)) return false;
    for (std::size_t i = 0; i < n; ++i) {
        if (!nonnegative(o.cast_damage[i]) || !nonnegative(o.incoming_per_phase[i])) return false;
        for (std::size_t j = 0; j < n; ++j)
            if (!nonnegative(o.before_basic[i][j]) || !nonnegative(o.contact_basic[i][j])) return false;
    }
    return true;
}

inline int target(const Input& in, const std::array<std::size_t, max_members>& order,
                  const Values& hp, const Reach& reach, const DamageMatrix& damage) {
    for (std::size_t k = 0; k < in.member_count; ++k) {
        const auto i = order[k];
        if (hp[i] > 0 && in.members[i].hp_known && reach[i] && damage[i][i] > 0)
            return static_cast<int>(i);
    }
    return -1;
}

inline void apply(Values& hp, const Values& damage, std::size_t n) {
    for (std::size_t i = 0; i < n; ++i) hp[i] = damage[i] >= hp[i] ? 0 : hp[i] - damage[i];
}

inline Plan evaluate(const Input& in, std::size_t oi,
                     const std::array<std::size_t, max_members>& order) {
    Plan p;
    p.stamp = in.stamp; p.member_count = in.member_count;
    p.selected_option = static_cast<int>(oi);
    for (std::size_t i = 0; i < in.member_count; ++i) {
        p.member_ids[i] = in.members[i].id;
        p.remaining_hp[i] = in.members[i].hp;
        p.kill_phase[i] = in.members[i].hp > 0 ? std::numeric_limits<std::uint32_t>::max() : 0;
    }
    const auto& o = in.options[oi];
    p.kind = o.kind;
    p.contact_token = o.contact_token; p.cast_token = o.cast_token;
    if (!numeric_option(o, in.member_count)) { p.reason = Reason::InvalidInput; return p; }
    if (static_cast<unsigned>(o.kind) > static_cast<unsigned>(Kind::CrowdControl) ||
        static_cast<unsigned>(o.basics) > static_cast<unsigned>(Basics::BeforeAndAfter) ||
        static_cast<unsigned>(o.shield.source) > static_cast<unsigned>(ShieldSource::SelectedCast)) {
        p.reason = Reason::InvalidInput; return p;
    }
    const bool cast = o.kind != Kind::BasicOnly;
    const bool before = o.basics == Basics::Before || o.basics == Basics::BeforeAndAfter;
    const bool after = o.basics == Basics::After || o.basics == Basics::BeforeAndAfter;
    if ((o.kind == Kind::BasicOnly && (o.cast_slot != -1 || o.cast_target_member != -1)) ||
        (cast && (o.cast_slot < 0 || o.cast_slot > 1)) ||
        o.cast_target_member < -1 || o.cast_target_member >= static_cast<int>(in.member_count) ||
        (before && after && o.kind != Kind::AttackReset) ||
        (o.shield.source == ShieldSource::SelectedCast && o.kind != Kind::Shield)) {
        p.reason = Reason::InvalidInput; return p;
    }
    if (!o.metadata_known || !o.prefix_geometry_known ||
        (o.kind == Kind::Dash && !o.relocation_qualified)) return p;
    if (!o.prefix_legal || (cast && (!in.can_cast || !o.cast_ready ||
        (in.moved && o.exclusive_with_movement)))) {
        p.classification = Class::Conditional; p.reason = Reason::IllegalPrefix; return p;
    }
    if (before) {
        if (!in.can_attack) { p.classification = Class::Conditional; p.reason = Reason::IllegalPrefix; return p; }
        p.pre_basic = target(in, order, p.remaining_hp, o.before_reach, o.before_basic);
        if (p.pre_basic < 0) { p.classification = Class::Conditional; p.reason = Reason::NoCurrentAction; return p; }
        apply(p.remaining_hp, o.before_basic[p.pre_basic], in.member_count);
    }
    if (cast) {
        // Never silently retarget a spell whose exact member target just died.
        if (o.cast_target_member >= 0 && (!in.members[o.cast_target_member].hp_known ||
            p.remaining_hp[o.cast_target_member] <= 0)) {
            p.pre_basic = -1; p.reason = Reason::DeadCastTarget; return p;
        }
        if (o.kind == Kind::AttackReset || o.kind == Kind::Shield)
            for (std::size_t i = 0; i < in.member_count; ++i)
                if (o.cast_damage[i] != 0) { p.pre_basic = -1; p.reason = Reason::InvalidInput; return p; }
        p.cast_option = static_cast<int>(oi); p.cast_slot = o.cast_slot;
        p.cast_target_member = o.cast_target_member;
        apply(p.remaining_hp, o.cast_damage, in.member_count);
    } else {
        for (std::size_t i = 0; i < in.member_count; ++i)
            if (o.cast_damage[i] != 0) { p.pre_basic = -1; p.reason = Reason::InvalidInput; return p; }
    }
    if (after) {
        // A cast may have cleared the whole group, leaving no post-basic target.
        bool remaining = false;
        for (std::size_t i = 0; i < in.member_count; ++i) remaining |= p.remaining_hp[i] > 0;
        if (remaining) {
            const bool reset = o.kind == Kind::AttackReset && o.reset_restores_attack;
            if (!o.post_basic_permitted || (!reset && (!in.can_attack || p.pre_basic >= 0))) {
                p.pre_basic = p.cast_option = -1; p.reason = Reason::IllegalPrefix; return p;
            }
            p.post_basic = target(in, order, p.remaining_hp, o.contact_reach, o.contact_basic);
            if (p.post_basic < 0) {
                p.pre_basic = p.cast_option = -1; p.reason = Reason::IllegalPrefix; return p;
            }
        }
        if (p.post_basic >= 0) apply(p.remaining_hp, o.contact_basic[p.post_basic], in.member_count);
    }
    p.prefix_executable = p.pre_basic >= 0 || p.cast_option >= 0 || p.post_basic >= 0;
    for (std::size_t i = 0; i < in.member_count; ++i) {
        p.prefix_work_removed += in.members[i].hp - p.remaining_hp[i];
        if (p.remaining_hp[i] <= 0) p.kill_phase[i] = 0;
    }
    if (!std::isfinite(p.prefix_work_removed)) { p.prefix_executable = false; p.reason = Reason::NumericOverflow; return p; }
    bool known = in.self_hp_known && o.prefix_effects_known && o.damage_estimates_known && o.exposure_known && o.future_wake_known;
    for (std::size_t i = 0; i < in.member_count; ++i)
        known = known && in.members[i].hp_known && in.members[i].effects_known &&
            (o.initial_suppressed_phases[i] == 0 || o.control_qualified);
    if (!known) return p; // Exact legal prefix can exist without full-clear credit.
    bool remaining = false;
    for (std::size_t i = 0; i < in.member_count; ++i) remaining |= p.remaining_hp[i] > 0;
    if (!p.prefix_executable && remaining) {
        p.classification = Class::Conditional; p.reason = Reason::NoCurrentAction; return p;
    }
    if (remaining && !o.tail_supported) { p.classification = Class::Conditional; p.reason = Reason::TailUnsupported; return p; }
    for (std::size_t i = 0; i < in.member_count; ++i) if (p.remaining_hp[i] > 0) {
        if (!o.contact_reach[i]) { p.classification = Class::Conditional; p.reason = Reason::TailUnreachable; return p; }
        if (o.contact_basic[i][i] <= 0) { p.reason = Reason::TailDamageUnknown; return p; }
    }

    Values tail_hp = p.remaining_hp;
    std::uint32_t phase = 0;
    for (std::size_t k = 0; k < in.member_count; ++k) {
        const auto primary = order[k];
        if (tail_hp[primary] <= 0) continue;
        const auto& damage = o.contact_basic[primary];
        const double count = std::ceil(tail_hp[primary] / damage[primary]);
        if (!std::isfinite(count) || count < 1) { p.reason = Reason::NumericOverflow; return p; }
        if (count > static_cast<double>(in.max_future_phases - phase)) {
            p.classification = Class::Conditional; p.reason = Reason::HorizonExceeded; return p;
        }
        const auto shots = static_cast<std::uint32_t>(count);
        p.tail_order[p.tail_order_count++] = static_cast<int>(primary);
        for (std::size_t i = 0; i < in.member_count; ++i) {
            if (tail_hp[i] <= 0 || damage[i] <= 0) continue;
            const double until_death = std::ceil(tail_hp[i] / damage[i]);
            if (!std::isfinite(until_death) || until_death < 1) { p.reason = Reason::NumericOverflow; return p; }
            if (until_death <= count) {
                p.kill_phase[i] = phase + static_cast<std::uint32_t>(until_death);
                tail_hp[i] = 0;
            } else {
                const double dealt = damage[i] * shots;
                if (!std::isfinite(dealt)) { p.reason = Reason::NumericOverflow; return p; }
                tail_hp[i] -= dealt;
            }
        }
        phase += shots;
    }
    p.tail_complete = true; p.clear_phases = phase;
    for (std::size_t i = 0; i < in.member_count; ++i)
        if (tail_hp[i] > 0) { p.tail_complete = false; p.reason = Reason::TailDamageUnknown; return p; }
    double damage = o.outside_per_phase * phase;
    const auto coverage = o.shield.qualified && o.shield.source != ShieldSource::None
        ? (o.shield.future_phases < phase ? o.shield.future_phases : phase) : 0;
    double covered = o.outside_per_phase * coverage;
    for (std::size_t i = 0; i < in.member_count; ++i) {
        const auto suppressed = o.initial_suppressed_phases[i];
        const auto hits = p.kill_phase[i] > suppressed ? p.kill_phase[i] - suppressed : 0;
        damage += o.incoming_per_phase[i] * hits;
        const auto covered_end = p.kill_phase[i] < coverage ? p.kill_phase[i] : coverage;
        const auto covered_start = suppressed < covered_end ? suppressed : covered_end;
        covered += o.incoming_per_phase[i] * (covered_end - covered_start);
    }
    if (!nonnegative(damage) || !nonnegative(covered)) { p.tail_complete = false; p.reason = Reason::NumericOverflow; return p; }
    const double absorb = o.shield.qualified && o.shield.source != ShieldSource::None
        ? (o.shield.value < covered ? o.shield.value : covered) : 0;
    p.hp_cost = o.prefix_hp_cost + (damage - absorb);
    if (!nonnegative(p.hp_cost)) { p.tail_complete = false; p.reason = Reason::NumericOverflow; return p; }
    p.classification = p.hp_cost < in.self_hp ? Class::ModeledSolo : Class::Conditional;
    p.reason = p.classification == Class::ModeledSolo ? Reason::None : Reason::HealthBudgetExceeded;
    return p;
}

inline bool better(const Plan& a, const Plan& b) {
    const auto rank = [](Class c) { return c == Class::ModeledSolo ? 0 : c == Class::Conditional ? 1 : 2; };
    if ((a.classification == Class::ModeledSolo) != (b.classification == Class::ModeledSolo))
        return a.classification == Class::ModeledSolo;
    if (a.prefix_executable != b.prefix_executable) return a.prefix_executable;
    if (rank(a.classification) != rank(b.classification)) return rank(a.classification) < rank(b.classification);
    if (a.tail_complete != b.tail_complete) return a.tail_complete;
    if (a.clear_phases != b.clear_phases) return a.clear_phases < b.clear_phases;
    if (a.hp_cost != b.hp_cost) return a.hp_cost < b.hp_cost;
    const auto actions = [](const Plan& p) { return int(p.pre_basic >= 0) + int(p.cast_option >= 0) + int(p.post_basic >= 0); };
    if (actions(a) != actions(b)) return actions(a) < actions(b);
    return a.prefix_work_removed > b.prefix_work_removed;
}
} // namespace detail

inline bool matches(const Input& in, const Plan& plan) {
    if (!(in.stamp == plan.stamp) || in.member_count != plan.member_count ||
        in.member_count > max_members || plan.selected_option < 0 ||
        static_cast<std::size_t>(plan.selected_option) >= in.option_count || in.option_count > max_options) return false;
    for (std::size_t i = 0; i < in.member_count; ++i)
        if (in.members[i].id != plan.member_ids[i]) return false;
    const auto& option = in.options[plan.selected_option];
    return option.kind == plan.kind && option.contact_token == plan.contact_token && option.cast_token == plan.cast_token &&
        option.cast_slot == plan.cast_slot && option.cast_target_member == plan.cast_target_member;
}

inline Plan choose(const Input& in) {
    Plan invalid; invalid.stamp = in.stamp; invalid.reason = Reason::InvalidInput;
    if (in.member_count == 0 || in.member_count > max_members || in.option_count > max_options ||
        !std::isfinite(in.self_hp) || in.self_hp <= 0) return invalid;
    std::array<std::size_t, max_members> order{};
    for (std::size_t i = 0; i < in.member_count; ++i) {
        if (!detail::nonnegative(in.members[i].hp)) return invalid;
        for (std::size_t j = 0; j < i; ++j) if (in.members[j].id == in.members[i].id) return invalid;
        order[i] = i;
        for (std::size_t j = i; j > 0; --j) {
            const auto& a = in.members[order[j]], &b = in.members[order[j - 1]];
            if (a.priority > b.priority || (a.priority == b.priority && a.id >= b.id)) break;
            const auto tmp = order[j]; order[j] = order[j - 1]; order[j - 1] = tmp;
        }
    }
    Plan best = invalid;
    bool selected = false;
    for (std::size_t o = 0; o < in.option_count; ++o) {
        auto plan = detail::evaluate(in, o, order);
        if (!selected || detail::better(plan, best)) { best = plan; selected = true; }
    }
    return best;
}

} // namespace qualifier::contact
