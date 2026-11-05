// cc_builds3g (29 Oct): Heart of Ruin Focus build, gated by cb3g_on() (exact contest, elkring, corners).
// Ladder evidence (research/claude-builds3): 94 other teams buy 0-3% Strength shards on HoR (Focus+Health);
// we bought 60% Strength / 0% Focus. HoR shield and dash scale with Focus; their HoR shields ~2x ours.
// Same emblem structure as our default melee path, attribute swapped: Jungle Focus > Bruiser Health >
// Durability Health, then spare gold to Power Focus. Included textually inside each policy's shop(),
// right after `struct Step`; needs me, gold, owned, it, Step, S in scope. No include guard on purpose.
#ifndef CB3_HOR
#define CB3_HOR 1
#endif
#if CB3_HOR
    if (S->type == champion_type::heart_of_ruin && cb3g_on()) {
        const std::array<Step, 3> cb3_plan{ Step{ it::jungle_shard_of_focus, it::jungle_emblem_of_focus },
                                            Step{ it::bruiser_shard_of_health, it::bruiser_emblem_of_health },
                                            Step{ it::durability_shard_of_health, it::durability_emblem_of_health } };
        for (const auto& cb3_step : cb3_plan) {
            if (gold < 30) return;
            if (owned[cb3_step.emblem] > 0) continue;
            const std::uint32_t cb3_need = 5 - std::min<std::uint32_t>(5, owned[cb3_step.shard]);
            const std::uint32_t cb3_n = std::min(cb3_need, gold / 30);
            if (cb3_n == 0 || !me.can_buy(cb3_step.shard, cb3_n)) return;
            me.buy(cb3_step.shard, cb3_n);
            gold -= cb3_n * 30;
            if (cb3_n < cb3_need) return;
        }
        const std::uint32_t cb3_n = gold / 30;
        if (cb3_n && me.can_buy(it::power_shard_of_focus, cb3_n)) me.buy(it::power_shard_of_focus, cb3_n);
        return;
    }
#endif
