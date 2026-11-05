#pragma once
// cc_early2 (30 Oct; docs/claude-early2-2026-10-30.md): map-gated levers against the specific situation in which our
// FIRST death happens in ranked losses (research/claude-early2/deaths.txt). Each switch is a side mask
// (1 = side A, 2 = side B, 0 = off = cc_pin43 bytes-equivalent behaviour).
//   E2_EA 01elk-arena (z61_stats_optimized.cpp, reuses the hoisted nx27 detect id 1). First deaths in A/B losses come
//         at r15-r53 in the centre group-0 (elk) fight against 4-5 enemies, right after (or while) we take the first
//         elk and then wait at the respawning group. Mode 1: before E2_EA_R a body skips group 0 while it is
//         respawning (wait > travel + E2_EA_W), so it farms the harmless crystal groups instead of waiting in the
//         contested centre. Mode 2: group 0 is skipped entirely before E2_EA_R (delay the contested camp).
//   E2_EI 00eight (optimized.cpp, nx25 row 320, same exact-layout signature as cc_vfirst row 110). First deaths in side-B
//         losses: Vesper at r44 / r48 near the centre groups (x 18-22) or at our first camp against 3-4 enemies.
//         Before E2_EI_R, the bodies in E2_EI_WHO skip groups whose first camp is on the enemy side of x = E2_EI_X.
#include <cambc.h>
#include "gc34_gate.h"
#include "mg_switch.h"
#if __has_include("e2_over.h")
#include "e2_over.h"   // local test variants only (not in the shipped folder)
#endif
#ifndef E2_EA
#define E2_EA 0
#endif
#ifndef E2_EA_MODE
#define E2_EA_MODE 2
#endif
#ifndef E2_EA_R
#define E2_EA_R 100
#endif
#ifndef E2_EA_W
#define E2_EA_W 10
#endif
#ifndef E2_EI
#define E2_EI 0
#endif
#ifndef E2_EI_R
#define E2_EI_R 100
#endif
#ifndef E2_EI_X
#define E2_EI_X 20
#endif
#ifndef E2_EI_WHO
#define E2_EI_WHO 1   // 0 = every body, 1 = Vesper Vale only
#endif
namespace e2_ {
// Elk Arena: true = skip group gi for this body now.
inline bool ea_skip(int gi, int st, int wait, int d, int r) {
    if (!E2_EA || gi != 0 || r >= E2_EA_R || !gc34_::side_on(E2_EA)) return false;
    if (E2_EA_MODE == 2) return true;
    return st == 2 && wait > d + E2_EA_W;
}
// Eight (east spawn = side B frame: enemy side is x < E2_EI_X; side A mirrored).
inline bool ei_skip(int x, int W, bool vesper, int r) {
    if (!E2_EI || r >= E2_EI_R) return false;
    if (E2_EI_WHO == 1 && !vesper) return false;
    return gc34_::side_on(2) ? x < E2_EI_X : x > W - 1 - E2_EI_X;
}
}  // namespace e2_
