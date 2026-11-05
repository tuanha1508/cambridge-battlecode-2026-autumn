// Fresh current stages only. No future PurchaseIntent or forecast is executed.
struct QualifierResourceProgress {
    bool used = false, review_due = true, named_primary_known = false;
    // A group is retained only as an observation intent. Its same-input epoch
    // token is rebuilt by the collector and is never persisted as a witness.
    std::uint32_t actor = 0, named_primary = 0;
    int last_round = -1, next_review = -1, inspect_started = -1;
};
std::array<QualifierResourceProgress, 16> qualifier_resource_progress_entries{};
QualifierResourceProgress* qualifier_resource_progress = nullptr;

void qualifier_resource_begin() {
    const auto actor = static_cast<std::uint32_t>(static_cast<handle_champion_t>(self()));
    qualifier_resource_progress = nullptr;
    for (auto& p : qualifier_resource_progress_entries)
        if (p.used && p.actor == actor) { qualifier_resource_progress = &p; break; }
    if (!qualifier_resource_progress) {
        auto* oldest = &qualifier_resource_progress_entries.front();
        for (auto& p : qualifier_resource_progress_entries) {
            if (!p.used) { oldest = &p; break; }
            if (p.last_round < oldest->last_round) oldest = &p;
        }
        *oldest = {}; oldest->used = true; oldest->actor = actor;
        qualifier_resource_progress = oldest;
    }
    auto& p = *qualifier_resource_progress;
    if (!self().is_alive()) {
        p.review_due = true; p.named_primary_known = false; p.inspect_started = -1;
        p.next_review = S->round; p.last_round = S->round; return;
    }
    // A skipped input invalidates the observation trip, not its obligation.
    if (p.last_round != S->round && p.last_round != S->round - 1) {
        p.named_primary_known = false; p.inspect_started = -1;
    }
    if (S->round >= p.next_review) p.review_due = true;
    p.last_round = S->round;
}

void qualifier_resource_review_defer(int until) {
    if (!qualifier_resource_progress) return;
    auto& p = *qualifier_resource_progress;
    p.review_due = false; p.named_primary_known = false; p.inspect_started = -1;
    p.next_review = std::max(S->round + 1, until);
}

bool qualifier_resource_budget(void* opaque) {
    const auto* frame = static_cast<const QualifierResourceFrame*>(opaque);
    return frame && qualifier_resource_frame_matches(*frame) && fuel_remaining() >= 24000;
}

void qualifier_resource_invalidate();

bool qualifier_resource_walk(const QualifierResourceFrame& frame, std::size_t node) {
    if (!frame.input || node >= frame.nodes.size() || !qualifier_resource_frame_matches(frame)) return true;
    auto& me = self();
    if (me.position() == frame.nodes[node]) return true;
    if (!frame.current_steps[node] || !frame.input->routes[0][node].current_witness
        || !me.can_move()) return true;
    const auto origin = me.position(), destination = *frame.current_steps[node];
    if (cheb(origin, destination) != 1) return true;
    refresh_occ(); vision::teleport_tiles_into(S->tmp); S->occ |= S->tmp;
    if (S->terrain.is_set(destination) || S->occ.is_set(destination)
        || !qualifier_resource_frame_matches(frame) || me.position() != origin) return true;
    try {
        // Adjacent moves have no unpriced intermediate host path. The quote
        // conservatively accounts for one such physical step per owner input.
        if (me.move_absolute(destination)) { S->moved = true; ++S->moves; }
    } catch (const std::exception&) {}
    qualifier_resource_invalidate();
    return true;
}

void qualifier_resource_invalidate() {
    ++qualifier_runtime.observation_revision;
    qualifier_contact_records.clear(); qualifier_direct_delivery.reset();
    qualifier_task_deliveries.clear();
}

bool qualifier_resource_purchase(const QualifierResourceFrame& frame,
    const qualifier::resource::CurrentStage& stage) {
    using namespace qualifier::resource;
    if (!frame.input || !qualifier_resource_frame_matches(frame) || !stage.witness_known
        || stage.stamp != frame.input->stamp || stage.kind != StageKind::Purchase) return false;
    const auto& in = *frame.input;
    const auto& intent = stage.purchase;
    if (!intent.known || intent.count == 0 || intent.shop_token != in.shop.token
        || stage.endpoint_token != in.shop.token || !in.shop.current_purchase_witness
        || qualifier_contact_detail::position_token(self().position()) != in.shop.token) return false;
    auto& me = self();
    const std::uint64_t total = std::uint64_t(intent.unit_cost) * intent.count;
    if (total != intent.total_cost || total > intent.per_visit_spend_cap
        || total + intent.reserved_route_cash > me.gold_count()) return false;
    bool attempted = false;
    try {
        if (intent.kind == PurchaseKind::Crystal) {
            const auto crystal = consumable_type::tp_crystal;
            if (intent.item != in.crystal_item || intent.count != 1 || !in.shop.consumables_legal
                || crystal.cost() != intent.unit_cost || me.consumables()[crystal].count() != 0
                || !me.can_buy(crystal) || !qualifier_resource_frame_matches(frame)) return false;
            attempted = true;
            me.buy(crystal);
        } else {
            if (intent.item >= static_cast<std::uint32_t>(item_type::count)) return false;
            const auto item = static_cast<item_type>(intent.item);
            const auto info = item.info();
            const auto owned = me.item_count();
            const auto loose = owned[item];
            if (!info.purchasable || info.cost != intent.unit_cost || loose != intent.old_loose
                || info.combine_count != intent.combine_count
                || (info.result ? static_cast<std::uint32_t>(*info.result) : no_item) != intent.recipe_result
                || loose != in.actor.owned[intent.item] || !me.can_buy(item, intent.count)
                || !qualifier_resource_frame_matches(frame)) return false;
            attempted = true;
            me.buy(item, intent.count);
        }
    } catch (const std::exception&) {
        // An attempted host mutation owns this input, with no rollback claim.
    }
    if (attempted) {
        qualifier_own_work_retire(); qualifier_inspection_cancel();
        qualifier_resource_invalidate();
    }
    return attempted;
}

// True owns the input even if a fresh check invalidates a selected stage. The
// parent may fall back only when there was no qualified current stage at all.
bool qualifier_execute_resource(const QualifierResourceFrame& frame,
    const qualifier::resource::Result& result, const std::vector<qualifier::Task>& tasks,
    qualifier::Context context) {
    using namespace qualifier::resource;
    const auto& stage = result.current_stage;
    if (stage.kind == StageKind::None || !stage.witness_known) return false;
    if (!frame.input || !qualifier_resource_frame_matches(frame)
        || stage.stamp != frame.input->stamp) return true;
    if (stage.kind == StageKind::Purchase) {
        qualifier_resource_purchase(frame, stage); return true;
    }
    auto& s = *S; auto& me = self();
    if (stage.kind == StageKind::Recover) {
        if (map::spawn_region(team::ally).is_set(me.position())
            && stage.endpoint_token == frame.input->shop.token) return true;
        return true;
    }
    if (stage.kind == StageKind::ReturnShop) {
        if (stage.endpoint_token != frame.input->shop.token) return true;
        qualifier_own_work_retire(); qualifier_inspection_cancel();
        s.target = s.qualifier_committed_target = -1; s.mode = Mode::farm;
        return qualifier_resource_walk(frame, 1);
    }
    if (stage.goal >= frame.input->goal_count
        || !qualifier_resource_goal_matches(frame, stage.goal)) return true;
    const auto& goal = frame.input->goals[stage.goal];
    const int gi = frame.groups[stage.goal];
    if (stage.group_id != goal.group_id || stage.epoch != goal.epoch || gi < 0
        || gi >= static_cast<int>(s.groups.size())) return true;
    auto& group = s.groups[gi];
    if (stage.kind == StageKind::Inspect || stage.kind == StageKind::ObserveAfterPurchase) {
        if (!qualifier_resource_progress) return true;
        auto& p = *qualifier_resource_progress;
        if (!p.review_due) return false;
        if (!p.named_primary_known || p.named_primary != goal.group_id)
            p.inspect_started = -1;
        p.named_primary_known = true; p.named_primary = goal.group_id;
        if (p.inspect_started < 0) p.inspect_started = s.round;
        qualifier::Task request;
        request.id = gi; request.reward_gems = goal.nominal_gems; request.reward_gold = goal.nominal_gold;
        for (const auto& task : tasks) if (task.id == gi) { request = task; break; }
        // A goal omitted from the old economic list can still request safe
        // information. The named overload never grants it economic feasibility.
        const bool owned = qualifier_inspect(std::vector<qualifier::Task>{request}, context, gi);
        if (gi < static_cast<int>(qualifier_inspection.retry_after.size())
            && qualifier_inspection.retry_after[gi] > s.round)
            qualifier_resource_review_defer(qualifier_inspection.retry_after[gi]);
        else if (!owned && s.round - p.inspect_started >= 8)
            qualifier_resource_review_defer(s.round + 24); // Explicit bounded physical deferral.
        return owned;
    }
    qualifier_inspection_cancel();
    if (s.target != gi) qualifier_own_work_retire();
    s.target = s.qualifier_committed_target = gi; s.mode = Mode::farm;
    qualifier_opening_retire();
    if (stage.kind == StageKind::Approach) {
        if (stage.endpoint_token == qualifier_contact_detail::position_token(frame.nodes[2 + stage.goal]))
            return qualifier_resource_walk(frame, 2 + stage.goal);
        if (stage.endpoint_token == qualifier_contact_detail::position_token(frame.nodes[10 + stage.goal])
            && group.last_observed_round == s.round && group.observed_state == 1)
            return qualifier_resource_walk(frame, 10 + stage.goal);
        return true;
    }
    if (stage.kind == StageKind::WaitRespawn) {
        // Current witness must be an actual outside-spawn waiting point.
        if (!goal.current_wait_witness || qualifier_tiles(group).is_set(me.position())) return true;
        return true;
    }
    if (stage.kind == StageKind::Capture) {
        if (!goal.current_capture_witness || group.last_observed_round != s.round
            || group.observed_state != 1) return true;
        qualifier_runtime.delivery_capture_group = gi;
        qualifier_runtime.delivery_receipt_tile = frame.nodes[10 + stage.goal];
        if (group.qualifier_capture_round == s.round
            && group.qualifier_capture_status == static_cast<int>(map::camp_group_capture_type::contested)) return true;
        if (qualifier_tiles(group).is_set(me.position())) return true;
        return qualifier_resource_walk(frame, 10 + stage.goal);
    }
    if (stage.kind == StageKind::Deliver) {
        if (me.gem_count() > 0) qualifier_bank();
        return true;
    }
    if (stage.kind != StageKind::Work || !goal.current_work_witness
        || qualifier_contact_detail::position_token(me.position()) != goal.contact_token) return true;
    if (stage.action == ActionKind::None) return true;
    auto find_member = [&](std::uint32_t id) -> Camp* {
        for (auto& camp : group.camps)
            if (static_cast<std::uint32_t>(static_cast<handle_camp_t>(camp.h)) == id) return &camp;
        return nullptr;
    };
    try {
        if (stage.action == ActionKind::Basic) {
            auto* camp = find_member(stage.target_member_id);
            if (!camp || !me.can_attack() || !camp->h.is_visible() || camp->h.is_dead()) return true;
            camp->qualifier_refresh_observation(s.round);
            if (!camp->health_observation_valid || camp->last_observed_round != s.round || camp->hp <= 0
                || cheb(me.position(), camp->pos) > me.attack_range()
                || !qualifier_resource_frame_matches(frame)) return true;
            me.attack(camp->h);
            qualifier_note_group_work(group, 1); ++s.attacks;
            qualifier_resource_invalidate(); return true;
        }
        if (stage.action != ActionKind::Cast || stage.cast_slot < 0 || stage.cast_slot > 1
            || s.cast || !me.can_cast() || !goal.current_cast_witness[stage.cast_slot]) return true;
        const auto& cast = frame.casts[stage.goal][stage.cast_slot];
        const auto ability = me.abilities()[stage.cast_slot];
        if (!ability.is_ready() || cast.slot != stage.cast_slot || cast.contact_position != me.position()
            || stage.cast_token != goal.cast_tokens[stage.cast_slot]) return true;
        bool accepted = false, affects_group = false;
        if (cast.target == QualifierContactCast::Target::Self) {
            if (stage.cast_token != frame.input->stamp.actor || !ability.can_target_self()
                || (!ability.is_heal() && !ability.is_shield())) return true;
            if (!qualifier_resource_frame_matches(frame)) return true;
            accepted = ability.use();
        } else if (cast.target == QualifierContactCast::Target::Camp) {
            auto* camp = find_member(stage.target_member_id);
            if (!camp || stage.cast_token != stage.target_member_id || !ability.can_target_camps()
                || !camp->h.is_visible() || camp->h.is_dead()) return true;
            double range = -1;
            if (ability.is_damage()) range = ability.get<ability_damage>().range;
            else if (ability.is_cc()) range = ability.get<ability_cc>().range;
            if (!std::isfinite(range) || range < 0 || cheb(me.position(), camp->h.position()) > range) return true;
            if (!qualifier_resource_frame_matches(frame)) return true;
            accepted = ability.use(camp->h); affects_group = true;
        } else if (cast.target == QualifierContactCast::Target::Ground) {
            if (!ability.is_aoe_damage() || !ability.can_target_ground()
                || stage.cast_token != qualifier_contact_detail::position_token(cast.tile)
                || cheb(me.position(), cast.tile) > ability.get<ability_aoe_damage>().range) return true;
            if (!qualifier_resource_frame_matches(frame)) return true;
            accepted = ability.use(cast.tile); affects_group = true;
        } else return true;
        if (accepted) {
            s.cast = true; ++s.casts;
            if (affects_group) qualifier_note_group_work(group, 2);
        }
        qualifier_resource_invalidate(); // False host results also own this input.
    } catch (const std::exception&) { qualifier_resource_invalidate(); }
    return true;
}

bool qualifier_resource_preferred(const qualifier::resource::Result& programme,
    const qualifier::Decision& alternative) {
    using O = qualifier::resource::Objective;
    const auto& f = programme.forecast;
    if (programme.current_stage.kind == qualifier::resource::StageKind::None
        || !programme.current_stage.witness_known) return false;
    if (f.objective == O::Information) return programme.primary_review_due
        && alternative.objective != qualifier::Objective::Gems
        && alternative.objective != qualifier::Objective::Win;
    const int programme_tier = f.timely_win ? 3 : f.submitted_gems > 0 ? 2
        : f.lifetime_gold_earned > 0 ? 1 : 0;
    const int alternative_tier = alternative.objective == qualifier::Objective::Win ? 3
        : alternative.objective == qualifier::Objective::Gems ? 2
        : alternative.objective == qualifier::Objective::Gold ? 1 : 0;
    if (programme_tier != alternative_tier) return programme_tier > alternative_tier;
    if (programme_tier == 3 && f.win_turn != alternative.completion_turns)
        return f.win_turn < alternative.completion_turns;
    if (programme_tier == 2 && f.submitted_gems != alternative.value)
        return f.submitted_gems > alternative.value;
    if (f.lifetime_gold_earned != alternative.lifetime_gold_earned)
        return f.lifetime_gold_earned > alternative.lifetime_gold_earned;
    return f.elapsed <= alternative.completion_turns;
}

bool qualifier_resource_preserve_owned(int selected, const std::vector<qualifier::Task>& tasks,
    qualifier::Context context) {
    const int owned = qualifier_own_work_group();
    if (owned < 0 || owned == selected) return false;
    bool priced = false;
    context.bank_turns = -1;
    for (const auto& task : tasks) if (task.id == owned) {
        priced = qualifier::choose_objective(context, std::vector<qualifier::Task>{task}).task == owned;
        break;
    }
    return !priced && qualifier_continue_owned_work();
}

void qualifier_resource_review_observed(const QualifierResourceFrame& frame,
    const qualifier::resource::Result& result) {
    auto* p = qualifier_resource_progress;
    if (!p || !p->review_due || !p->named_primary_known || result.operations == 0
        || !frame.input || !qualifier_resource_frame_matches(frame)) return;
    for (std::size_t i = 0; i < frame.input->goal_count; ++i) {
        const auto& goal = frame.input->goals[i];
        if (goal.group_id != p->named_primary || !goal.composition_known
            || (result.evaluated_goals_mask & (std::uint32_t{1} << i)) == 0
            || !qualifier_resource_goal_matches(frame, i)) continue;
        const auto& group = S->groups[frame.groups[i]];
        bool fresh = group.last_observed_round == S->round;
        for (const auto& camp : group.camps)
            fresh = fresh && camp.health_observation_valid && camp.last_observed_round == S->round;
        // This named target has actually been observed and passed through a
        // new capability search. Unsupported conclusions remain unsupported.
        if (fresh) qualifier_resource_review_defer(S->round + 24);
        return;
    }
}

// Protect the real bag. A conditional missing-resource quote may authorize
// acquiring cash, never activate a hypothetical delivery or spend future gold.
bool qualifier_cargo_resource(const QualifierResourceFrame& frame,
    const std::vector<qualifier::Task>& tasks, qualifier::Context context) {
    using namespace qualifier::resource;
    if (!frame.input || self().gem_count() == 0 || !qualifier_resource_frame_matches(frame)) return false;
    const auto need = qualifier_cargo_resource_need();
    if (!need || !qualifier_resource_frame_matches(frame)) return false;
    const auto crystal = consumable_type::tp_crystal;
    if (need->crystal_cost != crystal.cost() || need->crystals_observed != 0
        || self().consumables()[crystal].count() != 0) return false;
    if (self().gold_count() >= need->crystal_cost) {
        // A funded resource dependency supersedes further gold collection.
        if (frame.input->shop.current_purchase_witness && self().can_buy(crystal)) {
            CurrentStage stage;
            stage.kind = StageKind::Purchase; stage.stamp = frame.input->stamp;
            stage.witness_known = true; stage.endpoint_token = frame.input->shop.token;
            auto& buy = stage.purchase;
            buy.known = true; buy.kind = PurchaseKind::Crystal;
            buy.item = frame.input->crystal_item; buy.count = 1;
            buy.unit_cost = buy.total_cost = buy.per_visit_spend_cap = need->crystal_cost;
            buy.shop_token = frame.input->shop.token;
            qualifier_resource_purchase(frame, stage); return true;
        }
        qualifier_recover(); return true; // Fresh real bank plan after arrival.
    }
    // This isolated forecast prices ONLY a new zero-gem gold receipt. Removing
    // the existing bag from reward accounting neither changes its actual stats
    // nor grants a bank route. All purchases and primary jobs are disabled.
    auto funding = std::make_unique<Input>(*frame.input);
    funding->actor.carried_gems = 0; funding->primary_review_due = false;
    funding->actor.projected_stats_known = frame.cargo_preserving_stats_known;
    for (auto& item : funding->items) item.purchasable = false;
    for (auto& goal : funding->goals) if (goal.nominal_gems != 0) {
        goal.composition_known = false; goal.current_inspect_witness = false;
    }
    if (!frame.cargo_bank_from_shop || !frame.cargo_bank_from_shop->known
        || funding->crystal_item >= max_items || !funding->shop.known
        || !funding->shop.consumables_legal) return false;
    for (auto& goal : funding->goals) if (goal.nominal_gems == 0)
        goal.delivery = *frame.cargo_bank_from_shop;
    // The finite search ends at the ORIGINAL bag's completed bank. A larger
    // distant gold haul cannot outrank an earlier sufficient funding prefix.
    namespace detail = qualifier::resource::program_detail;
    detail::Search search{*funding, qualifier_resource_budget,
        const_cast<QualifierResourceFrame*>(&frame)};
    detail::State initial;
    initial.actor = funding->actor;
    if (!detail::refresh_passives(search, initial)) return false;
    for (std::size_t a = 0; a < max_abilities; ++a) initial.cooldown[a] = funding->abilities[a].cooldown;
    for (std::size_t g = 0; g < funding->goal_count; ++g) initial.available[g] = funding->goals[g].available_in;
    std::array<detail::State, max_states> frontier{}; frontier[0] = initial;
    std::size_t count = 1;
    std::optional<detail::State> best;
    // Enable only the exact required consumable in the private quote. It is
    // bought only after actual/projected funding, at the one qualified shop.
    funding->items[funding->crystal_item].purchasable = true;
    for (std::uint32_t depth = 0; depth < 32 && count && !search.exhausted; ++depth) {
        std::array<detail::State, max_states> next{}; std::size_t next_count = 0;
        for (std::size_t i = 0; i < count && !search.exhausted; ++i) {
            if (best && frontier[i].elapsed >= best->elapsed) continue;
            for (std::size_t g = 0; g < funding->goal_count && !search.exhausted; ++g) {
                const auto& goal = funding->goals[g];
                if (goal.nominal_gems != 0 || goal.nominal_gold == 0) continue;
                auto receipt = frontier[i];
                if (!detail::receive(search, receipt, g, true)) continue;
                bool completed = false;
                if (receipt.actor.wallet >= need->crystal_cost) {
                    auto bank = receipt;
                    if (detail::route(search, bank, 1, StageKind::ReturnShop, max_goals)
                        && bank.actor.wallet >= need->crystal_cost
                        && detail::buy(search, bank, funding->crystal_item, 1)) {
                        bank.actor.carried_gems = frame.input->actor.carried_gems;
                        if (detail::deliver(search, bank, g) && detail::stage_qualified(bank)) {
                            completed = true;
                            if (!best || bank.elapsed < best->elapsed
                                || (bank.elapsed == best->elapsed && bank.earned > best->earned)) best = bank;
                        }
                    }
                }
                if (!completed && (!best || receipt.elapsed < best->elapsed))
                    if (detail::insert(next, next_count, receipt)) search.space_capped = true;
            }
        }
        frontier = next; count = next_count;
        if (depth == 31 && count) search.space_capped = true;
    }
    if (!best || !qualifier_resource_frame_matches(frame)) return false;
    Result choice;
    choice.status = search.exhausted || search.space_capped ? Status::SearchBudgetExceeded : Status::Program;
    choice.current_stage = best->first; choice.forecast = detail::forecast(*best);
    choice.search_space_capped = search.exhausted || search.space_capped;
    // Execute against the ORIGINAL actual-cargo frame, not the isolated quote.
    return qualifier_execute_resource(frame, choice, tasks, context);
}
