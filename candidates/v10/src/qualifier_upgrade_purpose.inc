// Read-only conditional payback. Included after contact/finish-work helpers.
// No action, live-feasibility promotion, S.goals/df_go write, or future Plan.
struct QualifierUpgradePurpose {
    enum class Status { Unknown, NoPurpose, AlreadyAffordable, FundOneReceipt };
    int funding_task = -1;
    bool return_shop = false;
    double rate = 0;
    vec2 shop_tile{}, origin{};
    std::uint32_t actor = 0;
    int round = -1;
    std::uint64_t revision = 0;
    Status status = Status::Unknown;
    int gem_task = -1;
    std::uint32_t gem_group = 0, funding_group = 0;
    item_type item{};
    std::uint32_t count = 0, cost = 0, wallet = 0, gold_gap = 0;
    double nominal_strength_delta = 0, baseline_turns = 0, cycle_turns = 0;
    bool counterfactual = true, grants_live_feasibility = false;
};

namespace qualifier_upgrade_detail {
bool nonnegative(double x) { return std::isfinite(x) && x >= 0; }
bool positive(double x) { return std::isfinite(x) && x > 0; }
bool inside(vec2 p) { return p.x >= 0 && p.y >= 0 && p.x < S->W && p.y < S->H; }
std::size_t index(vec2 p) { return static_cast<std::size_t>(p.y) * S->W + p.x; }
int distance(const std::vector<std::uint16_t>& values, vec2 p) {
    return inside(p) && values.size() == static_cast<std::size_t>(S->W) * S->H ? values[index(p)] : UNREACH;
}
double turns(int steps, int speed) { return std::ceil(static_cast<double>(steps) / speed); }
double total(const qualifier::Task& t) {
    const std::array<double, 7> parts{t.travel_turns, t.work_turns, t.capture_turns,
        t.delivery_turns, t.bank_turns, t.recovery_turns, t.delivery_gold_cost};
    for (const auto v : parts) if (!nonnegative(v)) return -1;
    const double value = t.travel_turns + t.work_turns + t.capture_turns
        + t.delivery_turns + t.bank_turns + t.recovery_turns;
    return nonnegative(value) ? value : -1;
}

bool current_record(const QualifierContactRecord* record, int group, std::uint32_t actor,
                    int round, std::uint64_t revision, vec2 origin) {
    return record && record->group_index == group && record->origin == origin
        && qualifier_contact_expansion_ok(*record) && record->input
        && record->input->stamp.actor == actor && record->input->stamp.round == static_cast<std::uint32_t>(round)
        && record->input->stamp.revision == revision
        && record->input->stamp.group == static_cast<std::uint32_t>(static_cast<handle_camp_group_t>(S->groups[group].h));
}


struct Gem {
    int task = -1;
    std::uint32_t group = 0;
    double gems = 0, baseline = 0, ranking = 0;
    double capture = 0, delivery = 0;
    vec2 contact{}, receipt{};
    std::size_t members = 0;
    std::array<double, 16> hp{};
    std::vector<std::uint16_t> to_contact;
    int capture_steps = UNREACH;
};
struct Gold {
    int task = -1;
    std::uint32_t group = 0;
    std::uint32_t share = 0;
    double receipt_turns = 0, ranking = 0;
    vec2 receipt{};
};

template<class T> void retain_two(std::vector<T>& out, T value) {
    out.push_back(std::move(value));
    std::sort(out.begin(), out.end(), [](const T& a, const T& b) {
        return a.ranking != b.ranking ? a.ranking > b.ranking : a.group < b.group;
    });
    if (out.size() > 2) out.pop_back();
}
} // namespace qualifier_upgrade_detail

QualifierUpgradePurpose qualifier_upgrade_purpose(const std::vector<qualifier::Task>& tasks,
    const qualifier::Context& context, const qualifier::Decision& positive_decision) {
    namespace d = qualifier_upgrade_detail;
    QualifierUpgradePurpose best;
    try {
        if (!S || !qualifier_current() || fuel_remaining() < 65000 || context.carried_gems != 0
            || context.bank_gold_cost != 0
            || positive_decision.bank || !std::isfinite(positive_decision.rate)
            || !d::positive(context.remaining_turns) || S->groups.size() > 512) return best;
        auto& me = self();
        const auto actor = static_cast<std::uint32_t>(static_cast<handle_champion_t>(me));
        const auto round = S->round; const auto revision = qualifier_runtime.observation_revision;
        const auto origin = me.position(); const auto type = me.champion_name();
        if (!me.is_alive() || me.gem_count() != 0 || me.is_recalling() || me.is_submitting_gems()
            || me.is_teleporting() || type == champion_type::cassi || type == champion_type::issac) return best;
        const auto area = std::int64_t(S->W) * S->H;
        if (S->W <= 0 || S->H <= 0 || area <= 0 || area > 10000 || !d::inside(origin)) return best;
        const auto owned = me.item_count(); const auto wallet = me.gold_count();
        const auto selected = qualifier_shop_selection(owned);
        if (!selected.known || !selected.future_variant_fixed || selected.metadata.cost == 0) return best;
        const auto current_quote = qualifier_shop_quote(selected, wallet);
        if (!current_quote.known) return best;
        const bool affordable = current_quote.count > 0;
        const auto design = me.base_stats();
        const double strength = me.strength(); const int speed = design.base_move_speed;
        if (!d::positive(strength) || !d::positive(design.strength_growth) || speed <= 0 || speed >= UNREACH
            || !std::isfinite(design.camp_damage_multiplier) || design.camp_damage_multiplier != 1.0f) return best;
        std::optional<item_info> result_meta;
        if (selected.metadata.result) result_meta = selected.metadata.result->info();
        const auto effect_probe = affordable ? current_quote : qualifier_shop_quote(selected, selected.metadata.cost);
        if (!d::positive(qualifier_shop_strength_delta(selected, effect_probe, result_meta, design.strength_growth))) return best;
        const bool current_move_wait = !me.can_move();
        auto fresh = [&]() {
            return qualifier_current() && S->round == round && qualifier_runtime.observation_revision == revision
                && static_cast<std::uint32_t>(static_cast<handle_champion_t>(me)) == actor
                && me.is_alive() && me.position() == origin && me.gem_count() == 0;
        };
        auto receipt = [&](int group) -> std::optional<vec2> {
            for (const auto& choice : qualifier_receipt_choices) if (choice.group == group) return choice.receipt;
            return {};
        };
        std::vector<d::Gem> gems;
        std::vector<d::Gold> golds;
        for (const auto& task : tasks) {
            if (!task.feasible || task.id < 0 || task.id >= static_cast<int>(S->groups.size())
                || !d::nonnegative(task.reward_gems) || !d::nonnegative(task.reward_gold)) continue;
            const auto& group = S->groups[task.id];
            if (group.last_observed_round != round || group.observed_state != 0) continue;
            const auto* record = qualifier_contact_record(task.id);
            if (!d::current_record(record, task.id, actor, round, revision, origin)) continue;
            const auto r = receipt(task.id);
            if (!r || !d::inside(*r) || !record->capture_region.is_set(*r)) continue;
            const double baseline = d::total(task);
            if (!d::positive(baseline) || task.work_turns != 1.0 + record->plan.clear_phases) continue;
            const auto full = static_cast<std::uint32_t>(static_cast<handle_camp_group_t>(group.h));
            if (task.reward_gems > 0) {
                const auto* delivery = qualifier_task_delivery(task.id);
                if (!task.delivery_available || !delivery || !delivery->plan.found()
                    || !delivery->plan.resource_backed || delivery->plan.purchase_required
                    || delivery->plan.actor != actor || delivery->plan.observed_round != round
                    || delivery->plan.origin != *r || delivery->plan.cargo != task.reward_gems
                    || task.delivery_gold_cost != 0 || baseline > context.remaining_turns) continue;
                const auto option = record->plan.selected_option;
                const auto& in = *record->input;
                if (option < 0 || option >= static_cast<int>(in.option_count) || in.member_count > 16) continue;
                d::Gem gem; gem.task = task.id; gem.group = full; gem.gems = task.reward_gems;
                gem.baseline = baseline; gem.ranking = gem.gems / baseline;
                gem.capture = task.capture_turns; gem.delivery = task.delivery_turns + task.bank_turns + task.recovery_turns;
                gem.receipt = *r; gem.contact = record->casts[option].contact_position; gem.members = in.member_count;
                bool basic_reach = d::inside(gem.contact);
                for (std::size_t i = 0; i < in.member_count; ++i) {
                    gem.hp[i] = in.members[i].hp;
                    basic_reach = basic_reach && in.members[i].hp_known && in.members[i].effects_known
                        && d::nonnegative(gem.hp[i]) && (gem.hp[i] == 0 || in.options[option].contact_reach[i]);
                }
                if (basic_reach && d::positive(gem.ranking)) d::retain_two(gems, std::move(gem));
            } else if (!affordable && group.gems == 0 && task.reward_gems == 0 && task.delivery_gold_cost == 0
                && task.reward_gold >= 1 && task.reward_gold <= std::numeric_limits<std::uint32_t>::max()) {
                // Physical/contact qualification, NOT positive economic admission.
                // This is the existing task's nominal own integer share; future
                // capturers/bonuses may change the actual later wallet.
                const auto share = static_cast<std::uint32_t>(std::floor(task.reward_gold));
                const auto post_wallet = std::uint64_t(wallet) + share;
                const double current_wait = current_move_wait && task.travel_turns > 0 ? 1.0 : 0.0;
                const double receipt_turns = task.travel_turns + task.work_turns + task.capture_turns + current_wait;
                if (!share || post_wallet < selected.metadata.cost || post_wallet > std::numeric_limits<std::uint32_t>::max()
                    || !d::positive(receipt_turns)) continue;
                d::Gold gold; gold.task = task.id; gold.group = full; gold.share = share;
                gold.receipt = *r; gold.receipt_turns = receipt_turns; gold.ranking = share / receipt_turns;
                if (d::positive(gold.ranking)) d::retain_two(golds, std::move(gold));
            }
        }
        if (gems.empty() || (!affordable && golds.empty()) || !fresh()) return best;
        const auto spawns = map::spawn_region(team::ally).positions();
        if (spawns.empty() || spawns.size() > 256) return best;
        for (const auto p : spawns) if (!d::inside(p)) return best;
        // Clear only this actor's current start from noncamp occupancy. A
        // defeated gold group never removes a champion/teleport reservation.
        auto noncamp = S->terrain | vision::champion_tiles() | vision::teleport_tiles();
        noncamp.set(origin, false); noncamp |= S->terrain;
        const auto observed_camps = vision::camp_tiles();
        const auto physical = noncamp | observed_camps;
        int fields = 0;
        auto field = [&](const bitboard& walls, vec2 root) {
            if (++fields > 6 || !fresh() || fuel_remaining() < 30000 || !d::inside(root) || walls.is_set(root))
                throw std::runtime_error("upgrade route unknown");
            bitboard roots, visited; roots.set(root);
            distance_field distances; distances.compute(walls, roots, visited);
            if (!fresh()) throw std::runtime_error("upgrade route stale");
            auto values = distances.export_linear_buffer();
            if (values.size() != static_cast<std::size_t>(area) || !fresh()) throw std::runtime_error("upgrade field size");
            return values;
        };
        for (auto& gem : gems) {
            if (physical.is_set(gem.contact)) continue;
            gem.to_contact = field(physical, gem.contact);
            const auto* record = qualifier_contact_record(gem.task);
            if (!d::current_record(record, gem.task, actor, round, revision, origin)) return {};
            const auto capture = field(~record->capture_region, gem.receipt);
            gem.capture_steps = d::distance(capture, gem.contact);
        }
        auto consider = [&](const d::Gem& gem, const std::vector<std::uint16_t>& source_to_shop,
                            const QualifierShopQuote& quote, double prior_turns, int funding_task,
                            std::uint32_t funding_group) {
            if (gem.to_contact.empty() || gem.capture_steps == UNREACH) return;
            const double delta = qualifier_shop_strength_delta(selected, quote, result_meta, design.strength_growth);
            const double post_strength = strength + delta;
            if (!d::positive(delta) || !d::positive(post_strength)
                || post_strength > std::numeric_limits<float>::max()) return;
            // Full pre-prefix HP, not plan.remaining_hp after an unexecuted cast.
            // One arrival allowance before stationary basic-only owner inputs.
            double post_work = 1;
            for (std::size_t i = 0; i < gem.members; ++i)
                if (gem.hp[i] > 0) post_work += std::ceil(gem.hp[i] / post_strength);
            for (const auto shop : spawns) {
                const int before = d::distance(source_to_shop, shop), after = d::distance(gem.to_contact, shop);
                if (before == UNREACH || after == UNREACH) continue;
                const double wait = funding_task < 0 && current_move_wait && before > 0 ? 1.0 : 0.0;
                const double cycle = prior_turns + wait + d::turns(before, speed) + 1.0
                    + d::turns(after, speed) + post_work + d::turns(gem.capture_steps, speed) + gem.capture + gem.delivery;
                if (!d::positive(cycle) || cycle >= gem.baseline || cycle > context.remaining_turns) continue;
                const double rate = gem.gems / cycle;
                if (!d::positive(rate) || rate <= positive_decision.rate) continue;
                const bool tie = rate == best.rate;
                if (rate < best.rate || (tie && (gem.group > best.gem_group
                    || (gem.group == best.gem_group && (funding_group > best.funding_group
                        || (funding_group == best.funding_group && (shop.y > best.shop_tile.y
                            || (shop.y == best.shop_tile.y && shop.x >= best.shop_tile.x)))))))) continue;
                best.funding_task = funding_task; best.return_shop = funding_task < 0; best.rate = rate;
                best.status = funding_task < 0 ? QualifierUpgradePurpose::Status::AlreadyAffordable
                    : QualifierUpgradePurpose::Status::FundOneReceipt;
                best.actor = actor; best.round = round; best.revision = revision; best.origin = origin; best.shop_tile = shop;
                best.gem_task = gem.task; best.gem_group = gem.group; best.funding_group = funding_group;
                best.item = quote.item; best.count = quote.count; best.cost = quote.total_cost; best.wallet = wallet;
                best.gold_gap = wallet < selected.metadata.cost ? selected.metadata.cost - wallet : 0;
                best.nominal_strength_delta = delta; best.baseline_turns = gem.baseline; best.cycle_turns = cycle;
            }
        };
        if (affordable) {
            // No gold task can be funded when the exact current buy is affordable.
            const auto to_shop = field(physical, origin);
            for (const auto& gem : gems) consider(gem, to_shop, current_quote, 0, -1, 0);
        } else for (const auto& gold : golds) {
            const auto* record = qualifier_contact_record(gold.task);
            if (!d::current_record(record, gold.task, actor, round, revision, origin)) return {};
            auto after_clear = noncamp | (observed_camps & ~qualifier_tiles(S->groups[gold.task]));
            after_clear |= S->terrain; // Never erase terrain with a group mask.
            const auto to_shop = field(after_clear, gold.receipt);
            const auto quote = qualifier_shop_quote(selected,
                static_cast<std::uint32_t>(std::uint64_t(wallet) + gold.share)); // Checked on candidate intake.
            for (const auto& gem : gems) consider(gem, to_shop, quote, gold.receipt_turns, gold.task, gold.group);
        }
        if (!fresh() || me.gold_count() != wallet) return {};
        if (!d::positive(best.rate)) best.status = QualifierUpgradePurpose::Status::NoPurpose;
        return best;
    } catch (...) { return {}; } // Missing knowledge/caps never admit a new job.
}
