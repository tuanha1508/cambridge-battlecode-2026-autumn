// Shared policy adapter for the two existing distance-field backends.
// Included inside their anonymous namespaces after the action helpers.
// This isolated candidate is not match-qualified or fuel-qualified.
struct QualifierEnemy {
    champion unit;
    vec2 pos;
    float hp, strength;
    int range;
};
struct QualifierParty {
    std::uint32_t id;
    std::vector<champion> members;
};
enum class QualifierNpcKnowledge { confirmed_awake, design_awake, unknown_potential };
struct QualifierNpc {
    vec2 pos;
    float strength;
    int range;
    QualifierNpcKnowledge knowledge;
    int group_index = -1;
};
struct QualifierRuntime {
    std::vector<QualifierEnemy> enemies;
    std::vector<QualifierNpc> npcs;
    bool unresolved_awake_npc = false;
    std::vector<QualifierParty> parties;
    std::vector<int> preferred_party;
    std::vector<bool> party_preference_allowed;
    bool assignment_attempted = false;
    int own_party = -1;
    int progress_target = -1, progress_sample_round = -1, progress_round = -1;
    double progress_hp = 0;
    int previous_round = -1;
    float previous_hp = 0;
    float recent_damage = 0;
    int previous_channel = 0, previous_remaining = 0;
    std::uint32_t previous_cargo = 0;
    vec2 previous_position{};
    float previous_maxhp = 0;
    bool previous_shielded = false;
    bool support_heal_this_input = false;
    bool last_walk_used_cast_route = false;
    int avoid_channel_until = -1;
    int close_target = -1;
    std::uint64_t observation_revision = 0;
    int delivery_capture_group = -1;
    std::optional<vec2> delivery_receipt_tile;
    int contact_capture_group = -1;
    std::optional<bitboard> contact_capture_region;
};
QualifierRuntime qualifier_runtime;

const bitboard& qualifier_tiles(const Group& group) {
#if QUALIFIER_LEGACY_BACKEND
    return group.tiles;
#else
    return group.tiles();
#endif
}

bool qualifier_current() {
    return S && static_cast<int>(round_number()) == S->round;
}

#include "qualifier_owned_work.inc"

#include "qualifier_cooperation.inc"
void qualifier_inspection_cancel(bool defer = false);

void qualifier_prepare_assignment() {
    auto& rt = qualifier_runtime;
    if (rt.assignment_attempted || !qualifier_current()) return;
    // Optional work only. This reserve is unmeasured, not a fuel guarantee.
    if (fuel_remaining() < 30000) return;
    rt.assignment_attempted = true;
    try {
        auto bodies = champions(team::ally);
        if (bodies.empty() || bodies.size() > 16 || S->groups.size() > 512) return;
        auto identity = [](const champion& body) {
            return static_cast<std::uint32_t>(static_cast<handle_champion_t>(body));
        };
        std::sort(bodies.begin(), bodies.end(), [&](const auto& a, const auto& b) {
            return identity(a) < identity(b);
        });
        std::vector<champion_type> types;
        int cassi = -1, issac = -1;
        for (std::size_t i = 0; i < bodies.size(); ++i) {
            if (i > 0 && identity(bodies[i]) == identity(bodies[i - 1])) return;
            const auto type = bodies[i].champion_name(); // Immutable public identity.
            types.push_back(type);
            if (type == champion_type::cassi) { if (cassi >= 0) return; cassi = static_cast<int>(i); }
            if (type == champion_type::issac) { if (issac >= 0) return; issac = static_cast<int>(i); }
        }
        std::vector<QualifierParty> parties;
        for (std::size_t i = 0; i < bodies.size(); ++i) {
            if (types[i] == champion_type::cassi || types[i] == champion_type::issac) {
                // Missing partner gets no independent home assignment.
                if (cassi < 0 || issac < 0 || static_cast<int>(i) != std::min(cassi, issac)) continue;
                parties.push_back({std::min(identity(bodies[cassi]), identity(bodies[issac])),
                                   {bodies[cassi], bodies[issac]}});
            } else parties.push_back({identity(bodies[i]), {bodies[i]}});
        }
        std::vector<qualifier::PublicWorker> workers;
        for (const auto& party : parties) workers.push_back({party.id});
        std::vector<qualifier::PublicJob> jobs;
        for (const auto& group : S->groups) {
            double work = 0;
            bool known = !group.camps.empty() && (group.gems > 0 || group.gold > 0);
            for (const auto& camp : group.camps) {
                if ((group.gems > 0 || group.gold > 0) && !camp.design_known) return;
                known = known && camp.design_known;
                work += camp.maxhp;
            }
            jobs.push_back({static_cast<std::uint32_t>(static_cast<handle_camp_group_t>(group.h)),
                            known ? work : -1.0});
        }
        auto owners = qualifier::preferred_workers(workers, jobs);
        if (owners.size() != S->groups.size() || !qualifier_current()) return;
        int own = -1;
        for (std::size_t p = 0; p < parties.size(); ++p)
            for (const auto& member : parties[p].members)
                if (identity(member) == identity(self())) own = static_cast<int>(p);
        rt.parties = std::move(parties);
        rt.preferred_party = std::move(owners);
        rt.own_party = own;
    } catch (const std::exception&) {
        // No guessed membership or partial assignment after a query failure.
        rt.parties.clear(); rt.preferred_party.clear(); rt.own_party = -1;
    }
}

void qualifier_refresh_assignment() {
    auto& rt = qualifier_runtime;
    rt.party_preference_allowed.assign(rt.parties.size(), true);
    for (std::size_t p = 0; p < rt.parties.size(); ++p) {
        for (const auto& member : rt.parties[p].members) {
            try {
                if (!member.is_alive()) { rt.party_preference_allowed[p] = false; break; }
                // Hidden intent remains unknown. Only a static soft prior is
                // allowed here; this is not proof of available help.
                if (!member.is_visible()) continue;
                if (member.is_recalling() || member.is_submitting_gems() || member.is_teleporting()
                    || (map::spawn_region(team::ally).is_set(member.position()) && member.health() < member.max_health())) {
                    rt.party_preference_allowed[p] = false; break;
                }
            } catch (const cambc::error&) { rt.party_preference_allowed[p] = false; break; }
        }
    }
    // Progress is a newly measured HP decline, not a chosen target or a
    // fight() return. It may include actual visible work by a teammate.
    const int target = S->qualifier_committed_target;
    if (target != rt.progress_target) {
        rt.progress_target = target; rt.progress_sample_round = rt.progress_round = -1;
    }
    if (target < 0 || target >= static_cast<int>(S->groups.size())) return;
    const auto& group = S->groups[target];
    if (group.last_observed_round != S->round || group.observed_state != 0) return;
    double hp = 0;
    for (const auto& camp : group.camps) {
        if (!camp.health_observation_valid || camp.last_observed_round != S->round) return;
        hp += camp.hp;
    }
    if (!qualifier_current()) return;
    if (rt.progress_sample_round >= 0 && rt.progress_sample_round == S->round - 1
        && hp < rt.progress_hp) rt.progress_round = S->round;
    rt.progress_hp = hp; rt.progress_sample_round = S->round;
}

#include "qualifier_opening.inc"

double qualifier_ordinary_preference(int group, double carried,
    const std::optional<QualifierCooperationNomination>& nomination) {
    const auto& rt = qualifier_runtime;
    if (carried > 0 || rt.own_party < 0 || rt.preferred_party.size() != S->groups.size()) return 1;
    const auto& job = S->groups[group];
    if (job.last_observed_round == S->round && job.observed_state == 1) return 1;
    if (group == rt.progress_target && rt.progress_round >= 0 && rt.progress_round >= S->round - 2) return 1;
    const auto opening_owner = qualifier_opening_owner(group);
    const int owner = opening_owner ? *opening_owner : rt.preferred_party[group];
    const bool another_owner = owner >= 0 && owner != rt.own_party
        && owner < static_cast<int>(rt.party_preference_allowed.size())
        && rt.party_preference_allowed[owner];
    // A weak, uncalibrated score preference only. It grants no help, receipt,
    // damage or safety credit and never changes physical task duration.
    constexpr double nonowner_preference = 0.8;
    double preference = another_owner ? nonowner_preference : 1.0;
    // A dated partner intention can discourage a NEW duplicate trip. It is
    // never help, continued occupancy or reward proof, and does not dislodge
    // our existing commitment. Reuse the same uncalibrated soft prior.
    if (group != S->qualifier_committed_target && nomination
        && nomination->group_index == group && nomination->age >= 0 && nomination->age <= 1
        && nomination->full_group == static_cast<std::uint32_t>(static_cast<handle_camp_group_t>(job.h)))
        preference = std::min(preference, nonowner_preference);
    return preference;
}

void qualifier_refresh_npcs() {
    auto& rt = qualifier_runtime;
    rt.npcs.clear(); rt.unresolved_awake_npc = false;
    const auto origin = self().position();
    double span = std::max(0, self().base_stats().base_move_speed);
    double dash = 0;
    for (const auto& ability : self().abilities())
        if (ability.is_dash()) dash = std::max(dash, static_cast<double>(ability.get<ability_dash>().range));
    span += std::max(0.0, dash);
    for (int group_index = 0; group_index < static_cast<int>(S->groups.size()); ++group_index) {
        const auto& group = S->groups[group_index];
        if (group.last_observed_round == S->round && group.observed_state != 0) continue;
        for (const auto& camp : group.camps) {
            QualifierNpcKnowledge knowledge = QualifierNpcKnowledge::unknown_potential;
            float strength = camp.str;
            int range = camp.range;
            if (camp.state_observed_round == S->round) {
                if (!camp.observed_awake) continue; // Actually observed asleep or dead.
                if (camp.combat_observed_round == S->round && std::isfinite(camp.observed_strength)
                    && camp.observed_strength >= 0 && camp.observed_attack_range >= 0) {
                    strength = camp.observed_strength; range = camp.observed_attack_range;
                    knowledge = QualifierNpcKnowledge::confirmed_awake;
                } else if (camp.design_known) knowledge = QualifierNpcKnowledge::design_awake;
                else { rt.unresolved_awake_npc = true; continue; }
            } else if (!camp.design_known) continue;
            if (!std::isfinite(strength) || strength < 0 || range < 0) {
                if (knowledge != QualifierNpcKnowledge::unknown_potential) rt.unresolved_awake_npc = true;
                continue;
            }
            if (strength == 0 || static_cast<double>(cheb(origin, camp.pos)) > range + span) continue;
            rt.npcs.push_back({camp.pos, strength, range, knowledge, group_index});
        }
    }
}

float qualifier_unknown_npc_potential(vec2 position) {
    float potential = 0;
    for (const auto& npc : qualifier_runtime.npcs)
        if (npc.knowledge == QualifierNpcKnowledge::unknown_potential && cheb(position, npc.pos) <= npc.range)
            potential += npc.strength;
    return potential;
}

float qualifier_enemy_threat(vec2 p) {
    float damage = 0;
    for (const auto& e : qualifier_runtime.enemies)
        if (cheb(p, e.pos) <= e.range + 1) damage += e.strength;
    return damage;
}

float qualifier_threat(vec2 p) {
    float damage = qualifier_enemy_threat(p);
    for (const auto& npc : qualifier_runtime.npcs)
        if (npc.knowledge != QualifierNpcKnowledge::unknown_potential && cheb(p, npc.pos) <= npc.range)
            damage += npc.strength;
    return damage;
}

// Fresh observations are shared by contact combat and movement continuation.
bool qualifier_refresh_world() {
    if (!qualifier_current()) return false;
    qualifier_observe();
    if (!qualifier_current()) return false;
    qualifier_runtime.enemies.clear();
    for (const auto& enemy : vision::champions(team::enemy)) {
        if (!enemy.is_visible() || !enemy.is_alive()) continue;
        qualifier_runtime.enemies.push_back({enemy, enemy.position(), enemy.health(), enemy.strength(), enemy.attack_range()});
    }
    qualifier_refresh_npcs();
    if (!qualifier_current()) return false;
    ++qualifier_runtime.observation_revision;
    return true;
}

#include "qualifier_enemy_advice.inc"

struct QualifierMovePlan {
    bool found = false, allow_escape = false;
    QualifierMovePurpose purpose = QualifierMovePurpose::Ordinary;
    bool advice_used = false, avoid_route = false, protective_fallback = false;
    bool cast_steering_suppressed = false;
    bool neighborhood_complete = false;
    QualifierEnemyAdvice origin_advice, destination_advice;
    double origin_score = 0, destination_score = 0;
    double route_reference = 0;
    int origin_route_distance = UNREACH, destination_route_distance = UNREACH;
    std::uint32_t actor = 0;
    int round = -1;
    std::uint64_t revision = 0;
    std::uint64_t advice_serial = 0, cast_route_generation = 0, field_generation = 0;
    bool prepared_allow_goal_exit = false;
    vec2 origin{}, destination{};
    float origin_threat = 0, destination_threat = 0;
    float origin_unknown = 0, destination_unknown = 0;
    int origin_distance = UNREACH, destination_distance = UNREACH;
    bitboard goals, terrain;
};
bool qualifier_step_neighborhood_complete = false;
std::optional<bitboard> qualifier_step_declared_goals;
bool qualifier_step_physical_topology_valid = false;
std::optional<bitboard> qualifier_step_physical_walls;
std::uint64_t qualifier_step_field_generation = 0;
bool qualifier_step_allow_goal_exit = false;
QualifierMovePurpose qualifier_step_field_purpose = QualifierMovePurpose::Ordinary;

bool qualifier_prepare_step_field(bool allow_goal_exit = false,
    QualifierMovePurpose purpose = QualifierMovePurpose::Ordinary) {
    ++qualifier_step_field_generation;
    qualifier_step_allow_goal_exit = allow_goal_exit;
    qualifier_step_field_purpose = purpose;
    qualifier_step_neighborhood_complete = false;
    qualifier_step_physical_topology_valid = false;
    qualifier_step_physical_walls.reset();
    qualifier_step_declared_goals.reset();
    if (!S || !qualifier_current() || !self().is_alive() || !self().can_move()) return false;
    auto& s = *S;
    auto& me = self();
    qualifier_step_declared_goals = s.goals;
    const bool protective = purpose != QualifierMovePurpose::Ordinary;
    const auto start = me.position();
    refresh_occ();
    if (protective) { vision::teleport_tiles_into(s.tmp); s.occ |= s.tmp; }
    s.occ.set(start, false);
    s.goals &= ~s.terrain; s.goals &= ~s.occ;
    if (s.goals.is_empty() || (s.goals.is_set(start) && !allow_goal_exit && !protective)) return false;
    s.me_bb.clear(); s.me_bb.set(start);
    bool terrain_fallback = false;
    if ((allow_goal_exit || protective) && s.goals.is_set(start)) {
        // ComputeTo may stop at this already-reached root. Populate the entire
        // movement neighborhood so a real exit can be compared with staying.
        const auto speed = me.base_stats().base_move_speed;
        if (speed <= 0 || speed >= UNREACH) return false;
        s.df_go.compute(s.occ, s.goals, s.visited,
            static_cast<std::uint16_t>(std::min<int>(UNREACH - 1, speed + 1)));
        qualifier_step_neighborhood_complete = true;
    } else {
        s.df_go.compute_to(s.occ, s.goals, s.me_bb, s.visited);
        if (s.df_go.get(start) == UNREACH) {
            terrain_fallback = true;
            s.df_go.compute_to(s.terrain, s.goals, s.me_bb, s.visited);
        }
        if (protective && qualifier_current() && fuel_remaining() >= 24000) {
            const int distance = s.df_go.get(start), speed = me.base_stats().base_move_speed;
            if (distance != UNREACH && speed > 0 && speed < UNREACH
                && distance < UNREACH - speed) {
                // ComputeTo(origin) can omit lateral/backstep endpoints. Fill
                // that exact topology through the complete legal neighborhood.
                s.df_go.compute(terrain_fallback ? s.terrain : s.occ, s.goals, s.visited,
                    static_cast<std::uint16_t>(distance + speed));
                qualifier_step_neighborhood_complete = true;
            }
        }
    }
    if (qualifier_current() && !terrain_fallback) {
        qualifier_step_physical_walls = s.occ;
        qualifier_step_physical_topology_valid = true;
    }
    return qualifier_current();
}

// Candidate selection is shared with the legacy bounded retry wrapper. Threat
// values are current heuristics, not a prediction of the enemy's next input.
#include "qualifier_cast_route.inc"

QualifierMovePlan qualifier_choose_step(bool allow_escape, const bitboard& attempted,
    QualifierMovePurpose purpose = QualifierMovePurpose::Ordinary) {
    QualifierMovePlan plan;
    auto& s = *S;
    auto& me = self();
    if (!qualifier_current() || !me.is_alive() || !me.can_move()) return plan;
    const auto start = me.position();
    const int current = s.df_go.get(start);
    const float health = std::max(1.0f, me.health());
    const auto speed = me.base_stats().base_move_speed;
    if (speed <= 0 || speed > std::numeric_limits<std::uint16_t>::max()) return plan;
    refresh_occ(); vision::teleport_tiles_into(s.tmp); s.occ |= s.tmp; s.occ.set(start, false);
    qualifier_step_physical_topology_valid = qualifier_step_physical_topology_valid
        && qualifier_step_physical_walls && *qualifier_step_physical_walls == s.occ;
    const auto legal = s.occ.reachable_tiles(start, static_cast<std::uint16_t>(speed));
    const bool protective = purpose != QualifierMovePurpose::Ordinary;
    const auto* frame = protective ? qualifier_enemy_advice_detail::obtain() : nullptr;
    if (!qualifier_current()) return {};
    const bool advice_used = frame && qualifier_enemy_advice_detail::numeric_complete(*frame);
    const auto origin_advice = frame ? qualifier_enemy_advice_detail::project(*frame, start) : QualifierEnemyAdvice{};
    const double scale = allow_escape ? 100.0 : 10.0;
    const QualifierCastRouteView* route = nullptr;
    if (!protective) qualifier_cast_route_clear();
    else if (advice_used && qualifier_step_neighborhood_complete && qualifier_step_declared_goals)
        route = qualifier_cast_route_prepare(purpose, scale, *frame, *qualifier_step_declared_goals);
    if (!qualifier_current()) return {};
    const float origin_threat = qualifier_threat(start), origin_unknown = qualifier_unknown_npc_potential(start);
    const auto at = [&](const std::vector<std::uint16_t>& values, vec2 point) -> int {
        if (!route || point.x < 0 || point.y < 0 || point.x >= route->width || point.y >= route->height)
            return UNREACH;
        const auto index = static_cast<std::size_t>(point.y) * route->width + point.x;
        return index < values.size() ? static_cast<int>(values[index]) : UNREACH;
    };
    const bool admitted_route = route && route->admissible && route->neighborhood_complete;
    double best_cost = 0;
    float best_potential = 0;
    bool chose_route = false;
    bool failed_avoid_gradient = false;
    for (int pass = 0; pass < 2; ++pass) {
        const bool avoiding = pass == 0 && admitted_route;
        if (pass == 0 && !avoiding) continue;
        for (const auto point : legal.positions()) {
            if (point == start || attempted.is_set(point)) continue;
            const int distance = route && !route->base.empty() ? at(route->base, point) : s.df_go.get(point);
            const int route_distance = avoiding ? at(route->avoid, point) : distance;
            if (distance == UNREACH || route_distance == UNREACH) continue;
            const auto threat = qualifier_threat(point), potential = qualifier_unknown_npc_potential(point);
            const auto advice = frame ? qualifier_enemy_advice_detail::project(*frame, point) : QualifierEnemyAdvice{};
            if (avoiding) {
                if (!route->reachable.is_set(point) || route_distance >= route->avoid_distance
                    || !advice_used || advice.unknown > origin_advice.unknown) continue;
                // Only the separate descending route can admit a lateral or
                // backward physical step. Preserve its exposure constraints.
                if (distance >= current && (threat > origin_threat || potential > origin_unknown)) continue;
            } else if (!allow_escape && (current == UNREACH || distance >= current)) continue;
            // These are separate current heuristic cost terms, not HP damage
            // summed into a future burst or a guarantee of safe arrival.
            const double cost = route_distance + scale * threat / health
                + (avoiding && advice_used ? scale * advice.cast_reference / health : 0);
            if (avoiding) {
                const double staying = route->avoid_distance + scale * origin_threat / health
                    + scale * origin_advice.cast_reference / health;
                if (cost >= staying) continue; // No compatible improving route step.
            }
            if (!plan.found || cost < best_cost || (cost == best_cost && potential < best_potential)
                || (cost == best_cost && potential == best_potential
                    && (point.y < plan.destination.y || (point.y == plan.destination.y && point.x < plan.destination.x)))) {
                plan.found = true; plan.destination = point;
                plan.destination_distance = distance; plan.destination_route_distance = route_distance;
                plan.destination_threat = threat; plan.destination_unknown = potential;
                plan.destination_advice = advice; best_cost = cost; best_potential = potential;
            }
        }
        if (plan.found) { chose_route = avoiding; break; }
        if (avoiding) {
            failed_avoid_gradient = true;
            const auto generation = route->generation;
            route = nullptr; // fail() may clear its owning cache on an error.
            qualifier_cast_route_fail(generation);
        }
    }
    if (!plan.found || !qualifier_current()) return {};
    plan.allow_escape = allow_escape; plan.purpose = purpose;
    plan.field_generation = qualifier_step_field_generation;
    plan.prepared_allow_goal_exit = qualifier_step_allow_goal_exit;
    // A failed/unavailable avoid route falls back to the OLD nominal policy.
    // Keeping a local cast price here could itself reintroduce an A/B loop.
    plan.advice_used = advice_used && chose_route; plan.avoid_route = chose_route;
    plan.cast_steering_suppressed = failed_avoid_gradient || (route
        && route->fallback == QualifierCastRouteFallback::Suppressed);
    plan.protective_fallback = protective && !chose_route;
    plan.neighborhood_complete = qualifier_step_neighborhood_complete;
    plan.advice_serial = plan.advice_used ? frame->serial : 0;
    plan.cast_route_generation = chose_route ? route->generation : 0;
    plan.route_reference = route && route->witness_complete ? route->path_reference : 0;
    plan.actor = static_cast<std::uint32_t>(static_cast<handle_champion_t>(me));
    plan.round = s.round; plan.revision = qualifier_runtime.observation_revision;
    plan.origin = start; plan.origin_distance = current;
    plan.origin_route_distance = chose_route ? route->avoid_distance : current;
    plan.origin_threat = origin_threat; plan.origin_unknown = origin_unknown; plan.origin_advice = origin_advice;
    plan.origin_score = plan.origin_route_distance + scale * origin_threat / health
        + (plan.advice_used ? scale * origin_advice.cast_reference / health : 0);
    plan.destination_score = best_cost;
    plan.goals = s.goals; plan.terrain = s.terrain;
    return qualifier_current() ? plan : QualifierMovePlan{};
}

QualifierMovePlan qualifier_plan_step(bool allow_escape = false,
    QualifierMovePurpose purpose = QualifierMovePurpose::Ordinary) {
    if (!qualifier_prepare_step_field(allow_escape, purpose)) return {};
    bitboard attempted;
    return qualifier_choose_step(allow_escape, attempted, purpose);
}

// Execute only the chosen destination. A false/throw invalidates the frame;
// this function never silently substitutes a different scored movement.
bool qualifier_execute_step(const QualifierMovePlan& plan) {
    auto& me = self();
    if (!plan.found || !qualifier_current() || !me.is_alive() || !me.can_move()
        || plan.actor != static_cast<std::uint32_t>(static_cast<handle_champion_t>(me))
        || plan.round != S->round || plan.revision != qualifier_runtime.observation_revision
        || plan.origin != me.position() || !(plan.terrain == S->terrain)
        || plan.field_generation != qualifier_step_field_generation
        || plan.prepared_allow_goal_exit != qualifier_step_allow_goal_exit
        || plan.purpose != qualifier_step_field_purpose || !(plan.goals == S->goals)) return false;
    refresh_occ(); vision::teleport_tiles_into(S->tmp); S->occ |= S->tmp;
    S->occ.set(plan.origin, false);
    const auto speed = me.base_stats().base_move_speed;
    if (speed <= 0 || speed > std::numeric_limits<std::uint16_t>::max()
        || !S->occ.reachable_tiles(plan.origin, static_cast<std::uint16_t>(speed)).is_set(plan.destination)
        || !qualifier_current()) return false;
    if (plan.advice_used) {
        const auto* current = qualifier_enemy_advice_detail::obtain();
        if (!current || current->serial != plan.advice_serial) return false;
    }
    if (plan.avoid_route && !qualifier_cast_route_commit(plan.cast_route_generation, plan.destination)) return false;
    S->goals = plan.goals;
    try {
        if (me.move_absolute(plan.destination)) {
            S->moved = true; ++S->moves; ++qualifier_runtime.observation_revision;
            qualifier_runtime.last_walk_used_cast_route = plan.avoid_route;
            return qualifier_current() && me.is_alive() && me.position() == plan.destination;
        }
    } catch (const cambc::error&) {}
    ++qualifier_runtime.observation_revision;
    return false;
}

// Ordinary navigation retains <=8 host-validated hidden-blocker retries. The
// combat controller uses the same chooser but owns its own failed-prefix exit.
bool qualifier_step(bool allow_escape = false,
    QualifierMovePurpose purpose = QualifierMovePurpose::Ordinary) {
    if (!qualifier_current()) return false;
    const auto job_goals = S->goals;
    if (!qualifier_prepare_step_field(false, purpose)) return false;
    bitboard attempted;
    for (int retry = 0; retry < 8 && qualifier_current() && self().can_move(); ++retry) {
        const auto plan = qualifier_choose_step(allow_escape, attempted, purpose);
        if (!plan.found) return false;
        attempted.set(plan.destination);
        if (qualifier_execute_step(plan)) return true;
        if (!qualifier_current() || !self().is_alive() || S->moved) return false;
        // A hidden blocker can reveal fresh units. Refresh before selecting a
        // different destination, while keeping the same physical job goals.
        if (fuel_remaining() < 30000 || !qualifier_refresh_world()) return false;
        S->goals = job_goals;
        if (!qualifier_prepare_step_field(false, purpose)) return false;
    }
    return false;
}

void qualifier_publish(int target, int state) {
    // The end-input guard owns the complete eight-byte protocol. Legacy
    // call sites express local intent only; never overlay their old format.
    (void)target; (void)state;
}

#include "qualifier_combat.inc"
#include "qualifier_delivery.inc"

struct QualifierTaskDelivery {
    int group = -1;
    vec2 receipt{};
    QualifierDeliveryPlan plan;
};
std::vector<QualifierTaskDelivery> qualifier_task_deliveries;
struct QualifierReceiptChoice { int group; vec2 receipt; bool walking_delivery; };
std::vector<QualifierReceiptChoice> qualifier_receipt_choices;
std::optional<QualifierDeliveryPlan> qualifier_direct_delivery;

const QualifierTaskDelivery* qualifier_task_delivery(int group) {
    for (const auto& choice : qualifier_task_deliveries)
        if (choice.group == group) return &choice;
    return nullptr;
}

bool qualifier_delivery_usable(const QualifierDeliveryPlan& plan) {
    return plan.found() && std::isfinite(plan.eta_upper) && plan.eta_upper >= 0
        && (plan.resource_backed || (plan.purchase_required
            && self().can_buy(consumable_type::tp_crystal)));
}

bitboard qualifier_receipt_goals(int group) {
    bitboard goals;
    const QualifierReceiptChoice* chosen = nullptr;
    for (const auto& choice : qualifier_receipt_choices) if (choice.group == group) chosen = &choice;
    if (chosen) goals.set(chosen->receipt);
    else if (qualifier_runtime.delivery_capture_group == group
        && qualifier_runtime.delivery_receipt_tile) goals.set(*qualifier_runtime.delivery_receipt_tile);
    else goals = qualifier_tiles(S->groups[group]);
    return goals;
}

double qualifier_walk_turns(int distance) {
    if (distance == UNREACH || distance < 0) return -1;
    const auto speed = self().base_stats().base_move_speed;
    if (speed <= 0) return distance == 0 ? 0.0 : -1.0;
    return std::ceil(static_cast<double>(distance) / speed);
}


// Temporary optimistic scores select where to spend bounded route work. Only
// an actually constructed resource-backed route can reach the final scorer.
void qualifier_prepare_deliveries(std::vector<qualifier::Task>& tasks, qualifier::Context context,
    const std::optional<QualifierCooperationNomination>& nomination) {
    qualifier_task_deliveries.clear();
    auto candidates = tasks;
    for (auto& task : candidates) {
        const auto& group = S->groups[task.id];
        task.delivery_available = true; // Shortlist only, never final credit.
        task.feasible = task.feasible || (group.last_observed_round == S->round
            && group.observed_state == 0 && group.camps.size() <= qualifier::contact::max_members);
    }
    context.bank_turns = -1; context.current_task = -1;
    std::array<int, 4> selected{-1, -1, -1, -1};
    const auto disable = [&](int id) {
        for (auto& candidate : candidates) if (candidate.id == id) candidate.feasible = false;
    };
    for (const auto& candidate : candidates)
        if (candidate.id == S->qualifier_committed_target && candidate.feasible) selected[0] = candidate.id;
    if (selected[0] >= 0) disable(selected[0]);
    auto walking = candidates;
    for (auto& candidate : walking) {
        bool known_walk = false;
        for (const auto& choice : qualifier_receipt_choices)
            if (choice.group == candidate.id) known_walk = choice.walking_delivery;
        candidate.feasible = candidate.feasible && known_walk;
    }
    selected[1] = qualifier::choose(context, walking).task;
    if (selected[1] >= 0) disable(selected[1]);
    for (std::size_t i = 0; i < selected.size(); ++i) {
        // Keep the current job, a walking option and an economic alternative.
        // One remaining route budget may examine the exact nominated job;
        // only a constructed own route can make its delivery feasible.
        if (i == selected.size() - 1 && selected[i] < 0 && nomination)
            for (const auto& candidate : candidates)
                if (candidate.feasible && candidate.id == nomination->group_index
                    && nomination->full_group == static_cast<std::uint32_t>(
                        static_cast<handle_camp_group_t>(S->groups[candidate.id].h)))
                    selected[i] = candidate.id;
        if (selected[i] < 0) selected[i] = qualifier::choose(context, candidates).task;
        if (selected[i] >= 0) disable(selected[i]);
    }
    for (const int id : selected) {
        if (id < 0 || !qualifier_current()) continue;
        qualifier::Task* task = nullptr;
        const QualifierReceiptChoice* origin = nullptr;
        for (auto& candidate : tasks) if (candidate.id == id) task = &candidate;
        for (const auto& choice : qualifier_receipt_choices) if (choice.group == id) origin = &choice;
        if (!task || !origin || (context.carried_gems == 0 && task->reward_gems == 0)) continue;
        if (fuel_remaining() < 26000) break; // Optional reserve, not a runtime guarantee.
        auto plan = qualifier_delivery_estimate(origin->receipt, context.carried_gems + task->reward_gems);
        if (!qualifier_current()) return;
        if (qualifier_delivery_usable(plan)) {
            task->delivery_turns = plan.eta_upper; // Includes tribute and known lock service.
            task->bank_turns = 0;
            task->delivery_available = true;
            task->delivery_gold_cost = plan.purchase_required ? plan.crystal_cost : 0;
            if (plan.purchase_required) {
                ++task->travel_turns; // Root ends the shopping input before approach.
                const auto& group = S->groups[id];
                if (group.last_observed_round == S->round && group.observed_state == 1
                    && group.qualifier_capture_round == S->round
                    && group.qualifier_capture_status == static_cast<int>(map::camp_group_capture_type::ally_capturing)
                    && task->travel_turns > group.qualifier_capture_timer) task->feasible = false;
            }
        }
        qualifier_task_deliveries.push_back({id, origin->receipt, std::move(plan)});
    }
}

#include "qualifier_stat_request.inc"
#include "qualifier_shop.inc"

void qualifier_recover() {
    auto& s = *S;
    auto& me = self();
    if (!qualifier_current() || !me.is_alive()) return;
    qualifier_own_work_retire();
    qualifier_inspection_cancel();
    qualifier_runtime.contact_capture_group = -1;
    qualifier_runtime.contact_capture_region.reset();
    qualifier_runtime.delivery_capture_group = -1;
    qualifier_runtime.delivery_receipt_tile.reset();
    qualifier_delivery_clear();
    qualifier_publish(-1, 0);
    s.mode = Mode::recall;
    const auto pos = me.position();
    if (map::spawn_region(team::ally).is_set(pos)) return;
    const auto walk = s.df_spawn.get(pos);
    if (qualifier_runtime.recent_damage <= 0 && qualifier_threat(pos) <= 0
        && !qualifier_runtime.unresolved_awake_npc
        && s.round >= qualifier_runtime.avoid_channel_until
        && (walk == UNREACH || walk > constants::recall_time)) {
        try { if (qualifier_current()) me.recall(); } catch (const cambc::error&) {}
        return;
    }
    s.goals = map::spawn_region(team::ally);
    if (!qualifier_runtime.enemies.empty() || qualifier_threat(me.position()) > 0
        || qualifier_runtime.unresolved_awake_npc) {
        QualifierCombatIntent intent;
        intent.purpose = QualifierCombatPurpose::Recovery;
        intent.allow_escape = true;
        if (qualifier_combat_continue(intent) != QualifierCombatResult::NoAction) return;
    }
    qualifier_step(true, QualifierMovePurpose::Recovery);
}

enum class QualifierBankOutcome { NoRoute, Stop };
QualifierBankOutcome qualifier_bank() {
    auto& s = *S;
    auto& me = self();
    if (!qualifier_current() || !me.is_alive()) return QualifierBankOutcome::Stop;
    if (me.gem_count() == 0) return QualifierBankOutcome::NoRoute;
    qualifier_inspection_cancel();
    s.mode = Mode::bank;
    qualifier_runtime.contact_capture_group = -1;
    qualifier_runtime.contact_capture_region.reset();
    qualifier_runtime.delivery_capture_group = -1;
    qualifier_runtime.delivery_receipt_tile.reset();
    qualifier_publish(-1, 0);
    if (!qualifier_direct_delivery || qualifier_direct_delivery->origin != me.position()
        || qualifier_direct_delivery->cargo != me.gem_count()
        || qualifier_direct_delivery->observed_round != s.round)
        qualifier_direct_delivery = qualifier_delivery_estimate(me.position(), me.gem_count());
    if (!qualifier_current()) return QualifierBankOutcome::Stop;
    auto& plan = *qualifier_direct_delivery;
    if (!qualifier_delivery_usable(plan)) {
        // A failed/truncated search is not permission for an unrelated old
        // route or an indefinite sticky bank intent.
        s.mode = Mode::farm;
        return QualifierBankOutcome::NoRoute; // No physical action was attempted.
    }
    qualifier_own_work_retire(); // A usable bank intent now owns the input.
    if (plan.purchase_required && !plan.resource_backed) {
        if (qualifier_delivery_buy_required(plan)) ++qualifier_runtime.observation_revision;
        qualifier_direct_delivery.reset();
        return QualifierBankOutcome::Stop; // Replan after purchase or rejection.
    }
    const auto result = qualifier_delivery_execute(plan);
    if (result == QualifierDeliveryResult::Invalidated || result == QualifierDeliveryResult::Unknown) {
        qualifier_direct_delivery.reset();
        s.mode = Mode::farm;
    }
    if (result == QualifierDeliveryResult::Acted && qualifier_runtime.last_walk_used_cast_route)
        qualifier_direct_delivery.reset(); // Fresh route/resources/ETA next input.
    // Every executor call owns the rest of this input, including an uncertain
    // partial host action. Only the pre-execution NoRoute path may continue.
    return QualifierBankOutcome::Stop;
}

void qualifier_sustain() {
    auto& s = *S;
    auto& me = self();
    if (!qualifier_current() || s.cast || !me.can_cast()) return;
    for (const auto& ability : me.abilities()) {
        if (!ability.is_heal() || !ability.is_ready()) continue;
        const auto design = ability.get<ability_heal>();
        std::optional<champion> target;
        float best_heal = 0;
        for (const auto& ally : vision::champions(team::ally)) {
            if (!ally.is_visible() || !ally.is_alive()) continue;
            const bool own = static_cast<handle_champion_t>(ally) == static_cast<handle_champion_t>(me);
            if (own ? !ability.can_target_self() : !ability.can_target_allies()) continue;
            if (me.position().chebyshev_distance(ally.position()) > design.range) continue;
            const float heal = ability.heal_amount(ally);
            if (heal > best_heal) { target = ally; best_heal = heal; }
        }
        if (target && qualifier_current()) {
            try {
                if (ability.use(*target)) {
                    s.cast = true; ++s.casts;
                    qualifier_runtime.support_heal_this_input =
                        static_cast<handle_champion_t>(*target) != static_cast<handle_champion_t>(me);
                    ++qualifier_runtime.observation_revision;
                    return;
                }
            } catch (const cambc::error&) {}
        }
    }
}

bool qualifier_contest(const Group& group) {
    const QualifierEnemy* target = nullptr;
    for (const auto& enemy : qualifier_runtime.enemies)
        if (qualifier_tiles(group).is_set(enemy.pos) && (!target || enemy.hp < target->hp)) target = &enemy;
    if (!target) return false;
    S->goals.clear();
    S->goals.draw_chebyshev(target->pos, static_cast<float>(self().attack_range()));
    QualifierCombatIntent intent;
    intent.purpose = QualifierCombatPurpose::Contest;
    intent.group_index = S->target;
    intent.has_enemy = true;
    intent.enemy = static_cast<std::uint32_t>(static_cast<handle_champion_t>(target->unit));
    intent.allow_escape = false;
    const auto result = qualifier_combat_continue(intent);
    if (result == QualifierCombatResult::NoAction && qualifier_current()) qualifier_step();
    // This contest owns the input after any combat/move attempt, even a newly
    // revealed blocker. It never falls through to another fight or capture walk.
    return true;
}

void qualifier_set_work_goals(const Group& group) {
    S->goals.clear();
    const auto radius = qualifier_runtime.close_target == S->target
        ? 1.0f : static_cast<float>(self().attack_range());
    for (const auto& camp : group.camps) {
        if (camp.health_observation_valid && camp.last_observed_round == S->round && camp.hp <= 0) continue;
        S->goals.draw_chebyshev(camp.pos, radius);
    }
}

#include "qualifier_contact_collect.inc"
int qualifier_inspection_target();
#include "qualifier_contact_execute.inc"
#include "qualifier_inspection.inc"

// Current-prefix completion is separate from whole-fight survival. Visible
// enemies can leave the latter unknown without making a qualified camp hit
// disappear. The existing selected prefix is reused; no tail/ally work is added.
bool qualifier_work_finish_available(int group) {
    if (!qualifier_current() || !self().is_alive() || group < 0
        || group >= static_cast<int>(S->groups.size())) return false;
    const auto& job = S->groups[group];
    if (job.last_observed_round != S->round || job.observed_state != 0) return false;
    const auto* record = qualifier_contact_record(group);
    if (!record || !record->input || !record->neutral_multiplier) return false;
    const auto& input = *record->input;
    const auto& plan = record->plan;
    if (!plan.prefix_executable || !qualifier::contact::matches(input, plan)
        || input.stamp.round != static_cast<std::uint32_t>(S->round)
        || input.stamp.actor != static_cast<std::uint32_t>(static_cast<handle_champion_t>(self()))
        || input.stamp.revision != qualifier_runtime.observation_revision
        || input.stamp.group != static_cast<std::uint32_t>(static_cast<handle_camp_group_t>(job.h))
        || record->origin != self().position() || plan.selected_option < 0
        || plan.selected_option >= static_cast<int>(input.option_count)) return false;
    const auto& option = input.options[plan.selected_option];
    if (!option.damage_estimates_known || !option.prefix_effects_known || !option.future_wake_known) return false;
    bool living = false;
    for (std::size_t i = 0; i < input.member_count; ++i) {
        if (!input.members[i].hp_known || !input.members[i].effects_known
            || !std::isfinite(plan.remaining_hp[i]) || plan.remaining_hp[i] > 0) return false;
        living = living || input.members[i].hp > 0;
    }
    return living && qualifier_current();
}

// A finisher's availability does not erase its opportunity cost. Compare
// relocation/move-consuming prefixes with the actual alternative using the
// same current envelopes, subtracting only this exact group's contribution.
// This is a relative heuristic, not a guarantee about future enemy choices.
bool qualifier_work_finish_preferred(int group, const QualifierMovePlan* alternative,
                                    bool movement_followup) {
    if (!qualifier_work_finish_available(group)) return false;
    const auto* record = qualifier_contact_record(group);
    const auto& plan = record->plan;
    const auto& option = record->input->options[plan.selected_option];
    const auto contact = record->casts[plan.selected_option].contact_position;
    const bool ranged_basic = self().attack_range() > 1 && (plan.pre_basic >= 0 || plan.post_basic >= 0);
    const bool spends_move = ranged_basic || (plan.cast_option >= 0 && option.exclusive_with_movement);
    if (movement_followup && !spends_move && contact == self().position()) return true;
    float after_threat = qualifier_threat(contact);
    float after_unknown = qualifier_unknown_npc_potential(contact);
    for (const auto& npc : qualifier_runtime.npcs) {
        if (npc.group_index != group || cheb(contact, npc.pos) > npc.range) continue;
        if (npc.knowledge == QualifierNpcKnowledge::unknown_potential) after_unknown -= npc.strength;
        else after_threat -= npc.strength;
    }
    after_threat = std::max(0.0f, after_threat);
    after_unknown = std::max(0.0f, after_unknown);
    const float baseline_threat = alternative && alternative->found
        ? alternative->destination_threat : qualifier_threat(self().position());
    const float baseline_unknown = alternative && alternative->found
        ? alternative->destination_unknown : qualifier_unknown_npc_potential(self().position());
    return std::isfinite(after_threat) && std::isfinite(after_unknown)
        && std::isfinite(baseline_threat) && baseline_threat >= 0
        && std::isfinite(baseline_unknown) && baseline_unknown >= 0
        && after_threat <= baseline_threat && after_unknown <= baseline_unknown
        && qualifier_current();
}

QualifierCombatResult qualifier_finish_work_prefix(int group) {
    if (!qualifier_work_finish_available(group)) return QualifierCombatResult::NoAction;
    auto* record = qualifier_contact_record(group);
    const auto result = qualifier_execute_contact(*record, false, 0);
    if (result != QualifierContactExecution::Acted) return QualifierCombatResult::Invalidated;
    if (qualifier_current() && qualifier_contact_group_cleared(group))
        S->goals = qualifier_receipt_goals(group);
    return QualifierCombatResult::Acted;
}

#include "qualifier_upgrade_purpose.inc"

// A nonpositive NEW task is not a continuation decision. Only actual own work
// with uninterrupted fresh lifecycle observations can reach this boundary.
// true owns the input, including a partial/failed physical attempt.
bool qualifier_continue_owned_work() {
    const int index = qualifier_own_work_group();
    if (index < 0 || index != S->target || !qualifier_current()
        || !self().is_alive() || self().is_recalling()
        || self().is_submitting_gems() || self().is_teleporting()) {
        qualifier_own_work_retire(); return false;
    }
    const auto& group = S->groups[index];
    if (group.last_observed_round != S->round) {
        qualifier_own_work_retire(); return false;
    }
    if (group.observed_state == 1 && group.qualifier_capture_round == S->round) {
        const auto status = group.qualifier_capture_status;
        if (status == static_cast<int>(map::camp_group_capture_type::contested)) {
            if (qualifier_contest(group)) return true;
            qualifier_own_work_retire(); return false;
        }
        if (status != static_cast<int>(map::camp_group_capture_type::ally_capturing)
            && status != static_cast<int>(map::camp_group_capture_type::capture_available)) {
            qualifier_own_work_retire(); return false;
        }
        const bool on_tile = qualifier_tiles(group).is_set(self().position());
        if (on_tile) {
            if (!qualifier_contact_capture_valid(index)) { qualifier_recover(); return true; }
            // Current physical occupancy is participation, not a promised tick
            // or receipt. Even a zero local integer share can preserve a team
            // capture. Existing top-level allied countdown has precedence.
            qualifier_runtime.delivery_capture_group = index;
            qualifier_runtime.delivery_receipt_tile = self().position();
            qualifier_publish(index, 2);
            return true;
        }
        // An off-tile old worker adds no capture speed to another holder.
        if (status == static_cast<int>(map::camp_group_capture_type::ally_capturing)) {
            qualifier_own_work_retire(); return false;
        }
        // Finishing the current finite capture is distinct from admitting a
        // second zero-value respawn cycle. Retain only a freshly requalified
        // local region inherited from our actual modelled work prefix.
        if (qualifier_runtime.contact_capture_group == index
            && qualifier_runtime.contact_capture_region
            && qualifier_contact_capture_valid(index)) {
            qualifier_publish(index, 1);
            return qualifier_contact_capture_step(index);
        }
        qualifier_own_work_retire(); return false;
    }
    if (group.observed_state == 0 && qualifier_own_work->accepted_round >= S->round - 1) {
        // Compare the exact current finisher with a fresh legal exit preview.
        // This preview grants no persistent route or work feasibility.
        S->goals = map::spawn_region(team::ally);
        const auto exit = qualifier_plan_step(true, QualifierMovePurpose::Recovery);
        if (!qualifier_current()) return true;
        // This caller ends after the prefix. It cannot use the combat caller's
        // move-preserving fast path, which relies on a fresh follow-up move.
        if (qualifier_work_finish_preferred(index, exit.found ? &exit : nullptr, false)) {
            const auto result = qualifier_finish_work_prefix(index);
            if (result != QualifierCombatResult::NoAction) return true;
        }
    }
    // No unrestricted fight, passive wait or imagined helper renews the old
    // work. Normal fresh cargo/information/recovery decisions may now proceed.
    qualifier_own_work_retire();
    return false;
}

void qualifier_turn() {
    if (!S) qualifier_init();
    auto& s = *S;
    auto& me = self();
#if !QUALIFIER_LEGACY_BACKEND
    // The generic initializer does not execute the old roster classifier.
    // Enable its existing passive-aware Heart of Ruin action valuation.
    cxh_present = true;
#endif
    s.round = static_cast<int>(round_number());
    qualifier_own_work_begin();
    s.moved = s.cast = false;
    ++s.turns;
    qualifier_cooperation_begin();
    QualifierCooperationEndInput cooperation_end_input;
    auto& rt = qualifier_runtime;
    ++rt.observation_revision;
    rt.support_heal_this_input = false;
    rt.last_walk_used_cast_route = false;
    qualifier_stat_request_clear();
    qualifier_task_deliveries.clear(); qualifier_receipt_choices.clear(); qualifier_direct_delivery.reset();
    qualifier_contact_records.clear();
    qualifier_contact_modelled_groups.clear(); qualifier_contact_required_groups.clear();
    if (!me.is_alive()) {
        qualifier_own_work_retire();
        qualifier_opening_retire();
        qualifier_inspection_cancel();
        qualifier_delivery_clear();
        rt.delivery_capture_group = -1; rt.delivery_receipt_tile.reset();
        rt.contact_capture_group = -1; rt.contact_capture_region.reset();
        rt.previous_round = -1; rt.recent_damage = 0;
        s.target = s.qualifier_committed_target = -1;
        s.mode = Mode::farm;
        qualifier_publish(-1, 0); return;
    }
    const float hp = me.health();
    const bool recalling = me.is_recalling(), submitting = me.is_submitting_gems();
    const int channel = recalling ? 1 : submitting ? 2 : 0;
    const int remaining = channel ? me.remaining_channel_time() : 0;
    const auto cargo = me.gem_count();
    const auto position = me.position();
    const float maxhp = me.max_health();
    const bool shielded = me.buffs().has_shield();
    const bool channel_interval = channel != 0 && rt.previous_round == s.round - 1
        && rt.previous_channel == channel && remaining == rt.previous_remaining - 1
        && rt.previous_cargo == cargo && rt.previous_position == position
        && rt.previous_maxhp == maxhp && !shielded && !rt.previous_shielded;
    if (!qualifier_current()) { rt.previous_round = -1; return; }
    rt.recent_damage = rt.previous_round == s.round - 1
        ? std::max(0.0f, rt.previous_hp - hp) : 0;
    rt.previous_round = s.round; rt.previous_hp = hp;
    rt.previous_channel = channel; rt.previous_remaining = remaining;
    rt.previous_cargo = cargo; rt.previous_position = position;
    rt.previous_maxhp = maxhp; rt.previous_shielded = shielded;
    rt.enemies.clear();
    for (const auto& enemy : vision::champions(team::enemy)) {
        if (!enemy.is_visible() || !enemy.is_alive()) continue;
        rt.enemies.push_back({enemy, enemy.position(), enemy.health(), enemy.strength(), enemy.attack_range()});
    }
    if (!qualifier_current()) return;
    qualifier_delivery_refresh();
    if (!qualifier_current() || me.is_teleporting()) return;
    // Recovery and active-channel decisions need fresh NPC state too.
    qualifier_observe();
    if (!qualifier_current()) return;
    qualifier_refresh_npcs();
    if (!qualifier_current()) return;
    qualifier_own_work_observe();
    if (me.is_recalling() || me.is_submitting_gems()) {
        qualifier_own_work_retire();
        // No blanket low-HP cancellation: use actual consecutive-round loss.
        // Spell/stun prediction and fog risk remain unmodelled in this version.
        const bool doomed = channel_interval && remaining > 1 && rt.recent_damage > 0
            && hp <= rt.recent_damage * remaining;
        const bool interrupted_recall = channel_interval && recalling && rt.recent_damage > 0;
        if (!doomed && !interrupted_recall) return;
        try {
            if (me.is_recalling()) me.cancel_recall();
            else me.cancel_submit_gems();
        } catch (const cambc::error&) { return; }
        if (me.is_recalling() || me.is_submitting_gems()) return;
        rt.avoid_channel_until = s.round + 3; // explicit, uncalibrated recovery window
        s.mode = Mode::recall;
    }
    const bool at_spawn = map::spawn_region(team::ally).is_set(me.position());
    if (at_spawn) {
        if (s.mode == Mode::recall && hp < me.max_health()) { qualifier_opening_retire(); return; }
        if (s.mode == Mode::recall) s.mode = Mode::farm;
    }
    if (s.mode == Mode::recall || s.round < rt.avoid_channel_until) {
        qualifier_opening_retire();
        qualifier_recover(); return;
    }
    qualifier_prepare_assignment();
    qualifier_refresh_assignment();
    if (!qualifier_current()) return;
    qualifier_sustain();
    if (!qualifier_current()) return;
    qualifier_opening_refresh();
    if (!qualifier_current()) return;
    const auto pos = me.position();
    const double carried = me.gem_count();
    // Do not turn the NPC envelope into a new blanket retreat rule: an
    // available finishing attack can remove its incoming damage this turn.
    if (rt.recent_damage > 0 && qualifier_enemy_threat(pos) >= hp) {
        qualifier_recover(); return;
    }
    // Finishing cargo and an existing bank commitment retain a last-chance
    // attempt even when the policy's conservative upper ETA exceeds the round
    // horizon. Positive channel passives can shorten that estimate; no timely
    // score is promised. The actual route/resources must still be constructed.
    if (carried > 0 && (s.mode == Mode::bank ||
        global_submitted_gems(team::ally) + carried >= constants::winning_gems)) {
        if (qualifier_bank() == QualifierBankOutcome::Stop) return;
        if (!qualifier_current() || !me.is_alive() || me.is_recalling()
            || me.is_submitting_gems() || me.is_teleporting()) return;
    }
    // Preserve an accepted capture job through its countdown. Extra ordinary
    // bodies are not credited with faster capture in task estimates.
    if (s.target >= 0 && s.target < static_cast<int>(s.groups.size())) {
        const auto& current = s.groups[s.target];
        if (current.last_observed_round == s.round && current.observed_state == 1
            && qualifier_tiles(current).is_set(pos)) {
            // Contact can end on another valid receipt tile. The old forecast
            // is not a committed delivery route: preserve real capture progress,
            // bind its actual origin, and reprice banking from fresh state.
            rt.delivery_capture_group = s.target; rt.delivery_receipt_tile = pos;
            qualifier_direct_delivery.reset();
        }
        if (current.last_observed_round == s.round && current.observed_state == 1
            && !qualifier_contact_capture_valid(s.target)) { qualifier_recover(); return; }
        if (current.last_observed_round == s.round && current.observed_state == 1
            && current.qualifier_capture_round == s.round
            && current.qualifier_capture_status == static_cast<int>(map::camp_group_capture_type::contested)
            && qualifier_contest(current)) return;
        if (current.last_observed_round == s.round && current.observed_state == 1
            && qualifier_tiles(current).is_set(pos)
            && current.qualifier_capture_round == s.round
            && current.qualifier_capture_status == static_cast<int>(map::camp_group_capture_type::ally_capturing)) {
            qualifier_publish(s.target, 2); return;
        }
    }
    s.me_bb.clear(); s.me_bb.set(pos);
    s.df_me.compute(s.terrain, s.me_bb, s.visited);
    if (!qualifier_current()) return;
    if (carried > 0) qualifier_direct_delivery = qualifier_delivery_estimate(pos, carried);
    if (!qualifier_current()) return;
    const double bank_time = qualifier_direct_delivery && qualifier_delivery_usable(*qualifier_direct_delivery)
        ? qualifier_direct_delivery->eta_upper + (qualifier_direct_delivery->purchase_required ? 1.0 : 0.0) : -1.0;
    // Retain the baseline's early gold coefficient, without map-name rules.
    // It is a candidate heuristic, not a calibrated currency conversion.
    constexpr double initial_gold_weight = 0.4; // Baseline coefficient, uncalibrated here.
    const double gold_weight = initial_gold_weight * std::max(0.0, 1.0 - s.round / 400.0);
    // One dated advisory snapshot per planning frame. All physical credit
    // remains observation-backed; the final actor/round gate is checked again.
    const int planning_round = s.round;
    const auto planning_actor = static_cast<std::uint32_t>(static_cast<handle_champion_t>(me));
    const auto planning_nomination = qualifier_cooperation_nomination();
    std::vector<qualifier::Task> tasks;
    tasks.reserve(s.groups.size());
    const auto ally_views = vision::champions(team::ally);
    for (int gi = 0; gi < static_cast<int>(s.groups.size()); ++gi) {
        const auto& g = s.groups[gi];
        double work_hp = 0, monster_damage = 0;
        int distance = UNREACH, delivery = UNREACH;
        vec2 receipt{};
        double receipt_key = std::numeric_limits<double>::infinity();
        bool known_design = !g.camps.empty();
        const bool measured = g.last_observed_round == s.round;
        const bool reward = measured && g.observed_state == 1;
        for (const auto& c : g.camps) {
            known_design = known_design && c.design_known;
            const int approach = s.df_me.get(c.pos), exit = s.df_altar.get(c.pos);
            if (approach != UNREACH) {
                const double key = qualifier_walk_turns(approach)
                    + (exit == UNREACH ? 0.0 : qualifier_walk_turns(exit));
                if (key < receipt_key || (key == receipt_key
                    && (c.pos.y < receipt.y || (c.pos.y == receipt.y && c.pos.x < receipt.x)))) {
                    receipt_key = key; receipt = c.pos; distance = approach; delivery = exit;
                }
            }
            const bool future_spawn = g.observed_state == 2;
            const double health = reward ? 0 : !future_spawn && c.health_observation_valid
                && c.last_observed_round == s.round ? c.hp : c.maxhp;
            work_hp += health;
            if (health > 0) monster_damage += c.str;
        }
        if (!known_design || distance == UNREACH || qualifier_walk_turns(distance) < 0) continue;
        if (reward && qualifier_tiles(g).is_set(pos)) {
            // Preserve a physically accepted capture rather than walking away
            // merely to match an older hypothetical receipt origin.
            receipt = pos; distance = 0; delivery = s.df_altar.get(pos);
        }
        qualifier_receipt_choices.push_back({gi, receipt, delivery != UNREACH});
        int recipients = 1;
        // Only currently visible allies physically on this reward are counted.
        // No stale shared claim is credited as a helper or receipt guarantee.
        if (reward) for (const auto& ally : ally_views) {
            if (static_cast<handle_champion_t>(ally) == static_cast<handle_champion_t>(me)
                || !ally.is_visible() || !ally.is_alive()) continue;
            if (qualifier_tiles(g).is_set(ally.position())) ++recipients;
        }
        // Predict only integer shares. A possible lowest-ID remainder is not
        // credited until the actual final recipient set is known.
        const double reward_gems = g.gems / recipients;
        const double reward_gold = g.gold / recipients;
        const double work = std::ceil(work_hp / std::max(1.0f, me.strength()));
        // Include reaching the actual receipt tile, not just attack range.
        double travel = qualifier_walk_turns(distance);
        if (g.observed_state == 2 && g.last_observed_round >= 0)
            travel = std::max(travel, static_cast<double>(std::max(0, g.respawn_round - s.round)));
        const double capture = reward && g.qualifier_capture_round == s.round && g.qualifier_capture_timer >= 0
            && g.qualifier_capture_status == static_cast<int>(map::camp_group_capture_type::ally_capturing)
            ? g.qualifier_capture_timer : constants::capture_time;
        const double delivery_hint = delivery == UNREACH ? 0.0 : std::max(0.0, qualifier_walk_turns(delivery));
        // Retained basic-only risk screen, not a survival guarantee. Aggregated
        // HP omits separate last hits and progressive deaths; the contact model
        // may refine a bounded fresh case without inventing allied work.
        const bool survives = reward || monster_damage * work < hp;
        const bool ally_timer = reward && g.qualifier_capture_round == s.round
            && g.qualifier_capture_status == static_cast<int>(map::camp_group_capture_type::ally_capturing);
        const bool misses_receipt = ally_timer && travel > g.qualifier_capture_timer;
        const bool contested = reward && g.qualifier_capture_round == s.round
            && g.qualifier_capture_status == static_cast<int>(map::camp_group_capture_type::contested);
        bool visible_contest = false;
        if (contested) for (const auto& enemy : rt.enemies)
            if (qualifier_tiles(g).is_set(enemy.pos)) visible_contest = true;
        tasks.push_back({gi, reward_gems, reward_gold, travel, work, capture,
                         delivery_hint, 0.0, 0.0,
                         survives && !misses_receipt && (!contested || visible_contest),
                         qualifier_ordinary_preference(gi, carried, planning_nomination), false});
    }
    if (!qualifier_current()) return;
    const qualifier::Context context{carried, gold_weight, bank_time,
        s.qualifier_committed_target, std::max(0.0, static_cast<double>(constants::max_rounds) - s.round),
        qualifier_direct_delivery && qualifier_direct_delivery->purchase_required
            ? static_cast<double>(qualifier_direct_delivery->crystal_cost) : 0.0};
    qualifier_prepare_deliveries(tasks, context, planning_nomination);
    if (!qualifier_current()) return;
    qualifier_contact_refine_tasks(tasks, context, planning_nomination);
    if (!qualifier_current() || s.round != planning_round
        || static_cast<std::uint32_t>(static_cast<handle_champion_t>(me)) != planning_actor) return;
    auto decision = qualifier::choose(context, tasks, qualifier::Admission::PositiveEconomic);
    if (decision.bank) {
        qualifier_inspection_cancel();
        s.target = s.qualifier_committed_target = -1;
        qualifier_bank(); return;
    }
    // Future receipt utility remains a separate conditional purpose. It never
    // becomes current gold-task gems or an executable future contact Plan.
    const auto purpose = qualifier_upgrade_purpose(tasks, context, decision);
    if (!qualifier_current() || s.round != planning_round
        || static_cast<std::uint32_t>(static_cast<handle_champion_t>(me)) != planning_actor) return;
    if (purpose.return_shop || purpose.funding_task >= 0) {
        if (purpose.actor != planning_actor || purpose.round != planning_round
            || purpose.revision != rt.observation_revision || purpose.origin != me.position()
            || !purpose.counterfactual || purpose.grants_live_feasibility
            || !std::isfinite(purpose.rate) || purpose.rate <= 0 || purpose.rate <= decision.rate) return;
        if (purpose.return_shop && (purpose.funding_task >= 0
            || purpose.status != QualifierUpgradePurpose::Status::AlreadyAffordable
            || !map::spawn_region(team::ally).is_set(purpose.shop_tile))) return;
        if (purpose.funding_task >= 0 && (purpose.status != QualifierUpgradePurpose::Status::FundOneReceipt
            || purpose.funding_task >= static_cast<int>(s.groups.size())
            || purpose.funding_group != static_cast<std::uint32_t>(static_cast<handle_camp_group_t>(s.groups[purpose.funding_task].h)))) return;
    }
    const int selected_task = purpose.return_shop ? -1
        : purpose.funding_task >= 0 ? purpose.funding_task : decision.task;
    // A conservative horizon is not proof that actual cargo is worthless.
    // Recovery itself does not bank before returning to spawn.
    if (selected_task < 0 && carried > 0) {
        if (qualifier_bank() == QualifierBankOutcome::Stop) return;
        if (!qualifier_current() || !me.is_alive() || me.is_recalling()
            || me.is_submitting_gems() || me.is_teleporting()) return;
    }
    const int owned = qualifier_own_work_group();
    if (owned >= 0 && owned != selected_task) {
        bool priced = false;
        for (const auto& task : tasks) if (task.id == owned) {
            auto own_context = context;
            own_context.bank_turns = -1; // Positivity is not a bank competition.
            const auto own = qualifier::choose(own_context, std::vector<qualifier::Task>{task},
                qualifier::Admission::PositiveEconomic);
            priced = own.task == owned;
            break;
        }
        if (!priced && qualifier_continue_owned_work()) return;
    }
    if (purpose.return_shop) {
        qualifier_own_work_retire();
        qualifier_inspection_cancel();
        s.target = s.qualifier_committed_target = -1;
        rt.contact_capture_group = rt.delivery_capture_group = -1;
        rt.contact_capture_region.reset(); rt.delivery_receipt_tile.reset();
        // The quote is conditional on future state. Reprice after each actual
        // step and purchase; never turn a purpose into a sticky recovery mode.
        if (at_spawn) {
            qualifier_prepare_stat_request(-1, rt.support_heal_this_input
                ? QualifierStatPurpose::Support : QualifierStatPurpose::IndependentWork);
            if (qualifier_shop()) {
                ++rt.observation_revision; qualifier_contact_records.clear();
            }
            return;
        }
        s.mode = Mode::farm;
        s.goals.clear(); s.goals.set(purpose.shop_tile);
        QualifierCombatIntent intent;
        intent.purpose = QualifierCombatPurpose::Recovery;
        intent.allow_escape = true;
        if (qualifier_combat_continue(intent) != QualifierCombatResult::NoAction) return;
        qualifier_step(true, QualifierMovePurpose::Recovery); return;
    }
    if (purpose.funding_task >= 0) decision = {purpose.funding_task, false, purpose.rate};
    if (decision.task < 0) {
        // A required stat purchase can make work viable next input. Do not
        // strand an upgradeable body at home merely because no task is yet
        // admitted. No selected delivery dependency exists on this path.
        if (at_spawn) qualifier_prepare_stat_request(-1, rt.support_heal_this_input
            ? QualifierStatPurpose::Support : QualifierStatPurpose::IndependentWork);
        if (at_spawn && qualifier_shop()) {
            qualifier_own_work_retire();
            ++rt.observation_revision; qualifier_contact_records.clear(); return;
        }
        if (qualifier_inspect(tasks, context)) return;
        qualifier_recover(); return;
    }
    qualifier_inspection_cancel();
    if (at_spawn) qualifier_prepare_stat_request(decision.task, rt.support_heal_this_input
        ? QualifierStatPurpose::Support : QualifierStatPurpose::IndependentWork);
    const auto* selected_delivery = qualifier_task_delivery(decision.task);
    bool purchased = false;
    if (selected_delivery && selected_delivery->plan.purchase_required && !selected_delivery->plan.resource_backed) {
        purchased = qualifier_delivery_buy_required(selected_delivery->plan);
        if (!purchased) return; // No action may retain an unfulfilled dependency.
    }
    if (at_spawn) purchased = qualifier_shop() || purchased;
    if (purchased) {
        qualifier_own_work_retire();
        ++rt.observation_revision;
        qualifier_contact_records.clear(); qualifier_direct_delivery.reset();
        return; // Reobserve changed inventory/HP/Strength next input.
    }
    s.mode = Mode::farm;
    if (s.target != decision.task) {
        qualifier_own_work_retire();
        rt.contact_capture_group = -1; rt.contact_capture_region.reset();
        rt.delivery_capture_group = -1; rt.delivery_receipt_tile.reset();
    }
    for (const auto& choice : qualifier_receipt_choices) if (choice.group == decision.task) {
        rt.delivery_capture_group = decision.task; rt.delivery_receipt_tile = choice.receipt;
    }
    s.target = s.qualifier_committed_target = decision.task;
    qualifier_opening_retire(); // One actual assignment consumes this deployment prior.
    auto& group = s.groups[s.target];
    const bool reward = group.last_observed_round == s.round && group.observed_state == 1;
    if (reward) {
        if (!qualifier_contact_capture_valid(s.target)) { qualifier_recover(); return; }
        if (group.qualifier_capture_round == s.round
            && group.qualifier_capture_status == static_cast<int>(map::camp_group_capture_type::contested)
            && qualifier_contest(group)) return;
        if (qualifier_tiles(group).is_set(pos)) { qualifier_publish(s.target, 2); return; }
        qualifier_publish(s.target, 1);
        if (qualifier_contact_capture_step(s.target)) return;
        s.goals = qualifier_receipt_goals(s.target); qualifier_step(); return;
    }
    qualifier_publish(s.target, 1);
    if (group.observed_state == 2 &&
        (group.last_observed_round == s.round || group.respawn_round > s.round)) {
        // Any occupied spawn delays the whole group's respawn. Wait outside
        // the region, including actively moving out when already inside it.
        s.goals.clear();
        for (const auto& camp : group.camps)
            s.goals.draw_chebyshev(camp.pos, static_cast<float>(std::max(1, s.range)));
        s.goals &= ~qualifier_tiles(group);
        qualifier_step(); return;
    }
    auto* contact = qualifier_contact_record(s.target);
    const bool modelled = contact && qualifier_contact_expansion_ok(*contact);
    if (modelled) {
        rt.contact_capture_group = s.target;
        rt.contact_capture_region = contact->capture_region;
    } else { rt.contact_capture_group = -1; rt.contact_capture_region.reset(); }
    // The declared work goal exists before combat is compared with movement.
    // A qualified contact plan already excludes visible enemy interference.
    qualifier_set_work_goals(group);
    if (!rt.enemies.empty()) {
        QualifierCombatIntent intent;
        intent.purpose = QualifierCombatPurpose::Work;
        intent.group_index = s.target;
        intent.allow_escape = qualifier_enemy_threat(pos) > 0;
        if (qualifier_combat_continue(intent) != QualifierCombatResult::NoAction) return;
    }
    if (!qualifier_current()) return;
    if (contact) {
        const auto execution = qualifier_execute_contact(*contact, modelled);
        if (execution == QualifierContactExecution::Acted) return;
        if (execution == QualifierContactExecution::Invalidated || modelled
            || qualifier_contact_requires_model(s.target)) {
            if (qualifier_current()) qualifier_recover();
            return;
        }
    }
    if (group.last_observed_round == s.round && group.observed_state == 0 && fight(group)) return;
    qualifier_set_work_goals(group);
    if (!qualifier_step()) {
        // A ranged goal can lie beyond a wall or bush. Keep closing intent so
        // next turn does not undo the approach to a still-hidden camp.
        rt.close_target = s.target;
        s.goals.clear();
        for (const auto& camp : group.camps) s.goals.draw_chebyshev(camp.pos, 1.0f);
        qualifier_step();
    }
}
