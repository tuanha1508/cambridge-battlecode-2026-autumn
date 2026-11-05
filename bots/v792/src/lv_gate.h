#pragma once
// cc_levers (29 Oct; docs/claude-levers-2026-10-29.md). Exact-layout gate (same signature as g0_gate.h) with one
// compile switch per map.
// Lever 4 (camp groups, research/claude-wins/groups.txt): multiply one group's target value until a round.
//   LV_BC  00breadcrumbs: group 5 by r150 won 7/7 vs 2/9 -> x LV_BC_M before LV_BC_R.
//   LV_CR  00crater:      group 5 by r300 won 12/13 vs 3/9 -> x LV_CR_M before LV_CR_R.
//   LV_CO  01corridors:   group 1 by r300 won 2/13 vs 7/9 -> x LV_CO_M (avoid) before LV_CO_R.
// Lever 2 (Marcella r300 gold, research/claude-levers/marc_thr.txt): on these maps Marcella's gold weight has a floor
// of MG_W before MG_R (targeting only; fights/flee logic untouched).
//   MG_CR 00crater, MG_VA 00vault, MG_GW 02gift-wrapping.
#include <cambc.h>
#include <cstdint>
#if __has_include("lv_over.h")
#include "lv_over.h"   // local test variants only (not in the shipped folder)
#endif
#ifndef LV_BC
#define LV_BC 0   // off: G110 +2/-1
#endif
#ifndef LV_BC_M
#define LV_BC_M 4.0f
#endif
#ifndef LV_BC_R
#define LV_BC_R 150
#endif
#ifndef LV_CR
#define LV_CR 0   // local: lost to cc_pin16 on both sides (A 324-1018, B 651-1021)
#endif
#ifndef LV_CR_M
#define LV_CR_M 3.0f
#endif
#ifndef LV_CR_R
#define LV_CR_R 300
#endif
#ifndef LV_CO
#define LV_CO 0   // off: G107 -1
#endif
#ifndef LV_CO_M
#define LV_CO_M 0.15f
#endif
#ifndef LV_CO_R
#define LV_CO_R 300
#endif
#ifndef MG_CR
#define MG_CR 0   // off: untested on the platform
#endif
#ifndef MG_VA
#define MG_VA 1
#endif
#ifndef MG_GW
#define MG_GW 0   // local: lost to cc_pin16 on both sides (A 232-1044, B 276-1032)
#endif
#ifndef MG_W
#define MG_W 1.0f
#endif
#ifndef MG_R
#define MG_R 300
#endif
namespace lv_ {
inline void mix(std::uint64_t& h, std::uint64_t w) { h ^= w; h *= 1099511628211ULL; h ^= h >> 29; }
struct Row { int id, w, h, n, groups; std::uint64_t sig; };
// id: 1 breadcrumbs, 2 crater, 3 corridors, 4 vault, 5 gift-wrapping (signatures from research/claude-gold0/sigs.py)
inline constexpr Row rows[] = {
    { 1, 29, 29, 3, 34, 8697086349888846795ULL },
    { 2, 61, 61, 5, 6, 13731565649778941367ULL },
    { 3, 51, 40, 5, 12, 1682932226756082396ULL },
    { 4, 31, 31, 4, 7, 8295142255273240446ULL },
    { 5, 45, 45, 5, 11, 11481580556377659658ULL },
};
template <class G> inline int detect(int W, int H, const G& groups) {
    std::uint64_t h = 1469598103934665603ULL;
    for (const auto& g : groups) { mix(h, static_cast<std::uint64_t>(g.gems)); mix(h, static_cast<std::uint64_t>(g.gold)); mix(h, g.camps.size()); }
    const int n = static_cast<int>(cambc::draft::team_size()), ng = static_cast<int>(groups.size());
    for (const auto& r : rows) if (r.w == W && r.h == H && r.n == n && r.groups == ng && r.sig == h) return r.id;
    return 0;
}
// Lever 4: value multiplier for group gi at round r on layout id.
inline float group_mult(int id, int gi, int r) {
    if (LV_BC && id == 1 && gi == 5 && r < LV_BC_R) return LV_BC_M;
    if (LV_CR && id == 2 && gi == 5 && r < LV_CR_R) return LV_CR_M;
    if (LV_CO && id == 3 && gi == 1 && r < LV_CO_R) return LV_CO_M;
    return 1.0f;
}
// Lever 2: is the Marcella gold floor on for this layout?
inline bool marc_gold(int id) { return (MG_CR && id == 2) || (MG_VA && id == 4) || (MG_GW && id == 5); }
}  // namespace lv_
