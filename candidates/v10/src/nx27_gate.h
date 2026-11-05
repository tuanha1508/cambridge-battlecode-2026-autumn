#pragma once
// cc_next27 (29 Oct; docs/claude-next27-2026-10-29.md). Exact-layout rules, one switch per map.
// Evidence: ranked games since 27 Oct (research/claude-next27/).
//   NX_FD  02fields (optimized.cpp, nx25 row 7): g300 gold predicts the result (wins 1241-1943 vs 744-1117; losses
//          800-1300 vs 1000-1660); opponents clear their 80-gold groups 0/2 ~20 times, we clear ours ~7. Early gold
//          role: before NX_FD_R the 80-gold/4-gem groups 0-5 value x NX_FD_M.
//   NX_ER  00elkring (optimized.cpp, nx25 row 8): wins out-clear 46-53 vs 24-37; losses equal; the swing groups are the
//          altar-side groups 6-9. From NX_ER_R they value x NX_ER_M (v2: value only).
//   NX_AR  02arena (optimized.cpp, nx25 row 9): the 15-gem/170-gold groups 2/3 decide it (wins: we clear both 16-21
//          times; losses: the opponent does). From NX_AR_R they value x NX_AR_M (v2: value only).
//   NX_EA  01elk-arena (z61_stats_optimized.cpp): the 72-gem centre group 0 is the whole game. Wins: we hold it every
//          ~90 rounds; losses: we take the first 1-2 and the opponent takes the rest from r170-370. From NX_EA_R every
//          body counts NX_EA_N - 1 helpers on group 0 (gang-up) and
//          values it x NX_EA_M (helpers alone left the local game unchanged).
//   NX_CW  01concentrated-wealth (interim_optimized.cpp): the 71-gem centre group 0 decides it (wins 11-13:0-1,
//          losses opponent 8-15). From NX_CW_R every body counts NX_CW_N - 1 helpers on group 0 (gang-up).
#include <cambc.h>
#include <cstdint>
#include "sb7_gate.h"
#ifndef NX_FD
#define NX_FD 0
#endif
#ifndef NX_FD_R
#define NX_FD_R 400
#endif
#ifndef NX_FD_M
#define NX_FD_M 2.0f
#endif
#ifndef NX_ER
#define NX_ER 0
#endif
#ifndef NX_ER_R
#define NX_ER_R 100
#endif
#ifndef NX_ER_M
#define NX_ER_M 1.3f
#endif
#ifndef NX_AR
#define NX_AR 1
#endif
#ifndef NX_AR_R
#define NX_AR_R 300
#endif
#ifndef NX_AR_M
#define NX_AR_M 1.3f
#endif
#ifndef NX_EA
#define NX_EA 0   // off: inert locally (helpers, value x2, and an optimized.cpp copy all left finalInput unchanged)
#endif
#ifndef NX_EA_R
#define NX_EA_R 150
#endif
#ifndef NX_EA_M
#define NX_EA_M 2.0f
#endif
#ifndef NX_EA_N
#define NX_EA_N 3
#endif
#ifndef NX_CW
#define NX_CW 1
#endif
#ifndef NX_CW_R
#define NX_CW_R 300
#endif
#ifndef NX_CW_N
#define NX_CW_N 2
#endif
namespace nx27_ {
// Rows 7-9 live in nx25_gate.h (optimized.cpp hooks); this table covers the two other policy files.
inline void mix(std::uint64_t& h, std::uint64_t w) { h ^= w; h *= 1099511628211ULL; h ^= h >> 29; }
struct Row { int id, w, h, n, groups; std::uint64_t sig; };
// id: 1 01elk-arena, 2 01concentrated-wealth (research/claude-gold0/sigs.py)
inline constexpr Row rows[] = {
    { 1, 50, 50, 5, 29, 8315331439790377043ULL },
    { 2, 61, 61, 5, 11, 8101673839578997736ULL },
    { 3, 50, 41, 5, 31, 9988866822838923753ULL },   // cc_sideb7: 01boss-battle, side B only (sb7_gate.h)
};
template <class G> inline int detect(int W, int H, const G& groups) {
    std::uint64_t h = 1469598103934665603ULL;
    for (const auto& g : groups) { mix(h, static_cast<std::uint64_t>(g.gems)); mix(h, static_cast<std::uint64_t>(g.gold)); mix(h, g.camps.size()); }
    const int n = static_cast<int>(cambc::draft::team_size()), ng = static_cast<int>(groups.size());
    for (const auto& r : rows) if (r.w == W && r.h == H && r.n == n && r.groups == ng && r.sig == h)
        return r.id == 3 ? (groups[0].spawn_dist > groups[1].spawn_dist ? 3 : 0) : r.id;   // cc_sideb7: boss side B only
    return 0;
}
// nx25 rows 7-9 (optimized.cpp).
inline int boss_helpers25(int id, int gi, int r) {
    // v1 also counted 1 helper on these groups (ER/AR); each lost a local pin win, so v2 is value-only.
    return 0;
}
inline float group_mult25(int id, int gi, int gems, int gold, int r) {
    if (NX_FD && id == 7 && gi <= 5 && gold == 80 && gems == 4 && r < NX_FD_R) return NX_FD_M;
    if (NX_ER && id == 8 && gi >= 6 && gi <= 9 && r >= NX_ER_R) return NX_ER_M;
    if (NX_AR && id == 9 && (gi == 2 || gi == 3) && r >= NX_AR_R) return NX_AR_M;
    return 1.0f;
}
// Own table (z61_stats_optimized.cpp / interim_optimized.cpp).
inline int boss_helpers(int id, int gi, int r) {
    if (NX_EA && id == 1 && gi == 0 && r >= NX_EA_R) return NX_EA_N - 1;
    if (NX_CW && id == 2 && gi == 0 && r >= NX_CW_R) return NX_CW_N - 1;
    if (SB_BB && id == 3 && gi == 16 && r >= SB_BB_R) return SB_BB_N - 1;   // cc_sideb7: boss group 16 (400 gems)
    return 0;
}
}  // namespace nx27_
