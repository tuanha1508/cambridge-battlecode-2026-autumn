// A few current, task-bound continuations. No future enemy-cast simulator.
enum class QualifierCombatPurpose { Work, Contest, Delivery, Recovery };
struct QualifierCombatIntent {
    QualifierCombatPurpose purpose = QualifierCombatPurpose::Work;
    int group_index = -1;
    bool has_enemy = false;
    std::uint32_t enemy = 0;
    bool allow_escape = false;
};
enum class QualifierCombatResult { NoAction, Acted, Invalidated };
bool qualifier_work_finish_available(int);
bool qualifier_work_finish_preferred(int, const QualifierMovePlan*, bool movement_followup = true);
QualifierCombatResult qualifier_finish_work_prefix(int);
bitboard qualifier_receipt_goals(int);

namespace qualifier_combat_detail {
bool live() {
    const auto& me = self();
    return qualifier_current() && me.is_alive() && !me.is_recalling()
        && !me.is_submitting_gems() && !me.is_teleporting();
}
bool nonnegative(double value) { return std::isfinite(value) && value >= 0; }
std::uint32_t identity(const champion& unit) {
    return static_cast<std::uint32_t>(static_cast<handle_champion_t>(unit));
}
std::optional<champion> enemy(std::uint32_t id) {
    for (const auto& entry : qualifier_runtime.enemies)
        if (identity(entry.unit) == id && entry.unit.is_visible() && entry.unit.is_alive()) return entry.unit;
    return {};
}
bool improves(const QualifierMovePlan& move) {
    if (!move.found || !nonnegative(move.origin_threat) || !nonnegative(move.destination_threat)
        || !nonnegative(move.origin_unknown) || !nonnegative(move.destination_unknown)) return false;
    if (move.purpose != QualifierMovePurpose::Ordinary && move.advice_used
        && (!nonnegative(move.origin_score) || !nonnegative(move.destination_score)
            || move.destination_score >= move.origin_score)) return false;
    if (move.purpose != QualifierMovePurpose::Ordinary && move.avoid_route)
        return move.advice_used && move.neighborhood_complete
            && move.destination_route_distance != UNREACH
            && move.origin_route_distance != UNREACH
            && move.destination_route_distance < move.origin_route_distance;
    const bool no_worse = move.destination_threat <= move.origin_threat
        && move.destination_unknown <= move.origin_unknown;
    const bool progress = move.destination_distance != UNREACH
        && (move.origin_distance == UNREACH || move.destination_distance < move.origin_distance);
    return no_worse && (progress || move.destination_threat < move.origin_threat
        || move.destination_unknown < move.origin_unknown);
}
bool relevant(const QualifierCombatIntent& intent, const champion& unit, const QualifierMovePlan& move) {
    const auto id = identity(unit);
    if (intent.has_enemy) return id == intent.enemy;
    if (intent.purpose == QualifierCombatPurpose::Contest) return false;
    double span = unit.attack_range() + 1;
    // Geometry describes a possible local obstruction, not a promised cast or DPS.
    for (const auto& ability : unit.abilities()) if (ability.info())
        std::visit([&](const auto& metadata) {
            using T = std::decay_t<decltype(metadata)>;
            if constexpr (std::is_same_v<T, ability_damage> || std::is_same_v<T, ability_cc>)
                span = std::max(span, static_cast<double>(metadata.range));
            else if constexpr (std::is_same_v<T, ability_aoe_damage> || std::is_same_v<T, ability_dash>)
                span = std::max(span, static_cast<double>(metadata.range + metadata.radius));
        }, *ability.info());
    const auto p = unit.position();
    if (intent.purpose == QualifierCombatPurpose::Work && intent.group_index >= 0
        && intent.group_index < static_cast<int>(S->groups.size())
        && qualifier_tiles(S->groups[intent.group_index]).is_set(p)) return true;
    return cheb(self().position(), p) <= span || (move.found && cheb(move.destination, p) <= span);
}
// A missing contact record is not proof that ordinary camp work is absent.
// Unknown current camp queries conservatively reserve the basic for this job.
bool reserve_work_basic(const QualifierCombatIntent& intent) {
    if (intent.purpose != QualifierCombatPurpose::Work || intent.group_index < 0
        || intent.group_index >= static_cast<int>(S->groups.size()) || !self().can_attack()) return false;
    const auto& group = S->groups[intent.group_index];
    if (group.camps.size() > 16) return true;
    for (const auto& member : group.camps) {
        try {
            if (member.h.state().is_dead()) continue;
            const auto hp = member.h.health();
            if (!nonnegative(hp)) return true;
            if (hp > 0 && cheb(self().position(), member.h.position()) <= self().attack_range()) return true;
        } catch (const std::exception&) { return true; }
    }
    return false;
}
bool receipt_obstruction(const QualifierCombatIntent& intent, const champion& unit) {
    return intent.group_index >= 0 && intent.group_index < static_cast<int>(S->groups.size())
        && qualifier_tiles(S->groups[intent.group_index]).is_set(unit.position());
}
bool immediate_area_hit(champion_type type, int slot) {
    // Released current direct hits, not a generic duration-to-tick inference.
    return (type == champion_type::ryth && slot == 0)
        || (type == champion_type::prism && slot == 1)
        || (type == champion_type::stellar && slot == 1);
}
struct Collateral {
    int round = -1;
    std::uint64_t revision = 0;
    std::vector<vec2> rens, possible_wakes;
    bool complete = false;
};
Collateral collateral;
bool collateral_risk(vec2 center, double radius) {
    if (collateral.round != S->round || collateral.revision != qualifier_runtime.observation_revision) {
        collateral = {}; collateral.round = S->round;
        collateral.revision = qualifier_runtime.observation_revision;
        for (const auto& entry : qualifier_runtime.enemies)
            if (entry.unit.champion_name() == champion_type::ren) collateral.rens.push_back(entry.pos);
        std::size_t scanned = 0;
        collateral.complete = true;
        for (const auto& group : S->groups) {
            if (group.last_observed_round == S->round && group.observed_state != 0) continue;
            bool group_harmless = true, possible_attacker_wake = false;
            for (const auto& camp : group.camps) {
                if (++scanned > 4096 || (scanned % 64 == 0 && fuel_remaining() < 12000)) {
                    collateral.complete = false; break;
                }
                group_harmless = group_harmless && camp.design_known && camp.str <= 0;
                if (camp.health_observation_valid && camp.last_observed_round == S->round && camp.hp <= 0) continue;
                if ((!camp.design_known || camp.str > 0)
                    && (camp.state_observed_round != S->round || !camp.observed_awake)) possible_attacker_wake = true;
            }
            if (!collateral.complete) break;
            if (!group_harmless && possible_attacker_wake) for (const auto& camp : group.camps) {
                // Hitting even a harmless living member can wake its attacking
                // siblings. Only a completely known harmless group is exempt.
                if (!(camp.health_observation_valid && camp.last_observed_round == S->round && camp.hp <= 0))
                    collateral.possible_wakes.push_back(camp.pos);
            }
        }
    }
    if (!collateral.complete) return true;
    for (const auto p : collateral.rens) if (cheb(center, p) <= radius) return true;
    for (const auto p : collateral.possible_wakes) if (cheb(center, p) <= radius) return true;
    return false;
}
enum class Target { Self, Unit, Ground };
struct Cast {
    int slot = -1;
    Target target = Target::Self;
    std::uint32_t enemy = 0;
    vec2 tile{};
    double value = 0;
    bool finisher = false, reset = false, dash = false;
};
bool ready(const ability& ability) {
    if (!ability.info() || S->cast || !self().can_cast() || ability.cooldown() != 0) return false;
    const auto properties = std::visit([](const auto& value) { return static_cast<ability_properties>(value); }, *ability.info());
    return !(S->moved && properties.exclusive_with_movement);
}
double range(const ability& ability) {
    return std::visit([](const auto& metadata) -> double {
        if constexpr (requires { metadata.range; }) return metadata.range;
        else return 0;
    }, *ability.info());
}
bool refresh_after_attempt() {
    ++qualifier_runtime.observation_revision;
    return qualifier_refresh_world() && live();
}
} // namespace qualifier_combat_detail

QualifierCombatResult qualifier_combat_continue(const QualifierCombatIntent& intent) {
    namespace d = qualifier_combat_detail;
    bool attempted = false;
    if (!S) return QualifierCombatResult::NoAction;
    bitboard job_goals = S->goals;
    auto done = [&](QualifierCombatResult result) {
        S->goals = job_goals;
        return result;
    };
    try {
        if (!d::live()) return done(QualifierCombatResult::Invalidated);
        if (intent.purpose == QualifierCombatPurpose::Contest && !intent.has_enemy)
            return done(QualifierCombatResult::NoAction);
        if (qualifier_runtime.enemies.size() > 16) return done(QualifierCombatResult::Invalidated);
        auto& me = self();
        const auto type = me.champion_name();
        auto preview = [&]() {
            S->goals = job_goals;
            const auto purpose = intent.purpose == QualifierCombatPurpose::Delivery ? QualifierMovePurpose::Delivery
                : intent.purpose == QualifierCombatPurpose::Recovery ? QualifierMovePurpose::Recovery
                : QualifierMovePurpose::Ordinary;
            return qualifier_plan_step(intent.allow_escape, purpose);
        };
        auto move = preview();
        bool improving_move = d::improves(move);
        const int preview_target = S->target;
        const int preview_commitment = S->qualifier_committed_target;
        // This preview is local to this continuation. Never carry it through
        // done()/NoAction into a caller with a different preparation policy.
        auto original_preview_current = [&]() {
            if (attempted || !improving_move || !move.found || !d::live()
                || move.actor != d::identity(me) || move.round != S->round
                || move.revision != qualifier_runtime.observation_revision
                || move.origin != me.position() || move.allow_escape != intent.allow_escape
                || move.prepared_allow_goal_exit != intent.allow_escape
                || move.field_generation != qualifier_step_field_generation
                || move.prepared_allow_goal_exit != qualifier_step_allow_goal_exit
                || move.purpose != qualifier_step_field_purpose
                || move.neighborhood_complete != qualifier_step_neighborhood_complete
                || !(move.goals == S->goals) || !(move.terrain == S->terrain)
                || !qualifier_step_declared_goals || !(*qualifier_step_declared_goals == job_goals)
                || S->target != preview_target || S->qualifier_committed_target != preview_commitment) return false;
            if (move.advice_used && qualifier_enemy_advice_detail::runtime.frame.serial != move.advice_serial)
                return false;
            if (move.avoid_route) {
                const auto& route = qualifier_cast_route_detail::runtime;
                if (!route || !route->view || !route->view->admissible
                    || route->view->generation != move.cast_route_generation
                    || route->view->frame_serial != move.advice_serial) return false;
            }
            return true;
        };
        std::vector<std::uint32_t> targets;
        for (const auto& entry : qualifier_runtime.enemies)
            if (d::relevant(intent, entry.unit, move)) targets.push_back(d::identity(entry.unit));
        std::sort(targets.begin(), targets.end());
        const bool protective = intent.purpose == QualifierCombatPurpose::Delivery
            || intent.purpose == QualifierCombatPurpose::Recovery;
        const auto* spell_frame = protective ? qualifier_enemy_advice_detail::obtain() : nullptr;
        const auto move_purpose = intent.purpose == QualifierCombatPurpose::Delivery ? QualifierMovePurpose::Delivery
            : intent.purpose == QualifierCombatPurpose::Recovery ? QualifierMovePurpose::Recovery : QualifierMovePurpose::Ordinary;
        const bool spell_qualified = spell_frame && qualifier_enemy_advice_detail::numeric_complete(*spell_frame)
            && !move.cast_steering_suppressed && qualifier_cast_route_spell_steering_allowed(
                move_purpose, intent.allow_escape ? 100.0 : 10.0, *spell_frame, job_goals);
        const auto current_spell = spell_qualified
            ? qualifier_enemy_advice_detail::project(*spell_frame, me.position()) : QualifierEnemyAdvice{};
        const bool npc_concern = qualifier_unknown_npc_potential(me.position()) > 0
            || qualifier_runtime.unresolved_awake_npc
            || std::any_of(qualifier_runtime.npcs.begin(), qualifier_runtime.npcs.end(), [&](const auto& npc) {
                return cheb(me.position(), npc.pos) <= npc.range && npc.strength > 0;
            });
        const bool observed_concern = qualifier_threat(me.position()) > 0 || npc_concern
            || (protective && spell_qualified && (current_spell.cast_reference > 0 || move.route_reference > 0));
        const bool job_purpose = intent.purpose == QualifierCombatPurpose::Work
            || intent.purpose == QualifierCombatPurpose::Contest;
        const bool finishing_work = job_purpose && npc_concern
            && qualifier_work_finish_available(intent.group_index);
        const bool reserve_camp_basic = d::reserve_work_basic(intent);
        auto optional_target = [&](const champion& unit) {
            return !reserve_camp_basic || d::receipt_obstruction(intent, unit);
        };
        auto adopt_receipt_after_enemy_removal = [&](std::uint32_t id) {
            if (!job_purpose || d::enemy(id) || intent.group_index < 0
                || intent.group_index >= static_cast<int>(S->groups.size())) return;
            const auto& group = S->groups[intent.group_index];
            if (group.last_observed_round == S->round && group.observed_state == 1)
                job_goals = qualifier_receipt_goals(intent.group_index);
        };
        if (targets.empty() && (!protective || !observed_concern) && !finishing_work)
            return done(QualifierCombatResult::NoAction);

        auto continue_move = [&]() {
            if (!d::live()) return QualifierCombatResult::Invalidated;
            if (me.can_move()) {
                // Reuse only the owned, still-current, unattempted improving
                // preview. Any action or changed job/field needs a new one.
                if (original_preview_current()) {
                    attempted = true;
                    return qualifier_execute_step(move) ? QualifierCombatResult::Acted : QualifierCombatResult::Invalidated;
                }
                auto next = preview();
                if (d::improves(next)) {
                    attempted = true;
                    return qualifier_execute_step(next) ? QualifierCombatResult::Acted : QualifierCombatResult::Invalidated;
                }
            }
            return attempted ? QualifierCombatResult::Acted : QualifierCombatResult::NoAction;
        };
        auto attack = [&](std::uint32_t target_id) {
            const auto target = d::enemy(target_id);
            if (!target || !d::live() || !me.can_attack()
                || cheb(me.position(), target->position()) > me.attack_range()) return false;
            attempted = true;
            me.attack(*target); ++S->attacks;
            if (!d::refresh_after_attempt()) return false;
            adopt_receipt_after_enemy_removal(target_id);
            return true;
        };
        auto cast = [&](const d::Cast& choice) {
            if (!d::live()) return false;
            const auto ability = me.abilities()[choice.slot];
            if (!d::ready(ability)) return false;
            std::optional<champion> target;
            if (choice.target != d::Target::Self && !choice.dash) {
                target = d::enemy(choice.enemy);
                if (!target || target->position() != choice.tile
                    || cheb(me.position(), choice.tile) > d::range(ability)) return false;
                if (choice.finisher) {
                    const double hp = target->health(), loss = ability.damage_amount(*target);
                    if (!d::nonnegative(hp) || hp <= 0 || !d::nonnegative(loss) || loss < hp) return false;
                }
            }
            attempted = true;
            const bool accepted = choice.target == d::Target::Self ? ability.use()
                : choice.target == d::Target::Unit ? ability.use(*target) : ability.use(choice.tile);
            if (!accepted) { d::refresh_after_attempt(); return false; }
            S->cast = true; ++S->casts;
            if (choice.dash && me.position() != choice.tile) { d::refresh_after_attempt(); return false; }
            // Dash is not an ordinary walk. Fresh can_move, not S->moved alone,
            // decides whether a compatible normal step remains.
            if (!d::refresh_after_attempt()) return false;
            if (choice.target != d::Target::Self && !choice.dash)
                adopt_receipt_after_enemy_removal(choice.enemy);
            return true;
        };

        d::Cast finisher, offense, shield, control;
        double target_hp = std::numeric_limits<double>::infinity();
        std::optional<std::uint32_t> basic_target;
        for (const auto id : targets) {
            const auto target = d::enemy(id);
            if (!target) continue;
            const auto position = target->position();
            const double hp = target->health();
            if (!d::nonnegative(hp) || hp <= 0) continue;
            if (optional_target(*target) && cheb(me.position(), position) <= me.attack_range() && hp < target_hp) {
                target_hp = hp; basic_target = id;
            }
            for (int slot = 0; slot < 2; ++slot) {
                const auto ability = me.abilities()[slot];
                if (!d::ready(ability) || ability.is_dash() || ability.is_attack_reset()
                    || ability.is_shield() || ability.is_heal()) continue;
                const bool ground = ability.is_aoe_damage() && ability.can_target_ground();
                if (!ground && !ability.can_target_enemies()) continue;
                const double range = d::range(ability);
                if (!d::nonnegative(range) || cheb(me.position(), position) > range) continue;
                const double loss = ability.damage_amount(*target);
                if (!d::nonnegative(loss)) continue;
                double radius = 0;
                if (ground) radius = ability.get<ability_aoe_damage>().radius;
                if (!d::nonnegative(radius)) continue;
                d::Cast candidate{slot, ground ? d::Target::Ground : d::Target::Unit, id, position, loss};
                const bool retaliation_or_wake = d::collateral_risk(position, radius);
                // One target-aware current hit only. No summed slots/basic,
                // raw-bonus multiplication, Ren-safe trade or future ticks.
                candidate.finisher = loss >= hp && !retaliation_or_wake
                    && (!ability.is_aoe_damage() || d::immediate_area_hit(type, slot));
                if (candidate.finisher && (finisher.slot < 0 || loss > finisher.value)) finisher = candidate;
                if (optional_target(*target) && loss > offense.value) offense = candidate;
                const auto properties = std::visit([](const auto& value) { return static_cast<ability_properties>(value); }, *ability.info());
                if (ability.is_cc() && ability.get<ability_cc>().duration > 0
                    && (loss > 0 || d::receipt_obstruction(intent, *target))
                    && target->champion_name() != champion_type::marcella && !retaliation_or_wake
                    && (!improving_move || !properties.exclusive_with_movement)
                    && control.slot < 0) control = candidate;
            }
        }
        if (finisher.slot >= 0) {
            if (!cast(finisher)) return done(QualifierCombatResult::Invalidated);
            return done(continue_move());
        }
        if (finishing_work && qualifier_work_finish_preferred(intent.group_index, improving_move ? &move : nullptr)) {
            attempted = true;
            const auto result = qualifier_finish_work_prefix(intent.group_index);
            if (result != QualifierCombatResult::Acted) return done(QualifierCombatResult::Invalidated);
            // Root sets actual receipt roots only after a confirmed clear.
            job_goals = S->goals;
            return done(continue_move());
        }

        // A compatible protective cast can cover progress without spending its
        // ordinary move. Existing and prospective shields are never summed.
        if (observed_concern || !targets.empty()) {
            const bool existing_shield = me.buffs().has_shield();
            for (int slot = 0; slot < 2 && !existing_shield; ++slot) {
                const auto ability = me.abilities()[slot];
                if (!d::ready(ability) || !ability.is_shield() || !ability.can_target_self()) continue;
                const auto properties = ability.get<ability_shield>();
                if (properties.duration <= 0 || (improving_move && properties.exclusive_with_movement)) continue;
                const double amount = ability.shield_amount(me);
                if (d::nonnegative(amount) && amount > shield.value)
                    shield = {slot, d::Target::Self, 0, me.position(), amount};
            }
        }
        if (improving_move) {
            if (shield.slot >= 0 || control.slot >= 0) {
                if (!cast(shield.slot >= 0 ? shield : control)) return done(QualifierCombatResult::Invalidated);
                return done(continue_move());
            }
            // Optional ranged offense would consume this useful job/exit move.
            // A melee basic can retain it, except unqualified collateral risk.
            if (basic_target && me.can_attack() && me.attack_range() <= 1) {
                const auto target = d::enemy(*basic_target);
                const double radius = type == champion_type::varg ? constants::passive_abilities::varg_splash_radius : 0;
                if (target && !d::collateral_risk(target->position(), radius)) {
                    if (!attack(*basic_target)) return done(QualifierCombatResult::Invalidated);
                    return done(continue_move());
                }
            }
            return done(continue_move());
        }

        // A bounded defensive dash is another current continuation, not a
        // promised safe route or credit for its landing damage/control.
        if (!S->cast && me.can_cast() && observed_concern && fuel_remaining() >= 18000) {
            std::array<int, 2> dash_slots{};
            int dash_count = 0;
            for (int slot = 0; slot < 2; ++slot) {
                const auto ability = me.abilities()[slot];
                if (!d::ready(ability) || !ability.is_dash() || !ability.can_target_ground()) continue;
                const auto properties = ability.get<ability_dash>();
                if (!d::nonnegative(properties.range) || properties.range <= 0 || properties.range > 8
                    || !d::nonnegative(properties.radius)) continue;
                dash_slots[dash_count++] = slot;
            }
            // No typed candidate skips this search only. Later protection and
            // ordinary combat continuation remain available.
            if (dash_count > 0) {
                struct Landing { int slot = -1; vec2 tile{}; float threat = 0, unknown = 0; double score = 0; };
                std::array<Landing, 2> landings{};
                int landing_count = 0;
                const auto start = me.position();
                // This field is independent of ordinary walking permissions and
                // S->df_go. Truncated/unreachable distances cannot certify progress.
                std::optional<distance_field> job_field;
                std::optional<distance_field> cast_job_field;
                int origin_distance = UNREACH;
                if (!intent.allow_escape || protective) {
                    bitboard topology = S->terrain;
                    if (protective) topology |= vision::champion_tiles() | vision::camp_tiles() | vision::teleport_tiles();
                    topology.set(start, false);
                    bitboard roots = job_goals & ~topology, visited;
                    if (!roots.is_empty()) {
                        job_field.emplace();
                        job_field->compute(topology, roots, visited, 64);
                        origin_distance = job_field->get(start);
                        if (protective && spell_qualified && qualifier_cast_route_reserve_field()) {
                            const auto hazards = qualifier_enemy_advice_detail::positive_envelopes(*spell_frame);
                            const auto cast_roots = roots & ~hazards;
                            if (!cast_roots.is_empty()) {
                                cast_job_field.emplace();
                                // Independent landing-to-job walking field. A dash
                                // may cross holes; it never borrows a walking mask.
                                cast_job_field->compute(topology | hazards, cast_roots, visited, 64);
                            }
                        }
                    }
                }
                if (!d::live()) return done(QualifierCombatResult::Invalidated);
                const float current_threat = qualifier_threat(start), current_unknown = qualifier_unknown_npc_potential(start);
                const double scale = intent.allow_escape ? 100.0 : 10.0;
                const double health = std::max(1.0f, me.health());
                const bool scored = protective && spell_qualified && origin_distance != UNREACH && cast_job_field;
                const double origin_score = move.avoid_route ? move.origin_score
                    : origin_distance + scale * current_threat / health
                        + (scored ? scale * current_spell.cast_reference / health : 0);
                for (int dash_index = 0; dash_index < dash_count; ++dash_index) {
                    const int slot = dash_slots[dash_index];
                    const auto ability = me.abilities()[slot];
                    if (!d::ready(ability) || !ability.is_dash() || !ability.can_target_ground()) continue;
                    const auto properties = ability.get<ability_dash>();
                    if (!d::nonnegative(properties.range) || properties.range <= 0 || properties.range > 8
                        || !d::nonnegative(properties.radius)) continue;
                    auto path_blocked = map::walls() | map::altars() | map::spawn_region(team::enemy)
                        | vision::champion_tiles() | vision::camp_tiles() | vision::teleport_tiles();
                    path_blocked.set(start, false);
                    const auto landing_blocked = path_blocked | map::holes();
                    // Holes may be crossed, but are not legal landing tiles. Walls
                    // and observed occupants block this conservative path subset.
                    for (const auto p : path_blocked.reachable_tiles(start, static_cast<std::uint16_t>(properties.range)).positions()) {
                        if (p == start || cheb(start, p) > properties.range || landing_blocked.is_set(p)) continue;
                        if (!intent.allow_escape) {
                            if (!job_field || origin_distance == UNREACH) continue;
                            const auto distance = job_field->get(p);
                            if (distance == UNREACH || (!scored && distance >= origin_distance)) continue;
                        }
                        const float threat = qualifier_threat(p), unknown = qualifier_unknown_npc_potential(p);
                        if (!d::nonnegative(threat) || !d::nonnegative(unknown) || threat > current_threat
                            || unknown > current_unknown) continue;
                        double score = threat;
                        bool improved = threat < current_threat || unknown < current_unknown;
                        if (scored) {
                            const auto advice = qualifier_enemy_advice_detail::project(*spell_frame, p);
                            if (!advice.available || advice.unknown > 0 || !job_field) continue;
                            int distance = job_field->get(p);
                            if (distance == UNREACH) continue;
                            bool qualified_avoid_landing = false;
                            if (cast_job_field && advice.cast_reference == 0) {
                                const int avoiding = cast_job_field->get(p);
                                const double budget = scale * std::max(current_spell.cast_reference, move.route_reference) / health;
                                if (avoiding != UNREACH && avoiding >= distance && avoiding - distance <= budget) {
                                    distance = avoiding;
                                    qualified_avoid_landing = true;
                                }
                            }
                            // Compare like route valuations. A missing/too-long
                            // avoid landing cannot borrow an avoid-priced origin.
                            if (!qualified_avoid_landing) continue;
                            // Separate cost terms, never a summed damage/burst proof.
                            score = distance + scale * threat / health + scale * advice.cast_reference / health;
                            improved = score < origin_score;
                        }
                        if (!improved) continue;
                        Landing candidate{slot, p, threat, unknown, score};
                        int at = landing_count < 2 ? landing_count++ : 1;
                        if (at == 1 && landings[at].slot >= 0
                            && (scored ? score >= landings[at].score
                                : (threat > landings[at].threat || (threat == landings[at].threat && unknown >= landings[at].unknown)))) continue;
                        landings[at] = candidate;
                        if (at == 1 && (scored ? landings[1].score < landings[0].score
                            : (landings[1].threat < landings[0].threat
                                || (landings[1].threat == landings[0].threat && landings[1].unknown < landings[0].unknown))))
                            std::swap(landings[0], landings[1]);
                    }
                }
                if (landing_count > 0) {
                    for (int selected = 0; selected < landing_count; ++selected) {
                        const auto properties = me.abilities()[landings[selected].slot].get<ability_dash>();
                        if (properties.modifier > 0 && d::collateral_risk(landings[selected].tile, properties.radius)) continue;
                        d::Cast dash{landings[selected].slot, d::Target::Ground, 0, landings[selected].tile}; dash.dash = true;
                        if (!cast(dash)) return done(QualifierCombatResult::Invalidated);
                        return done(continue_move());
                    }
                }
            }
        }

        if (protective) {
            // Delivery/recovery do not initiate unrelated nonfinishing damage.
            if (shield.slot >= 0 || control.slot >= 0) {
                if (!cast(shield.slot >= 0 ? shield : control)) return done(QualifierCombatResult::Invalidated);
                return done(continue_move());
            }
            return done(QualifierCombatResult::NoAction);
        }
        if (offense.slot >= 0 && offense.value > 0) {
            if (!cast(offense)) return done(QualifierCombatResult::Invalidated);
            const auto target = d::enemy(offense.enemy);
            if (target && me.can_attack() && cheb(me.position(), target->position()) <= me.attack_range())
                if (!attack(offense.enemy)) return done(QualifierCombatResult::Invalidated);
            return done(continue_move());
        }
        // No improving exit: useful compatible basics/reset remain available.
        if (basic_target) {
            if (me.can_attack() && !attack(*basic_target)) return done(QualifierCombatResult::Invalidated);
            if (d::enemy(*basic_target)) for (int slot = 0; slot < 2; ++slot) {
                const auto ability = me.abilities()[slot];
                if (!d::ready(ability) || !ability.is_attack_reset() || !ability.can_target_self()) continue;
                d::Cast reset{slot, d::Target::Self}; reset.reset = true;
                if (!cast(reset)) return done(QualifierCombatResult::Invalidated);
                const auto target = d::enemy(*basic_target);
                if (target && me.can_attack() && cheb(me.position(), target->position()) <= me.attack_range())
                    if (!attack(*basic_target)) return done(QualifierCombatResult::Invalidated);
                break;
            }
            return done(continue_move());
        }
        return done(QualifierCombatResult::NoAction);
    } catch (const std::exception&) {
        if (attempted) {
            try { d::refresh_after_attempt(); } catch (...) {}
        }
        return done(QualifierCombatResult::Invalidated);
    }
}
