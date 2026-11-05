#pragma once
#include <cambc.h>
#include "qualifier_policy.h"
#include "qualifier_opening_plan.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

namespace qualifier_draft {
using namespace cambc;
using pick = champion_draft;
namespace opening = qualifier_opening_plan;
// The exact previous unknown-feature fallback, not a learned roster ranking.
inline constexpr std::array<pick, 12> fallback{
    pick::marcella, pick::heart_of_ruin, pick::vesper_vale, pick::vendetta,
    pick::cassi_and_issac, pick::willow, pick::varg, pick::stellar,
    pick::ryth, pick::tavi, pick::ren, pick::prism};
inline bool nonnegative(double x) { return std::isfinite(x) && x >= 0; }
inline bool positive(double x) { return std::isfinite(x) && x > 0; }
inline std::size_t rank(pick p) {
    return static_cast<std::size_t>(std::find(fallback.begin(), fallback.end(), p) - fallback.begin());
}

struct Controller {
    opening::Plan accepted;
    opening::Plan proposed; // Full suffix is local, never serialized as accepted.
    bool optional = true, ready = false, dirty = false;
    std::uint64_t starting_fuel = 0;

    void disable() {
        optional = false; ready = false;
        for (std::size_t i = 0; i < accepted.accepted; ++i) {
            accepted.entries[i].assigned = false; accepted.entries[i].group = 0;
        }
        dirty = true;
    }
    bool same_prefix(const opening::Plan& plan) const {
        if (plan.slots != accepted.slots) return false;
        for (std::size_t i = 0; i < accepted.accepted; ++i)
            if (plan.entries[i].pick != accepted.entries[i].pick) return false;
        return true;
    }
    void publish() {
        if (!dirty || accepted.slots < 3 || accepted.slots > 5) return;
        dirty = false;
        try {
            const auto bytes = opening::encode(accepted);
            if (bytes.size() > 1024 || bytes.size() > draft_memory.size())
                throw std::runtime_error("opening draft memory capacity");
            draft_memory = bytes; // SDK startup snapshot is NOT acknowledgement.
        } catch (...) {
            disable(); // Clear future local advice, never undo an accepted pick or host write.
        }
    }
    // Best-effort only. Status0 appends to our ledger;134 merely reports that
    // the host had a selection at the evaluated call. Neither is an input token
    // if call/return spans suspension. No round_number/list-length ordinal.
    void checkpoint() {
        try {
            const auto slots = draft::team_size();
            if (slots < 3 || slots > 5) throw std::runtime_error("opening draft size");
            if (accepted.slots && accepted.slots != slots) disable();
            accepted.slots = static_cast<std::uint8_t>(slots);
            const auto actual = draft::current_draft(team::ally);
            if (actual.size() > slots) throw std::runtime_error("opening draft count");
            for (std::size_t i = 0; i < actual.size(); ++i) {
                if (!opening::valid_pick(actual[i])) throw std::runtime_error("opening draft enum");
                for (std::size_t j = 0; j < i; ++j)
                    if (actual[i] == actual[j]) throw std::runtime_error("opening duplicate draft");
            }
            // The completed list may omit our pending accepted selection. An
            // extra/reordered/revealed random pick disables optional advice.
            bool prefix = actual.size() <= accepted.accepted;
            for (std::size_t i = 0; prefix && i < actual.size(); ++i)
                prefix = actual[i] == accepted.entries[i].pick;
            if (!prefix) disable();
            if (ready && !same_prefix(proposed)) disable();
            if (accepted.accepted >= slots || actual.size() >= slots) { publish(); return; }
            auto used = [&](pick p) {
                if (std::find(actual.begin(), actual.end(), p) != actual.end()) return true;
                for (std::size_t i = 0; i < accepted.accepted; ++i)
                    if (accepted.entries[i].pick == p) return true;
                return false;
            };
            opening::Entry next;
            bool found = false;
            if (ready && same_prefix(proposed)) {
                next = proposed.entries[accepted.accepted]; found = !used(next.pick);
                if (!found) disable();
            }
            if (!found) for (const auto p : fallback) if (!used(p)) {
                next = {p, opening::default_body(p), 0, false}; found = true; break;
            }
            if (!found) throw std::runtime_error("opening no unused pick");
            try {
                draft::pick(next.pick);
                // ONLY a successful API return owns this accepted prefix entry.
                accepted.entries[accepted.accepted++] = next;
                dirty = true;
            } catch (const error& e) {
                if (e.code() != error_code::draft_already_picked) disable();
                //134 never appends next or acknowledges a current local epoch.
            }
            publish();
        } catch (...) { disable(); }
    }
    void boundary() {
        checkpoint(); // First/next legal pick takes priority over another chunk.
        if (!optional || accepted.accepted == 0 || accepted.accepted >= accepted.slots)
            throw std::runtime_error("opening optional stopped");
        const auto left = fuel_remaining();
        if (left < 200000 || (starting_fuel > left && starting_fuel - left > 2000000))
            throw std::runtime_error("opening optional budget");
        // A poll/exception cannot catch fuel suspension INSIDE the next call.
    }
};

struct Member { vec2 position{}; double hp = 0; };
struct Job {
    std::uint32_t id = 0;
    double gems = 0, gold = 0, optimistic = 0;
    vec2 receipt{}, hub{};
    int approach = 0, delivery = 0;
    std::array<Member, 16> members{};
    std::size_t count = 0;
};
struct Capability {
    champion_type body = champion_type::marcella;
    double strength = 0;
    int speed = 0, range = 0;
    bool known = false;
};
struct Edge { double rate = 0; champion_type body = champion_type::marcella; };
struct Portfolio {
    std::vector<Job> jobs;
    std::array<std::array<Edge, 8>, 12> edges{};
};
inline std::uint32_t full_id(const map::camp_group& group) {
    return static_cast<std::uint32_t>(static_cast<handle_camp_group_t>(group));
}
inline bool inside(vec2 p, int w, int h) { return p.x >= 0 && p.y >= 0 && p.x < w && p.y < h; }
inline double walk(int steps, int speed) { return std::ceil(static_cast<double>(steps) / speed); }
inline int distance(const std::vector<std::uint16_t>& values, vec2 p, int width) {
    return values[static_cast<std::size_t>(p.y) * width + p.x];
}
inline bool blocked(const std::vector<std::uint8_t>& values, vec2 p, int width) {
    return values[static_cast<std::size_t>(p.y) * width + p.x] != 0;
}
inline double nominal_rate(const Job& job, int speed, double travel, double work,
    std::vector<qualifier::Task>& scratch, int delivery_steps = -1) {
    const double bank = job.gems > 0 ? 1.0 + std::max<double>(constants::submit_gem_minimum_duration,
        std::ceil(job.gems * constants::submit_gem_duration_multiplier)
            + constants::submit_gem_base_duration) : 0;
    qualifier::Task task;
    task.id = 0; task.reward_gems = job.gems; task.reward_gold = job.gold;
    task.travel_turns = travel; task.work_turns = work; task.capture_turns = constants::capture_time;
    task.delivery_turns = job.gems > 0 ? walk(delivery_steps >= 0 ? delivery_steps : job.delivery, speed) : 0;
    task.bank_turns = bank; task.feasible = true; // Nominal ranking ONLY.
    qualifier::Context context; context.gold_weight = 0.4; context.remaining_turns = constants::max_rounds;
    scratch[0] = task; // One reusable owned slot, no per-cell allocation.
    const auto decision = qualifier::choose(context, scratch);
    return decision.task == task.id && positive(decision.rate) ? decision.rate : 0;
}

// Static nominal design ranking only. No live abilities, permission, inventory,
// awake state, shields, helper damage or survival/actual route certificate.
inline Portfolio extract(Controller& controller) {
    controller.boundary();
    const auto w = map::width(), h = map::height();
    const auto area = std::int64_t(w) * h;
    if (w <= 0 || h <= 0 || area > 16384) throw std::runtime_error("opening area cap");
    const auto terrain = map::walls() | map::holes() | map::altars() | map::spawn_region(team::enemy);
    const auto spawns = map::spawn_region(team::ally);
    if (spawns.is_empty()) throw std::runtime_error("opening empty spawn");
    controller.boundary();
    std::vector<std::uint8_t> terrain_cells(static_cast<std::size_t>(area), 0);
    for (const auto p : terrain.positions()) {
        if (!inside(p, w, h)) throw std::runtime_error("opening bad terrain tile");
        terrain_cells[static_cast<std::size_t>(p.y) * w + p.x] = 1;
    }
    controller.boundary();
    auto blocked_cells = terrain_cells;
    // This API materializes the WHOLE layout before caller caps. The unresolved
    // in-flight-call fuel gap is not fixed by the surrounding checkpoints.
    const auto groups = map::camp_groups();
    controller.boundary();
    if (groups.size() > 256) throw std::runtime_error("opening group cap");
    bitboard camp_tiles;
    std::size_t members = 0;
    std::vector<std::uint32_t> ids;
    for (const auto& group : groups) {
        controller.boundary();
        ids.push_back(full_id(group));
        for (const auto& camp : group.camps()) {
            if (++members > 4096) throw std::runtime_error("opening member cap");
            const auto p = camp.spawn_position();
            if (!inside(p, w, h)) throw std::runtime_error("opening bad camp tile");
            camp_tiles.set(p);
            blocked_cells[static_cast<std::size_t>(p.y) * w + p.x] = 1;
        }
    }
    std::sort(ids.begin(), ids.end());
    if (std::adjacent_find(ids.begin(), ids.end()) != ids.end()) throw std::runtime_error("opening duplicate group");
    const auto altars = map::altars().positions();
    if (altars.size() > 256) throw std::runtime_error("opening altar cap");
    bitboard service;
    for (const auto altar : altars) service.draw_chebyshev(altar, static_cast<float>(constants::submit_gems_range));
    service &= ~terrain;
    distance_field from_spawn, from_altar;
    bitboard visited;
    const auto static_walk = terrain | camp_tiles;
    controller.boundary();
    from_spawn.compute(static_walk, spawns & ~static_walk, visited, static_cast<std::uint16_t>(area));
    controller.boundary();
    // Future cleared-group delivery remains optimistic about other dynamic NPCs.
    from_altar.compute(terrain, service, visited, static_cast<std::uint16_t>(area));
    controller.boundary();
    const auto spawn_distances = from_spawn.export_linear_buffer();
    controller.boundary();
    const auto altar_distances = from_altar.export_linear_buffer();
    controller.boundary();
    if (spawn_distances.size() != static_cast<std::size_t>(area)
        || altar_distances.size() != static_cast<std::size_t>(area))
        throw std::runtime_error("opening field buffer size");
    std::array<Capability, 13> capabilities{};
    double greatest_strength = 0;
    int greatest_speed = 0;
    for (std::size_t i = 0; i < capabilities.size(); ++i) {
        controller.boundary();
        const auto body = static_cast<champion_type>(i); // Physical design table only.
        const auto stats = champion_designs[i].base_stats();
        controller.boundary();
        auto& cap = capabilities[i]; cap.body = body;
        cap.strength = stats.base_strength; cap.speed = stats.base_move_speed; cap.range = stats.base_attack_range;
        cap.known = positive(cap.strength) && cap.speed > 0 && cap.speed < distance_field::unreachable
            && cap.range > 0 && cap.range <= 8 && stats.camp_damage_multiplier == 1.0f;
        if (cap.known) {
            greatest_strength = std::max(greatest_strength, cap.strength);
            greatest_speed = std::max(greatest_speed, cap.speed);
        }
    }
    if (!positive(greatest_strength)) throw std::runtime_error("opening unknown basic designs");
    if (constants::capture_time < 0 || !nonnegative(constants::submit_gem_duration_multiplier)
        || constants::submit_gem_base_duration < 0 || constants::submit_gem_minimum_duration < 1)
        throw std::runtime_error("opening bad timing");
    std::array<std::optional<camp_stats>, 6> designs{};
    std::vector<qualifier::Task> rate_scratch(1);
    Portfolio result;
    for (const auto& group : groups) {
        controller.boundary();
        if ((group.gem_value() == 0 && group.gold_value() == 0) || group.camps().empty()
            || group.camps().size() > 16) continue; // Unmodeled, not impossible.
        Job job; job.id = full_id(group); job.gems = group.gem_value(); job.gold = group.gold_value();
        double basic_lower = 0;
        bool known = true;
        for (const auto& camp : group.camps()) {
            const auto k = static_cast<std::size_t>(camp.kind());
            if (k >= designs.size()) { known = false; break; }
            if (!designs[k]) { controller.boundary(); designs[k] = camp.base_stats(); controller.boundary(); }
            const auto hp = designs[k]->base_health;
            if (!positive(hp)) { known = false; break; }
            job.members[job.count++] = {camp.spawn_position(), hp};
            basic_lower += std::ceil(hp / greatest_strength); // Every member's last hit.
        }
        if (!known || !positive(basic_lower)) continue;
        double best_receipt_rate = 0;
        for (std::size_t i = 0; i < job.count; ++i) {
            const auto receipt = job.members[i].position;
            if (blocked(terrain_cells, receipt, w)) continue;
            const auto delivery = distance(altar_distances, receipt, w);
            if (job.gems > 0 && delivery == distance_field::unreachable) continue;
            for (int dy = -1; dy <= 1; ++dy) for (int dx = -1; dx <= 1; ++dx) {
                const vec2 hub{receipt.x + dx, receipt.y + dy};
                if ((dx == 0 && dy == 0) || !inside(hub, w, h) || blocked(blocked_cells, hub, w)) continue;
                const auto approach = distance(spawn_distances, hub, w);
                if (approach == distance_field::unreachable) continue;
                const double rate = nominal_rate(job, greatest_speed,
                    walk(approach, greatest_speed) + walk(1, greatest_speed), basic_lower,
                    rate_scratch, job.gems > 0 ? delivery : 0);
                const bool earlier = receipt.y < job.receipt.y || (receipt.y == job.receipt.y
                    && (receipt.x < job.receipt.x || (receipt.x == job.receipt.x
                        && (hub.y < job.hub.y || (hub.y == job.hub.y && hub.x < job.hub.x)))));
                if (positive(rate) && (rate > best_receipt_rate || (rate == best_receipt_rate && earlier))) {
                    best_receipt_rate = rate; job.receipt = receipt; job.hub = hub;
                    job.approach = approach; job.delivery = job.gems > 0 ? delivery : 0;
                }
            }
        }
        if (!positive(best_receipt_rate)) continue;
        // A bounded optimistic shortlist, not one actual combined champion or
        // complete-task credit. Gold still arrives at receipt. Each retained
        // real-body edge below adds its full per-member/star itinerary.
        job.optimistic = best_receipt_rate;
        if (!positive(job.optimistic)) continue;
        result.jobs.push_back(job);
        std::sort(result.jobs.begin(), result.jobs.end(), [](const Job& a, const Job& b) {
            return a.optimistic != b.optimistic ? a.optimistic > b.optimistic : a.id < b.id;
        });
        if (result.jobs.size() > 8) result.jobs.pop_back();
    }
    for (std::size_t j = 0; j < result.jobs.size(); ++j) {
        controller.boundary();
        const auto& job = result.jobs[j];
        bitboard root; root.set(job.hub);
        distance_field from_hub;
        from_hub.compute(static_walk, root, visited, static_cast<std::uint16_t>(area));
        controller.boundary();
        const auto hub_distances = from_hub.export_linear_buffer();
        controller.boundary();
        if (hub_distances.size() != static_cast<std::size_t>(area))
            throw std::runtime_error("opening hub buffer size");
        for (const auto p : fallback) {
            controller.boundary();
            std::array<champion_type, 2> bodies{opening::default_body(p), champion_type::issac};
            const std::size_t count = p == pick::cassi_and_issac ? 2 : 1;
            Edge best; best.body = bodies[0];
            for (std::size_t b = 0; b < count; ++b) {
                const auto& cap = capabilities[static_cast<std::size_t>(bodies[b])];
                if (!cap.known) continue;
                double travel = walk(job.approach, cap.speed) + walk(1, cap.speed), work = 0;
                bool reachable = true;
                for (std::size_t i = 0; i < job.count; ++i) {
                    int nearest = distance_field::unreachable;
                    const auto target = job.members[i].position;
                    for (int dy = -cap.range; dy <= cap.range; ++dy) for (int dx = -cap.range; dx <= cap.range; ++dx) {
                        const vec2 contact{target.x + dx, target.y + dy};
                        if (!inside(contact, w, h) || blocked(blocked_cells, contact, w)) continue;
                        nearest = std::min<int>(nearest, distance(hub_distances, contact, w));
                    }
                    if (nearest == distance_field::unreachable) { reachable = false; break; }
                    // Declared star itinerary: hub->contact_i->hub for EVERY
                    // member, then final adjacent hub->same receipt. Nominal
                    // geometry/basic turns only; not a safe action sequence.
                    travel += 2 * walk(nearest, cap.speed);
                    work += std::ceil(job.members[i].hp / cap.strength);
                }
                if (!reachable || !nonnegative(travel) || !positive(work)) continue;
                const double rate = nominal_rate(job, cap.speed, travel, work, rate_scratch);
                if (positive(rate) && (rate > best.rate || (rate == best.rate
                        && static_cast<int>(cap.body) < static_cast<int>(best.body)))) best = {rate, cap.body};
            }
            result.edges[static_cast<std::size_t>(p)][j] = best;
        }
    }
    controller.boundary();
    return result;
}

struct Beam { opening::Plan plan; std::uint16_t used = 0; double rate = 0; };
inline bool better(const Beam& a, const Beam& b) {
    if (a.rate != b.rate) return a.rate > b.rate;
    for (std::size_t i = 0; i < a.plan.accepted; ++i) {
        const auto& x = a.plan.entries[i]; const auto& y = b.plan.entries[i];
        if (x.pick != y.pick) return rank(x.pick) < rank(y.pick);
        if (x.assigned != y.assigned) return x.assigned;
        if (x.group != y.group) return x.group < y.group;
        if (x.body != y.body) return static_cast<int>(x.body) < static_cast<int>(y.body);
    }
    return false;
}
inline void search(Controller& controller, const Portfolio& portfolio) {
    std::vector<Beam> beam(1); beam[0].plan.slots = controller.accepted.slots;
    for (std::size_t depth = 0; depth < controller.accepted.slots; ++depth) {
        controller.boundary();
        std::vector<Beam> children; children.reserve(4 * 12 * 9);
        for (const auto& state : beam) for (const auto p : fallback) {
            if (depth < controller.accepted.accepted && p != controller.accepted.entries[depth].pick) continue;
            bool used = false;
            for (std::size_t i = 0; i < depth; ++i) if (state.plan.entries[i].pick == p) used = true;
            if (used) continue;
            auto add = [&](int job) {
                Beam child = state;
                auto& entry = child.plan.entries[depth]; entry = {p, opening::default_body(p), 0, false};
                if (job >= 0) {
                    const auto& edge = portfolio.edges[static_cast<std::size_t>(p)][job];
                    if (!positive(edge.rate)) return;
                    entry.body = edge.body; entry.group = portfolio.jobs[job].id; entry.assigned = true;
                    child.used |= std::uint16_t(1u << job); child.rate += edge.rate;
                }
                if (!nonnegative(child.rate)) return;
                child.plan.accepted = static_cast<std::uint8_t>(depth + 1); children.push_back(std::move(child));
            };
            add(-1); // A zero-job extension always permits a complete unique roster.
            for (std::size_t j = 0; j < portfolio.jobs.size(); ++j)
                if (!(state.used & (1u << j))) add(static_cast<int>(j));
        }
        std::sort(children.begin(), children.end(), better);
        if (children.empty()) throw std::runtime_error("opening empty beam");
        if (children.size() > 4) children.resize(4);
        beam = std::move(children);
        controller.boundary();
    }
    if (!controller.same_prefix(beam[0].plan)) throw std::runtime_error("opening stale prefix");
    controller.proposed = beam[0].plan; controller.ready = true;
    for (std::size_t i = 0; i < controller.accepted.accepted; ++i)
        controller.accepted.entries[i] = controller.proposed.entries[i];
    controller.dirty = true; controller.publish();
}

inline int run() {
    Controller controller;
    bool extraction_attempted = false;
    while (true) {
        controller.checkpoint(); // Accepted fallback first, before costly work.
        if (!extraction_attempted && controller.optional && controller.accepted.accepted > 0
            && controller.accepted.accepted < controller.accepted.slots) {
            extraction_attempted = true; // Never restart a full scan on later inputs.
            try {
                controller.starting_fuel = fuel_remaining();
                controller.boundary();
                const auto portfolio = extract(controller);
                search(controller, portfolio);
            } catch (...) { controller.disable(); controller.publish(); }
        }
        // Recheck after optional work/resume before deliberate yield. This is
        // best-effort, not a proven scheduler or an atomic-call fuel guarantee.
        controller.checkpoint(); controller.publish();
        end_turn();
    }
}
} // namespace qualifier_draft
