#pragma once
// cc_next5 (30 Oct; docs/claude-next5-2026-10-30.md), ported for cc_batchA3/B3 (docs/claude-batches3-2026-10-30.md).
// Only the GC2 B flee (161) and Road A late flee (162) hooks are ported; the Gem Fields A group_mult hook (160) is not.
// Three side-masked levers on map/sides nobody had studied
// (ranked games since 27 Oct; research/claude-next5/). Each switch is a side mask (1 = side A, 2 = side B, 0 = off),
// tested with gc34_::side_on. No new map detection: each lever reuses an existing exact-layout flag.
//   N5_GF  (lever id 160) 00gem-fields SIDE A, nx25 row 2 (z115_policy.cpp group_mult hook). Side-A losses (4-14):
//          the side-B winners clear the 71-gem/90-gold centre group g5 4.0 times by r600 (we 1.2; our wins 3.2) and
//          our r300 gold is 570 vs 981 in wins. From N5_GF_R group 5 values x N5_GF_M (r100 lost 2 local wins; r300 kept all).
//   N5_GC2 (lever id 161) adgato-gemcombat2 SIDE B, existing g_cargo flag (optimized.cpp, set on turn 3). Side-B
//          losses (10-13): clears equal wins, but we die 13 times (wins 4, opponent 2.2) and drop 263 gems (wins 51).
//          On the gated side the fight flee ratio (g_mm_flee) becomes N5_GC2_FLEE from turn 3.
//   N5_RR  (lever id 162) 00road-of-riches SIDE A, z113 executor (exact Road layout only). Side-A losses (4-12): clears
//          equal wins, but we die 11.3 times (wins 3.8) and drop 266 gems, mostly r600+ near (8-16,16).
//          On the gated side the fight flee ratio becomes N5_RR_FLEE from round N5_RR_FR (all-game 1.15/1.3/1.5 lost
//          the local win vs cc_pin16). N5_RR_BANK (late own-altar bank) was inert locally and is off.
#include "gc34_gate.h"
#include "mg_switch.h"   // cc_batch3: N5_RR / N5_GC2 / N5_GF switches (default 0 in mg_switch.h)
#if __has_include("n5_over.h")
#include "n5_over.h"   // local test variants only (not in the shipped folder)
#endif
#ifndef N5_GF
#define N5_GF 1
#endif
#ifndef N5_GF_R
#define N5_GF_R 300
#endif
#ifndef N5_GF_M
#define N5_GF_M 1.5f
#endif
#ifndef N5_GC2
#define N5_GC2 2
#endif
#ifndef N5_GC2_FLEE
#define N5_GC2_FLEE 1.15f
#endif
#ifndef N5_RR
#define N5_RR 1
#endif
#ifndef N5_RR_FLEE
#define N5_RR_FLEE 1.3f
#endif
#ifndef N5_RR_FR
#define N5_RR_FR 600
#endif
#ifndef N5_RR_BANK
#define N5_RR_BANK 0
#endif
#ifndef N5_RR_BR
#define N5_RR_BR 600
#endif
#ifndef N5_RR_NEAR
#define N5_RR_NEAR 12
#endif
namespace n5_ {
inline bool gf(int id, int gi, int gems, int r) { return N5_GF && id == 2 && gi == 5 && gems == 71 && r >= N5_GF_R && gc34_::side_on(N5_GF); }
inline bool gc2_on() { return N5_GC2 && N5_GC2_FLEE > 0.0f && gc34_::side_on(N5_GC2); }
inline bool rr_flee(int r) { return N5_RR && N5_RR_FLEE > 0.0f && r >= N5_RR_FR && gc34_::side_on(N5_RR); }
inline bool rr_bank(int r, int gems, int dalt) { return N5_RR && N5_RR_BANK > 0 && r >= N5_RR_BR && gems >= N5_RR_BANK && dalt <= N5_RR_NEAR && gc34_::side_on(N5_RR); }
}  // namespace n5_
