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
};

struct Decision {
    int task = -1;
    bool bank = false;
    double rate = 0;
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

}  // namespace qualifier
