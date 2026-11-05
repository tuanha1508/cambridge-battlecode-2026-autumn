#pragma once
// cc_next6 (30 Oct; docs/claude-next6-2026-10-30.md). Three map/side value levers on cc_pin41, each behind its own
// side-mask switch (1 = side A, 2 = side B, 0 = compiled out). No new layout hash: Arena/Swirl reuse the static
// nx25_::detect (rows 200/201 share rows 9/11's signature and sit first; the other side falls through to 9/11),
// Concentrated Wealth reuses interim_optimized.cpp's exact ccw_init() table.
// Evidence: research/claude-next6/ (ranked games since 27 Oct).
//   N6_CW  01concentrated-wealth A: wins clear the 71-gem centre g0 2.1x by r300 (losses 0.6x) and the 60-gold centre
//          flank groups g7/g8 2.1/2.4x by r600 (losses 0.6/0.8x; side-B winners 3.2/3.1x). From N6_CW_R0 (300) until
//          N6_CW_R (1000) groups g7/g8 value x N6_CW_M (1.5). N6_CW_FLEE > 0 also sets the fight flee ratio (ccw_init, once).
//   N6_AR  02arena B: wins clear the 170-gold groups g3/g2 6.1/4.1x by r600, losses 2.2/1.0x; the pinned NX_AR x1.3
//          starts only at r300. Before N6_AR_R groups N6_AR_G value x N6_AR_M (x NX_AR afterwards, unchanged).
//   N6_SW  00swirl B: losses die 14.5x by r600 while side-A winners farm their own near-spawn g10/g20 4.8/5.9x by r600;
//          our side-B win farmed our own g11/g21 8/8x. Before N6_SW_R groups N6_SW_G value x N6_SW_M (x sb7 g19 x1.5).
#include "gc34_gate.h"
#if __has_include("n6_over.h")
#include "n6_over.h"   // local test variants only (not in the shipped folder)
#endif
#ifndef N6_CW
#define N6_CW 0   // OFF in cc_pin44 (side-A lever, not passed); cc_next6 value 1
#endif
#ifndef N6_CW_R0
#define N6_CW_R0 300
#endif
#ifndef N6_CW_R
#define N6_CW_R 1000
#endif
#ifndef N6_CW_M
#define N6_CW_M 1.5f
#endif
#ifndef N6_CW_FLEE
#define N6_CW_FLEE 0.0f
#endif
#ifndef N6_AR
#define N6_AR 2
#endif
#ifndef N6_AR_R
#define N6_AR_R 300
#endif
#ifndef N6_AR_M
#define N6_AR_M 1.5f
#endif
#ifndef N6_AR_G
#define N6_AR_G (1 << 3)
#endif
#ifndef N6_SW
#define N6_SW 0   // OFF in cc_pin44 (only N6_AR passed in G329); cc_next6 value 2
#endif
#ifndef N6_SW_R
#define N6_SW_R 300
#endif
#ifndef N6_SW_M
#define N6_SW_M 1.5f
#endif
#ifndef N6_SW_G
#define N6_SW_G ((1 << 9) | (1 << 11) | (1 << 21))
#endif
namespace n6_ {
inline bool g_in(int mask, int gi) { return gi >= 0 && gi < 31 && ((mask >> gi) & 1); }
inline float cw(int gi, int r) {   // caller: ccw_g == 0 (exact Concentrated Wealth)
    return (N6_CW && gc34_::side_on(N6_CW) && (gi == 7 || gi == 8) && r >= N6_CW_R0 && r < N6_CW_R) ? N6_CW_M : 1.0f;
}
inline float ar(int gi, int r) { return (r < N6_AR_R && g_in(N6_AR_G, gi)) ? N6_AR_M : 1.0f; }   // row 200
inline float sw(int gi, int r) { return (r < N6_SW_R && g_in(N6_SW_G, gi)) ? N6_SW_M : 1.0f; }   // row 201
}  // namespace n6_
