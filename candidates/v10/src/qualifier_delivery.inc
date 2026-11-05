// Actor-local, resource-bound delivery. Included after qualifier_publish().
// Static/service ETAs are conditional on later occupancy and channel success.
enum class QualifierDeliveryStatus { Found, Unknown, Unavailable };
enum class QualifierDeliveryKind { Walk, Crystal, Recall };
enum class QualifierDeliveryResult { NoAction, Acted, Pending, Complete, Invalidated, Unknown };

struct QualifierDeliveryPlan {
    QualifierDeliveryStatus status = QualifierDeliveryStatus::Unknown;
    QualifierDeliveryKind kind = QualifierDeliveryKind::Walk;
    std::uint32_t actor = 0;
    int observed_round = -1;
    std::uint64_t revision = 0;
    vec2 origin{}, altar{}, portal{}, landing{};
    double cargo = 0, eta_lower = 0, eta_upper = 0;
    int walk_before_steps = 0, walk_after_steps = 0, lock_hits_upper = 0;
    std::uint32_t crystals_observed = 0, crystals_required = 0, crystal_cost = 0;
    bool resource_backed = false, purchase_required = false;
    bool hypothetical_origin = false, search_complete = false, channel_safety_known = false;
    bool found() const { return status == QualifierDeliveryStatus::Found; }
};

namespace qualifier_delivery_detail {
constexpr int max_altars = 16, max_origins = 5;
constexpr int crystal_range = 3; // Released rules and SDK consumable contract.
struct Altar {
    vec2 tile{};
    std::vector<vec2> stances;
    std::unique_ptr<distance_field> field;
    std::vector<std::uint16_t> distances;
};
struct Origin {
    struct Portal { bool known = false; vec2 from{}, to{}; int before = 0, after = 0; };
    vec2 tile{};
    int used_round = -1;
    std::unique_ptr<distance_field> field;
    std::vector<std::uint16_t> distances;
    std::array<std::size_t, max_altars> cursor{};
    std::array<int, max_altars> offset{};
    std::array<Portal, max_altars> best{};
    int next_altar = 0;
};
enum class Pending { None, Crystal, Recall, Tribute };
struct Runtime {
    struct Trace {
        vec2 origin{}, cursor{};
        std::uint64_t generation = 0;
        int used_round = -1;
        std::size_t altar_cursor = 0;
        bool actual = false, at_root = false;
    };
    std::uint64_t revision = 1;
    std::optional<QualifierDeliveryPlan> active;
    Pending pending = Pending::None;
    vec2 channel_origin{}, channel_destination{};
    std::uint32_t channel_cargo = 0;
    bool channel_owned = false;
    int last_round = -1;
    std::uint32_t actor = 0, cargo = 0, crystals = 0;
    vec2 position{};
    bool alive = false;
    int width = 0, height = 0, budget_round = -1;
    int fields_left = 0, portal_checks_left = 0, scans_left = 0, trace_steps_left = 0;
    bool topology_ready = false, all_altars_included = false;
    std::vector<Altar> altars;
    std::vector<std::uint8_t> terrain;
    std::vector<std::uint16_t> union_static, union_current, committed_distances;
    std::optional<vec2> committed_portal;
    std::vector<std::uint8_t> union_roots, union_blockers;
    std::size_t root_cursor = 0;
    std::unique_ptr<distance_field> union_field;
    std::uint64_t union_generation = 1;
    std::array<Trace, max_origins> traces;
    std::array<Origin, max_origins> origins;
};
Runtime runtime;

bool finite_nonnegative(double x) { return std::isfinite(x) && x >= 0; }
bool inside(vec2 p) { return p.x >= 0 && p.y >= 0 && p.x < S->W && p.y < S->H; }
std::size_t index(vec2 p) { return static_cast<std::size_t>(p.y) * S->W + p.x; }
vec2 tile(std::size_t i) { return { static_cast<int>(i % S->W), static_cast<int>(i / S->W) }; }
bool blocked(vec2 p) {
    return !inside(p) || (runtime.terrain[index(p) / 8] & (std::uint8_t{1} << (index(p) % 8)));
}
int distance(const std::vector<std::uint16_t>& values, vec2 p) {
    return inside(p) && values.size() == static_cast<std::size_t>(S->W) * S->H ? values[index(p)] : UNREACH;
}
double turns(int steps, double speed) { return std::ceil(steps / speed); }
void budget() {
    if (runtime.budget_round == S->round) return;
    runtime.budget_round = S->round;
    runtime.fields_left = 6;
    runtime.portal_checks_left = 4096;
    runtime.scans_left = 2048;
    runtime.trace_steps_left = 512;
}
bool field_budget() {
    return qualifier_current() && runtime.fields_left > 0 && fuel_remaining() >= 30000;
}
bool topology() {
    budget();
    if (runtime.topology_ready) return true;
    if (fuel_remaining() < 30000 || S->W <= 0 || S->H <= 0
        || constants::submit_gems_range < 0 || constants::submit_gems_range > 8) return false;
    runtime.width = S->W; runtime.height = S->H;
    runtime.terrain = S->terrain.export_linear_buffer();
    const auto area = static_cast<std::uint64_t>(S->W) * S->H;
    if (runtime.terrain.size() < (area + 7) / 8) return false;
    runtime.union_static = S->df_altar.export_linear_buffer();
    if (runtime.union_static.size() != area) return false;
    runtime.union_roots.assign((area + 7) / 8, 0);
    runtime.root_cursor = 0;
    runtime.altars.clear();
    auto tiles = S->altars;
    std::sort(tiles.begin(), tiles.end(), [](vec2 a, vec2 b) { return a.y < b.y || (a.y == b.y && a.x < b.x); });
    runtime.all_altars_included = tiles.size() <= max_altars;
    for (int a = 0; a < static_cast<int>(tiles.size()) && a < max_altars; ++a) {
        Altar altar; altar.tile = tiles[a];
        for (int y = -constants::submit_gems_range; y <= constants::submit_gems_range; ++y)
            for (int x = -constants::submit_gems_range; x <= constants::submit_gems_range; ++x) {
                const vec2 p{altar.tile.x + x, altar.tile.y + y};
                if (!blocked(p)) altar.stances.push_back(p);
            }
        runtime.altars.push_back(std::move(altar));
    }
    runtime.topology_ready = qualifier_current();
    return runtime.topology_ready;
}
bool altar_field(Altar& altar) {
    if (altar.field) return true;
    if (altar.stances.empty() || !field_budget()) return false;
    bitboard roots, visited;
    for (auto p : altar.stances) roots.set(p);
    auto field = std::make_unique<distance_field>();
    --runtime.fields_left;
    field->compute(S->terrain, roots, visited);
    auto values = field->export_linear_buffer();
    if (!qualifier_current()) return false;
    altar.field = std::move(field); altar.distances = std::move(values);
    return true;
}
Origin* origin_field(vec2 p) {
    // Optional crystal analysis only. Walking/recall fields are not disabled
    // by this resource bound, and no map-rule limit is inferred from it.
    if (static_cast<std::uint64_t>(S->W) * S->H > 10000) return nullptr;
    for (auto& origin : runtime.origins) if (origin.field && origin.tile == p) {
        origin.used_round = S->round; return &origin;
    }
    if (!field_budget()) return nullptr;
    Origin* selected = nullptr;
    for (auto& origin : runtime.origins) {
        if (!origin.field) { selected = &origin; break; }
        if (origin.used_round != S->round && (!selected || origin.used_round < selected->used_round)) selected = &origin;
    }
    if (!selected) return nullptr;
    bitboard roots, visited; roots.set(p);
    Origin fresh; fresh.tile = p; fresh.used_round = S->round;
    fresh.field = std::make_unique<distance_field>();
    --runtime.fields_left;
    fresh.field->compute(S->terrain, roots, visited);
    fresh.distances = fresh.field->export_linear_buffer();
    if (!qualifier_current()) return nullptr;
    *selected = std::move(fresh);
    return selected;
}
double tribute_upper(double cargo) {
    const double base = constants::submit_gem_base_duration;
    const double multiplier = constants::submit_gem_duration_multiplier;
    const double minimum = constants::submit_gem_minimum_duration;
    if (!finite_nonnegative(base) || !finite_nonnegative(multiplier) || !finite_nonnegative(minimum)) return -1;
    // Completed Tribute can shorten this. Do not guess intermediate rounding.
    return std::max(minimum, base + std::ceil(cargo * multiplier));
}
int lock_upper(vec2 altar, const bitboard& visible, const std::vector<altar_lock>& locks) {
    std::uint64_t enemy_hits = 0;
    for (const auto& lock : locks)
        if (lock.position() == altar && lock.is_enemy()) enemy_hits += lock.hits_remaining();
    // Both teams' locks can coexist. An allied lock never hides an enemy one.
    if (enemy_hits > static_cast<std::uint64_t>(std::numeric_limits<int>::max()))
        throw std::runtime_error("unbounded delivery lock service");
    if (enemy_hits > 0) return static_cast<int>(enemy_hits);
    return visible.is_set(altar) ? 0 : static_cast<int>(constants::altar_lock_hits);
}
bool better(const QualifierDeliveryPlan& a, const QualifierDeliveryPlan& b) {
    if (!b.found()) return a.found();
    if (a.eta_upper != b.eta_upper) return a.eta_upper < b.eta_upper;
    if (a.crystal_cost != b.crystal_cost) return a.crystal_cost < b.crystal_cost;
    if (a.kind != b.kind) return static_cast<int>(a.kind) < static_cast<int>(b.kind);
    if (a.altar.y != b.altar.y) return a.altar.y < b.altar.y;
    if (a.altar.x != b.altar.x) return a.altar.x < b.altar.x;
    if (a.portal.y != b.portal.y) return a.portal.y < b.portal.y;
    if (a.portal.x != b.portal.x) return a.portal.x < b.portal.x;
    return a.landing.y < b.landing.y || (a.landing.y == b.landing.y && a.landing.x < b.landing.x);
}
bool recall_start_eligible(vec2 position) {
    return !map::spawn_region(team::ally).is_set(position)
        && qualifier_runtime.recent_damage <= 0 && qualifier_threat(position) <= 0
        && qualifier_unknown_npc_potential(position) <= 0 && !qualifier_runtime.unresolved_awake_npc
        && S->round >= qualifier_runtime.avoid_channel_until;
}
bool crystal_start_eligible(vec2 position) {
    try {
        if (self().restrictions().contains(buff_restrictions::consumable)) return false;
        const double exposure = qualifier_threat(position) * constants::tp_crystal_time;
        const double health = self().health();
        // An explicit current-exposure heuristic, never a future-cast or safety proof.
        return finite_nonnegative(health) && health > 0 && finite_nonnegative(exposure)
            && exposure < health && !qualifier_runtime.unresolved_awake_npc;
    } catch (const std::exception&) { return false; }
}
// Covers every altar, independently of the optional first16 portal cache.
// Work and path tracing resume per origin; budget exhaustion is not NoRoute.
bool union_walk(vec2 origin, bool actual, const std::vector<std::uint8_t>& occupied,
                vec2& altar, int& steps) {
    auto& rt = runtime;
    const std::size_t area = rt.union_static.size();
    int build = 512;
    while (rt.root_cursor < area && build-- > 0 && rt.scans_left > 0 && fuel_remaining() >= 16000) {
        const auto i = rt.root_cursor++;
        if (rt.union_static[i] == 0) rt.union_roots[i / 8] |= std::uint8_t{1} << (i % 8);
        --rt.scans_left;
    }
    const std::vector<std::uint16_t>* values = &rt.union_static;
    std::uint64_t generation = 0;
    if (actual && rt.root_cursor == area) {
        auto blockers = occupied;
        const auto own = index(self().position());
        blockers[own / 8] &= ~(std::uint8_t{1} << (own % 8));
        auto free_roots = rt.union_roots;
        for (std::size_t i = 0; i < free_roots.size(); ++i) free_roots[i] &= ~blockers[i];
        // Only goal-stances affect this static trace generation. A moving unit
        // elsewhere must not repeatedly restart a long omitted-altar trace.
        if (!rt.union_field || free_roots != rt.union_blockers) {
            if (!field_budget()) return false;
            bitboard roots, visited;
            roots.import_linear_buffer(free_roots);
            if (roots.is_empty()) return false;
            auto field = std::make_unique<distance_field>();
            --rt.fields_left;
            field->compute(S->terrain, roots, visited);
            auto result = field->export_linear_buffer();
            if (!qualifier_current()) return false;
            rt.union_field = std::move(field); rt.union_current = std::move(result);
            rt.union_blockers = std::move(free_roots); ++rt.union_generation;
        }
        values = &rt.union_current; generation = rt.union_generation;
    }
    steps = distance(*values, origin);
    if (steps == UNREACH) return false;
    Runtime::Trace* trace = nullptr;
    for (auto& item : rt.traces)
        if (item.used_round >= 0 && item.origin == origin && item.actual == actual && item.generation == generation) {
            trace = &item; break;
        }
    if (!trace) {
        for (auto& item : rt.traces) if (item.used_round < 0) { trace = &item; break; }
        if (!trace) for (auto& item : rt.traces)
            if (item.used_round != S->round && (!trace || item.used_round < trace->used_round)) trace = &item;
        if (!trace) return false;
        *trace = {origin, origin, generation, S->round, 0, actual, false};
    }
    trace->used_round = S->round;
    if (distance(*values, trace->cursor) == 0) trace->at_root = true;
    int local = 128;
    while (!trace->at_root && local-- > 0 && rt.trace_steps_left > 0 && fuel_remaining() >= 12000) {
        const int here = distance(*values, trace->cursor);
        if (here == 0) { trace->at_root = true; break; }
        vec2 next = trace->cursor;
        for (int y = -1; y <= 1; ++y) for (int x = -1; x <= 1; ++x) {
            const vec2 p{trace->cursor.x + x, trace->cursor.y + y};
            if (inside(p) && distance(*values, p) < here
                && (next == trace->cursor || p.y < next.y || (p.y == next.y && p.x < next.x))) next = p;
        }
        if (next == trace->cursor) return false;
        trace->cursor = next; --rt.trace_steps_left;
    }
    if (!trace->at_root) return false;
    const auto root = index(trace->cursor);
    if (actual && trace->cursor != self().position()
        && (occupied[root / 8] & (std::uint8_t{1} << (root % 8)))) return false;
    int identify = 128;
    while (trace->altar_cursor < S->altars.size() && identify-- > 0) {
        const auto candidate = S->altars[trace->altar_cursor];
        if (cheb(trace->cursor, candidate) <= constants::submit_gems_range) { altar = candidate; return qualifier_current(); }
        ++trace->altar_cursor;
    }
    return false;
}
} // namespace qualifier_delivery_detail

void qualifier_delivery_clear();
QualifierDeliveryResult qualifier_delivery_refresh();
QualifierDeliveryPlan qualifier_delivery_estimate(vec2 origin, double cargo);
QualifierDeliveryResult qualifier_delivery_execute(const QualifierDeliveryPlan& plan);
bool qualifier_delivery_buy_required(const QualifierDeliveryPlan& plan);
bool qualifier_delivery_buy_required();

void qualifier_delivery_clear() {
    auto& rt = qualifier_delivery_detail::runtime;
    ++rt.revision;
    rt.active.reset(); rt.pending = qualifier_delivery_detail::Pending::None;
    rt.committed_distances.clear(); rt.committed_portal.reset();
    rt.channel_owned = false;
    // This forgets intent; it does not cancel an actual host channel.
}

QualifierDeliveryResult qualifier_delivery_refresh() {
    namespace d = qualifier_delivery_detail;
    auto& rt = d::runtime;
    try {
        if (!qualifier_current()) return QualifierDeliveryResult::Unknown;
        const auto& me = self();
        const auto actor = static_cast<std::uint32_t>(static_cast<handle_champion_t>(me));
        const auto flags = me.state();
        if (!flags.contains(champion_state::alive)) {
            qualifier_delivery_clear(); rt.alive = false; rt.last_round = S->round; rt.actor = actor;
            return QualifierDeliveryResult::Invalidated;
        }
        const auto pos = me.position();
        const auto cargo = me.gem_count();
        const auto crystals = me.consumables()[consumable_type::tp_crystal].count();
        const bool discontinuity = rt.last_round >= 0
            && (rt.actor != actor || S->round < rt.last_round || S->round > rt.last_round + 1 || !rt.alive);
        if (discontinuity) qualifier_delivery_clear();
        const bool continuity = rt.alive && rt.actor == actor && rt.position == pos
            && rt.cargo == cargo && rt.crystals == crystals && !discontinuity;
        const bool changed = rt.last_round != S->round || rt.actor != actor || rt.position != pos
            || rt.cargo != cargo || rt.crystals != crystals || !rt.alive;
        if (changed) ++rt.revision;
        rt.actor = actor; rt.last_round = S->round; rt.position = pos;
        rt.cargo = cargo; rt.crystals = crystals; rt.alive = true;
        d::Pending actual = d::Pending::None;
        if (flags.contains(champion_state::teleporting)) actual = d::Pending::Crystal;
        else if (flags.contains(champion_state::recalling)) actual = d::Pending::Recall;
        else if (flags.contains(champion_state::submitting_gems)) actual = d::Pending::Tribute;
        if (actual != d::Pending::None) {
            // Query a timer only after an actual active-channel bit.
            if (me.remaining_channel_time() < 0) return QualifierDeliveryResult::Unknown;
            if (actual != rt.pending || discontinuity || (rt.channel_owned && pos != rt.channel_origin)) {
                rt.active.reset(); rt.channel_owned = false;
            }
            rt.pending = actual;
            return QualifierDeliveryResult::Pending;
        }
        if (rt.pending != d::Pending::None) {
            const auto ended = rt.pending;
            const bool owned = rt.channel_owned;
            const bool expected = owned && (ended == d::Pending::Crystal ? pos == rt.channel_destination
                : ended == d::Pending::Recall ? map::spawn_region(team::ally).is_set(pos)
                : pos == rt.channel_origin && cargo == 0);
            qualifier_delivery_clear();
            if (!expected) return QualifierDeliveryResult::Invalidated;
            if (ended == d::Pending::Tribute) return QualifierDeliveryResult::Complete;
            // A real arrival, not the old ETA, authorizes a fresh route estimate.
            return QualifierDeliveryResult::NoAction;
        }
        if (changed && !continuity) { rt.active.reset(); rt.committed_distances.clear(); rt.committed_portal.reset(); }
        return QualifierDeliveryResult::NoAction;
    } catch (const std::exception&) {
        ++rt.revision; rt.active.reset();
        // Do not pretend a failed observation canceled a noncancelable channel.
        return QualifierDeliveryResult::Unknown;
    }
}

QualifierDeliveryPlan qualifier_delivery_estimate(vec2 origin, double cargo) {
    namespace d = qualifier_delivery_detail;
    QualifierDeliveryPlan best;
    try {
        if (!qualifier_current() || !d::finite_nonnegative(cargo) || cargo <= 0) return best;
        const auto refreshed = qualifier_delivery_refresh();
        auto& rt = d::runtime;
        if (!rt.alive || refreshed == QualifierDeliveryResult::Unknown
            || refreshed == QualifierDeliveryResult::Pending || !d::inside(origin)) return best;
        const auto& me = self();
        const double design_speed = me.base_stats().base_move_speed;
        // Match qualifier_step's public-base-speed uint16 conversion exactly.
        if (!d::finite_nonnegative(design_speed) || design_speed < 1
            || design_speed > std::numeric_limits<std::uint16_t>::max()) return best;
        const double speed = static_cast<std::uint16_t>(design_speed);
        const double tribute = d::tribute_upper(cargo);
        if (!d::finite_nonnegative(speed) || speed <= 0 || !d::finite_nonnegative(tribute)
            || !d::topology() || d::blocked(origin)) return best;
        const auto locks = vision::altar_locks();
        const auto visible = me.visible_tiles();
        const auto occupied = (vision::champion_tiles() | vision::camp_tiles() | vision::teleport_tiles()).export_linear_buffer();
        auto occupied_at = [&](vec2 p) {
            return (occupied[d::index(p) / 8] & (std::uint8_t{1} << (d::index(p) % 8))) != 0;
        };
        const auto crystal = consumable_type::tp_crystal;
        const bool held = rt.crystals > 0;
        const bool buy_now = !held && me.can_buy(crystal);
        const bool hypothetical = origin != me.position() || cargo != me.gem_count();
        const bool current_move_wait = !hypothetical && !me.can_move();
        const bool immediate_recall_allowed = hypothetical || d::recall_start_eligible(origin);
        const bool immediate_crystal_allowed = hypothetical || d::crystal_start_eligible(origin);
        bool complete = rt.all_altars_included;
        bool any_stance = false;
        int local_checks = 768, local_scans = 256;
        int current_lock_upper = 0;
        d::Origin* origin_data = nullptr;
        int altar_start = 0;
        if ((held || buy_now) && constants::tp_crystal_time > 0 && !rt.altars.empty()) {
            origin_data = d::origin_field(origin);
            if (origin_data) {
                altar_start = origin_data->next_altar;
                origin_data->next_altar = (altar_start + 1) % static_cast<int>(rt.altars.size());
            } else complete = false;
        }
        auto make = [&](const d::Altar& altar, QualifierDeliveryKind kind, int before, int after,
                        vec2 portal, vec2 landing, double mobility) {
            QualifierDeliveryPlan plan;
            plan.status = QualifierDeliveryStatus::Found; plan.kind = kind;
            plan.actor = rt.actor; plan.observed_round = S->round; plan.revision = rt.revision;
            plan.origin = origin; plan.cargo = cargo; plan.altar = altar.tile;
            plan.portal = portal; plan.landing = landing;
            plan.walk_before_steps = before; plan.walk_after_steps = after;
            plan.lock_hits_upper = current_lock_upper;
            plan.crystals_observed = rt.crystals;
            plan.crystals_required = kind == QualifierDeliveryKind::Crystal ? 1 : 0;
            plan.purchase_required = plan.crystals_required > 0 && !held && buy_now;
            plan.resource_backed = plan.crystals_required == 0 || held;
            plan.crystal_cost = plan.purchase_required ? crystal.cost() : 0;
            plan.hypothetical_origin = hypothetical;
            const double walk = d::turns(before, speed) + d::turns(after, speed);
            const double initial_wait = current_move_wait && (before > 0 || (kind == QualifierDeliveryKind::Walk && after > 0)) ? 1.0 : 0.0;
            const double lock_wait = !hypothetical && kind == QualifierDeliveryKind::Walk && before == 0
                && plan.lock_hits_upper > 0 && !me.can_attack() ? 1.0 : 0.0;
            // Recall's actual spawn and Home shortening are not promised.
            plan.eta_lower = (kind == QualifierDeliveryKind::Recall ? 1.0 : walk + mobility)
                + std::max(1, constants::submit_gem_minimum_duration);
            plan.eta_upper = walk + mobility + tribute + plan.lock_hits_upper
                + std::max(initial_wait, lock_wait);
            // Root charges a required purchase's input before receipt/banking;
            // it must not be counted again in this delivery-only ETA.
            // Snapshot geometry/service bounds, not a survival or future-occupancy certificate.
            plan.channel_safety_known = false;
            if (d::finite_nonnegative(plan.eta_upper) && d::better(plan, best)) best = plan;
        };
        // Preserve a committed portal while our own accepted walks progress.
        // Rebind the frame/resources and price the saved actual goal field;
        // do not restart a late-discovered general bridge after every step.
        if (!hypothetical && held && rt.active && rt.active->kind == QualifierDeliveryKind::Crystal
            && rt.committed_portal && *rt.committed_portal == rt.active->portal
            && !rt.committed_distances.empty()) {
            const auto committed = *rt.active;
            const int before = d::distance(rt.committed_distances, origin);
            bool goal_free = true;
            for (const auto& item : rt.altars) if (item.tile == committed.altar)
                for (const auto p : item.stances) if (p != rt.position && occupied_at(p)) goal_free = false;
            if (before != UNREACH && goal_free
                && (committed.portal == rt.position || !occupied_at(committed.portal))
                && !occupied_at(committed.landing) && (before > 0 || immediate_crystal_allowed)) {
                d::Altar altar; altar.tile = committed.altar;
                current_lock_upper = d::lock_upper(altar.tile, visible, locks);
                make(altar, QualifierDeliveryKind::Crystal, before, committed.walk_after_steps,
                    committed.portal, committed.landing, constants::tp_crystal_time);
                if (best.found()) { best.search_complete = false; return best; }
            }
            rt.active.reset(); rt.committed_distances.clear(); rt.committed_portal.reset();
        }
        // The union field covers every public altar, not only cached portals.
        vec2 union_altar{}; int union_steps = UNREACH;
        if (d::union_walk(origin, !hypothetical, occupied, union_altar, union_steps)) {
            d::Altar altar; altar.tile = union_altar;
            current_lock_upper = d::lock_upper(altar.tile, visible, locks);
            make(altar, QualifierDeliveryKind::Walk, union_steps, 0, origin, origin, 0);
        }
        for (int altar_offset = 0; altar_offset < static_cast<int>(rt.altars.size()); ++altar_offset) {
            const int a = (altar_start + altar_offset) % static_cast<int>(rt.altars.size());
            auto& altar = rt.altars[a];
            if (altar.stances.empty()) continue;
            any_stance = true;
            current_lock_upper = d::lock_upper(altar.tile, visible, locks);
            bool occupied_stance = false, free_stance = false;
            for (const auto p : altar.stances) {
                const bool occupied_here = p != rt.position && occupied_at(p);
                occupied_stance = occupied_stance || occupied_here;
                free_stance = free_stance || !occupied_here;
            }
            if (!hypothetical && !free_stance) { complete = false; continue; }
            // A current stance needs no path-field budget merely to tribute.
            if (cheb(origin, altar.tile) <= constants::submit_gems_range)
                make(altar, QualifierDeliveryKind::Walk, 0, 0, origin, origin, 0);
            if (!d::altar_field(altar)) { complete = false; continue; }
            const std::vector<std::uint16_t>* distances = &altar.distances;
            std::vector<std::uint16_t> free_distances;
            if (!hypothetical && occupied_stance) {
                if (!d::field_budget()) { complete = false; continue; }
                bitboard roots, visited;
                for (const auto p : altar.stances) if (p == rt.position || !occupied_at(p)) roots.set(p);
                distance_field field; --rt.fields_left;
                field.compute(S->terrain, roots, visited);
                free_distances = field.export_linear_buffer();
                distances = &free_distances;
            }
            const int direct = d::distance(*distances, origin);
            if (direct != UNREACH) make(altar, QualifierDeliveryKind::Walk, direct, 0, origin, origin, 0);
            int spawn_upper = 0;
            bool spawn_known = true, saw_spawn = false;
            for (auto p : map::spawn_region(team::ally).positions()) {
                saw_spawn = true;
                const int dist = d::distance(*distances, p);
                if (dist == UNREACH) { spawn_known = false; break; }
                spawn_upper = std::max(spawn_upper, dist);
            }
            // Do not promise the host's best spawn tile or a Home reduction.
            if (spawn_known && saw_spawn && constants::recall_time > 0
                && immediate_recall_allowed)
                make(altar, QualifierDeliveryKind::Recall, 0, spawn_upper, origin, {}, constants::recall_time);
            if ((!held && !buy_now) || constants::tp_crystal_time <= 0) continue;
            if (!origin_data) { complete = false; continue; }
            auto portal = [&](vec2 p, vec2 q) {
                if (local_checks <= 0 || rt.portal_checks_left <= 0) { complete = false; return; }
                --local_checks; --rt.portal_checks_left;
                if (p == q || d::blocked(p) || d::blocked(q) || cheb(p, q) > d::crystal_range) return;
                if (!hypothetical && p == origin && !immediate_crystal_allowed) return;
                if ((p != rt.position && p != origin && occupied_at(p)) || occupied_at(q)) return;
                const int first = d::distance(origin_data->distances, p);
                const int last = d::distance(*distances, q);
                if (first == UNREACH || last == UNREACH) return;
                auto& cached = origin_data->best[a];
                if (!cached.known || first + last < cached.before + cached.after) cached = {true, p, q, first, last};
                make(altar, QualifierDeliveryKind::Crystal, first, last, p, q, constants::tp_crystal_time);
            };
            auto& known = origin_data->best[a];
            if (known.known && (d::blocked(known.from) || d::blocked(known.to)
                || (known.from != rt.position && known.from != origin && occupied_at(known.from))
                || occupied_at(known.to) || d::distance(origin_data->distances, known.from) == UNREACH
                || d::distance(*distances, known.to) == UNREACH)) {
                known.known = false;
                origin_data->cursor[a] = 0; origin_data->offset[a] = 0;
            }
            const auto cached = known;
            if (cached.known) portal(cached.from, cached.to);
            // Goal-side pocket and source-side bridge probes come first.
            for (const auto q : altar.stances)
                for (int y = -d::crystal_range; y <= d::crystal_range; ++y)
                    for (int x = -d::crystal_range; x <= d::crystal_range; ++x) {
                        if (local_checks <= 0 || rt.portal_checks_left <= 0) break;
                        portal({q.x + x, q.y + y}, q);
                    }
            for (int y = -d::crystal_range; y <= d::crystal_range; ++y)
                for (int x = -d::crystal_range; x <= d::crystal_range; ++x)
                    if (local_checks > 0 && rt.portal_checks_left > 0)
                        portal(origin, {origin.x + x, origin.y + y});
            // General one-edge bridge search resumes for this immutable origin.
            // Exhausting a budget is Unknown, never an impossibility proof.
            auto& cursor = origin_data->cursor[a];
            auto& offset = origin_data->offset[a];
            const std::size_t count = static_cast<std::size_t>(S->W) * S->H;
            while (cursor < count && local_scans > 0 && rt.scans_left > 0
                && local_checks > 0 && rt.portal_checks_left > 0 && fuel_remaining() >= 16000) {
                const auto q = d::tile(cursor);
                if (d::blocked(q) || d::distance(*distances, q) == UNREACH) {
                    ++cursor; offset = 0; --local_scans; --rt.scans_left; continue;
                }
                while (offset < 49 && local_checks > 0 && rt.portal_checks_left > 0) {
                    const int i = offset++;
                    portal({q.x + i % 7 - 3, q.y + i / 7 - 3}, q);
                }
                if (offset == 49) { ++cursor; offset = 0; --local_scans; --rt.scans_left; }
            }
            if (cursor < count) complete = false;
        }
        if (!any_stance && rt.all_altars_included) best.status = QualifierDeliveryStatus::Unavailable;
        best.search_complete = complete;
        if (!qualifier_current()) return {};
        return best;
    } catch (const std::exception&) { return {}; }
}

bool qualifier_delivery_buy_required(const QualifierDeliveryPlan& plan) {
    namespace d = qualifier_delivery_detail;
    try {
        if (!qualifier_current()) return false;
        const auto observed = qualifier_delivery_refresh();
        auto& rt = d::runtime;
        if (observed == QualifierDeliveryResult::Pending || observed == QualifierDeliveryResult::Unknown
            || !plan.found() || !plan.purchase_required || plan.resource_backed || plan.crystals_required != 1
            || plan.actor != rt.actor || plan.observed_round != S->round || plan.revision != rt.revision
            || rt.crystals != 0 || plan.crystals_observed != 0) return false;
        const auto type = consumable_type::tp_crystal;
        if (type.cost() != plan.crystal_cost || !self().can_buy(type)) return false;
        self().buy(type); // Successful void host return is an accepted purchase.
        qualifier_delivery_clear();
        return true; // Never means merely 'already satisfied'. Root ends this input.
    } catch (const std::exception&) { return false; }
}

bool qualifier_delivery_buy_required() {
    const auto active = qualifier_delivery_detail::runtime.active;
    return active && qualifier_delivery_buy_required(*active);
}

QualifierDeliveryResult qualifier_delivery_execute(const QualifierDeliveryPlan& plan) {
    namespace d = qualifier_delivery_detail;
    auto& rt = d::runtime;
    try {
        if (!qualifier_current()) return QualifierDeliveryResult::Unknown;
        const auto refreshed = qualifier_delivery_refresh();
        if (refreshed == QualifierDeliveryResult::Pending || refreshed == QualifierDeliveryResult::Complete
            || refreshed == QualifierDeliveryResult::Unknown) return refreshed;
        auto& me = self();
        if (!plan.found() || plan.hypothetical_origin || !rt.alive
            || plan.actor != rt.actor || plan.observed_round != S->round || plan.revision != rt.revision
            || plan.origin != me.position() || plan.cargo != me.gem_count()
            || plan.crystals_observed != rt.crystals || !d::finite_nonnegative(plan.eta_upper))
            return QualifierDeliveryResult::Invalidated;
        if (plan.purchase_required) return qualifier_delivery_buy_required(plan)
            ? QualifierDeliveryResult::Acted : QualifierDeliveryResult::Invalidated;
        if (!plan.resource_backed || plan.crystals_required > rt.crystals || plan.crystals_required > 1)
            return QualifierDeliveryResult::Invalidated;
        rt.active = plan;
        auto pending = [&](d::Pending type, vec2 destination) {
            rt.pending = type; rt.channel_owned = true;
            rt.channel_origin = me.position(); rt.channel_destination = destination;
            rt.channel_cargo = me.gem_count();
        };
        auto walk = [&](const bitboard& goals) {
            S->goals = goals;
            if (!qualifier_runtime.enemies.empty() || qualifier_threat(me.position()) > 0
                || qualifier_runtime.unresolved_awake_npc) {
                const auto before_moves = S->moves;
                QualifierCombatIntent intent;
                intent.purpose = QualifierCombatPurpose::Delivery;
                intent.allow_escape = false;
                const auto combat = qualifier_combat_continue(intent);
                if (combat != QualifierCombatResult::NoAction) {
                    if (!qualifier_current() || !me.is_alive()) return QualifierDeliveryResult::Unknown;
                    ++rt.revision; rt.position = me.position();
                    if (combat == QualifierCombatResult::Invalidated) {
                        rt.active.reset(); return QualifierDeliveryResult::Invalidated;
                    }
                    if (plan.kind == QualifierDeliveryKind::Crystal) {
                        if (S->moves > before_moves && S->df_go.get(plan.portal) == 0
                            && S->df_go.get(rt.position) != UNREACH) {
                            rt.committed_distances = S->df_go.export_linear_buffer();
                            rt.committed_portal = plan.portal;
                        } else if (!rt.committed_portal || *rt.committed_portal != plan.portal
                            || d::distance(rt.committed_distances, rt.position) == UNREACH) {
                            rt.committed_distances.clear(); rt.committed_portal.reset();
                        }
                    }
                    if (S->moves > before_moves && qualifier_runtime.last_walk_used_cast_route) {
                        // The optional detour is not the old executable route
                        // or ETA. Keep only the outer semantic bank objective.
                        rt.active.reset(); rt.committed_distances.clear(); rt.committed_portal.reset();
                    }
                    return QualifierDeliveryResult::Acted;
                }
            }
            if (!me.can_move()) return QualifierDeliveryResult::Unknown;
            // qualifier_step owns <=8 false-return retries using fresh occupancy.
            const bool moved = qualifier_step(false, QualifierMovePurpose::Delivery);
            if (!qualifier_current()) return QualifierDeliveryResult::Unknown;
            if (!moved) { rt.active.reset(); return QualifierDeliveryResult::Invalidated; }
            ++rt.revision; rt.position = me.position();
            if (plan.kind == QualifierDeliveryKind::Crystal) {
                rt.committed_distances = S->df_go.export_linear_buffer();
                rt.committed_portal = plan.portal;
            }
            if (qualifier_runtime.last_walk_used_cast_route) {
                rt.active.reset(); rt.committed_distances.clear(); rt.committed_portal.reset();
            }
            return QualifierDeliveryResult::Acted;
        };
        if (plan.kind == QualifierDeliveryKind::Crystal) {
            if (plan.crystals_required != 1 || d::blocked(plan.portal) || d::blocked(plan.landing)
                || cheb(plan.portal, plan.landing) > d::crystal_range || plan.portal == plan.landing)
                return QualifierDeliveryResult::Invalidated;
            if (me.position() != plan.portal) { bitboard goals; goals.set(plan.portal); return walk(goals); }
            // TP is damage-resistant, unlike recall. This is only a current
            // exposure warning, not a forecast of enemy casts or safe survival.
            if (!d::crystal_start_eligible(me.position())) {
                rt.active.reset();
                return QualifierDeliveryResult::Unknown;
            }
            const auto blocked = S->terrain | vision::champion_tiles() | vision::camp_tiles() | vision::teleport_tiles();
            if (blocked.is_set(plan.landing)) return QualifierDeliveryResult::Invalidated;
            if (!me.consumables()[consumable_type::tp_crystal].use(plan.landing)) {
                ++rt.revision; rt.active.reset();
                // One bounded attempt; false revealed occupancy and spent no
                // crystal. A fresh estimate, not a silently retargeted old Plan,
                // may choose another landing.
                return QualifierDeliveryResult::Invalidated;
            }
            if (!qualifier_current() || !me.is_teleporting()) return QualifierDeliveryResult::Unknown;
            pending(d::Pending::Crystal, plan.landing);
            return QualifierDeliveryResult::Pending;
        }
        if (plan.kind == QualifierDeliveryKind::Recall) {
            if (!d::recall_start_eligible(me.position())) {
                rt.active.reset(); return QualifierDeliveryResult::Unknown;
            }
            me.recall();
            if (!qualifier_current() || !me.is_recalling()) return QualifierDeliveryResult::Unknown;
            pending(d::Pending::Recall, {});
            return QualifierDeliveryResult::Pending;
        }
        if (cheb(me.position(), plan.altar) > constants::submit_gems_range) {
            bitboard goals;
            goals.draw_chebyshev(plan.altar, static_cast<float>(constants::submit_gems_range));
            goals &= ~S->terrain;
            return walk(goals);
        }
        // Both teams can lock one altar. Never let an allied lock mask an enemy.
        for (const auto& lock : vision::altar_locks()) if (lock.position() == plan.altar && lock.is_enemy()) {
            if (lock.hits_remaining() > static_cast<std::uint32_t>(plan.lock_hits_upper)
                || !me.can_attack()) return QualifierDeliveryResult::Invalidated;
            me.attack(lock);
            ++S->attacks; ++rt.revision;
            return QualifierDeliveryResult::Acted;
        }
        me.submit_gems(plan.altar);
        if (!qualifier_current() || !me.is_submitting_gems()) return QualifierDeliveryResult::Invalidated;
        ++S->channel;
        pending(d::Pending::Tribute, plan.altar);
        return QualifierDeliveryResult::Pending;
    } catch (const std::exception&) {
        ++rt.revision; rt.active.reset();
        return QualifierDeliveryResult::Invalidated;
    }
}
