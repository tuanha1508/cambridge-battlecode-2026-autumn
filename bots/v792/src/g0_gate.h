#pragma once
// cc_gold0 (28 Oct; docs/claude-gold0-2026-10-28.md): the 00quarry CCQ rule (gem-less groups at CCQ_GOLD before
// CCQ_R) extended to more maps through an exact-layout gate. One compile switch per map (G0_<MAP>, 1 = on).
// Gate: width, height, team size, group count and a 64-bit word mix over (gems, gold, camp count) of every camp group
// in index order (research/claude-gold0/sigs.py computes the same value from research/claude-maps/layouts_all).
#include <cambc.h>
#include <cstdint>
#include <iostream>
#ifndef G0_DUMP
#define G0_DUMP 0
#endif
namespace g0_ {
inline void mix(std::uint64_t& h, std::uint64_t w) { h ^= w; h *= 1099511628211ULL; h ^= h >> 29; }
struct Row { int w, h, n, groups; std::uint64_t sig; int on; };
inline constexpr Row rows[] = {
#include "g0_rows.h"
    { 0, 0, 0, 0, 0ULL, 0 }
};
template <class G> inline bool on(int W, int H, const G& groups) {
    std::uint64_t h = 1469598103934665603ULL;
    for (const auto& g : groups) { mix(h, static_cast<std::uint64_t>(g.gems)); mix(h, static_cast<std::uint64_t>(g.gold)); mix(h, g.camps.size()); }
    const int n = static_cast<int>(cambc::draft::team_size()), ng = static_cast<int>(groups.size());
    bool hit = false;
    for (const auto& r : rows) if (r.on && r.w == W && r.h == H && r.n == n && r.groups == ng && r.sig == h) hit = true;
#if G0_DUMP
    std::cout << "G0SIG " << W << " " << H << " " << n << " " << ng << " " << h << "ULL hit " << hit << "\n";
#endif
    return hit;
}
}  // namespace g0_
