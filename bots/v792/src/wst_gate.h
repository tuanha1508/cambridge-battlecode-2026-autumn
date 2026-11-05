#pragma once
// cc_worst (30 Oct; docs/claude-worst-2026-10-30.md). Two side-gated, exact-layout levers for our worst remaining
// map/sides in ranked play (research/claude-worst/). Each switch is a side mask: 1 = side A, 2 = side B, 0 = off.
//   WS_EA 01elk-arena (z61_stats_optimized.cpp, nx27 row 1). Side A 2-12 since 28 Oct: by r300 the opponent's
//         bodies clear 4-13 of the 28 harmless 40-gold groups each (team gold ~1900) while ours clear 0-3 (~450)
//         and only contest the 72-gem centre group 0. Before WS_EA_R, gold-only groups within WS_EA_SD spawn
//         distance value x WS_EA_M.
//   WS_AV 00anvil (optimized.cpp, nx25 row 32). Side A 3-16 since 26 Oct (0-9 since 29 Oct): the 200-gem/300-gold
//         monolith group 10 decides it; in losses the opponent clears it first (r692-1291) and we never do, in wins
//         we clear it first (r769-1068). Final (local screen, research/claude-worst/screen.txt): WS_EA M5/R300/SD25,
//         WS_AV mode 2 (helpers + x1.5) from r500. Mode 1: from WS_AV_R every body counts WS_AV_N - 1 helpers on group 10
//         (boss co-op); mode 2: group 10 also values x WS_AV_M.
#include <cambc.h>
#include "gc34_gate.h"
#include "mg_switch.h"   // cc_merge_all: all switches (default 0)
#if __has_include("wst_over.h")
#include "wst_over.h"   // local test variants only (not in the shipped folder)
#endif
#ifndef WS_EA
#define WS_EA 1
#endif
#ifndef WS_EA_R
#define WS_EA_R 300
#endif
#ifndef WS_EA_M
#define WS_EA_M 5.0f
#endif
#ifndef WS_EA_SD
#define WS_EA_SD 25
#endif
#ifndef WS_AV
#define WS_AV 1
#endif
#ifndef WS_AV_MODE
#define WS_AV_MODE 2
#endif
#ifndef WS_AV_R
#define WS_AV_R 500
#endif
#ifndef WS_AV_N
#define WS_AV_N 3
#endif
#ifndef WS_AV_M
#define WS_AV_M 1.5f
#endif
namespace wst_ {
inline float ea_mult(int gi, int gems, int gold, int sd, int r) {
    return (WS_EA && gi != 0 && gems == 0 && gold > 0 && sd <= WS_EA_SD && r < WS_EA_R && gc34_::side_on(WS_EA)) ? WS_EA_M : 1.0f;
}
inline int av_helpers(int gi, int r) {
    return (WS_AV && WS_AV_MODE >= 1 && gi == 10 && r >= WS_AV_R && gc34_::side_on(WS_AV)) ? WS_AV_N - 1 : 0;
}
inline float av_mult(int gi, int r) {
    return (WS_AV && WS_AV_MODE == 2 && gi == 10 && r >= WS_AV_R && gc34_::side_on(WS_AV)) ? WS_AV_M : 1.0f;
}
}  // namespace wst_
