#pragma once
#include <cambc.h>
#include <cstdint>
#include "gc34_gate.h"
#include "mg_switch.h"   // batch2: N4_GW_SIDES / N4_BB_SIDES / N4_FA_SIDES (default 0)
// cc_next4 (30 Oct; docs/claude-next4-2026-10-30.md). Next worst unworked map/sides since 27 Oct (research/claude-next4/).
//   N4_FA 01the-farm SIDE A (3-12): losses stall the bank after r900 (516->536; wins 623->974) and lose 179 gems/game
//   N4_BB 01boss-battle SIDE B (3-11): losses bank 0 by r900 (winners 964) and drop 1436 gems/game in the centre
//   N4_GW 00great-wall SIDE A (6-12): losses drop 143 gems/game (wins 64), 1.5 bags >= 40 lost per game after r600
// Own exact-layout rows (ids >= 120, FNV signature as nx25 rows); each lever is gated by its side mask
// (gc34_::side_on: 1 = side A, 2 = side B). Levers: fight flee ratio (g_mm_flee) and bank-at-N-gems near an altar.
#if __has_include("n4_over.h")
#include "n4_over.h"   // local test variants only (not in the shipped folder)
#endif
#ifndef N4_FA_FLEE
#define N4_FA_FLEE 0.0f
#endif
#ifndef N4_FA_BANK
#define N4_FA_BANK 8      // late bank: side A banks at 8 gems near an altar from N4_FA_BR (keeps every local win, wider margins)
#endif
#ifndef N4_FA_BR
#define N4_FA_BR 900      // r600 start (and 12/16 from r600, flee 1.3-1.5) lost the mirror win
#endif
#ifndef N4_FA_NEAR
#define N4_FA_NEAR 12
#endif
#ifndef N4_FA_SIDES
#define N4_FA_SIDES 1
#endif
#ifndef N4_BB_FLEE
#define N4_BB_FLEE 0.0f
#endif
#ifndef N4_BB_BANK
#define N4_BB_BANK 20     // own-altar banking for big bags from r900 within N4_BB_NEAR (keeps wins; mirror loss 1052-158 vs 2216-320)
#endif
#ifndef N4_BB_BR
#define N4_BB_BR 900
#endif
#ifndef N4_BB_NEAR
#define N4_BB_NEAR 40
#endif
#ifndef N4_BB_SIDES
#define N4_BB_SIDES 2
#endif
#ifndef N4_GW_FLEE
#define N4_GW_FLEE 1.3f   // per-map flee (Lost World B pattern): keeps every local win; bank 4/8/12/16 no gain or lost mirror
#endif
#ifndef N4_GW_BANK
#define N4_GW_BANK 0
#endif
#ifndef N4_GW_BR
#define N4_GW_BR 600
#endif
#ifndef N4_GW_NEAR
#define N4_GW_NEAR 12
#endif
#ifndef N4_GW_SIDES
#define N4_GW_SIDES 1
#endif
namespace n4_ {
struct Row { int id, w, h, n, groups; std::uint64_t sig; };
inline constexpr Row rows[] = {
    { 120, 50, 40, 3, 28, 18027621674019500262ULL },  // 01the-farm
    { 121, 50, 41, 5, 31, 9988866822838923753ULL },   // 01boss-battle
    { 122, 50, 40, 5, 17, 9383502894566448885ULL },   // 00great-wall
};
template <class G> inline int detect(int W, int H, const G& groups) {
    std::uint64_t h = 1469598103934665603ULL;
    auto mix = [&](std::uint64_t w) { h ^= w; h *= 1099511628211ULL; h ^= h >> 29; };
    for (const auto& g : groups) { mix(static_cast<std::uint64_t>(g.gems)); mix(static_cast<std::uint64_t>(g.gold)); mix(g.camps.size()); }
    const int n = static_cast<int>(cambc::draft::team_size()), ng = static_cast<int>(groups.size());
    for (const auto& r : rows) if (r.w == W && r.h == H && r.n == n && r.groups == ng && r.sig == h) return r.id;
    return 0;
}
inline float flee(int id) {   // 0 = keep the pin's flee ratio
    if (id == 120 && N4_FA_FLEE > 0.0f && gc34_::side_on(N4_FA_SIDES)) return N4_FA_FLEE;
    if (id == 121 && N4_BB_FLEE > 0.0f && gc34_::side_on(N4_BB_SIDES)) return N4_BB_FLEE;
    if (id == 122 && N4_GW_FLEE > 0.0f && gc34_::side_on(N4_GW_SIDES)) return N4_GW_FLEE;
    return 0.0f;
}
inline int bank(int id, int r) {   // 0 = no change
    if (id == 120 && N4_FA_BANK > 0 && r >= N4_FA_BR && gc34_::side_on(N4_FA_SIDES)) return N4_FA_BANK;
    if (id == 121 && N4_BB_BANK > 0 && r >= N4_BB_BR && gc34_::side_on(N4_BB_SIDES)) return N4_BB_BANK;
#if LT_BB
    if (id == 121 && r >= LT_BB_R && gc34_::side_on(LT_BB)) return LT_BB_BANK;   // cc_late row 360: Boss Battle A late bank
#endif
    if (id == 122 && N4_GW_BANK > 0 && r >= N4_GW_BR && gc34_::side_on(N4_GW_SIDES)) return N4_GW_BANK;
    return 0;
}
inline int near(int id) {
#if LT_BB
    if (id == 121 && gc34_::side_on(LT_BB)) return LT_BB_NEAR;   // cc_late row 360
#endif
    return id == 120 ? N4_FA_NEAR : id == 121 ? N4_BB_NEAR : N4_GW_NEAR; }
}  // namespace n4_
