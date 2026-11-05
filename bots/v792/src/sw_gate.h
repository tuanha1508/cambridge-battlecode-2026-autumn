#pragma once
// cc_sweep (30 Oct; docs/claude-sweep-2026-10-30.md). SIDE B only (side masks via gc34_::side_on), exact-layout nx25
// rows 35/36. Evidence: research/claude-sweep/ (ranked side-B games since 27 Oct; worst side-B records on the maps of
// the 04:30 Argon enjoyers 0:5 sweep).
//   SW_RW  00rows (row 35, optimized.cpp): side B 10-12. In 8 of 12 losses the opponent clears the 470-gem/600-gold
//          centre monolith g6 at r1097-1351 while we lead on banked gems; we clear it in 1 win and never otherwise.
//          From SW_RW_R every body counts SW_RW_N - 1 helpers on g6 (passes the P_MONO gate together) and values it
//          x SW_RW_M.
//   SW_EA  01elk-arena (row 36, z61_stats_optimized.cpp hook): side B 5-15 (0-5 since 29 Oct). Losses earn 871:1476
//          gold by r300 and 1476:3141 by r600; the side-A opponent farms its 40-gold harmless groups (g5/g18/g23/g27,
//          ~3 each before r300) and we never clear the mirrors (g6/g17/g24/g28). In [SW_EA_R0, SW_EA_R1) gem-less
//          groups with own spawn_dist <= SW_EA_SD value x SW_EA_M.
#include <cambc.h>
#include "gc34_gate.h"
#include "mg_switch.h"   // cc_merge_all: all switches (default 0)
#if __has_include("sw_over.h")
#include "sw_over.h"   // local test variants only (not in the shipped folder)
#endif
#ifndef SW_RW
#define SW_RW 2
#endif
#ifndef SW_RW_R
#define SW_RW_R 900
#endif
#ifndef SW_RW_N
#define SW_RW_N 2   // v4: N 4 (gang-up) died to the monolith 9-24 times and lost the nyx win; N 2 keeps the P_MONO strength check
#endif
#ifndef SW_RW_M
#define SW_RW_M 1.5f
#endif
#ifndef SW_EA
#define SW_EA 2
#endif
#ifndef SW_EA_R0
#define SW_EA_R0 300
#endif
#ifndef SW_EA_R1
#define SW_EA_R1 600
#endif
#ifndef SW_EA_SD
#define SW_EA_SD 12
#endif
#ifndef SW_EA_M
#define SW_EA_M 2.0f
#endif
namespace sw_ {
inline bool rw_g(int gi, int gems, int r) { return SW_RW && gi == 6 && gems == 470 && r >= SW_RW_R && gc34_::side_on(SW_RW); }
inline int rw_helpers(int gi, int gems, int r) { return (SW_RW_N > 1 && rw_g(gi, gems, r)) ? SW_RW_N - 1 : 0; }
inline float rw_mult(int gi, int gems, int r) { return rw_g(gi, gems, r) ? SW_RW_M : 1.0f; }
inline float ea(int gi, int gems, int gold, int sd, int r) {
    return (SW_EA && gi != 0 && gems == 0 && gold > 0 && sd <= SW_EA_SD && r >= SW_EA_R0 && r < SW_EA_R1 && gc34_::side_on(SW_EA)) ? SW_EA_M : 1.0f;
}
}  // namespace sw_
