#pragma once
// cc_firstdeath (30 Oct; docs/claude-firstdeath-2026-10-30.md): map-gated "skip camp group X before round R" levers,
// generalising cc_early2 E2_EA (pinned in cc_pin47). Targets come from the FIRST death of each ranked loss in the last
// 300 ranked games (research/claude-firstdeath/firstdeaths.txt): map/sides where the first death before r100 happens
// repeatedly at the same contested camp group. Each switch is a side mask (1 = side A, 2 = side B, 0 = off =
// cc_pin47 behaviour). Detection reuses existing hoisted ids only (lv_gate, nx25 row 20, nx27 row 3, z68 layout
// dimensions), so no round-1 fuel and no new layout row is added (none of rows >= 330 was needed). The skip is one
// `continue` in the choose_target group loop of the policy file the map runs on.
#include <cambc.h>
#include "gc34_gate.h"
#include "mg_switch.h"
#if __has_include("fd_over.h")
#include "fd_over.h"   // local test variants only (not in the shipped folder)
#endif
// Switches (side masks) and the group each one skips before FD_*_R:
//   FD_QD 00quadrants A (z68_golem_optimized.cpp; z68 runs only quadrants at 45x45): group 5 (25 gems, (36.5,3.5)).
//         Cassi dies there at r73-90 after our r58 clear (3/3 side-A losses).
//   FD_VA 00vault B (optimized.cpp, lv_gate id 4): centre vault group 0 (343 gems). First deaths r45-82 at/near it
//         against 2-4 enemies (5 side-B losses, 3 at group 0).
//   FD_CO 01corridors B (optimized.cpp, lv_gate id 3): 58-gem group 1. Heart of Ruin dies to the camp at r29-33
//         (4/4 side-B losses).
//   FD_BB 01boss-battle B (interim_optimized.cpp, nx27 id 3 = side B only): 30-gem/220-gold group 9. Varg dies to
//         the camp at r68 (4/4 side-B losses).
//   FD_AN 00anvil A (optimized.cpp, nx25 id 20 = side A only): 21-gem crystal group 9 next to the boss. First deaths
//         r70/74 there (2 of 5 side-A losses; 2 more at gold group 5 at r86).
#ifndef FD_QD
#define FD_QD 0
#endif
#ifndef FD_QD_G
#define FD_QD_G 5
#endif
#ifndef FD_QD_R
#define FD_QD_R 100
#endif
#ifndef FD_VA
#define FD_VA 0
#endif
#ifndef FD_VA_G
#define FD_VA_G 0
#endif
#ifndef FD_VA_R
#define FD_VA_R 100
#endif
#ifndef FD_CO
#define FD_CO 0
#endif
#ifndef FD_CO_G
#define FD_CO_G 1
#endif
#ifndef FD_CO_R
#define FD_CO_R 100
#endif
#ifndef FD_CO_WHO
#define FD_CO_WHO 0   // 0 = every body, 1 = Heart of Ruin only (the body that dies to the camp)
#endif
#ifndef FD_BB
#define FD_BB 0
#endif
#ifndef FD_BB_G
#define FD_BB_G 9
#endif
#ifndef FD_BB_R
#define FD_BB_R 100
#endif
#ifndef FD_AN
#define FD_AN 0
#endif
#ifndef FD_AN_G
#define FD_AN_G 9
#endif
#ifndef FD_AN_R
#define FD_AN_R 100
#endif
namespace fd_ {
// optimized.cpp: lvid = hoisted lv_::detect, nxid = hoisted nx25_::detect.
inline bool skip_opt(int lvid, int nxid, int gi, int r, bool hor) {
    if (FD_VA && lvid == 4 && gi == FD_VA_G && r < FD_VA_R && gc34_::side_on(FD_VA)) return true;
    if (FD_CO && lvid == 3 && gi == FD_CO_G && (FD_CO_WHO == 0 || hor) && r < FD_CO_R && gc34_::side_on(FD_CO)) return true;
    if (FD_AN && nxid == 20 && gi == FD_AN_G && r < FD_AN_R && gc34_::side_on(FD_AN)) return true;
    return false;
}
// interim_optimized.cpp: nx27id = hoisted nx27_::detect (3 = 01boss-battle side B).
inline bool skip_interim(int nx27id, int gi, int r) {
    return FD_BB && nx27id == 3 && gi == FD_BB_G && r < FD_BB_R && gc34_::side_on(FD_BB);
}
// z68_golem_optimized.cpp: the only 45x45 z68 layout is 00quadrants.
inline bool skip_z68(int W, int H, int gi, int r) {
    return FD_QD && r < FD_QD_R && W == 45 && H == 45 && gi == FD_QD_G && gc34_::side_on(FD_QD);
}
}  // namespace fd_
