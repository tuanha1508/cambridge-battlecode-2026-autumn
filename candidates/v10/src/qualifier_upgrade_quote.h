#pragma once
// Included inside the backend namespace after stat_request. No std includes.
// One informational selection supplies actual shopping and hypothetical quotes.
struct QualifierShopSelection {
    bool known = false, mixed_partial = false, first_monster = false;
    bool future_variant_fixed = false;
    item_type item{}, emblem{};
    item_info metadata{};
    std::uint32_t loose = 0, remaining = 0;
};
struct QualifierShopQuote {
    bool known = false;
    item_type item{};
    std::uint32_t count = 0, unit_cost = 0, total_cost = 0, minimum_cost = 0;
};

QualifierShopSelection qualifier_shop_selection(const enum_map<item_type>& owned) {
    QualifierShopSelection selected;
    const auto fallback = qualifier_stat_fallback(S->type);
    const bool caster = fallback == QualifierMonsterStat::Focus;
    const auto monster = qualifier_monster_inventory(owned);
    if (monster.family == QualifierMonsterFamily::MixedPartial) {
        selected.mixed_partial = true; return selected;
    }
    auto stat = fallback;
    if (monster.family != QualifierMonsterFamily::Unstarted) stat = monster.stat;
    else if (qualifier_stat_request_matches()) stat = qualifier_stat_request.preferred;
    const auto first = qualifier_monster_variant(stat);
    struct Step { item_type shard, emblem; };
    const std::array<Step, 4> steps{{
        {first.shard, first.emblem},
        {item_type::capture_shard_of_health, item_type::capture_emblem_of_health},
        {item_type::tribute_shard_of_health, item_type::tribute_emblem_of_health},
        {caster ? item_type::bruiser_shard_of_focus : item_type::bruiser_shard_of_strength,
         caster ? item_type::bruiser_emblem_of_focus : item_type::bruiser_emblem_of_strength}
    }};
    for (std::size_t i = 0; i < steps.size(); ++i) {
        if (owned[steps[i].emblem] > 0) continue;
        selected.item = steps[i].shard; selected.emblem = steps[i].emblem;
        selected.metadata = selected.item.info(); selected.loose = owned[selected.item];
        selected.first_monster = i == 0;
        selected.future_variant_fixed = i != 0 || monster.family != QualifierMonsterFamily::Unstarted;
        if (i == 0) {
            const auto& meta = selected.metadata;
            if (!meta.purchasable || meta.cost == 0 || meta.combine_count == 0
                || !meta.result || *meta.result != selected.emblem || selected.loose >= meta.combine_count) return selected;
            selected.remaining = meta.combine_count - selected.loose;
        }
        selected.known = true; return selected;
    }
    selected.item = caster ? item_type::power_shard_of_focus : item_type::power_shard_of_strength;
    selected.emblem = caster ? item_type::power_emblem_of_focus : item_type::power_emblem_of_strength;
    selected.metadata = selected.item.info(); selected.loose = owned[selected.item];
    selected.future_variant_fixed = true; selected.known = true;
    return selected;
}

// Pure budget arithmetic after one metadata selection. This is not can_buy at
// a future shop. A quote with count0 identifies a genuine unaffordable next buy.
QualifierShopQuote qualifier_shop_quote(const QualifierShopSelection& selected, std::uint32_t budget) {
    QualifierShopQuote quote;
    if (!selected.known) return quote;
    quote.known = true; quote.item = selected.item;
    quote.unit_cost = quote.minimum_cost = selected.metadata.cost;
    if (selected.first_monster) {
        if (selected.metadata.cost == 0) return {};
        quote.count = std::min<std::uint32_t>(5, std::min(selected.remaining, budget / selected.metadata.cost));
    } else quote.count = budget >= selected.metadata.cost ? 1 : 0;
    const auto total = std::uint64_t(quote.count) * quote.unit_cost;
    if (total > budget || total > std::numeric_limits<std::uint32_t>::max()) return {};
    quote.total_cost = static_cast<std::uint32_t>(total);
    return quote;
}

// One actual next-buy quantity; combinations consume the old loose ingredients.
// No passive/HP/Focus benefit or reconstruction of already-owned live stats.
double qualifier_shop_strength_delta(const QualifierShopSelection& selected, const QualifierShopQuote& quote,
                      const std::optional<item_info>& result, double growth) {
    if (!selected.future_variant_fixed || !quote.known || quote.count == 0
        || !selected.metadata.purchasable || selected.metadata.cost == 0 || !(std::isfinite(growth) && growth > 0)
        || selected.metadata.combine_count == 0 || !selected.metadata.result
        || *selected.metadata.result != selected.emblem) return -1;
    const auto n = selected.metadata.combine_count;
    const auto accumulated = std::uint64_t(selected.loose) + quote.count;
    if (selected.loose >= n || accumulated > n) return -1;
    std::array<std::int64_t, 3> delta{
        std::int64_t(quote.count) * selected.metadata.strength_points,
        std::int64_t(quote.count) * selected.metadata.health_points,
        std::int64_t(quote.count) * selected.metadata.focus_points};
    if (accumulated == n) {
        if (!result) return -1;
        delta = { std::int64_t(result->strength_points) - std::int64_t(selected.loose) * selected.metadata.strength_points,
                  std::int64_t(result->health_points) - std::int64_t(selected.loose) * selected.metadata.health_points,
                  std::int64_t(result->focus_points) - std::int64_t(selected.loose) * selected.metadata.focus_points };
    }
    if (delta[0] <= 0 || delta[1] != 0 || delta[2] != 0) return -1;
    const double value = static_cast<double>(delta[0]) * growth;
    return std::isfinite(value) && value > 0 ? value : -1;
}


// Optional collector admission only, never positive economic/live feasibility.
// The first real next-buy action must need gold; an affordable larger batch is
// not a funding gap. Unknown state and unsupported recipe/effect return false.
bool qualifier_upgrade_funding_needed() {
    try {
        if (!S || !qualifier_current() || fuel_remaining() < 45000) return false;
        auto& me = self();
        const auto type = me.champion_name();
        if (!me.is_alive() || me.gem_count() != 0 || me.is_recalling() || me.is_submitting_gems()
            || me.is_teleporting() || type == champion_type::cassi || type == champion_type::issac) return false;
        const auto owned = me.item_count();
        const auto selected = qualifier_shop_selection(owned);
        if (!selected.known || !selected.future_variant_fixed || !selected.metadata.purchasable
            || selected.metadata.cost == 0 || selected.metadata.strength_points <= 0
            || selected.metadata.health_points != 0 || selected.metadata.focus_points != 0) return false;
        const auto wallet = me.gold_count();
        if (wallet >= selected.metadata.cost) return false;
        const auto design = me.base_stats();
        if (!std::isfinite(design.camp_damage_multiplier) || design.camp_damage_multiplier != 1.0f) return false;
        std::optional<item_info> result;
        if (selected.metadata.result) result = selected.metadata.result->info();
        const auto minimum = qualifier_shop_quote(selected, selected.metadata.cost);
        const double delta = qualifier_shop_strength_delta(selected, minimum, result, design.strength_growth);
        return std::isfinite(delta) && delta > 0 && qualifier_current();
    } catch (...) { return false; }
}
