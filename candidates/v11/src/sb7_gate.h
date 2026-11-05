#pragma once
// cc_sideb7 (29 Oct; docs/claude-sideb7-2026-10-29.md). Side-B-only levers on 5 exact maps.
// Gate: the existing exact-layout tables (nx25_gate.h rows 11-13, nx27_gate.h row 3), so maps without a lever
// run no extra instruction (Treasure Box side A wards read fuel_remaining(); any per-call overhead moved a ward).
// Side B = the bot's own spawn_dist of two mirror groups (side A is nearer the first); evaluated once with the
// table's static detect. Elk Ring side B (cb3g_gate.h HoR build, optimized.cpp ET_ELK_BANK) tests spawn tile (28,28).
// Evidence: research/claude-sideb7/ (ranked games since 27 Oct, own frame).
//   SB_TB  02treasure-box: B wins take the own far 61-gem group (spawn_dist 27 from own spawn) 122 gems/game,
//          B losses 39. From SB_TB_R it values x SB_TB_M.
//   SB_SW  00swirl: B losses take the own 23-gem group (sd 11) 144 gems/game vs 312 in A wins. Values x SB_SW_M.
//   SB_PG  00pilgrimage: the 88-gem centre herd 12 decides it (B wins we take 440/game, B losses 94, opponent 533).
//          From SB_PG_R every body counts SB_PG_N - 1 helpers there and values it x SB_PG_M.
//   SB_GR  01gold-rush: the 200-gem centre group 0 (B wins 667/game, B losses 44, opponent 511).
//          From SB_GR_R every body counts SB_GR_N - 1 helpers there and values it x SB_GR_M.
//   SB_BB  01boss-battle: the 400-gem boss group 16 (B wins 667/game banked, B losses 400 taken but bank 184:
//          we die carrying it). From SB_BB_R every body counts SB_BB_N - 1 helpers there.
#include <cambc.h>
#include <cstdint>
#ifndef SB_TB
#define SB_TB 0   // dropped (row removed); Elk Ring side-B gate lives in cb3g_gate.h / optimized.cpp
#endif
#ifndef SB_TB_R
#define SB_TB_R 500
#endif
#ifndef SB_TB_M
#define SB_TB_M 1.3f
#endif
#ifndef SB_SW
#define SB_SW 1
#endif
#ifndef SB_SW_M
#define SB_SW_M 1.5f
#endif
#ifndef SB_PG
#define SB_PG 0
#endif
#ifndef SB_PG_R
#define SB_PG_R 700
#endif
#ifndef SB_PG_M
#define SB_PG_M 1.3f
#endif
#ifndef SB_PG_N
#define SB_PG_N 3
#endif
#ifndef SB_GR
#define SB_GR 1
#endif
#ifndef SB_GR_R
#define SB_GR_R 600
#endif
#ifndef SB_GR_M
#define SB_GR_M 1.3f
#endif
#ifndef SB_GR_N
#define SB_GR_N 1   // v2: value only (helpers N=3 lost the local side-B win)
#endif
#ifndef SB_BB
#define SB_BB 0
#endif
#ifndef SB_BB_R
#define SB_BB_R 900
#endif
#ifndef SB_BB_N
#define SB_BB_N 2
#endif
namespace sb7_ {
// nx25 ids: 11 00swirl, 12 00pilgrimage, 13 01gold-rush. Returns id on side B, else 0.
// Mirror groups (side-A spawn_dist from research/claude-maps/layouts_all): swirl 15-gem 10/11 (6/8),
// pilgrimage 15-gem 10/11 (40/82), gold rush 11-gem 13/14 (8/53).
template <class G> inline int side_b_id(int id, const G& groups) {
    const int ga = id == 13 ? 13 : 10, gb = id == 13 ? 14 : 11;
    return groups[ga].spawn_dist > groups[gb].spawn_dist ? id : 0;
}
inline int helpers(int id, int gems, int r) {
    if (SB_PG && id == 12 && gems == 88 && r >= SB_PG_R) return SB_PG_N - 1;
    if (SB_GR && id == 13 && gems == 200 && r >= SB_GR_R) return SB_GR_N - 1;
    return 0;
}
inline float mult(int id, int gems, int sd, int r) {
    if (SB_SW && id == 11 && gems == 23 && sd <= 12) return SB_SW_M;
    if (SB_PG && id == 12 && gems == 88 && r >= SB_PG_R) return SB_PG_M;
    if (SB_GR && id == 13 && gems == 200 && r >= SB_GR_R) return SB_GR_M;
    return 1.0f;
}
}  // namespace sb7_
