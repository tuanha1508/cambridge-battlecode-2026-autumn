#pragma once
// cc_cargo: exact-layout gate (copy of cc_bank3 cc_layout.h, renamed). 64-bit FNV-1a digest over the map size, team
// size, symmetry, walls/holes/bushes/altars masks, both spawn masks (side-independent order) and the full camp layout.
#include <cambc.h>
#include <vector>
#include "cargo_switch.h"
namespace cargo_layout_ {
using namespace cambc;
inline void fnv(std::uint64_t& h, const void* p, std::size_t n) {
    const auto* b = static_cast<const unsigned char*>(p);
    for (std::size_t i = 0; i < n; ++i) { h ^= b[i]; h *= 1099511628211ULL; }
}
inline std::uint64_t digest() {
    std::uint64_t h = 1469598103934665603ULL;
    const std::int32_t hd[4] = { map::width(), map::height(), static_cast<std::int32_t>(draft::team_size()), static_cast<std::int32_t>(map::symmetry()) };
    fnv(h, hd, sizeof hd);
    const std::size_t bytes = bitboard::raw_buffer_size();
    const bitboard* bb[] = { &map::walls(), &map::holes(), &map::bushes(), &map::altars() };
    for (auto* b : bb) fnv(h, b->ptr(), bytes);
    std::uint64_t ha = 1469598103934665603ULL, hb = 1469598103934665603ULL;
    fnv(ha, map::spawn_region(team::ally).ptr(), bytes); fnv(hb, map::spawn_region(team::enemy).ptr(), bytes);
    const std::uint64_t lo = ha < hb ? ha : hb, hi = ha < hb ? hb : ha;
    fnv(h, &lo, 8); fnv(h, &hi, 8);
    std::vector<std::uint32_t> w(256); std::uint32_t count = 0;
    detail::check(detail::sdk_map_camp_layout(w.data(), static_cast<std::uint32_t>(w.size()), &count));
    if (count > w.size()) { w.resize(count); detail::check(detail::sdk_map_camp_layout(w.data(), count, &count)); }
    fnv(h, w.data(), count * sizeof(std::uint32_t));
    return h;
}
inline constexpr std::uint64_t on_maps[] = { CARGO_LAYOUTS };
inline constexpr int dims[][3] = { CARGO_DIMS };
}  // namespace cargo_layout_
// Call only from turn 3 on (keeps turns 1-2 free of new SDK work). Cheap size/team prefilter first, so no other
// layout pays for the digest.
inline bool cargo_map_on() {
    static int v = -1;
    if (v < 0) {
        const int w = cambc::map::width(), h = cambc::map::height(), n = static_cast<int>(cambc::draft::team_size());
        bool cand = false;
        for (const auto& c : cargo_layout_::dims) if (c[0] == w && c[1] == h && c[2] == n) cand = true;
        v = 0;
        if (!cand) return false;
        const std::uint64_t d = cargo_layout_::digest();
        for (auto x : cargo_layout_::on_maps) if (x != 0 && x == d) v = 1;
    }
    return v == 1;
}
