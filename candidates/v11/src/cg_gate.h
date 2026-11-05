#pragma once
// cc_cheapgold (30 Oct; docs/claude-cheapgold-2026-10-30.md). Early cheap gold on own side, per map and side.
// Evidence: ranked games 28-30 Oct (research/claude-cheapgold/): in our losses on these map/sides the opponent clears
// many gem-less, harmless (mdps 0) gold groups before r300 and out-earns us by r300; we clear few.
// Each switch is a side mask (gc34_::side_on: 1 = side A, 2 = side B, 3 = both, 0 = off). Own-frame spawn_dist.
//   CG_GR 01gold-rush (nx25 row 13; on side B sb7_::side_b_id returns 13): losses 594:2129 r300 gold; opponent
//         clears the 100-gold centre groups 1-4 ~12x by r300, we ~3. Before CG_GR_R gem-less groups x CG_GR_M.
//   CG_QY 00quarry (nx25 row 33): side-B losses 844:1270; the opponent clears its own 40/60-gold groups 2/4/6 ~12x
//         by r300, we ours (3/5/7) ~6x (wins ~10x). The CCQ discount (x0.25 before r300, slots 2+) blocks them.
//         Before CG_QY_R gem-less groups with spawn_dist <= CG_QY_SD value x CG_QY_M.
//   CG_EA 01elk-arena (z61_stats_optimized.cpp, nx27 row 1): side-B losses 612:1792; opponent bodies clear ~29
//         harmless 40-gold groups by r300, we ~2. Before CG_EA_R gem-less groups within CG_EA_SD value x CG_EA_M
//         (side-B mirror of cc_worst WS_EA).
//   CG_SP 00stampede (z69_optimized.cpp, nx25 row 34): side-A losses 908:1770; the side-B opponent clears its five
//         40-gold lane groups and the 80-gold centre pair 11/12 ~34x by r300, we ~5 (one pass of our lane).
//         Before CG_SP_R gem-less groups with spawn_dist <= CG_SP_SD (own lane 0/2/4/6/8 and 11/12) value x CG_SP_M.
#include <cambc.h>
#include "gc34_gate.h"
#include "mg_switch.h"   // cc_merge_all: all switches (default 0)
#if __has_include("cg_over.h")
#include "cg_over.h"   // local test variants only (not in the shipped folder)
#endif
#ifndef CG_GR
#define CG_GR 2
#endif
#ifndef CG_GR_R
#define CG_GR_R 300
#endif
#ifndef CG_GR_M
#define CG_GR_M 1.5f
#endif
#ifndef CG_QY
#define CG_QY 2
#endif
#ifndef CG_QY_R
#define CG_QY_R 300
#endif
#ifndef CG_QY_M
#define CG_QY_M 4.0f
#endif
#ifndef CG_QY_SD
#define CG_QY_SD 15
#endif
#ifndef CG_EA
#define CG_EA 0   // off: every local variant (x5/sd25, x2/sd25, x3/sd12, x1.5/sd40) lost side-B wins
#endif
#ifndef CG_EA_R
#define CG_EA_R 300
#endif
#ifndef CG_EA_M
#define CG_EA_M 5.0f
#endif
#ifndef CG_EA_SD
#define CG_EA_SD 25
#endif
#ifndef CG_SP
#define CG_SP 1
#endif
#ifndef CG_SP_R
#define CG_SP_R 300
#endif
#ifndef CG_SP_M
#define CG_SP_M 3.0f   // local screen: x2 kept wins but no gain, x3 gained the mirror and cc_pond games
#endif
#ifndef CG_SP_SD
#define CG_SP_SD 48
#endif
namespace cg_ {
inline float sp(int gems, int gold, int sd, int r) {
    return (CG_SP && gems == 0 && gold > 0 && sd <= CG_SP_SD && r < CG_SP_R && gc34_::side_on(CG_SP)) ? CG_SP_M : 1.0f;
}
inline float gr(int gems, int gold, int r) {
    return (CG_GR && gems == 0 && gold > 0 && r < CG_GR_R && gc34_::side_on(CG_GR)) ? CG_GR_M : 1.0f;
}
inline float qy(int gems, int gold, int sd, int r) {
    return (CG_QY && gems == 0 && gold > 0 && sd <= CG_QY_SD && r < CG_QY_R && gc34_::side_on(CG_QY)) ? CG_QY_M : 1.0f;
}
inline float ea(int gi, int gems, int gold, int sd, int r) {
    return (CG_EA && gi != 0 && gems == 0 && gold > 0 && sd <= CG_EA_SD && r < CG_EA_R && gc34_::side_on(CG_EA)) ? CG_EA_M : 1.0f;
}
}  // namespace cg_
