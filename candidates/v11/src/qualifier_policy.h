#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace qualifier {

// These are caller-supplied estimates, not observations or a strength claim.
// The caller owns feasibility, legal execution, uncertainty and available help.
struct Task {
    int id = -1;
    double reward_gems = 0;
    double reward_gold = 0;
    double travel_turns = 0;
    double work_turns = 0;
    double capture_turns = 0;
    double delivery_turns = 0;
    double bank_turns = 0;
    // Post-deposit readiness time for cycle estimates. Pre-deposit recovery
    // belongs to an explicit ResourceAccessProgram stage.
    double recovery_turns = 0;
    bool feasible = false;
    // A score-only preference. It never changes physical ETA or feasibility.
    double ordinary_preference = 1.0;
    // False permits a gold-only receipt with no current cargo. It never awards
    // delivered gems or promises a route that the caller could not construct.
    bool delivery_available = true;
    // Current required purchase only, not a second charge for held inventory.
    double delivery_gold_cost = 0;
};

struct Context {
    double carried_gems = 0;
    double gold_weight = 0;
    // Negative or nonfinite means direct banking is not feasible.
    double bank_turns = -1;
    int current_task = -1;
    // An omitted horizon preserves the caller's unbounded estimate contract.
    double remaining_turns = std::numeric_limits<double>::infinity();
    double bank_gold_cost = 0;
    double team_submitted_gems = 0;
    double winning_gems = 1000;
};

enum class Objective { Estimate, None, Gold, Gems, Win };

struct Decision {
    int task = -1;
    bool bank = false;
    double rate = 0;
    Objective objective = Objective::Estimate;
    double value = 0;
    double completion_turns = 0;
    double lifetime_gold_earned = 0;
};

// Contact shortlists and information gathering deliberately retain their old
// estimate contract. Only a final economic assignment requires positive value.
enum class Admission { Estimate, PositiveEconomic };

// Explicitly uncalibrated persistence band, not an empirically chosen weight.
inline constexpr double kUncalibratedTargetHysteresisFraction = 0.05;

inline Decision choose(Context context, const std::vector<Task>& tasks,
                       Admission admission = Admission::Estimate) {
    const auto nonnegative_finite = [](double value) {
        return std::isfinite(value) && value >= 0;
    };
    Decision best{-1, false, -1};
    if (!nonnegative_finite(context.carried_gems) ||
        !nonnegative_finite(context.gold_weight) || !nonnegative_finite(context.bank_gold_cost) ||
        context.current_task < -1 ||
        std::isnan(context.remaining_turns) || context.remaining_turns < 0) {
        return best;
    }

    if (context.carried_gems > 0 && nonnegative_finite(context.bank_turns) &&
        context.bank_turns <= context.remaining_turns) {
        const double rate = (context.carried_gems - context.gold_weight * context.bank_gold_cost)
            / std::max(1.0, context.bank_turns);
        if (std::isfinite(rate) && rate > 0) best = {-1, true, rate};
    }

    Decision current{-1, false, -1};
    for (const Task& task : tasks) {
        if (!task.feasible || task.id < 0 ||
            !nonnegative_finite(task.reward_gems) ||
            !nonnegative_finite(task.reward_gold) ||
            !nonnegative_finite(task.travel_turns) ||
            !nonnegative_finite(task.work_turns) ||
            !nonnegative_finite(task.capture_turns) ||
            !nonnegative_finite(task.delivery_turns) ||
            !nonnegative_finite(task.bank_turns) ||
            !nonnegative_finite(task.recovery_turns) ||
            !nonnegative_finite(task.delivery_gold_cost)) {
            continue;
        }

        if (!task.delivery_available &&
            (context.carried_gems > 0 || task.reward_gems > 0)) continue;

        const double receipt_turns =
            task.travel_turns + task.work_turns + task.capture_turns;
        const double total_turns = receipt_turns + task.delivery_turns +
                                   task.bank_turns + task.recovery_turns;
        const double delivered_gems = context.carried_gems + task.reward_gems;
        if (!std::isfinite(receipt_turns) || !std::isfinite(total_turns) ||
            !std::isfinite(delivered_gems) || receipt_turns > context.remaining_turns) {
            continue;
        }

        // Existing cargo must complete this task's entire trip too. Gold is
        // earned at receipt, so its estimate does not wait for gem delivery.
        // A timely gold receipt can still have value when gem delivery is late.
        const double gem_rate = task.delivery_available && total_turns <= context.remaining_turns
            ? delivered_gems / std::max(1.0, total_turns) : 0.0;
        const double gold_rate = context.gold_weight *
            ((task.reward_gold - task.delivery_gold_cost) / std::max(1.0, receipt_turns));
        const double preference = std::isfinite(task.ordinary_preference) &&
            task.ordinary_preference > 0 && task.ordinary_preference <= 1
                ? task.ordinary_preference : 1.0;
        const double rate = (gem_rate + gold_rate) * preference;
        if (!std::isfinite(gem_rate) || !std::isfinite(gold_rate) ||
            !std::isfinite(rate)) {
            continue;
        }

        if ((task.delivery_gold_cost > 0 || admission == Admission::PositiveEconomic)
            && rate <= 0) continue;
        if (task.id == context.current_task && rate > current.rate) {
            current = {task.id, false, rate};
        }
        // Outside persistence, direct banking wins an exact tie. Equal task
        // rates choose the smallest ID, independent of vector ordering.
        if (rate > best.rate ||
            (rate == best.rate && !best.bank &&
             (best.task < 0 || task.id < best.task))) {
            best = {task.id, false, rate};
        }
    }

    if (current.task >= 0 &&
        current.rate >= best.rate *
            (1.0 - kUncalibratedTargetHysteresisFraction)) {
        return current;
    }
    return best;
}

// Current, individually qualified stages only. A ResourceAccessProgram prices
// multi-stage alternatives separately; its hypothetical work is never inserted
// in Task.reward_gems. This fallback uses the released terminal objective:
// tribute, then lifetime income. Cash expenditure is an affordability constraint
// owned by the caller, not negative lifetime gold or a gem conversion rate.
inline Decision choose_objective(Context context, const std::vector<Task>& tasks) {
    const auto nn = [](double v) { return std::isfinite(v) && v >= 0; };
    Decision best; best.objective = Objective::None;
    if (!nn(context.carried_gems) || !nn(context.team_submitted_gems)
        || !std::isfinite(context.winning_gems) || context.winning_gems <= 0
        || std::isnan(context.remaining_turns) || context.remaining_turns < 0
        || context.current_task < -1) return best;
    const double needed = std::max(0.0, context.winning_gems - context.team_submitted_gems);
    auto better = [](const Decision& a, const Decision& b) {
        if (a.objective != b.objective) return a.objective > b.objective;
        if (a.objective != Objective::Win && a.value != b.value) return a.value > b.value;
        if (a.objective != Objective::Win && a.lifetime_gold_earned != b.lifetime_gold_earned)
            return a.lifetime_gold_earned > b.lifetime_gold_earned;
        if (a.completion_turns != b.completion_turns)
            return a.completion_turns < b.completion_turns;
        if (a.bank != b.bank) return a.bank;
        if (a.rate != b.rate) return a.rate > b.rate; // Advisory tie only.
        return a.task >= 0 && (b.task < 0 || a.task < b.task);
    };
    auto gem = [&](int task, bool bank, double amount, double turns, double preference, double earned_gold) {
        Decision d;
        if (!nn(amount) || amount <= 0 || !nn(turns) || turns > context.remaining_turns) return d;
        d.task = task; d.bank = bank; d.value = amount; d.completion_turns = turns;
        d.lifetime_gold_earned = earned_gold;
        d.objective = needed > 0 && amount >= needed ? Objective::Win : Objective::Gems;
        d.rate = std::min(amount, needed > 0 ? needed : amount) / std::max(1.0, turns) * preference;
        return d;
    };
    if (context.carried_gems > 0 && nn(context.bank_turns)) {
        const auto d = gem(-1, true, context.carried_gems, context.bank_turns, 1, 0);
        if (d.objective != Objective::Estimate && better(d, best)) best = d;
    }
    Decision current; current.objective = Objective::None;
    for (const auto& t : tasks) {
        if (!t.feasible || t.id < 0 || !nn(t.reward_gems) || !nn(t.reward_gold)
            || !nn(t.travel_turns) || !nn(t.work_turns) || !nn(t.capture_turns)
            || !nn(t.delivery_turns) || !nn(t.bank_turns) || !nn(t.recovery_turns)
            || !nn(t.delivery_gold_cost)) continue;
        const double receipt = t.travel_turns + t.work_turns + t.capture_turns;
        const double complete = receipt + t.delivery_turns + t.bank_turns;
        if (!nn(receipt) || !nn(complete) || receipt > context.remaining_turns) continue;
        const double preference = std::isfinite(t.ordinary_preference)
            && t.ordinary_preference > 0 && t.ordinary_preference <= 1 ? t.ordinary_preference : 1;
        Decision d; d.objective = Objective::None;
        if (t.delivery_available) {
            const auto g = gem(t.id, false, context.carried_gems + t.reward_gems, complete, preference, t.reward_gold);
            if (g.objective != Objective::Estimate) d = g;
        }
        // Existing cargo receives the separate last-chance bank/resource path.
        // An unreachable/late current bank is not permission to farm its bag.
        if (d.objective == Objective::None && context.carried_gems == 0 && t.reward_gold > 0) {
            d = {t.id, false, t.reward_gold / std::max(1.0, receipt) * preference,
                 Objective::Gold, t.reward_gold, receipt, t.reward_gold};
        }
        if (d.objective == Objective::None || !std::isfinite(d.rate) || d.rate <= 0) continue;
        if (t.id == context.current_task) current = d;
        if (better(d, best)) best = d;
    }
    if (current.task >= 0 && current.objective == best.objective
        && !best.bank && current.value == best.value
        && current.lifetime_gold_earned == best.lifetime_gold_earned
        && current.completion_turns == best.completion_turns) return current;
    return best;
}

}  // namespace qualifier
