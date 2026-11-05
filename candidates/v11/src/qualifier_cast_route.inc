// Optional current-cast-envelope route advice. No physical field/ETA mutation.
enum class QualifierCastRouteFallback { None, Ordinary, InvalidFrame, TopologyUnknown, ResourceBound,
    AtGoal, NoHazards, NoRoots, Suppressed, Budget, Unreached, Detour, NoCompatibleGradient, QueryError };
struct QualifierCastRouteView {
    std::uint64_t generation = 0, frame_serial = 0, revision = 0;
    std::uint32_t actor = 0, cargo = 0;
    int round = -1, width = 0, height = 0;
    vec2 origin{};
    QualifierMovePurpose purpose = QualifierMovePurpose::Ordinary;
    bool admissible = false, retained_intent = false, witness_complete = false;
    bool neighborhood_complete = false, search_truncated = false, witness_unknown = false;
    bool durable_suppressed = false;
    int original_distance = UNREACH, avoid_distance = UNREACH, search_depth = 0;
    int witness_points = 0;
    double path_reference = 0, detour_budget = 0, extra_distance = 0, health = 0, K = 0;
    QualifierCastRouteFallback fallback = QualifierCastRouteFallback::InvalidFrame;
    bitboard hazards, navigation_blockers, reachable, physical_roots, physical_blockers;
    std::vector<std::uint16_t> base, avoid;
};
namespace qualifier_cast_route_detail {
using Frame = qualifier_enemy_advice_detail::Frame;
using Enemy = qualifier_enemy_advice_detail::Enemy;
using Slot = qualifier_enemy_advice_detail::Slot;
struct Job {
    std::uint32_t actor = 0, cargo = 0, target_full = 0, committed_full = 0;
    int target = -1, committed = -1;
    QualifierMovePurpose purpose = QualifierMovePurpose::Ordinary;
    double K = 0;
    bitboard declared;
};
struct Environment {
    Frame frame;
    std::vector<QualifierNpc> npcs;
    bool unresolved = false;
    int speed = 0;
    bitboard blockers, roots, hazards;
};
// Failure availability is distinct from a current executable frame. All cached
// camp descriptors are keyed by full identity, never by own-origin proximity.
struct NpcFact {
    std::uint32_t group = 0, camp = 0;
    vec2 position{};
    bool design_known = false, group_known = false, health_known = false;
    bool state_known = false, awake = false, combat_known = false;
    int group_state = -1, design_range = 0, combat_range = 0;
    double maxhp = 0, design_strength = 0, health = 0, combat_strength = 0;
};
struct FailureEnvironment {
    Frame frame; // Sorted enemies; own origin is ignored ONLY by failure comparison.
    std::vector<NpcFact> npcs;
    int width = 0, height = 0, speed = 0;
    bitboard blockers, roots, hazards;
};
struct FailureKey {
    std::uint32_t actor = 0, cargo = 0, target_full = 0, committed_full = 0;
    int target = -1, committed = -1;
    QualifierMovePurpose purpose = QualifierMovePurpose::Ordinary;
    double K = 0;
};
struct Intent { Job job; double budget = 0; };
struct Failure { Job job; FailureEnvironment environment; };
struct Runtime {
    std::optional<QualifierCastRouteView> view;
    std::optional<Job> job;
    std::optional<Environment> environment;
    std::optional<Intent> intent;
    std::optional<Failure> failure;
    std::optional<FailureKey> failure_latch; // Primitive, nonallocating throw fallback.
    std::optional<FailureEnvironment> failure_environment;
    std::vector<NpcFact> npc_knowledge;
    std::optional<distance_field> field;
    std::uint32_t budget_actor = 0;
    int budget_round = -1, builds = 0;
    std::uint64_t generation = 0;
};
// No map-sized object is constructed before the SDK has initialized dimensions.
std::optional<Runtime> runtime;
bool finite(double x) { return std::isfinite(x) && x >= 0; }
bool same_job(const Job& a, const Job& b) {
    return a.actor == b.actor && a.cargo == b.cargo && a.target == b.target && a.committed == b.committed
        && a.target_full == b.target_full && a.committed_full == b.committed_full
        && a.purpose == b.purpose && a.K == b.K && a.declared == b.declared;
}
bool same_slot(const Slot& a, const Slot& b) {
    if (a.metadata_known != b.metadata_known) return false;
    if (!a.metadata_known) return true;
    return a.kind == b.kind && a.target_enemy == b.target_enemy && a.target_ground == b.target_ground
        && a.exclusive == b.exclusive && a.damage_known == b.damage_known && a.raw_known == b.raw_known
        && a.damage_kind == b.damage_kind && a.control == b.control && a.range == b.range && a.radius == b.radius
        && (!a.damage_known || a.reference == b.reference) && (!a.raw_known || a.raw == b.raw);
}
bool same_failure_frame(const Frame& a, const Frame& b) {
    if (a.actor != b.actor || a.health != b.health || a.omitted != b.omitted
        || a.stun_immune != b.stun_immune || a.count != b.count || a.defenses.has_value() != b.defenses.has_value()) return false;
    if (a.defenses && !qualifier_enemy_advice_detail::same_defenses(*a.defenses, *b.defenses)) return false;
    for (std::size_t i = 0; i < a.count; ++i) {
        const auto& x = a.enemies[i]; const auto& y = b.enemies[i];
        if (x.id != y.id || x.position != y.position || x.strength != y.strength
            || x.move_speed != y.move_speed || x.basic_range != y.basic_range) return false;
        for (int slot = 0; slot < 2; ++slot) if (!same_slot(x.slots[slot], y.slots[slot])) return false;
    }
    return true;
}
bool same_frame_environment(const Frame& a, const Frame& b) {
    return a.origin == b.origin && same_failure_frame(a, b);
}
bool same_npcs(const std::vector<QualifierNpc>& a, const std::vector<QualifierNpc>& b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i)
        if (a[i].pos != b[i].pos || a[i].strength != b[i].strength || a[i].range != b[i].range
            || a[i].knowledge != b[i].knowledge || a[i].group_index != b[i].group_index) return false;
    return true;
}
FailureKey failure_key(const Job& j) {
    return {j.actor, j.cargo, j.target_full, j.committed_full, j.target, j.committed, j.purpose, j.K};
}
bool same_failure_key(const FailureKey& a, const Job& b) {
    return a.actor == b.actor && a.cargo == b.cargo && a.target_full == b.target_full
        && a.committed_full == b.committed_full && a.target == b.target && a.committed == b.committed
        && a.purpose == b.purpose && a.K == b.K;
}
bool npc_less(const NpcFact& a, const NpcFact& b) {
    return a.group < b.group || (a.group == b.group && a.camp < b.camp);
}
bool same_fact(const NpcFact& a, const NpcFact& b) {
    return a.group == b.group && a.camp == b.camp && a.position == b.position
        && a.design_known == b.design_known && a.group_known == b.group_known
        && a.health_known == b.health_known && a.state_known == b.state_known
        && a.awake == b.awake && a.combat_known == b.combat_known
        && a.group_state == b.group_state && a.design_range == b.design_range
        && a.combat_range == b.combat_range && a.maxhp == b.maxhp
        && a.design_strength == b.design_strength && a.health == b.health
        && a.combat_strength == b.combat_strength;
}
bool collect_failure_npcs(Runtime& rt, std::vector<NpcFact>& result) {
    std::size_t count = 0;
    for (const auto& group : S->groups) {
        if (group.camps.size() > 4096 - count) return false;
        count += group.camps.size();
    }
    result.reserve(count);
    for (const auto& group : S->groups) for (const auto& camp : group.camps) {
        NpcFact fact;
        fact.group = static_cast<std::uint32_t>(static_cast<handle_camp_group_t>(group.h));
        fact.camp = static_cast<std::uint32_t>(static_cast<handle_camp_t>(camp.h));
        fact.position = camp.pos; fact.design_known = camp.design_known;
        if (fact.design_known) {
            fact.maxhp = camp.maxhp; fact.design_strength = camp.str; fact.design_range = camp.range;
            if (!finite(fact.maxhp) || !finite(fact.design_strength) || fact.design_range < 0) return false;
        }
        fact.group_known = group.last_observed_round >= 0;
        if (fact.group_known) fact.group_state = group.observed_state;
        fact.health_known = camp.last_observed_round >= 0;
        if (fact.health_known) { fact.health = camp.hp; if (!finite(fact.health)) return false; }
        const auto old = std::lower_bound(rt.npc_knowledge.begin(), rt.npc_knowledge.end(), fact, npc_less);
        const bool prior = old != rt.npc_knowledge.end() && old->group == fact.group && old->camp == fact.camp;
        // Visibility loss/current-valid flags and sample dates are not new facts.
        // These sticky bits describe last-known knowledge ONLY, never current safety.
        fact.state_known = camp.state_observed_round >= 0 || fact.health_known || (prior && old->state_known);
        if (fact.state_known) fact.awake = camp.observed_awake;
        fact.combat_known = camp.combat_observed_round >= 0 || (prior && old->combat_known);
        if (fact.combat_known) {
            fact.combat_strength = camp.observed_strength; fact.combat_range = camp.observed_attack_range;
            if (!finite(fact.combat_strength) || fact.combat_range < 0) return false;
        }
        result.push_back(fact);
    }
    std::sort(result.begin(), result.end(), npc_less);
    for (std::size_t i = 1; i < result.size(); ++i)
        if (!npc_less(result[i - 1], result[i])) return false;
    rt.npc_knowledge = result;
    return true;
}
bool make_failure_environment(Runtime& rt, const Frame& frame, int speed,
    const bitboard& hazards, FailureEnvironment& result) {
    result.frame = frame;
    std::sort(result.frame.enemies.begin(), result.frame.enemies.begin() + result.frame.count,
        [](const Enemy& a, const Enemy& b) { return a.id < b.id; });
    for (std::size_t i = 1; i < result.frame.count; ++i)
        if (result.frame.enemies[i - 1].id == result.frame.enemies[i].id) return false;
    result.width = S->W; result.height = S->H; result.speed = speed;
    result.blockers = S->occ; result.roots = S->goals; result.hazards = hazards;
    return collect_failure_npcs(rt, result.npcs);
}
bool same_failure_environment(const FailureEnvironment& a, const FailureEnvironment& b) {
    if (a.width != b.width || a.height != b.height || a.speed != b.speed
        || !(a.blockers == b.blockers) || !(a.roots == b.roots) || !(a.hazards == b.hazards)
        || !same_failure_frame(a.frame, b.frame) || a.npcs.size() != b.npcs.size()) return false;
    for (std::size_t i = 0; i < a.npcs.size(); ++i) if (!same_fact(a.npcs[i], b.npcs[i])) return false;
    return true;
}
bool prior_failed_casters_visible(const Frame& failed, const Frame& current) {
    // Own movement can hide an unchanged caster behind a wall. Missing current
    // visibility is unknown, so it cannot release a rejected route by itself.
    for (std::size_t i = 0; i < failed.count; ++i) {
        bool found = false;
        for (std::size_t j = 0; j < current.count; ++j)
            if (failed.enemies[i].id == current.enemies[j].id) { found = true; break; }
        if (!found) return false;
    }
    return true;
}
bitboard known_hazards(const Frame& frame) {
    auto known = frame;
    for (std::size_t i = 0; i < known.count; ++i) for (auto& slot : known.enemies[i].slots)
        if (!slot.metadata_known) slot.damage_known = false;
    return qualifier_enemy_advice_detail::positive_envelopes(known);
}
Job job(QualifierMovePurpose purpose, double K, const Frame& frame, const bitboard& declared) {
    Job result;
    result.actor = frame.actor; result.cargo = self().gem_count(); result.purpose = purpose; result.K = K;
    result.declared = declared;
    result.target = S->target; result.committed = S->qualifier_committed_target;
    if (result.target >= 0 && result.target < static_cast<int>(S->groups.size()))
        result.target_full = static_cast<std::uint32_t>(static_cast<handle_camp_group_t>(S->groups[result.target].h));
    if (result.committed >= 0 && result.committed < static_cast<int>(S->groups.size()))
        result.committed_full = static_cast<std::uint32_t>(static_cast<handle_camp_group_t>(S->groups[result.committed].h));
    return result;
}
bool in_bounds(vec2 p, int width, int height) { return p.x >= 0 && p.y >= 0 && p.x < width && p.y < height; }
std::size_t index(vec2 p, int width) { return static_cast<std::size_t>(p.y) * width + p.x; }
std::vector<std::uint8_t> members(const bitboard& board, int width, int height) {
    std::vector<std::uint8_t> result(static_cast<std::size_t>(width) * height, 0);
    for (const auto p : board.positions()) {
        if (!in_bounds(p, width, height)) throw std::runtime_error("route mask coordinate");
        result[index(p, width)] = 1;
    }
    return result;
}
void trace(QualifierCastRouteView& view, const Frame& frame, const std::vector<std::uint8_t>& walls,
    const std::vector<std::uint8_t>& roots) {
    auto p = view.origin;
    for (int point = 0; point < 512; ++point) {
        ++view.witness_points;
        // project() and all per-point/index/neighbor work are pure guest code.
        const auto advice = qualifier_enemy_advice_detail::project(frame, p);
        view.path_reference = std::max(view.path_reference, advice.cast_reference);
        view.witness_unknown = view.witness_unknown || advice.unknown > 0 || advice.raw_unknown > 0 || advice.controls > 0;
        const auto current = view.base[index(p, view.width)];
        if (current == 0) { view.witness_complete = roots[index(p, view.width)] != 0; return; }
        if (current == UNREACH) return;
        bool found = false; vec2 next{};
        for (int dy = -1; dy <= 1; ++dy) for (int dx = -1; dx <= 1; ++dx) {
            const vec2 q{p.x + dx, p.y + dy};
            if ((dx == 0 && dy == 0) || !in_bounds(q, view.width, view.height)) continue;
            const auto at = index(q, view.width);
            if (walls[at] || view.base[at] == UNREACH || int(view.base[at]) + 1 != current) continue;
            if (!found || q.y < next.y || (q.y == next.y && q.x < next.x)) { found = true; next = q; }
        }
        if (!found) return;
        p = next;
    }
}
} // namespace qualifier_cast_route_detail

bool qualifier_cast_route_reserve_field() noexcept {
    namespace d = qualifier_cast_route_detail;
    try {
        if (!S || !qualifier_current() || fuel_remaining() < 24000) return false;
        const auto cells = std::int64_t(S->W) * S->H;
        if (S->W <= 0 || S->H <= 0 || cells <= 0 || cells > 10000) return false;
        if (!d::runtime) d::runtime.emplace();
        auto& rt = *d::runtime;
        const auto actor = static_cast<std::uint32_t>(static_cast<handle_champion_t>(self()));
        if (rt.budget_actor != actor || rt.budget_round != S->round) {
            rt.budget_actor = actor; rt.budget_round = S->round; rt.builds = 0;
        }
        if (rt.builds >= 2) return false;
        ++rt.builds; // Reserve before any fallible field work, including Root dash work.
        return true;
    } catch (...) { return false; }
}
void qualifier_cast_route_clear() noexcept {
    if (!qualifier_cast_route_detail::runtime) return;
    auto& rt = *qualifier_cast_route_detail::runtime;
    rt.intent.reset(); rt.view.reset(); rt.job.reset(); rt.environment.reset(); rt.failure_environment.reset();
    // Ordinary fallback cannot erase durable failed-job suppression or its
    // primitive exception latch. A changed validated job/environment may do so.
    // Field budget and origin-independent last-known NPC facts also survive.
}

const QualifierCastRouteView* qualifier_cast_route_prepare(QualifierMovePurpose purpose, double K,
    const qualifier_enemy_advice_detail::Frame& frame, const bitboard& declared_goals) noexcept {
    namespace d = qualifier_cast_route_detail;
    try {
        if (purpose == QualifierMovePurpose::Ordinary) { qualifier_cast_route_clear(); return nullptr; }
        if (!S || !qualifier_current()) return nullptr;
        const auto cells = std::int64_t(S->W) * S->H;
        if (S->W <= 0 || S->H <= 0 || cells <= 0 || cells > 10000) return nullptr;
        if (!d::runtime) d::runtime.emplace();
        auto& rt = *d::runtime;
        const auto current_job = d::job(purpose, K, frame, declared_goals);
        if (rt.intent && !d::same_job(rt.intent->job, current_job)) rt.intent.reset();
        if (rt.failure_latch && !d::same_failure_key(*rt.failure_latch, current_job)) {
            rt.failure.reset(); rt.failure_latch.reset();
        }
        if (rt.failure && !d::same_job(rt.failure->job, current_job)) {
            rt.failure.reset(); rt.failure_latch.reset();
        }
        const bool retained = rt.intent.has_value();
        // serial is deliberately exact even when observation revision is unchanged.
        if (rt.view && rt.job && rt.environment && d::same_job(*rt.job, current_job)
            && rt.view->round == S->round && rt.view->revision == frame.revision && rt.view->frame_serial == frame.serial
            && rt.environment->blockers == S->occ && rt.environment->roots == S->goals
            && rt.environment->unresolved == qualifier_runtime.unresolved_awake_npc
            && d::same_npcs(rt.environment->npcs, qualifier_runtime.npcs)
            && d::same_frame_environment(rt.environment->frame, frame)
            && qualifier_step_physical_topology_valid && qualifier_enemy_advice_detail::matching(frame)) return &*rt.view;
        rt.view.emplace(); auto& view = *rt.view;
        view.generation = ++rt.generation; view.frame_serial = frame.serial; view.revision = frame.revision;
        view.actor = frame.actor; view.round = S->round; view.width = S->W; view.height = S->H;
        view.origin = frame.origin; view.health = frame.health; view.K = K; view.purpose = purpose;
        view.cargo = current_job.cargo; view.retained_intent = retained;
        view.durable_suppressed = rt.failure_latch.has_value();
        rt.job = current_job; rt.environment.reset(); rt.failure_environment.reset();
        if (!qualifier_step_physical_topology_valid) { view.fallback = QualifierCastRouteFallback::TopologyUnknown; return &view; }
        if (!d::finite(K) || !d::finite(frame.health) || frame.health <= 0
            || frame.count > frame.enemies.size()
            || !qualifier_enemy_advice_detail::matching(frame) || !d::in_bounds(frame.origin, S->W, S->H)) return &view;
        if (qualifier_runtime.npcs.size() > 256) { view.fallback = QualifierCastRouteFallback::ResourceBound; return &view; }
        for (const auto& npc : qualifier_runtime.npcs) if (!d::finite(npc.strength) || npc.range < 0) {
            view.fallback = QualifierCastRouteFallback::InvalidFrame; return &view;
        }
        view.physical_roots = S->goals; view.physical_blockers = S->occ;
        if (view.physical_roots.is_empty()) { view.fallback = QualifierCastRouteFallback::NoRoots; return &view; }
        if (view.physical_roots.is_set(view.origin)) {
            rt.intent.reset(); view.retained_intent = false;
            view.fallback = QualifierCastRouteFallback::AtGoal; return &view;
        }
        const int speed = self().base_stats().base_move_speed;
        if (speed <= 0 || speed >= UNREACH) { view.fallback = QualifierCastRouteFallback::ResourceBound; return &view; }
        double maximum = 0;
        for (std::size_t i = 0; i < frame.count; ++i) for (const auto& slot : frame.enemies[i].slots)
            if (slot.metadata_known && slot.damage_kind && slot.damage_known && d::finite(slot.reference)) maximum = std::max(maximum, slot.reference);
        if (maximum == 0 && !retained) { view.fallback = QualifierCastRouteFallback::NoHazards; return &view; }
        // The same validated failed job cannot reopen while a prior caster is
        // missing. No canonical NPC/topology snapshot can change that result.
        if (rt.failure_latch && rt.failure
            && !d::prior_failed_casters_visible(rt.failure->environment.frame, frame)) {
            view.durable_suppressed = true;
            view.fallback = QualifierCastRouteFallback::Suppressed; return &view;
        }
        if (fuel_remaining() < 24000) { view.fallback = QualifierCastRouteFallback::Budget; return &view; }
        view.hazards = d::known_hazards(frame);
        d::Environment environment;
        environment.frame = frame; environment.speed = speed;
        environment.npcs = qualifier_runtime.npcs; environment.unresolved = qualifier_runtime.unresolved_awake_npc;
        environment.blockers = view.physical_blockers; environment.roots = view.physical_roots; environment.hazards = view.hazards;
        rt.environment = std::move(environment);
        d::FailureEnvironment durable;
        if (!d::make_failure_environment(rt, frame, speed, view.hazards, durable)) {
            view.fallback = QualifierCastRouteFallback::ResourceBound; return &view;
        }
        rt.failure_environment = std::move(durable);
        if (rt.failure_latch && (!rt.failure
            || !d::prior_failed_casters_visible(rt.failure->environment.frame, rt.failure_environment->frame)
            || d::same_failure_environment(rt.failure->environment, *rt.failure_environment))) {
            view.durable_suppressed = true;
            view.fallback = QualifierCastRouteFallback::Suppressed; return &view;
        }
        rt.failure.reset(); rt.failure_latch.reset(); view.durable_suppressed = false;
        if (!qualifier_cast_route_reserve_field()) { view.fallback = QualifierCastRouteFallback::Budget; return &view; }
        view.base = S->df_go.export_linear_buffer();
        if (view.base.size() != static_cast<std::size_t>(cells)) throw std::runtime_error("route base buffer size");
        const auto at = d::index(view.origin, view.width);
        view.original_distance = view.base[at];
        if (view.original_distance == UNREACH || view.original_distance <= 0) {
            view.fallback = QualifierCastRouteFallback::Unreached; return &view;
        }
        const auto walls = d::members(view.physical_blockers, view.width, view.height);
        const auto roots = d::members(view.physical_roots, view.width, view.height);
        d::trace(view, frame, walls, roots);
        const double price = view.witness_complete ? K * view.path_reference / std::max(1.0, frame.health) : 0.0;
        const double capacity = retained ? rt.intent->budget : K * maximum / std::max(1.0, frame.health);
        if (!d::finite(price) || !d::finite(capacity)) throw std::runtime_error("route price");
        view.detour_budget = retained ? rt.intent->budget : price;
        auto safe_roots = view.physical_roots & ~view.hazards;
        if (safe_roots.is_empty()) { view.fallback = QualifierCastRouteFallback::NoRoots; return &view; }
        view.navigation_blockers = view.physical_blockers | view.hazards;
        view.navigation_blockers.set(view.origin, false); // Origin-only exit exception.
        const double depth = double(view.original_distance) + std::ceil(capacity) + speed;
        if (!d::finite(depth)) throw std::runtime_error("route depth");
        // A simple shortest path on cells vertices has at most cells-1 edges.
        view.search_depth = static_cast<int>(std::min(double(cells - 1), depth));
        if (view.search_depth < 0 || view.search_depth >= UNREACH) throw std::runtime_error("route depth range");
        if (!rt.field) rt.field.emplace();
        bitboard visited;
        rt.field->compute(view.navigation_blockers, safe_roots, visited, static_cast<std::uint16_t>(view.search_depth));
        view.avoid = rt.field->export_linear_buffer();
        if (view.avoid.size() != static_cast<std::size_t>(cells)) throw std::runtime_error("route avoid buffer size");
        view.avoid_distance = view.avoid[at];
        if (view.avoid_distance == UNREACH) {
            view.search_truncated = view.search_depth < cells - 1;
            view.fallback = QualifierCastRouteFallback::Unreached; return &view;
        }
        view.extra_distance = double(view.avoid_distance) - view.original_distance;
        if (view.extra_distance < 0 || (view.extra_distance > 0 && ((!retained && !view.witness_complete)
            || view.extra_distance > view.detour_budget))) {
            view.fallback = QualifierCastRouteFallback::Detour; return &view;
        }
        view.neighborhood_complete = view.search_depth >= cells - 1
            || double(view.avoid_distance) + speed <= view.search_depth;
        if (!view.neighborhood_complete) { view.search_truncated = true; view.fallback = QualifierCastRouteFallback::Unreached; return &view; }
        view.reachable = view.navigation_blockers.reachable_tiles(view.origin, static_cast<std::uint16_t>(speed));
        view.reachable.set(view.origin, false);
        if (!qualifier_current() || !qualifier_step_physical_topology_valid || !(view.physical_blockers == S->occ)
            || !(view.physical_roots == S->goals) || !qualifier_enemy_advice_detail::matching(frame)) return &view;
        view.admissible = true; view.fallback = QualifierCastRouteFallback::None;
        return &view;
    } catch (...) {
        if (d::runtime && d::runtime->view) {
            auto& view = *d::runtime->view;
            view.admissible = false; view.fallback = QualifierCastRouteFallback::QueryError;
            return &view;
        }
        return nullptr;
    }
}

bool qualifier_cast_route_commit(std::uint64_t generation, vec2 destination) noexcept {
    namespace d = qualifier_cast_route_detail;
    try {
        if (!d::runtime || !S || !qualifier_current()) return false;
        auto& rt = *d::runtime;
        if (!rt.view || !rt.job || !rt.environment) return false;
        const auto& view = *rt.view;
        const auto& frame = qualifier_enemy_advice_detail::runtime.frame;
        if (!view.admissible || generation != view.generation || frame.serial != view.frame_serial
            || view.round != S->round || view.revision != qualifier_runtime.observation_revision
            || !qualifier_enemy_advice_detail::matching(frame) || !qualifier_step_physical_topology_valid
            || !(view.physical_blockers == S->occ) || !(view.physical_roots == S->goals)
            || !d::in_bounds(destination, view.width, view.height) || !view.reachable.is_set(destination)
            || self().gem_count() != view.cargo) return false;
        const auto next = view.avoid[d::index(destination, view.width)];
        if (next == UNREACH || next >= view.avoid_distance) return false;
        if (!rt.intent) rt.intent = d::Intent{*rt.job, view.detour_budget};
        return true; // Semantic intent only, no accepted-move or ETA claim.
    } catch (...) { return false; }
}
void qualifier_cast_route_fail(std::uint64_t generation) noexcept {
    namespace d = qualifier_cast_route_detail;
    try {
        if (!d::runtime) return;
        auto& rt = *d::runtime;
        if (!rt.view || generation != rt.view->generation || !rt.job || !rt.environment) return;
        // Latch before any fallible query/copy. Uncertain validation cannot
        // reopen spell steering for the same primitive job.
        rt.failure_latch = d::failure_key(*rt.job);
        rt.view->durable_suppressed = true;
        const auto& frame = qualifier_enemy_advice_detail::runtime.frame;
        if (!S || !qualifier_current() || frame.serial != rt.view->frame_serial
            || rt.view->revision != qualifier_runtime.observation_revision
            || !qualifier_step_physical_topology_valid || !(rt.environment->blockers == S->occ)
            || !(rt.environment->roots == S->goals)
            || rt.environment->unresolved != qualifier_runtime.unresolved_awake_npc
            || !d::same_npcs(rt.environment->npcs, qualifier_runtime.npcs)
            || !d::same_frame_environment(rt.environment->frame, frame)
            || !qualifier_enemy_advice_detail::matching(frame)) {
            rt.view->admissible = false; rt.intent.reset(); return;
        }
        if (!rt.failure_environment) throw std::runtime_error("route failure snapshot missing");
        d::Failure failed{*rt.job, *rt.failure_environment};
        rt.failure = std::move(failed); rt.intent.reset();
        rt.view->durable_suppressed = true;
        rt.view->admissible = false; rt.view->fallback = QualifierCastRouteFallback::NoCompatibleGradient;
    } catch (...) {
        // The caller may still hold view for its ordinary fallback pass.
        // Preserve that storage even if suppression snapshotting failed.
        if (d::runtime) {
            auto& rt = *d::runtime;
            rt.intent.reset();
            if (rt.view) {
                rt.view->admissible = false;
                rt.view->durable_suppressed = rt.failure_latch.has_value();
                rt.view->fallback = QualifierCastRouteFallback::QueryError;
            }
        }
    }
}

// A non-walking caller (notably defensive dash) must honor the same durable
// availability gate. No df_go read; unknown current topology remains suppressed.
bool qualifier_cast_route_spell_steering_allowed(QualifierMovePurpose purpose, double K,
    const qualifier_enemy_advice_detail::Frame& frame, const bitboard& declared_goals) noexcept {
    namespace d = qualifier_cast_route_detail;
    try {
        if (!d::runtime || (!d::runtime->failure && !d::runtime->failure_latch)) return true;
        auto& rt = *d::runtime;
        if (!S || !qualifier_current() || !d::finite(K) || frame.count > frame.enemies.size()
            || !qualifier_enemy_advice_detail::matching(frame)) return false;
        const auto current_job = d::job(purpose, K, frame, declared_goals);
        if ((rt.failure_latch && !d::same_failure_key(*rt.failure_latch, current_job))
            || (rt.failure && !d::same_job(rt.failure->job, current_job))) {
            rt.failure.reset(); rt.failure_latch.reset(); return true;
        }
        if (!rt.failure || !d::prior_failed_casters_visible(rt.failure->environment.frame, frame)) return false;
        if (!qualifier_step_physical_topology_valid || fuel_remaining() < 24000) return false;
        const int speed = self().base_stats().base_move_speed;
        if (speed <= 0 || speed >= UNREACH) return false;
        d::FailureEnvironment current;
        const auto hazards = d::known_hazards(frame);
        if (!d::make_failure_environment(rt, frame, speed, hazards, current)) return false;
        if (!d::prior_failed_casters_visible(rt.failure->environment.frame, current.frame)) return false;
        if (d::same_failure_environment(rt.failure->environment, current)) return false;
        rt.failure.reset(); rt.failure_latch.reset(); return true;
    } catch (...) { return false; }
}
