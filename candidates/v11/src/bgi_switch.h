#pragma once
#include "mg_switch.h"   // cc_merge_all
// cc_bgi (28 Oct 2026): exact-layout fixes for 00breadcrumbs / 00galaxy (docs/claude-bgi-2026-10-28.md).
// All three maps dispatch to optimized_main() (draft_memory has no marker byte).
// Gate: word-wise FNV hash of walls/holes/bushes/altars bitboards + the full SDK camp layout + both spawn
// masks combined order-independently (so both spawn sides match), checked once on turn 2 and only when
// width/height/team size already match. Values printed by a BGI_LOG build on the kit 4.0.0 maps.
#include <cstdint>
#include <iostream>
#include <vector>
#ifndef BGI_BC
#define BGI_BC 1        // 00breadcrumbs: bank as soon as the bag reaches BGI_BC_BANK (single centre altar <= ~8 steps)
#endif
#ifndef BGI_BC_BANK
#define BGI_BC_BANK 14  // two 7-gem camps
#endif
#ifndef BGI_GX
#define BGI_GX 0        // 00galaxy: bank-route to our own (spawn-nearer) centre altar
#endif
#ifndef BGI_GX_SLACK
#define BGI_GX_SLACK 10 // ... while that route is at most this many steps longer than the nearest altar
#endif
#ifndef BGI_LOG
#define BGI_LOG 0
#endif
// cc_rs (29 Oct): exact 02stalactites (30x30, 4 picks), docs/claude-rs-2026-10-29.md.
#ifndef RS_ST
#define RS_ST 1         // 02stalactites: bank as soon as the bag reaches RS_ST_BANK (base 30x30 threshold is 36)
#endif
#ifndef RS_ST_BANK
#define RS_ST_BANK 0     // 28 tested: lost to z502_proxy_nyx as side A (970:1022); off
#endif
#ifndef RS_ST_HOLD
#define RS_ST_HOLD 1    // 02stalactites: loaded and outnumbered near the altars -> wait outside instead of walking in
#endif
// cc_bread (30 Oct; docs/claude-bread-2026-10-30.md). 00breadcrumbs (g_bgi 1) only, sides in BC_LB_SIDES (bit0 A, bit1 B).
// Ranked since 27 Oct (research/claude-bread/): after r600 the opponent picks up 133/115 of our dropped gems per lost
// game (A/B) against 31/35 per won game; our late deaths carrying 14+ gems are 6/5 per loss against 2/1 per win.
// Lever: from round BC_LB_R the bank threshold drops from BGI_BC_BANK (14) to BC_LB_BANK (one 7-gem camp).
#ifndef BC_LB_SIDES
#define BC_LB_SIDES 3
#endif
#ifndef BC_LB_R
#define BC_LB_R 600
#endif
#ifndef BC_LB_BANK
#define BC_LB_BANK 7
#endif
namespace bgi_ {
inline int bc_bank(int r) { return (BC_LB_BANK > 0 && r >= BC_LB_R && ((BC_LB_SIDES >> (cambc::detail::cached_my_team & 1)) & 1)) ? BC_LB_BANK : BGI_BC_BANK; }
}
namespace bgi_ {
using namespace cambc;
inline std::uint64_t mix(std::uint64_t h, std::uint64_t w) { h ^= w; h *= 1099511628211ull; return h ^ (h >> 29); }
inline std::uint64_t hash_bb(const bitboard& b, std::uint64_t h) {
    const std::size_t n = bitboard::raw_buffer_size() / sizeof(std::uint64_t);
    const std::uint64_t* p = b.ptr();
    for (std::size_t i = 0; i < n; ++i) h = mix(h, p[i]);
    return h;
}
inline std::uint64_t layout_hash() {
    std::uint64_t h = 1469598103934665603ull;
    h = mix(h, static_cast<std::uint64_t>(map::width()) << 32 | static_cast<std::uint64_t>(map::height()) << 8 | draft::team_size());
    h = mix(h, static_cast<std::uint64_t>(map::symmetry()));
    h = hash_bb(map::walls(), h); h = hash_bb(map::holes(), h); h = hash_bb(map::bushes(), h); h = hash_bb(map::altars(), h);
    const std::uint64_t a = hash_bb(map::spawn_region(team::ally), 7), b = hash_bb(map::spawn_region(team::enemy), 7);
    h = mix(h, a + b); h = mix(h, a ^ b);
    std::uint32_t count = 0;
    std::vector<std::uint32_t> camps(512);
    detail::check(detail::sdk_map_camp_layout(camps.data(), static_cast<std::uint32_t>(camps.size()), &count));
    if (count > camps.size()) { camps.resize(count); detail::check(detail::sdk_map_camp_layout(camps.data(), count, &count)); }
    h = mix(h, count);
    for (std::uint32_t i = 0; i + 1 < count; i += 2) h = mix(h, static_cast<std::uint64_t>(camps[i]) << 32 | camps[i + 1]);
    if (count & 1) h = mix(h, camps[count - 1]);
    return h;
}
// Filled from BGI_LOG runs (kit 4.0.0 maps/00breadcrumbs.map26a, 00galaxy.map26a), same on both sides.
inline constexpr std::uint64_t H_BREADCRUMBS = 2475071230441835280ull;
inline constexpr std::uint64_t H_GALAXY = 298346527533708829ull;
inline constexpr std::uint64_t H_STALACTITES = 15616096567787568983ull;  // BGI_LOG run, kit 4.0.0 maps/02stalactites.map26a
// 1 breadcrumbs, 2 galaxy, 3 stalactites, 0 other. Size/team-size prefilter keeps every other map free of the hash.
inline int detect() {
    const int w = map::width(), h = map::height(), n = static_cast<int>(draft::team_size());
    const bool bc = w == 29 && h == 29 && n == 3, gx = w == 93 && h == 93 && n == 5, st = w == 30 && h == 30 && n == 4;
    if (!bc && !gx && !st) return 0;
    const std::uint64_t v = layout_hash();
#if BGI_LOG
    std::cerr << "BGIHASH " << w << "x" << h << " " << v << std::endl;
#endif
    if (bc && v == H_BREADCRUMBS) return 1;
    if (gx && v == H_GALAXY) return 2;
    if (st && v == H_STALACTITES) return 3;
    return 0;
}
}
