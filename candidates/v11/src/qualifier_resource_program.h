#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>

// Owning conditional future models. No SDK objects, live contact records, or
// promises of future legality are stored here. Only current_stage is actionable
// after the adapter freshly revalidates its stamp and exact witness.
namespace qualifier::resource {
inline constexpr std::size_t max_goals = 8, max_members = 16, max_items = 128;
inline constexpr std::size_t max_nodes = 26, max_states = 16, max_abilities = 2;
inline constexpr std::uint32_t no_item = 0xffffffffu;
enum class Status { Program, NeedsObserve, NeedsMechanicEvidence, SearchBudgetExceeded, NoQualifiedCurrentStage, InvalidInput };
enum class Objective { Information, Gold, Gems, Win };
enum class StageKind { None, Inspect, Approach, Work, Capture, ReturnShop, Recover, Purchase, Deliver, ObserveAfterPurchase, WaitRespawn };
enum class AbilityKind { Unsupported, Damage, AreaDamage, Heal, Shield, Control, AttackReset };
enum class ActionKind { None, Basic, Cast };
enum class PurchaseKind { Item, Crystal };
enum class Passive { None, Monster, Durability, Treasure, Jungle, Home, Tribute, Capture, Prospector, Hunter, Skirmish, Collectors, Unsupported };
struct Stamp {
    std::uint32_t actor = 0, round = 0;
    std::uint64_t revision = 0, origin_token = 0;
    friend constexpr bool operator==(const Stamp&, const Stamp&) = default;
};
struct PurchaseIntent {
    PurchaseKind kind = PurchaseKind::Item;
    bool known = false, milestone_complete = false, requires_observation = false;
    std::uint32_t item = no_item, count = 0, recipe_result = no_item;
    std::uint32_t unit_cost = 0, total_cost = 0, per_visit_spend_cap = 0, reserved_route_cash = 0;
    std::uint32_t old_loose = 0, combine_count = 0;
    std::uint32_t primary_group_id = 0, primary_epoch = 0, capability_reason_bits = 0;
    std::uint64_t shop_token = 0;
};
struct CurrentStage {
    StageKind kind = StageKind::None;
    Stamp stamp{};
    bool witness_known = false;
    std::uint32_t goal = 0xffffffffu, group_id = 0, epoch = 0;
    std::uint64_t endpoint_token = 0;
    ActionKind action = ActionKind::None;
    std::uint32_t target_member_id = 0;
    int cast_slot = -1;
    std::uint64_t cast_token = 0;
    PurchaseIntent purchase{};
};
struct Item {
    bool known = false, purchasable = false, future_effect_known = false;
    std::uint32_t cost = 0, combine_count = 0, result = no_item;
    std::int32_t health_points = 0, strength_points = 0, focus_points = 0;
    Passive passive = Passive::None;
    double passive_magnitude = 0;
};
struct Ability {
    bool metadata_known = false, self_target_allowed = false;
    bool multi_target = false, retarget_single = false;
    AbilityKind kind = AbilityKind::Unsupported;
    std::uint32_t max_cooldown = 0, cooldown = 0, duration = 0;
    double focus_modifier = 0;
};
struct Actor {
    double hp = 0, max_hp = 0, strength = 0, focus = 0;
    double health_growth = 0, strength_growth = 0, focus_growth = 0;
    double ryth_health_strength_ratio = 0;
    bool independent_health_known = false, at_spawn = false, field_shop = false;
    bool projected_stats_known = false, current_allowances_known = false;
    bool can_attack = false, can_cast = false, can_move = false, currently_stunned = false;
    double intrinsic_gold_bonus = 0;
    std::uint32_t tribute_gold_per_gem = 0;
    std::uint32_t wallet = 0, crystals = 0, carried_gems = 0;
    std::array<std::uint32_t, max_items> owned{};
    // Qualified current shield only. Zero conservatively omits an unknown pool.
    double shield = 0;
    std::uint32_t shield_rounds = 0;
};
struct Route {
    // known includes qualified whole-leg NPC exposure, not just BFS distance.
    bool known = false, current_witness = false;
    bool same_endpoint_alias = false;
    bool includes_destination_owner_end = false;
    std::uint32_t turns = 0, gold_cost = 0, crystals_used = 0;
    double hp_cost_upper = 0;
    std::uint64_t endpoint_token = 0;
};
struct Delivery {
    bool known = false, current_witness = false;
    // Complete from the actual receipt node through deposit and next own start.
    std::uint32_t turns = 0, gold_cost = 0, crystals_used = 0;
    double hp_cost_upper = 0;
    std::uint32_t terminal_node = 0;
    // turns must upper-bound the COMPLETE channel for this bag capacity.
    // Zero binds the quote to Goal.nominal_gems. A new Prospector receipt must
    // not reuse a shorter tribute quote for the old smaller bag.
    std::uint32_t max_supported_gems = 0;
    std::uint64_t endpoint_token = 0;
};
struct Member {
    std::uint32_t id = 0;
    double hp = 0, incoming_per_phase = 0;
    double respawn_hp = 0;
    bool respawn_hp_known = false;
    // Actual controller's basic damage is strength*basic_factor. Do not set
    // known for nonlinear/unsupported effects or unreachable stationary tails.
    double basic_factor = 0;
    bool known = false, basic_reachable = false;
    std::array<bool, max_abilities> cast_hit{};
    std::array<double, max_abilities> cast_factor{{1, 1}};
};
struct Goal {
    std::uint32_t group_id = 0, epoch = 0, member_count = 0;
    bool composition_known = false, solo_independent = false, future_geometry_known = false;
    bool contact_outside_spawn = false;
    // Set only when supplied damage factors/incoming upper bounds exclude the
    // item effects the programme applies. Otherwise those benefits are omitted.
    bool damage_pre_item_bonuses = false, incoming_pre_item_mitigation = false;
    bool current_work_witness = false, current_inspect_witness = false, current_capture_witness = false;
    bool current_wait_witness = false;
    bool receipt_known = false, repeat_known = false;
    // If true nominal values are the exact solo pre-personal-bonus rewards.
    // Otherwise they are a qualified lower bound and future reward passives
    // are omitted instead of being applied twice to an observed receipt.
    bool personal_reward_model_known = false;
    // A current-engine HP-persistence proof is deliberately absent by default.
    bool chip_recover_evidence = false;
    std::uint32_t nominal_gold = 0, nominal_gems = 0, capture_turns = 0;
    std::uint32_t available_in = 0, respawn_turns = 0, departure_turns = 0;
    std::uint64_t contact_token = 0;
    // Exact current cast geometry, independently witnessed per slot. Future
    // cast_hit masks do not themselves establish a current executable cast.
    std::array<bool, max_abilities> current_cast_witness{};
    std::array<std::uint64_t, max_abilities> cast_tokens{};
    std::array<std::uint32_t, max_abilities> cast_target_ids{};
    std::array<Member, max_members> members{};
    Delivery delivery{};
};
struct Shop {
    bool known = false, current_purchase_witness = false, heals_health_delta = false;
    bool full_recovery_on_owner_end = false;
    bool consumables_legal = false;
    std::uint64_t token = 0;
    // A qualified lower healing amount per complete owner end at this spawn.
    // Zero means no forecast recovery rather than guessing the rules value.
    double recovery_per_turn = 0;
    std::uint32_t purchase_turns = 1;
};
struct Input {
    Stamp stamp{};
    Actor actor{};
    Shop shop{};
    std::array<Item, max_items> items{};
    std::array<Ability, max_abilities> abilities{};
    std::array<Goal, max_goals> goals{};
    std::size_t goal_count = 0;
    // Optional reserved synthetic slot outside the actual item enum range.
    // The adapter binds this slot to consumable_type::tp_crystal, never casts
    // its numeric value to item_type. PurchaseIntent.kind distinguishes it.
    std::uint32_t crystal_item = no_item;
    // Nodes: current0, the ONE chosen shop1, contact2+g, receipt10+g,
    // exact post-deposit stance18+g. Never alias a remote altar back to origin.
    std::array<std::array<Route, max_nodes>, max_nodes> routes{};
    std::uint32_t horizon_turns = 0, known_team_submitted = 0, win_target = 0;
    std::uint32_t operation_limit = 50000, combat_turn_limit = 512;
    // An outstanding information obligation survives gold choices in adapter
    // memory. It is cleared only by a fresh observation or physical deferral.
    bool primary_review_due = false;
};
struct Forecast {
    Objective objective = Objective::Information;
    bool conditional = true, timely_win = false;
    std::uint32_t elapsed = 0, win_turn = 0, submitted_gems = 0;
    std::uint64_t lifetime_gold_earned = 0;
    std::uint32_t wallet = 0, carried_gems = 0;
    double remaining_hp = 0;
};
struct Result {
    Status status = Status::NoQualifiedCurrentStage;
    CurrentStage current_stage{};
    Forecast forecast{};
    // This is a descriptive milestone, never an executable future purchase.
    PurchaseIntent intended_milestone{};
    bool primary_review_due = false, unsupported_primary = false;
    bool search_space_capped = false;
    std::uint32_t operations = 0, expanded_states = 0, fund_receipts = 0, shop_visits = 0;
    std::uint32_t evaluated_goals_mask = 0;
};
using BudgetCheck = bool (*)(void*);
Result select(const Input&, BudgetCheck = nullptr, void* = nullptr);
} // namespace qualifier::resource

#include "qualifier_resource_program.inc"
