#pragma once
// cc_late (30 Oct; docs/claude-late-2026-10-30.md). Late-collapse levers: map/sides where our ranked losses were level or
// ahead at r600 (banked + carried gems) and were lost after r600 (research/claude-late/). Each lever is map-gated (exact
// layout fingerprint, computed once per game) and side-masked (1 = side A, 2 = side B). 0 compiles back to cc_pin47.
//   row 360 LT_BB 01boss-battle A: bank LT_BB_BANK gems within LT_BB_NEAR of the altar from LT_BB_R (n4_ row 121 hook)
//   row 361 LT_HV 02harvest A: the enemy-side 29-gem group 15 is valued xLT_HV_M from LT_HV_R (legacy.cpp)
//   row 362 LT_MO 00monolith B: from LT_MO_R bank at LT_MO_BANK gems within LD_NEAR of an altar, and gem-less centre
//            groups 12-15 valued xLT_MO_M (optimized.cpp; the value part alone was inert locally)
//   row 363 LT_BW 01big-world A: the 228-gem monolith group 17 is valued xLT_BW_M from LT_BW_R (z78_m1.cpp)
#include <cambc.h>
#include "mg_switch.h"
namespace lt_ {
inline bool side_on(int mask) { return (mask >> (cambc::detail::cached_my_team & 1)) & 1; }
// Fingerprints (static per game). G is the policy's group vector (gems, gold, camps).
template <class G> inline bool is_harvest(int W, int H, const G& g) {
    return W == 100 && H == 100 && g.size() == 21 && g[20].gems == 114 && g[14].gems == 29 && g[15].gems == 29 && g[18].gems == 43;
}
template <class G> inline bool is_monolith(int W, int H, const G& g) {
    return W == 50 && H == 50 && g.size() == 20 && g[10].gems == 22 && g[11].gems == 22 && g[12].gems == 0 && g[13].gems == 0 && g[14].gems == 0 && g[15].gems == 0;
}
template <class G> inline bool is_bigworld(int W, int H, const G& g) {
    return W == 100 && H == 100 && g.size() == 24 && g[16].gems == 228 && g[17].gems == 228;
}
inline float hv(int gi, int r) { return (LT_HV && gi == 15 && r >= LT_HV_R && side_on(LT_HV)) ? LT_HV_M : 1.0f; }
inline float mo(int gi, int gems, int r) { return (LT_MO && gi >= 12 && gi <= 15 && gems == 0 && r >= LT_MO_R && side_on(LT_MO)) ? LT_MO_M : 1.0f; }
inline float bw(int gi, int r) { return (LT_BW && gi == 17 && r >= LT_BW_R && side_on(LT_BW)) ? LT_BW_M : 1.0f; }
}  // namespace lt_
