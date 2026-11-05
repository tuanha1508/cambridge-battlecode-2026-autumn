#pragma once
// cc_gc34 (29 Oct; docs/claude-gc34-2026-10-29.md). Exact-layout levers for adgato-gemcombat3/4, reached through the
// cc_pin32 (v721 + GC3 only): GC4 is off and its nx25 row is not ported.
// nx25_gate.h hooks in optimized.cpp (rows 10/11). Evidence: ranked games since 27 Oct (research/claude-gc34/).
//   GC3 (51x51, nx25 row 10): the 42-gem centre groups 8/9 separate wins from losses on both sides by r600
//       (A: g9 200:76 W vs 112:177 L; B: g8/g9 168:112, 182:98 W vs 131:147, 131:165 L). From GC3_R, groups 8/9
//       value x GC3_M (GC3 & 1) and/or every body counts one helper there (GC3 & 2). Side mask GC3_SIDES (bit0 A, bit1 B).
//   GC4 (41x61, nx25 row 11): in wins we also take the enemy-side 42-gem group (A: g3 42 W vs 9 L by r600; B: g2 74
//       vs 37); in losses the opponent farms it unopposed. From GC4_R the enemy-side 42-gem group (spawn_dist > 30)
//       values x GC4_M (GC4 & 1) and/or every body counts one helper there (GC4 & 2). Side mask GC4_SIDES.
#include <cambc.h>
#ifndef GC3
#define GC3 1
#endif
#ifndef GC3_R
#define GC3_R 300
#endif
#ifndef GC3_M
#define GC3_M 1.3f
#endif
#ifndef GC3_SIDES
#define GC3_SIDES 3
#endif
#ifndef GC4
#define GC4 0   // cc_pin32: off (G-trial scored 0); row 11 not ported (id 11 is sb7 00swirl)
#endif
#ifndef GC4_R
#define GC4_R 300
#endif
#ifndef GC4_M
#define GC4_M 1.3f
#endif
#ifndef GC4_SIDES
#define GC4_SIDES 3
#endif
namespace gc34_ {
inline bool side_on(int mask) { return (mask >> (cambc::detail::cached_my_team & 1)) & 1; }
inline bool gc3g(int gi, int gems, int r) { return (gi == 8 || gi == 9) && gems == 42 && r >= GC3_R && side_on(GC3_SIDES); }
inline bool gc4g(int gems, int sd, int r) { return gems == 42 && sd > 30 && r >= GC4_R && side_on(GC4_SIDES); }
inline int boss_helpers(int id, int gi, int gems, int sd, int r) {
    if ((GC3 & 2) && id == 10 && gc3g(gi, gems, r)) return 1;
    if ((GC4 & 2) && id == 11 && gc4g(gems, sd, r)) return 1;
    return 0;
}
inline float group_mult(int id, int gi, int gems, int sd, int r) {
    if ((GC3 & 1) && id == 10 && gc3g(gi, gems, r)) return GC3_M;
    if ((GC4 & 1) && id == 11 && gc4g(gems, sd, r)) return GC4_M;
    return 1.0f;
}
}  // namespace gc34_
