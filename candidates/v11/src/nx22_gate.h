#pragma once
// cc_next22b (29 Oct; docs/claude-next22-2026-10-29.md). Exact-layout gate (same FNV group signature as lv_gate.h /
// nx21_gate.h), one switch per map. Evidence: ranked games since 29 Oct 00:00 (research/claude-next22/*.txt).
//   NX_BW  01big-world (z78_m1.cpp): we lead at r800 in 3/4 losses but take the 228-gem centre groups 16/17 late
//          (first clear r937-1105 in losses vs r884-1021 in wins; opponents almost never take them). From NX_BW_R
//          every body counts NX_BW_N helpers on a 200+ gem group (P_MONO gate divided by the helpers).
//   NX_AN  00anvil: opponents' Vesper solos the 200-gem centre monolith (group 10) at r692-988 in all 4 losses; we take
//          it only in wins. From NX_AN_R every body counts NX_AN_N helpers on group 10.
//   NX_G4  adgato-gemcombat4: opponents bank first at r39-102 in 28-70 gem trips; our first bank is r200+ and 4-6
//          bodies die holding gems (0/4). Bank at NX_G4_BANK carried gems (cf. Gemcombat2 bank-at-21, pinned).
//   NX_GR  01gold-rush: only 11-gem groups pay gems; P_BANK 60 means 0-2 banks per body and bodies die holding 11-22
//          (we bank 0/77/132 in three losses); winners bank every 11. Bank at NX_GR_BANK carried gems.
//   NX_RB  00ribs: all gems are the centre-south groups 0-2; in losses all six bodies have 0 gold at r300 and die there
//          30-39 times; winners farm the 20-gold single wolves first. Before NX_RB_R, bodies other than Ryth/Vendetta
//          value groups 0-2 at x NX_RB_M.
#include <cambc.h>
#include <cstdint>
#ifndef NX_BW
#define NX_BW 1
#endif
#ifndef NX_BW_R
#define NX_BW_R 600
#endif
#ifndef NX_BW_N
#define NX_BW_N 3
#endif
#ifndef NX_AN
#define NX_AN 0
#endif
#ifndef NX_AN_R
#define NX_AN_R 500
#endif
#ifndef NX_AN_N
#define NX_AN_N 3
#endif
#ifndef NX_G4
#define NX_G4 0
#endif
#ifndef NX_G4_BANK
#define NX_G4_BANK 21
#endif
#ifndef NX_GR
#define NX_GR 0
#endif
#ifndef NX_GR_BANK
#define NX_GR_BANK 11
#endif
#ifndef NX_RB
#define NX_RB 1
#endif
#ifndef NX_RB_R
#define NX_RB_R 250
#endif
#ifndef NX_RB_M
#define NX_RB_M 0.25f
#endif
namespace nx22_ {
inline void mix(std::uint64_t& h, std::uint64_t w) { h ^= w; h *= 1099511628211ULL; h ^= h >> 29; }
struct Row { int id, w, h, n, groups; std::uint64_t sig; };
// id: 1 01big-world, 2 00anvil, 3 adgato-gemcombat4, 4 01gold-rush, 5 00ribs (research/claude-gold0/sigs.py)
inline constexpr Row rows[] = {
    { 1, 100, 100, 5, 24, 5696588763722880782ULL },
    { 2, 37, 31, 3, 13, 8103515028965712125ULL },
    { 3, 41, 61, 5, 8, 12213907621481157360ULL },
    { 4, 31, 31, 3, 15, 5939772593377448505ULL },
    { 5, 51, 85, 5, 13, 7795748330913759336ULL },
};
template <class G> inline int detect(int W, int H, const G& groups) {
    std::uint64_t h = 1469598103934665603ULL;
    for (const auto& g : groups) { mix(h, static_cast<std::uint64_t>(g.gems)); mix(h, static_cast<std::uint64_t>(g.gold)); mix(h, g.camps.size()); }
    const int n = static_cast<int>(cambc::draft::team_size()), ng = static_cast<int>(groups.size());
    for (const auto& r : rows) if (r.w == W && r.h == H && r.n == n && r.groups == ng && r.sig == h) return r.id;
    return 0;
}
// Boss co-op: helper count to assume on group gi (0 = no change).
inline int boss_helpers(int id, int gi, int gems, int r) {
    if (NX_BW && id == 1 && gems >= 200 && r >= NX_BW_R) return NX_BW_N - 1;
    if (NX_AN && id == 2 && gi == 10 && r >= NX_AN_R) return NX_AN_N - 1;
    return 0;
}
// Bank threshold override (0 = none).
inline int bank(int id) {
    if (NX_G4 && id == 3) return NX_G4_BANK;
    if (NX_GR && id == 4) return NX_GR_BANK;
    return 0;
}
// Targeting multiplier (optimized.cpp only).
inline float group_mult(int id, cambc::champion_type t, int gi, int r) {
    if (NX_RB && id == 5 && r < NX_RB_R && gi <= 2 && t != cambc::champion_type::ryth && t != cambc::champion_type::vendetta) return NX_RB_M;
    return 1.0f;
}
}  // namespace nx22_
