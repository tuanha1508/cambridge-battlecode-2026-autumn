#pragma once
// cc_next25b (29 Oct; docs/claude-next25-2026-10-29.md). Exact-layout gate (same FNV group signature as nx22/nx24),
// one switch per map. Evidence: ranked games since 27 Oct 00:00 UTC (research/claude-next25/c_*.txt).
//   NX_PK  02the-pack (optimized.cpp): opponents earn 1400-2400 gold by r300 (ours ~900), bank nothing before r500,
//          then win the centre fights (we die 11-42 times there in losses). Early gold role: before NX_PK_R every
//          body values gem-only groups (gold 0) x NX_PK_M.
//   NX_GF  00gem-fields (z115_policy.cpp): in wins we clear our own 36-gem/90-gold corner group (10 side A, 11 side B);
//          in losses the opponent farms theirs and we do not. From NX_GF_R, own corner group (36 gems, spawn_dist
//          <= 40) values x NX_GF_M.
//   NX_FM  02famine (optimized.cpp): winners farm their own 50-gem/200-gold group (0 side A, 1 side B) every ~100
//          rounds from r400-800 (our Varg in side-A wins; opponents in all losses). From NX_FM_R the own 50-gem
//          group (spawn_dist <= 66) values x NX_FM_M and every body counts NX_FM_N - 1 helpers there.
//   NX_GW  02gift-wrapping (optimized.cpp): the 128-gem centre group 0 decides the game; in losses the opponent takes
//          it in rotation with us and we die 20-35 times. From NX_GW_R every body counts NX_GW_N - 1 helpers on group 0.
//   NX_UL  02uncharted-lands (optimized.cpp): see the doc; from NX_UL_R every body counts NX_UL_N - 1 helpers on the
//          57-gem groups 0/1 (the biggest regularly cleared groups).
#include <cambc.h>
#include <cstdint>
#include "nx27_gate.h"
#include "sb7_gate.h"
#include "gc34_gate.h"
#include "nx29_gate.h"
#include "mg_switch.h"   // cc_merge_all: all candidate switches (default 0)
#include "wst_gate.h"   // cc_worst
#include "cg_gate.h"   // cc_cheapgold
#include "sw_gate.h"   // cc_sweep
#include "n4_gate.h"   // cc_next4 rows 121/122 (batch2: N4_BB_SIDES / N4_GW_SIDES)
#include "n6_gate.h"   // cc_next6 row 200 (N6_AR, 02arena B; N6_SW / N6_CW off in cc_pin44)
#include "n5_gate.h"   // cc_next5 port (from cc_batchB3; N5_GC2 ON in cc_pin45, N5_RR / N5_GF 0)
#include "e2_gate.h"   // cc_early2 E2_EA (cc_pin47)
// cc_pin34 (29 Oct; docs/claude-pin34-2026-10-29.md): Oasis row from cc_weak2 (docs/claude-weak-2026-10-29.md) only.
// 00oasis, SIDE A only: from round WK_R the 36-gem/200-gold centre groups g6/g7 value x WK_M. Side A = oasis g0 is
// nearer our spawn than g1 (spawn_dist 8/20). The cc_weak2 Line and Gemfight rows (ids 21, 22) are not ported.
// cc_igb (30 Oct; docs/claude-igb-2026-10-30.md). SIDE B only (side mask, gc34_::side_on), value only, exact-layout
// rows 25/26. Evidence: research/claude-igb/ (ranked games since 27 Oct).
//   IGB_IL 00interlock: side-B wins take our own big groups g1 (37 gems/200 gold) + g3 (44/180) 1.4+0.9 times by r600,
//          losses 0.5+0.5 while the side-A opponent farms g0/g2/g3. From IGB_IL_R groups IGB_IL_G value x IGB_IL_M.
//   IGB_GX 00galaxy: the 35-gem centre group g6 decides side B (wins 5.9:3.0 clears by r600, losses 2.6:8.2).
//          From IGB_GX_R groups IGB_GX_G value x IGB_GX_M.
#if __has_include("igb_over.h")
#include "igb_over.h"   // local test variants only (not in the shipped folder)
#endif
#ifndef IGB_IL
#define IGB_IL 2
#endif
#ifndef IGB_IL_R
#define IGB_IL_R 100
#endif
#ifndef IGB_IL_M
#define IGB_IL_M 1.5f
#endif
#ifndef IGB_IL_G
#define IGB_IL_G ((1 << 1) | (1 << 3))
#endif
#ifndef IGB_GX
#define IGB_GX 0
#endif
#ifndef IGB_GX_R
#define IGB_GX_R 100
#endif
#ifndef IGB_GX_M
#define IGB_GX_M 1.5f
#endif
#ifndef IGB_GX_G
#define IGB_GX_G (1 << 6)
#endif
#ifndef WK_OA
#define WK_OA 1
#endif
#ifndef WK_R
#define WK_R 300
#endif
#ifndef WK_M
#define WK_M 1.5f
#endif
#ifndef NX_PK
#define NX_PK 0
#endif
#ifndef NX_PK_R
#define NX_PK_R 400
#endif
#ifndef NX_PK_M
#define NX_PK_M 0.3f
#endif
#ifndef NX_GF
#define NX_GF 0
#endif
#ifndef NX_GF_R
#define NX_GF_R 150
#endif
#ifndef NX_GF_M
#define NX_GF_M 2.0f
#endif
// cc_gfb (30 Oct; docs/claude-gfb-2026-10-30.md). 00gem-fields SIDE B only (side mask, gc34_::side_on). Side-B losses
// since 27 Oct: opponents (side A) out-earn us 1067:502 gold by r300 and farm their 36-gem/80-90-gold corner groups
// (g10, g3); we on side B almost never take the mirror corners (g11, g2). From NX_GFB_R the groups in NX_GFB_G
// (bit = group index, 36 gems) value x NX_GFB_M.
#ifndef NX_GFB
#define NX_GFB 2
#endif
#ifndef NX_GFB_R
#define NX_GFB_R 100
#endif
#ifndef NX_GFB_M
#define NX_GFB_M 1.5f
#endif
#ifndef NX_GFB_G
#define NX_GFB_G ((1 << 2) | (1 << 11))
#endif
#ifndef NX_FM
#define NX_FM 1
#endif
#ifndef NX_FM_R
#define NX_FM_R 300
#endif
#ifndef NX_FM_M
#define NX_FM_M 2.0f
#endif
#ifndef NX_FM_N
#define NX_FM_N 2
#endif
#ifndef NX_GW
#define NX_GW 0
#endif
#ifndef NX_GW_R
#define NX_GW_R 400
#endif
#ifndef NX_GW_N
#define NX_GW_N 3
#endif
#ifndef NX_UL
#define NX_UL 0
#endif
#ifndef NX_UL_R
#define NX_UL_R 400
#endif
#ifndef NX_UL_N
#define NX_UL_N 3
#endif
// cc_loaded (29 Oct; docs/claude-loaded-2026-10-29.md). Loaded deaths: per map and side, our ranked losses since
// 28 Oct drop far more gems on death than our wins (research/claude-loaded/table.txt). Bank-at-one-reward: on the
// gated side only, a farming body enters bank mode once it carries >= one typical camp reward (LD_*_BANK) and an
// altar is within LD_NEAR steps (df_altar). Own exact-layout rows (same FNV signature as above), separate from
// nx25_::rows so no other nx25 lever sees these maps. Each LD_* switch is a side mask: 1 = side A, 2 = side B, 0 off.
//   LD_SV 01starvation (A: L11 lost 120/game vs W0; farm-altar 5; reward 28 = group 5; base threshold 32)
//   LD_GX 00galaxy     (A: lost 224 L vs 106 W; farm-altar 8; reward 21 = groups 0/1)
//   LD_LW 00lost-world (A: lost 106 L vs 11 W; farm-altar 3; reward 21 = groups 13-16)
//   LD_BC 01broken-curve (A: lost 77 L vs 11 W; farm-altar 6; reward 26 = groups 10/11)
#ifndef LD_NEAR
#define LD_NEAR 12
#endif
#ifndef LD_SV
#define LD_SV 0
#endif
#ifndef LD_SV_BANK
#define LD_SV_BANK 28
#endif
#ifndef LD_GX
#define LD_GX 1
#endif
#ifndef LD_GX_BANK
#define LD_GX_BANK 21
#endif
#ifndef LD_LW
#define LD_LW 0
#endif
#ifndef LD_LW_BANK
#define LD_LW_BANK 21
#endif
#ifndef LD_BC
#define LD_BC 0
#endif
#ifndef LD_BC_BANK
#define LD_BC_BANK 26
#endif
// cc_pin39 (30 Oct; docs/claude-pin39-2026-10-30.md): only A3_PD (02the-pond side A) of cc_3a is live.
// cc_3a (29 Oct; docs/claude-3a-2026-10-29.md). Side-A-only levers on three 3-champion maps (most side-A ranked
// losses since 27 Oct: Anvil 2-9, Pond 3-8, Gold Rush 11-8; research/claude-3a/). Side A = absolute team 0; rows 20/21
// and the Gold Rush side-A id 22 return 0 on side B, so side B and every other map run the pin's code path.
//   A3_AN  00anvil (row 20): opponents' Vesper farms 950-1570 gold by r600 and solos the 200-gem monolith late;
//          A3_AN_V 2: from A3_R the monolith (group 10) values x A3_AN_M (value only; the r500 gang-up NX_AN failed).
//   A3_PD  02the-pond (row 21): our Vesper earns 0 gold by r600 in all 5 [11,1,6] games; opponents bank every 12-15
//          gems from their gold groups. A3_PD 2: from A3_R our own gold groups 5/7/8 value x A3_PD_M2.
//   A3_GR  01gold-rush (row 13 on side A -> id 22): r300 gold separates side-A wins (898-2194, mostly 1400+) from
//          losses (419-952); winners clear the 100-gold centre groups 1-4 9-13 times by r300, we clear 3-6 in losses.
//          Before A3_R the gem-less 100-gold groups 1-4 value x A3_GR_M.
#ifndef A3_R
#define A3_R 300
#endif
#ifndef A3_AN
#define A3_AN 0   // cc_pin39: Anvil side A off (platform -1)
#endif
#ifndef A3_AN_M
#define A3_AN_M 1.3f
#endif
#ifndef A3_AN_V
#define A3_AN_V 2   // v2 (v1 group 8 and v3 Vesper early gold each lost the local pin win). 1: own 21-gem group 8 from A3_R; 2: 200-gem monolith 10 from A3_R; 3: Vesper early gold before A3_R
#endif
#ifndef A3_PD
#define A3_PD 2   // v3: mode 1 (early gold, Vesper or all bodies) was inert locally
#endif
#ifndef A3_PD_M
#define A3_PD_M 1.5f
#endif
#ifndef A3_PD_M2
#define A3_PD_M2 1.3f   // A3_PD 2: own gold groups 5/7/8 from A3_R (mode 1, early gold, was inert locally)
#endif
#ifndef A3_PD_ALL
#define A3_PD_ALL 1   // v2: every body (Vesper-only was inert locally: its pond ccp role, optimized.cpp ccp_init)
#endif
#ifndef A3_GR
#define A3_GR 0   // cc_pin39: Gold Rush side A off (platform -2); row 13 -> id 22 redirect inert
#endif
#ifndef A3_GR_M
#define A3_GR_M 1.5f
#endif
namespace a3_ {
inline bool side_a() { return cambc::detail::cached_my_team == 0; }
inline float mult(int id, int gi, int gems, int gold, int sd, int r) {
    if (A3_AN && id == 20) {
        if (A3_AN_V == 1) return (gi == 8 && gems == 21 && sd <= 12 && r >= A3_R) ? A3_AN_M : 1.0f;
        if (A3_AN_V == 2) return (gi == 10 && gems == 200 && r >= A3_R) ? A3_AN_M : 1.0f;
        if (A3_AN_V == 3 && r < A3_R && gold > 0 && gems <= 4 && sd <= 12) {
            static const int ct = static_cast<int>(cambc::self().champion_name());
            return ct == 1 ? A3_AN_M : 1.0f;   // Vesper Vale: own 60/4, 40/0, 20/0 groups
        }
        return 1.0f;
    }
    if (A3_PD == 2 && id == 21) return (r >= A3_R && (gi == 5 || gi == 7 || gi == 8) && gold >= 8 * gems) ? A3_PD_M2 : 1.0f;
    if (A3_PD == 1 && id == 21 && r < A3_R && (gi == 5 || gi == 7 || gi == 8) && gold >= 8 * gems) {
        static const int ct = static_cast<int>(cambc::self().champion_name());
        return (A3_PD_ALL || ct == 1) ? A3_PD_M : 1.0f;   // Vesper Vale only (its pond ccp role overrides targeting) or all
    }
    if (A3_GR && id == 22 && r < A3_R && gi >= 1 && gi <= 4 && gems == 0 && gold == 100) return A3_GR_M;
    return 1.0f;
}
}  // namespace a3_
// cc_pin37 (30 Oct; docs/claude-on36-2026-10-30.md): only the Elk Ring side-A row of cc_emi is live (G208/G209 +1 on
// Elk Ring A; Islands/Monolith lost), rows 30/31 removed from the table and EM_IS/EM_MO 0.
// cc_emi (29 Oct; docs/claude-emi-2026-10-29.md). Side-A-only levers (absolute team 0) on 00elkring (existing row 8),
// 01islands (row 30) and 00monolith (row 31). Evidence: research/claude-emi/ (ranked side-A games since 27 Oct).
//   EM_ER  Elk Ring A (4-7): losses die 32x near the flank groups 6/7/9 before r1000 (wins 6x) and the opponent
//          clears its flank 7/8 more from r300. Mode 1: own flank 6/9 count +1 helper from EM_ER_R; mode 2 (final): x EM_ER_M.
//          Final per local screen (research/claude-emi/screen.txt): ER mode 2, IS mode 1, MO mode 1.
//   EM_IS  Islands A (11-4): we clear only 38/41/44 then 2/3; opponents out-earn us r300-600 (g600 3717-4163 vs
//          2226-3495 in losses), taking the east 58-gem group 3. Mode 1: group 3 x EM_IS_M from EM_IS_R; mode 2: groups
//          2/3 +1 helper from EM_IS_R; mode 3: groups 2/3 x EM_IS_M from EM_IS_R.
//   EM_MO  Monolith A (6-2): the opponent farms its 120-gold corner group 17, we never clear our mirror group 16;
//          both losses trail r300 gold (1818:2004, 1825:2082). Mode 1: group 16 x EM_MO_M before EM_MO_R; mode 2: from.
#ifndef EM_ER
#define EM_ER 2
#endif
#ifndef EM_ER_R
#define EM_ER_R 300
#endif
#ifndef EM_ER_M
#define EM_ER_M 1.3f
#endif
#ifndef EM_IS
#define EM_IS 0   // cc_pin37: Islands row off (G209 lost)
#endif
#ifndef EM_IS_R
#define EM_IS_R 300
#endif
#ifndef EM_IS_M
#define EM_IS_M 1.3f
#endif
#ifndef EM_MO
#define EM_MO 0   // cc_pin37: Monolith row off (G209 lost)
#endif
#ifndef EM_MO_R
#define EM_MO_R 300
#endif
#ifndef EM_MO_M
#define EM_MO_M 1.5f
#endif
namespace em_ {
inline bool side_a() { return cambc::detail::cached_my_team == 0; }
inline int helpers(int id, int gi, int r) {
    if (!side_a()) return 0;
    if (id == 8 && EM_ER == 1 && (gi == 6 || gi == 9) && r >= EM_ER_R) return 1;
    if (id == 30 && EM_IS == 2 && (gi == 2 || gi == 3) && r >= EM_IS_R) return 1;
    return 0;
}
inline float mult(int id, int gi, int r) {
    if (!side_a()) return 1.0f;
    if (id == 8 && EM_ER == 2 && (gi == 6 || gi == 9) && r >= EM_ER_R) return EM_ER_M;
    if (id == 30 && ((EM_IS == 1 && gi == 3) || (EM_IS == 3 && (gi == 2 || gi == 3))) && r >= EM_IS_R) return EM_IS_M;
    if (id == 31 && gi == 16 && ((EM_MO == 1 && r < EM_MO_R) || (EM_MO == 2 && r >= EM_MO_R))) return EM_MO_M;
    return 1.0f;
}
}  // namespace em_
#if VF_SIDES
// cc_vfirst (30 Oct; docs/claude-vfirst-2026-10-30.md), batch2 port: 00dunes SIDE B only (VF_SIDES 2), exact-layout row 111
// in nx25 rows (value) and ld_rows (bank/flee, both 0 as in the candidate). The 00eight row 110 is NOT ported.
//   00dunes B own groups 2/4/6/7/9 value x1.5 from r0.
#ifndef VF_D_G
#define VF_D_G ((1u << 2) | (1u << 4) | (1u << 6) | (1u << 7) | (1u << 9))
#endif
#ifndef VF_D_M
#define VF_D_M 1.5f
#endif
#ifndef VF_D_R
#define VF_D_R 0
#endif
#ifndef VF_D_HG
#define VF_D_HG 0u
#endif
#ifndef VF_D_H
#define VF_D_H 0
#endif
#ifndef VF_D_HR
#define VF_D_HR 600
#endif
#ifndef VF_D_BANK
#define VF_D_BANK 0
#endif
#ifndef VF_D_FLEE
#define VF_D_FLEE 0.0f
#endif
namespace vf_ {
inline bool on() { return gc34_::side_on(VF_SIDES); }
inline float mult(int id, int gi, int r) {
    if (id == 111) return (VF_D_G && r >= VF_D_R && gi < 31 && ((VF_D_G >> gi) & 1)) ? VF_D_M : 1.0f;
    return 1.0f;
}
inline int helpers(int id, int gi, int r) {
    if (id == 111) return (VF_D_H > 1 && r < VF_D_HR && gi < 31 && ((VF_D_HG >> gi) & 1)) ? VF_D_H - 1 : 0;
    return 0;
}
inline float flee(int ldid) {
    if (!on()) return 0.0f;
    if (ldid == 111) return VF_D_FLEE;
    return 0.0f;
}
}  // namespace vf_
#endif
// cc_tbb3 (30 Oct; docs/claude-tbb3-2026-10-30.md). 02treasure-box SIDE B only (ld_rows id 5, side mask TB3_SIDES).
// Ranked/unranked side-B games since 27 Oct (research/claude-tbb/tbb3/): see the doc. Levers (0 = off):
//   TB3_BANK: bank at TB3_BANK carried gems when an altar is within LD_NEAR (cc_loaded hook; base trigger 32).
//   TB3_FLEE: fight flee ratio (g_mm_flee, the cc_lwb hook) on the gated side.
// cc_merge_all: TB3_SIDES is the cc_tbb3 switch (mg_switch.h, default 0; candidate value 2). It also gates ld row 5 and row 16.
#ifndef TB3_BANK
#define TB3_BANK 0
#endif
#ifndef TB3_FLEE
#define TB3_FLEE 0.0f
#endif
//   TB3_GM:   nx25 row 16 (value only). From round TB3_GM_R the own-side gold groups in mask TB3_GM_G value x TB3_GM_M
//             (side-B ids: 26 = 12-gem/200-gold corner (32,32), 28 = 30-gem/300-gold group (32,24)). Side-A winners who
//             beat us take their mirror of 26 by r300 in 73% of games and of 28 in 43% (we: 33% and 10%).
#ifndef TB3_GM
#define TB3_GM 1
#endif
#ifndef TB3_GM_G
#define TB3_GM_G ((1u << 26) | (1u << 28))
#endif
#ifndef TB3_GM_R
#define TB3_GM_R 0
#endif
#ifndef TB3_GM_M
#define TB3_GM_M 2.0f
#endif
//   TB3_AV_G: enemy-side 4-gem/80-gold wolf group 19 (7,27), next to the enemy spawn: we take it by r300 in 70% of side-B
//             losses (39% of wins) and die there; before round TB3_AV_R it values x TB3_AV_M (0 = off with TB3_AV_G 0).
//   cc_tbagold (30 Oct; docs/claude-tbagold-2026-10-30.md): the same row 16 lever on SIDE A (mask TBAG_SIDES), own
//             side-A mirrors 25 = 12-gem/200-gold corner and 27 = 30-gem/300-gold group, x TBAG_M from round TBAG_R.
//             cc_merge_all: TBAG_SIDES (mg_switch.h, default 0; candidate 1) is independent of TB3_SIDES (side B).
#ifndef TBAG_G
#define TBAG_G ((1u << 25) | (1u << 27))
#endif
#ifndef TBAG_R
#define TBAG_R 0
#endif
#ifndef TBAG_M
#define TBAG_M 2.5f
#endif
#ifndef TB3_AV_G
#define TB3_AV_G 0u
#endif
#ifndef TB3_AV_R
#define TB3_AV_R 600
#endif
#ifndef TB3_AV_M
#define TB3_AV_M 0.0f
#endif
// cc_hco (30 Oct; docs/claude-hco-2026-10-30.md). SIDE B only (side masks via gc34_::side_on, value 2), exact-layout rows
// 140 01hooks, 141 00collision, 142 00oasis (oasis shares row 24's signature: detect() redirects side B to 142).
// Evidence: research/claude-hco/ (ranked side-B games since 27 Oct; ana.txt, cad.txt). Group ids are the bot's (B-own:
// hooks 1,3,5,6,9,11; collision odd ids; oasis 1,3,4,7,8). Each switch 0 = off.
//   HCO_HK  01hooks B: winners clear their own 22-gem groups (B frame 3/5) 2.7/3.3 times by r600, our losses 1.9/1.3
//           (wins 3.4/2.9); we die 12 times by r600 (winner 4.1). From HCO_HK_R groups 3/5 value x HCO_HK_M (2.0; 1.5 inert
//           locally). HCO_HK_FLEE > 0: fight flee ratio on the gated side (cc_lwb hook, ld row 140; 1.3/1.5 lost a local win, off).
//   HCO_CO  00collision B: winners clear their spawn-corner 60-gold/4-gem group (B: g1) 2.6 times by r300 and the 11-gem
//           g5 2.4 times (r600: 4.6 / 5.4); our losses 0.2 / 1.2. From HCO_CO_R own gold groups 1/3/5 (HCO_CO_G) value x HCO_CO_M (1.3;
//           g1+g5 x1.5 and g1 x2.0 were inert locally).
//   HCO_OA  00oasis B: winners clear their own 36-gem/200-gold centre group (B: g7) 2.7 and cheap 40-gold corner g8 2.6
//           times by r600 (our losses 1.0 / 1.1), we die 9.4 vs 3.4 and drop 48 gems by r600 (winners 5).
//           From HCO_OA_R groups in HCO_OA_G value x HCO_OA_M. HCO_OA_FLEE / HCO_OA_BANK: flee ratio / bank-at-N (ld row 142;
//           flee 1.3 also passed locally but adds nothing, off; g7-only + bank 16 lost a local win, off).
#if __has_include("hco_over.h")
#include "hco_over.h"   // local test variants only (not in the shipped folder)
#endif
#ifndef HCO_HK
#define HCO_HK 2
#endif
#ifndef HCO_HK_R
#define HCO_HK_R 0
#endif
#ifndef HCO_HK_M
#define HCO_HK_M 2.0f
#endif
#ifndef HCO_HK_G
#define HCO_HK_G ((1 << 3) | (1 << 5))
#endif
#ifndef HCO_HK_FLEE
#define HCO_HK_FLEE 0.0f
#endif
#ifndef HCO_CO
#define HCO_CO 2
#endif
#ifndef HCO_CO_R
#define HCO_CO_R 0
#endif
#ifndef HCO_CO_M
#define HCO_CO_M 1.3f
#endif
#ifndef HCO_CO_G
#define HCO_CO_G ((1 << 1) | (1 << 3) | (1 << 5))
#endif
#ifndef HCO_OA
#define HCO_OA 2
#endif
#ifndef HCO_OA_R
#define HCO_OA_R 0
#endif
#ifndef HCO_OA_M
#define HCO_OA_M 1.5f
#endif
#ifndef HCO_OA_G
#define HCO_OA_G ((1 << 7) | (1 << 8))
#endif
#ifndef HCO_OA_FLEE
#define HCO_OA_FLEE 0.0f
#endif
#ifndef HCO_OA_BANK
#define HCO_OA_BANK 0
#endif
namespace hco_ {
inline int mask(int id) { return id == 140 ? HCO_HK : id == 141 ? HCO_CO : id == 142 ? HCO_OA : 0; }
inline bool on(int id) { return mask(id) && gc34_::side_on(mask(id)); }
inline float mult(int id, int gi, int r) {
    if (!on(id) || gi >= 31) return 1.0f;
    if (id == 140) return (r >= HCO_HK_R && ((HCO_HK_G >> gi) & 1)) ? HCO_HK_M : 1.0f;
    if (id == 141) return (r >= HCO_CO_R && ((HCO_CO_G >> gi) & 1)) ? HCO_CO_M : 1.0f;
    if (id == 142) return (r >= HCO_OA_R && ((HCO_OA_G >> gi) & 1)) ? HCO_OA_M : 1.0f;
    return 1.0f;
}
inline float flee(int ldid) {   // ld_rows ids 140/142
    if (!on(ldid)) return 0.0f;
    return ldid == 140 ? HCO_HK_FLEE : ldid == 142 ? HCO_OA_FLEE : 0.0f;
}
inline int bank(int ldid) { return (ldid == 142 && on(ldid)) ? HCO_OA_BANK : 0; }
}  // namespace hco_
namespace nx25_ {
struct LdRow { int id, w, h, n, groups; std::uint64_t sig; };
inline constexpr LdRow ld_rows[] = {
    { 1, 80, 80, 5, 14, 12143212097066919148ULL },  // 01starvation
    { 2, 93, 93, 5, 9, 17500354938131429551ULL },   // 00galaxy
    { 3, 35, 31, 5, 34, 750036608126919652ULL },    // 00lost-world
    { 4, 40, 40, 3, 22, 1441456107915437094ULL },   // 01broken-curve
#if TB3_SIDES || TBAG_SIDES
    { 5, 35, 35, 5, 34, 14908480345724299507ULL },  // cc_tbb3: 02treasure-box (TB3_BANK / TB3_FLEE, side mask TB3_SIDES)
#endif
#if N2_SF
    { 70, 50, 50, 5, 16, 6682637091941530765ULL },  // cc_next2: 01starfish (bank, side mask N2_SF)
#endif
#if N2_G4
    { 71, 41, 61, 5, 8, 12213907621481157360ULL },  // cc_next2: adgato-gemcombat4 (flee, side mask N2_G4)
#endif
#if SWF_SIDES
    { 55, 45, 30, 4, 22, 4302484650963072677ULL },  // cc_swirla: 00swirl (flee, side mask SWF_SIDES)
#endif
#if VF_SIDES
    { 111, 79, 79, 5, 11, 5235557922892194375ULL }, // cc_vfirst: 00dunes (VF_D_BANK / VF_D_FLEE, side mask VF_SIDES)
#endif
#if HCO_HK || HCO_OA
    { 140, 31, 31, 4, 12, 1475814001451297608ULL },  // cc_hco: 01hooks (HCO_HK_FLEE), side B
    { 142, 29, 29, 4, 10, 3905983543361468089ULL },  // cc_hco: 00oasis (HCO_OA_FLEE / HCO_OA_BANK), side B
#endif
};
template <class G> inline int ld_detect(int W, int H, const G& groups);
// Bank threshold for this map on our side (0 = no lever).
inline int ld_bank(int id) {
    if (id == 1 && gc34_::side_on(LD_SV)) return LD_SV_BANK;
    if (id == 2 && gc34_::side_on(LD_GX)) return LD_GX_BANK;
    if (id == 3 && gc34_::side_on(LD_LW)) return LD_LW_BANK;
    if (id == 4 && gc34_::side_on(LD_BC)) return LD_BC_BANK;
#if TB3_SIDES || TBAG_SIDES
    if (id == 5 && TB3_BANK > 0 && gc34_::side_on(TB3_SIDES)) return TB3_BANK;   // cc_tbb3
#endif
#if VF_SIDES
    if (id == 111 && VF_D_BANK > 0 && vf_::on()) return VF_D_BANK;   // cc_vfirst
#endif
#if HCO_HK || HCO_OA
    if (id >= 140 && id <= 142) return hco_::bank(id);   // cc_hco
#endif
    return 0;
}
}  // namespace nx25_
// cc_lwb (30 Oct; docs/claude-lwb-2026-10-30.md). 00lost-world SIDE B only (ld_rows id 3, side mask LWB_SIDES).
// Ranked side-B games since 27 Oct (research/claude-lwb/): wins and losses are level at r300 (gold, captures), but
// in losses we die 9-12 times in r300-600 around the centre altar (opp 2-5) and captures collapse (2-10 vs 17-33);
// the opponent's r300-600 gold gain is mostly kill gold. Lever: on the gated side the fight flee ratio
// (g_mm_flee, used where P_FLEE applies) becomes LWB_FLEE, so a body leaves fights it is not clearly winning.
#ifndef LWB_FLEE
#define LWB_FLEE 1.3f
#endif
#ifndef LWB_SIDES
#define LWB_SIDES 2
#endif
namespace nx25_ { inline bool lwb_on(int ldid) { return ldid == 3 && LWB_FLEE > 0.0f && gc34_::side_on(LWB_SIDES); } }
namespace nx25_ { inline bool tb3_flee_on(int ldid) { return ldid == 5 && TB3_FLEE > 0.0f && gc34_::side_on(TB3_SIDES); } }   // cc_tbb3
#if N2_SF || N2_G4 || SWF_SIDES
namespace nx25_ {   // cc_batch: cc_next2 (Starfish A bank, GC4 B flee), cc_swirla (Swirl A flee)
inline float bt_flee(int ldid) {   // 0 = keep the pin's flee ratio
#if N2_G4
    if (ldid == 71 && gc34_::side_on(N2_G4)) return N2_G4_FLEE;
#endif
#if SWF_SIDES
    if (ldid == 55 && gc34_::side_on(SWF_SIDES)) return SWF_FLEE;
#endif
    return 0.0f;
}
inline int bt_bank(int ldid, int r) {   // 0 = no change
#if N2_SF
    if (ldid == 70 && r >= N2_SF_BR && gc34_::side_on(N2_SF)) return N2_SF_BANK;
#endif
    return 0;
}
}  // namespace nx25_
#endif
#if N3_SA
namespace n3_ {   // cc_next3b: 01starvation side A late bank (own row 90; rows 92 / side B levers of cc_next3b not ported)
struct Row { int id, w, h, n, groups; std::uint64_t sig; };
inline constexpr Row rows[] = {
    { 90, 80, 80, 5, 14, 12143212097066919148ULL },  // 01starvation
};
template <class G> inline int detect(int W, int H, const G& groups) {
    std::uint64_t h = 1469598103934665603ULL;
    auto mix = [&](std::uint64_t w) { h ^= w; h *= 1099511628211ULL; h ^= h >> 29; };
    for (const auto& g : groups) { mix(static_cast<std::uint64_t>(g.gems)); mix(static_cast<std::uint64_t>(g.gold)); mix(g.camps.size()); }
    const int n = static_cast<int>(cambc::draft::team_size()), ng = static_cast<int>(groups.size());
    for (const auto& r : rows) if (r.w == W && r.h == H && r.n == n && r.groups == ng && r.sig == h) return r.id;
    return 0;
}
inline int bank(int id, int r) { return (id == 90 && r >= N3_SA_BR && gc34_::side_on(N3_SA)) ? N3_SA_BANK : 0; }
}  // namespace n3_
#endif
#if SVB_SIDES
// cc_starvb: 01starvation SIDE B (nx25 row 100). From r0 own groups 1/9 x2.0; from r600 centre group 2 x1.5 with 3 bodies.
namespace svb_ {
inline float mult(int gi, int r) {
    float m = 1.0f;
    if (r >= 0 && gi >= 0 && gi < 31 && ((((1u << 1) | (1u << 9)) >> gi) & 1)) m *= 2.0f;
    if (r >= 600 && gi == 2) m *= 1.5f;
    return m;
}
inline int helpers(int gi, int r) { return (r >= 600 && gi == 2) ? 3 - 1 : 0; }
}  // namespace svb_
#endif
#if VF_E
// cc_vfirst, 00eight row only (row 110, side mask VF_E): own gold groups g1 (37 gems/200 gold) + g5 (59/290) x1.5.
namespace vf_ {
inline float mult(int gi, int r) { return (r >= 0 && gi < 31 && ((((1u << 1) | (1u << 5)) >> gi) & 1)) ? 1.5f : 1.0f; }
}  // namespace vf_
#endif
// cc_gcfam (30 Oct; docs/claude-gcfam-2026-10-30.md). adgato-gemcombat5, SIDE B only (side mask GC5B, gc34_::side_on),
// exact-layout row 27. Ranked GC5 side-B games since 27 Oct (research/claude-gcfam/gc5.txt, g0_gc5.txt): the 140-gem
// centre-south group g0 (20 camps, resp 50) decides the game. B wins: we clear it 2-4 times by r600, first at r69-98,
// the opponent 0-3. B losses: we clear it 0-2 times and then stop; the side-A opponent clears it 3-5 times and banks
// 140-gem bags. From GC5B_R group 0 values x GC5B_M; GC5B_N > 1 also makes every body count GC5B_N - 1 helpers there.
#if __has_include("gcf_over.h")
#include "gcf_over.h"   // local test variants only (not in the shipped folder)
#endif
#ifndef GC5B
#define GC5B 2
#endif
#ifndef GC5B_R
#define GC5B_R 0
#endif
#ifndef GC5B_M
#define GC5B_M 1.3f
#endif
#ifndef GC5B_N
#define GC5B_N 0
#endif
// cc_gcfam: adgato-gemcombat3 SIDE A only (side mask GC3A), existing row 10. Ranked GC3 side-A games since 27 Oct
// (research/claude-gcfam/grp_gc3a.txt): our-side 42-gem centre group g9 gives 92:42 (us:opp) by r300 and 202:78 by
// r600 in wins, but 52:85 and 108:178 in losses. The pinned GC3 lever (x GC3_M on g8/g9) only starts at r300. From
// GC3A_R until GC3_R, group GC3A_G values x GC3A_M on side A.
#ifndef GC3A
#define GC3A 1
#endif
#ifndef GC3A_R
#define GC3A_R 100
#endif
#ifndef GC3A_M
#define GC3A_M 1.3f
#endif
#ifndef GC3A_G
#define GC3A_G 9
#endif
namespace nx25_ { inline bool gc3a_g(int gi, int gems, int r) { return GC3A && gi == GC3A_G && gems == 42 && r >= GC3A_R && r < GC3_R && gc34_::side_on(GC3A); } }
namespace nx25_ { inline bool gc5b_g0(int gi, int gems, int r) { return GC5B && gi == 0 && gems == 140 && r >= GC5B_R && gc34_::side_on(GC5B); } }
// cc_line (30 Oct; docs/claude-line-2026-10-30.md). 00the-line SIDE A only (row 38, side mask LN_SIDES via
// gc34_::side_on). Ranked side-A games since 27 Oct (research/claude-line/): 14-8. In losses we take the 161-gem
// centre line g2 early but the carrier dies near the shared altar (3,16) and drops 169-656 gems before r600 (wins 0-258);
// we bank 0-165 by r300 (wins 300+) and trail r300 gold 360-830 : 580-925 (wins mostly ahead).
//   LN_V 1: before LN_R the 60-gold/4-gem side groups value x LN_M (cheap gold first).
//   LN_V 2: from LN_R the centre group g2 values x LN_M.
//   LN_V 3: before LN_R every body counts LN_N - 1 helpers on g2 (go together).
#if __has_include("ln_over.h")
#include "ln_over.h"   // local test variants only (not in the shipped folder)
#endif
#ifndef LN_SIDES
#define LN_SIDES 1
#endif
#ifndef LN_V
#define LN_V 3
#endif
#ifndef LN_R
#define LN_R 600
#endif
#ifndef LN_M
#define LN_M 2.0f
#endif
#ifndef LN_N
#define LN_N 2
#endif
namespace ln_ {
inline float mult(int gi, int gems, int gold, int r) {
    if (!gc34_::side_on(LN_SIDES)) return 1.0f;
    if (LN_V == 1 && r < LN_R && gi != 2 && gems == 4 && gold == 60) return LN_M;
    if (LN_V == 2 && r >= LN_R && gi == 2 && gems == 161) return LN_M;
    return 1.0f;
}
inline int helpers(int gi, int gems, int r) {
    if (!gc34_::side_on(LN_SIDES)) return 0;
    return (LN_V == 3 && r < LN_R && gi == 2 && gems == 161) ? LN_N - 1 : 0;
}
}  // namespace ln_
// cc_lwa (30 Oct; docs/claude-lwa-2026-10-30.md). 00lost-world SIDE A only (ld_rows id 3, side mask LWA_SIDES).
// Ranked side-A games since 27 Oct (research/claude-lwa/): losses die 6-14 times to champions before r300 (wins 0-8,
// mostly 0-4) in our half / centre lane (x 1-17, y 13-18) while the opponent dies 1-4 times. Same lever as cc_lwb:
// on the gated side the fight flee ratio becomes LWA_FLEE before round LWA_R (all-game 1.15/1.3/1.6 and r300/r600
// variants lost the local side-A win vs cc_pond; r150 kept every win).
#ifndef LWA_FLEE
#define LWA_FLEE 1.3f
#endif
#ifndef LWA_R
#define LWA_R 150
#endif
#ifndef LWA_SIDES
#define LWA_SIDES 1
#endif
namespace nx25_ { inline bool lwa_on(int ldid) { return ldid == 3 && LWA_FLEE > 0.0f && gc34_::side_on(LWA_SIDES); } }
// cc_contest (30 Oct; docs/claude-contest-2026-10-30.md). 00contest only (exact-layout row 27), side mask CTO_SIDES.
// Ranked games since 27 Oct (research/claude-contest/): our [11,6,1,2,10] never clears an outer 40-gold group
// (g6-g13) before r300 (0 in 28/28 games) and fights for all four central 80-gold groups; Mite, same five champions,
// clears ~11 own-half outer groups and ~8 own-half central groups by r300 and leads r300 gold. Lever: before CTO_R,
// gem-less 40-gold groups within CTO_SD of our spawn (our own four outer groups) are exempt from the CT_OPEN skip
// (optimized_params.h CT_OPEN 2000 skips every gem-less group under 80 gold all game) and value x CTO_M.
#if __has_include("cto_over.h")
#include "cto_over.h"   // local test variants only (not in the shipped folder)
#endif
#ifndef CTO_SIDES
#define CTO_SIDES 3
#endif
#ifndef CTO_R
#define CTO_R 2000
#endif
#ifndef CTO_M
#define CTO_M 1.0f
#endif
#ifndef CTO_SD
#define CTO_SD 32
#endif
// cc_merge_all: cc_contest row 27 renumbered to 37 (27 is cc_gcfam's GC5 row).
namespace nx25_ {
inline bool cto_free(int id, int gems, int gold, int sd, int r) {
    return id == 37 && CTO_SIDES && r < CTO_R && gems == 0 && gold == 40 && sd <= CTO_SD && gc34_::side_on(CTO_SIDES);
}
}  // namespace nx25_
// cc_scb (30 Oct; docs/claude-scb-2026-10-30.md). SIDE B only (absolute team 1), exact-layout rows 27/28 (cc_merge_all: renumbered 42/43; 27 is cc_gcfam GC5).
// Evidence: research/claude-scb/ (ranked games since 27 Oct; split_*.txt, crossing_*.txt).
//   SCB_SP 00split (z68_golem_optimized.cpp hook, row 27): the 25-gem bottom pair g14/g15 decides side B. Side-B wins
//          clear g15 7 and g14 4.2 times by r600 (opponent 0.2/1.2); losses 4.0/1.2 while the side-A opponent takes
//          g14 4.8 and g15 2.6 times and we die 16 times. From SCB_SP_R groups SCB_SP_G (g14+g15) value x SCB_SP_M.
//          (Screened: g3+g11+g13 x1.5, g11+g15 x1.5, g3+g11+g13+g15 x1.3; all kept wins, g14+g15 had the best margins.)
//   SCB_CR 02crossing (optimized.cpp, row 28): since v680 every side-B game sends the CC_CR_BIG co-op to the own 43-gem
//          corner g3 at r73 and loses the early centre g0 (side-B wins: first g0 clear r74-78, 5.2 clears by r600).
//          Mode 1 (kept): no co-op on side B; mode 2: co-op only from SCB_CR_R on side B; mode 3: g0 x SCB_CR_M on side B;
//          mode 4: modes 2+3. Side A keeps the co-op unchanged.
#if __has_include("scb_over.h")
#include "scb_over.h"   // local test variants only (not in the shipped folder)
#endif
#ifndef SCB_SP
#define SCB_SP 1
#endif
#ifndef SCB_SP_R
#define SCB_SP_R 100
#endif
#ifndef SCB_SP_M
#define SCB_SP_M 1.5f
#endif
#ifndef SCB_SP_G
#define SCB_SP_G ((1 << 14) | (1 << 15))
#endif
#ifndef SCB_CR
#define SCB_CR 1
#endif
#ifndef SCB_CR_R
#define SCB_CR_R 300
#endif
#ifndef SCB_CR_M
#define SCB_CR_M 1.5f
#endif
namespace scb_ {
inline bool side_b() { return cambc::detail::cached_my_team == 1; }
// Crossing co-op allowed this round (side A: always, unchanged).
inline bool ccr_ok(int r) {
    if (!side_b()) return true;
    if (SCB_CR == 1) return false;
    if (SCB_CR == 2 || SCB_CR == 4) return r >= SCB_CR_R;
    return true;
}
inline float mult(int id, int gi, int r) {
    if (!side_b()) return 1.0f;
    if (id == 42 && SCB_SP && r >= SCB_SP_R && gi < 31 && ((SCB_SP_G >> gi) & 1)) return SCB_SP_M;
    if (id == 43 && (SCB_CR == 3 || SCB_CR == 4) && gi == 0) return SCB_CR_M;
    return 1.0f;
}
}  // namespace scb_
#if AMB_AN || AMB_BC
// cc_amb (30 Oct; docs/claude-amb-2026-10-30.md). SIDE A only (side mask, gc34_::side_on), exact-layout rows 150/151
// placed first in nx25_::rows so they win over row 20 (Anvil) on side A. Side B returns 0 exactly as before (row 20
// was side-A only; Broken Curve had no nx25 row). Value/helpers only through the existing group_mult / boss_helpers.
//   AMB_AN 00anvil (row 150): in the exact [11,6,1] mirror 3c1c2d44 and every side-A [11,6,1]/[11,1,6] game, the
//          side-A winners (NEETCoders, DiscordMess) clear the two 21-gem crystal groups g8/g9 24-30 times in r300-1000,
//          we clear them 2-11 times while the opponent takes 7-14 (research/claude-amb/). From AMB_AN_R groups
//          AMB_AN_G value x AMB_AN_M, and every body counts AMB_AN_N - 1 helpers there.
//   AMB_BC 01broken-curve (row 151): side-A [11,2,0] wins take our 200-gem boss g0 at r839-1018; in losses we take
//          it at r1125 or never and the opponent takes its g1 (r795-866). From AMB_BC_R every body counts AMB_BC_N - 1
//          helpers on groups AMB_BC_G (boss co-op), value x AMB_BC_M.
#if __has_include("amb_over.h")
#include "amb_over.h"   // local test variants only (not in the shipped folder)
#endif
#ifndef AMB_AN
#define AMB_AN 1
#endif
#ifndef AMB_AN_R
#define AMB_AN_R 600   // screen: 300 x1.5 lost the pin win; 300 x1.25, 600 x1.5 and helpers-only kept all
#endif
#ifndef AMB_AN_M
#define AMB_AN_M 1.5f
#endif
#ifndef AMB_AN_G
#define AMB_AN_G ((1 << 8) | (1 << 9))
#endif
#ifndef AMB_AN_N
#define AMB_AN_N 1   // 1 = no helpers (value only)
#endif
#ifndef AMB_BC
#define AMB_BC 1
#endif
#ifndef AMB_BC_R
#define AMB_BC_R 600
#endif
#ifndef AMB_BC_M
#define AMB_BC_M 1.0f
#endif
#ifndef AMB_BC_G
#define AMB_BC_G (1 << 0)
#endif
#ifndef AMB_BC_N
#define AMB_BC_N 3   // screen: x1.5 / x1.5+N3 / x2 from r500 lost a win; N3 value 1.0 kept all + mirror gain
#endif
#ifndef AMB_BC_END
#define AMB_BC_END 100000
#endif
namespace amb_ {
inline bool g_in(int mask, int gi) { return gi >= 0 && gi < 31 && ((mask >> gi) & 1); }
inline float mult(int id, int gi, int r) {
    if (id == 150) return (AMB_AN && r >= AMB_AN_R && g_in(AMB_AN_G, gi)) ? AMB_AN_M : 1.0f;
    if (id == 151) return (AMB_BC && r >= AMB_BC_R && r < AMB_BC_END && g_in(AMB_BC_G, gi)) ? AMB_BC_M : 1.0f;
    return 1.0f;
}
inline int helpers(int id, int gi, int r) {
    if (id == 150) return (AMB_AN && r >= AMB_AN_R && g_in(AMB_AN_G, gi)) ? AMB_AN_N - 1 : 0;
    if (id == 151) return (AMB_BC && r >= AMB_BC_R && r < AMB_BC_END && g_in(AMB_BC_G, gi)) ? AMB_BC_N - 1 : 0;
    return 0;
}
}  // namespace amb_
#endif
// cc_gc1 (30 Oct; docs/claude-gc1-2026-10-30.md). adgato-gemcombat1 SIDE A (side mask GC1A, gc34_::side_on), existing
// exact-layout row 14 (no new detection, no round-1 fuel). From GC1A_R until GC1A_E, groups in mask GC1A_G value
// x GC1A_M; GC1A_N > 1 makes every body count GC1A_N - 1 helpers there. Outside that window the pinned NX_GC1 applies.
#if __has_include("gc1_over.h")
#include "gc1_over.h"   // local test variants only (not in the shipped folder)
#endif
#ifndef GC1A
#define GC1A 1
#endif
#ifndef GC1A_G
#define GC1A_G 12   // groups 2/3 (the contested 21-gem corners)
#endif
#ifndef GC1A_R
#define GC1A_R 50
#endif
#ifndef GC1A_E
#define GC1A_E 300
#endif
#ifndef GC1A_M
#define GC1A_M 1.6f
#endif
#ifndef GC1A_N
#define GC1A_N 0
#endif
namespace nx25_ { inline bool gc1a_g(int gi, int r) { return GC1A && gi >= 0 && gi < 31 && ((GC1A_G >> gi) & 1) && r >= GC1A_R && r < GC1A_E && gc34_::side_on(GC1A); } }
namespace nx25_ {
inline void mix(std::uint64_t& h, std::uint64_t w) { h ^= w; h *= 1099511628211ULL; h ^= h >> 29; }
struct Row { int id, w, h, n, groups; std::uint64_t sig; };
// cc_pilgrim (30 Oct; docs/claude-pilgrim-2026-10-30.md; ported in cc_pilgrim41 with id 28, since 27 is cc_gcfam GC5).
// 00pilgrimage (row 12), side mask PGA (mg_switch.h; 1 = A, 2 = B). Ranked since 27 Oct: our Vesper dies in the south
// band (y >= 48 = the 88-gem herd, group 12) before r700 in 9/12 side-A losses vs 1/5 side-A wins. Vesper skips groups
// whose first camp is at y >= PGA_Y before PGA_R. Side A is nx25 id 28 (same exact-layout row); side B keeps id 12.
#ifndef PGA
#define PGA 0
#endif
#ifndef PGA_R
#define PGA_R 1000
#endif
#ifndef PGA_Y
#define PGA_Y 48
#endif
namespace pga_ {
inline bool skip(int id, bool vesper, int y, int r) {
    return vesper && r < PGA_R && y >= PGA_Y && ((id == 28 && (PGA & 1)) || (id == 12 && (PGA & 2)));
}
}  // namespace pga_
// id: 1 02the-pack, 2 00gem-fields, 3 02famine, 4 02gift-wrapping, 5 02uncharted-lands (research/claude-gold0/sigs.py)
inline constexpr Row rows[] = {
#if N6_AR
    { 200, 30, 30, 3, 14, 14313152201178162884ULL }, // cc_next6: 02arena (N6_AR side mask; other side falls through to row 9)
#endif
#if AMB_AN
    { 150, 37, 31, 3, 13, 8103515028965712125ULL }, // cc_amb: 00anvil side A (AMB_AN); before row 20 (same sig)
#endif
#if AMB_BC
    { 151, 40, 40, 3, 22, 1441456107915437094ULL }, // cc_amb: 01broken-curve side A (AMB_BC; ld_rows id 4 sig)
#endif
    { 1, 45, 45, 3, 35, 4863571223943573496ULL },
    { 2, 50, 40, 5, 18, 16417007778437666435ULL },
    { 3, 91, 75, 5, 11, 14959131892789867922ULL },
    { 4, 45, 45, 5, 11, 11481580556377659658ULL },
    { 5, 50, 46, 5, 20, 1330899391631764732ULL },
    { 7, 51, 51, 5, 22, 9454141637299587625ULL },   // cc_next27: 02fields (nx27_gate.h NX_FD)
    { 8, 30, 30, 5, 10, 2469936211982920337ULL },   // cc_next27: 00elkring (NX_ER)
    { 9, 30, 30, 3, 14, 14313152201178162884ULL },  // cc_next27: 02arena (NX_AR)
    { 10, 51, 51, 4, 10, 16807749217801047994ULL }, // cc_pin32 (from cc_gc34): adgato-gemcombat3 (gc34_gate.h GC3)
    { 11, 45, 30, 4, 22, 4302484650963072677ULL },  // cc_sideb7: 00swirl, side B only (sb7_gate.h)
    { 12, 50, 65, 5, 23, 15853346185077564553ULL }, // cc_sideb7: 00pilgrimage, side B only
    { 13, 31, 31, 3, 15, 5939772593377448505ULL },  // cc_sideb7: 01gold-rush, side B only (cc_3a: side A -> id 22 if A3_GR)
    { 20, 37, 31, 3, 13, 8103515028965712125ULL },  // cc_3a: 00anvil, side A only (A3_AN 0 in cc_pin39)
    { 21, 30, 30, 3, 20, 6652089063345738947ULL },  // cc_3a: 02the-pond, side A only
    { 14, 31, 31, 3, 6, 3785710431241735916ULL },   // cc_pin32 (from cc_next29r row 12): adgato-gemcombat1 (nx29_gate.h NX_GC1)
    { 24, 29, 29, 4, 10, 3905983543361468089ULL },  // cc_pin34 (cc_weak2 row 24): 00oasis (WK_OA), side A only
    { 25, 40, 40, 4, 14, 17158315846157864510ULL }, // cc_igb: 00interlock (IGB_IL), side B only
    { 26, 93, 93, 5, 9, 17500354938131429551ULL },   // cc_igb: 00galaxy (IGB_GX), side B only
#if TB3_SIDES || TBAG_SIDES
    { 16, 35, 35, 5, 34, 14908480345724299507ULL },  // cc_tbb3: 02treasure-box (TB3_GM), side B only
#endif
#if GC5B
    { 27, 99, 99, 5, 24, 11718167551225063855ULL },  // cc_gcfam: adgato-gemcombat5 (GC5B), side B only
#endif
#if CG_QY
    { 33, 35, 29, 4, 14, 7715880573455138917ULL },  // cc_cheapgold: 00quarry (cg_gate.h CG_QY), side mask in cg_
#endif
#if CG_SP
    { 34, 60, 50, 5, 13, 13570939716589467277ULL }, // cc_cheapgold: 00stampede (CG_SP; z69_optimized.cpp hook), side mask in cg_
#endif
#if SW_RW
    { 35, 50, 50, 4, 9, 4568159074858272539ULL },    // cc_sweep: 00rows (sw_gate.h SW_RW), side mask in sw_
#endif
#if SW_EA
    { 36, 50, 50, 5, 29, 8315331439790377043ULL },   // cc_sweep: 01elk-arena (SW_EA; z61_stats_optimized.cpp hook), side mask in sw_
#endif
#if CTO_SIDES
    { 37, 50, 50, 5, 14, 681473445471679836ULL },   // cc_contest: 00contest (CTO_*), candidate row 27
#endif
#if LN_SIDES
    { 38, 33, 33, 4, 9, 14220719709654359365ULL },   // cc_line: 00the-line (LN_V), side A only (LN_SIDES)
#endif
#if GRC_CO
    { 29, 51, 40, 5, 12, 1682932226756082396ULL },  // cc_grc: 01corridors (GRC_CO), side A only
#endif
#if SCB_SP
    { 42, 45, 35, 5, 32, 11056589604306084976ULL }, // cc_scb: 00split (SCB_SP), side B only, z68 executor (candidate row 27)
#endif
#if HCO_HK
    { 140, 31, 31, 4, 12, 1475814001451297608ULL },  // cc_hco: 01hooks (HCO_HK), side B
#endif
#if HCO_CO
    { 141, 41, 41, 3, 20, 4163877698567655129ULL },  // cc_hco: 00collision (HCO_CO), side B
#endif
#if SCB_CR
    { 43, 51, 51, 5, 24, 12189233962575625736ULL }, // cc_scb: 02crossing (SCB_CR), side B only (candidate row 28)
#endif
#if SVB_SIDES
    { 100, 80, 80, 5, 14, 12143212097066919148ULL }, // cc_starvb: 01starvation (svb_), side mask SVB_SIDES
#endif
#if VF_E
    { 110, 41, 30, 5, 8, 9926964676673152361ULL },  // cc_vfirst: 00eight, side mask VF_E
#endif
#if VF_SIDES
    { 111, 79, 79, 5, 11, 5235557922892194375ULL }, // cc_vfirst: 00dunes, side B only
#endif
#if ST_SIDES
    { 290, 30, 30, 4, 15, 4682953521339066479ULL }, // cc_stal: 02stalactites (ST_*), side mask ST_SIDES
#endif
};
template <class G> inline int wk_side_a(int id, const G& groups) {   // cc_pin34 (cc_weak2, oasis g0/g1)
    return groups[0].spawn_dist < groups[1].spawn_dist ? id : 0;
}
template <class G> inline int detect(int W, int H, const G& groups) {
    std::uint64_t h = 1469598103934665603ULL;
    for (const auto& g : groups) { mix(h, static_cast<std::uint64_t>(g.gems)); mix(h, static_cast<std::uint64_t>(g.gold)); mix(h, g.camps.size()); }
    const int n = static_cast<int>(cambc::draft::team_size()), ng = static_cast<int>(groups.size());
    for (const auto& r : rows) if (r.w == W && r.h == H && r.n == n && r.groups == ng && r.sig == h) {
#if N6_AR
        if (r.id == 200) { if (gc34_::side_on(N6_AR)) return r.id; continue; }   // cc_next6 Arena
#endif
#if AMB_AN || AMB_BC
        if (r.id == 150) return (AMB_AN && gc34_::side_on(AMB_AN)) ? r.id : 0;   // cc_amb Anvil (side B: 0, as row 20)
        if (r.id == 151) return (AMB_BC && gc34_::side_on(AMB_BC)) ? r.id : 0;   // cc_amb Broken Curve (no prior row)
#endif
#if SVB_SIDES
        if (r.id == 100) return gc34_::side_on(SVB_SIDES) ? r.id : 0;   // cc_starvb
#endif
#if ST_SIDES
        if (r.id == 290) return gc34_::side_on(ST_SIDES) ? r.id : 0;   // cc_stal
#endif
#if VF_E
        if (r.id == 110) return gc34_::side_on(VF_E) ? r.id : 0;   // cc_vfirst (Eight)
#endif
#if VF_SIDES
        if (r.id == 111) return vf_::on() ? r.id : 0;   // cc_vfirst (Dunes only; 110 is cc_batchA's Eight row)
#endif
#if WS_AV
        if (r.id == 20) return 32;   // cc_worst: 00anvil (cc_pin39 row 20 sig = cc_worst row 32), side mask in wst_
#endif
#if TB3_SIDES || TBAG_SIDES
        if (r.id == 16) return (TB3_GM && gc34_::side_on(TB3_SIDES | TBAG_SIDES)) ? r.id : 0;   // cc_tbb3 + cc_tbagold
#endif
#if LN_SIDES
        if (r.id == 38) return gc34_::side_on(LN_SIDES) ? r.id : 0;   // cc_line
#endif
#if GRC_CO
        if (r.id == 29) return gc34_::side_on(GRC_CO) ? r.id : 0;   // cc_grc (Corridors)
#endif
#if CG_QY || CG_SP
        if (r.id == 33 || r.id == 34) return r.id;   // cc_cheapgold (side mask in cg_)
#endif
#if SW_RW || SW_EA
        if (r.id == 35 || r.id == 36) return r.id;   // cc_sweep (side mask in sw_)
#endif
#if CTO_SIDES
        if (r.id == 37) return r.id;   // cc_contest
#endif
#if SCB_SP || SCB_CR
        if (r.id == 42 || r.id == 43) return r.id;   // cc_scb
#endif
#if HCO_HK || HCO_CO || HCO_OA
        if (r.id == 140 || r.id == 141) return hco_::on(r.id) ? r.id : 0;   // cc_hco
        if (r.id == 24 && !wk_side_a(r.id, groups)) return hco_::on(142) ? 142 : 0;   // cc_hco: oasis side B
#endif
        if (r.id == 24) return wk_side_a(r.id, groups);   // cc_pin34
        if (r.id >= 30) return em_::side_a() ? r.id : 0;   // cc_emi
        if (r.id >= 20 && r.id <= 22) return a3_::side_a() ? r.id : 0;   // cc_3a (rows 20/21; ids 20-22 only)
        if (r.id == 13 && A3_GR && a3_::side_a()) return 22;   // cc_3a: Gold Rush side A (inert while A3_GR 0)
#if PGA & 1
        if (r.id == 12 && !sb7_::side_b_id(r.id, groups)) return 28;   // cc_pilgrim: 00pilgrimage side A
#endif
        return (r.id >= 11 && r.id <= 13) ? sb7_::side_b_id(r.id, groups) : r.id;
    }
    return 0;
}
template <class G> inline int ld_detect(int W, int H, const G& groups) {   // cc_loaded
    std::uint64_t h = 1469598103934665603ULL;
    for (const auto& g : groups) { mix(h, static_cast<std::uint64_t>(g.gems)); mix(h, static_cast<std::uint64_t>(g.gold)); mix(h, g.camps.size()); }
    const int n = static_cast<int>(cambc::draft::team_size()), ng = static_cast<int>(groups.size());
    for (const auto& r : ld_rows) if (r.w == W && r.h == H && r.n == n && r.groups == ng && r.sig == h) return r.id;
    return 0;
}
// Boss co-op: helper count to assume on group gi (0 = no change).
inline int boss_helpers(int id, int gi, int gems, int sd, int r) {
#if N6_AR
    if (id == 200) return nx27_::boss_helpers25(9, gi, r);   // cc_next6: Arena as row 9
#endif
#if AMB_AN || AMB_BC
    if (id >= 150) return amb_::helpers(id, gi, r);   // cc_amb
#endif
#if SVB_SIDES
    if (id == 100) return svb_::helpers(gi, r);   // cc_starvb
#endif
#if ST_SIDES
    if (id == 290) return 0;   // cc_stal: value only
#endif
#if VF_E
    if (id == 110) return 0;   // cc_vfirst: value only
#endif
    if (NX_FM && id == 3 && (gi == 0 || gi == 1) && gems == 50 && sd <= 66 && r >= NX_FM_R) return NX_FM_N - 1;
    if (NX_GW && id == 4 && gi == 0 && r >= NX_GW_R) return NX_GW_N - 1;
    if (NX_UL && id == 5 && (gi == 0 || gi == 1) && r >= NX_UL_R) return NX_UL_N - 1;
    if (id >= 20 && id <= 22) return 0;   // cc_3a: value only
    if (id == 10) return gc34_::boss_helpers(id, gi, gems, sd, r);   // cc_pin32 (cc_gc34 GC3)
    if (id == 14 && GC1A_N > 1 && gc1a_g(gi, r)) return GC1A_N - 1;   // cc_gc1 GC1 side A
    if (id == 14) return 0;   // cc_pin32 (cc_next29r GC1): value-only row
    if (id == 24) return 0;   // cc_pin34 (cc_weak2 oasis): value only
    if (id == 25 || id == 26) return 0;   // cc_igb: value only
#if GRC_CO
    if (id == 29) return 0;   // cc_grc: value only
#endif
#if WS_AV
    if (id == 32) return wst_::av_helpers(gi, r);   // cc_worst Anvil
#endif
#if TB3_SIDES || TBAG_SIDES
    if (id == 16) return 0;   // cc_tbb3: value only
#endif
#if GC5B
    if (id == 27) return (GC5B_N > 1 && gc5b_g0(gi, gems, r)) ? GC5B_N - 1 : 0;   // cc_gcfam GC5 side B
#endif
#if CG_QY || CG_SP
    if (id == 33 || id == 34) return 0;   // cc_cheapgold: value only
#endif
#if SW_RW || SW_EA
    if (id == 35) return sw_::rw_helpers(gi, gems, r);   // cc_sweep Rows
    if (id == 36) return 0;   // cc_sweep Elk Arena: value only
#endif
#if CTO_SIDES
    if (id == 37) return 0;   // cc_contest: value only
#endif
#if LN_SIDES
    if (id == 38) return ln_::helpers(gi, gems, r);   // cc_line
#endif
#if SCB_SP || SCB_CR
    if (id == 42 || id == 43) return 0;   // cc_scb: value only
#endif
#if VF_SIDES
    if (id == 111) return vf_::helpers(id, gi, r);   // cc_vfirst
#endif
#if HCO_HK || HCO_CO || HCO_OA
    if (id >= 140 && id <= 142) return 0;   // cc_hco: value only
#endif
    if (id >= 30) return em_::helpers(id, gi, r);   // cc_emi
    if (id == 8 && em_::helpers(id, gi, r)) return em_::helpers(id, gi, r);   // cc_emi Elk Ring A (else pin path)
    if (id >= 11) return sb7_::helpers(id, gems, r);   // cc_sideb7
    if (id >= 7) return nx27_::boss_helpers25(id, gi, r);   // cc_next27
    return 0;
}
// Targeting multiplier.
inline float group_mult(int id, int gi, int gems, int gold, int sd, int r) {
#if N6_AR
    if (id == 200) return nx27_::group_mult25(9, gi, gems, gold, r) * n6_::ar(gi, r);   // cc_next6 Arena B
#endif
#if AMB_AN || AMB_BC
    if (id >= 150) return amb_::mult(id, gi, r);   // cc_amb
#endif
#if SVB_SIDES
    if (id == 100) return svb_::mult(gi, r);   // cc_starvb
#endif
#if ST_SIDES
    if (id == 290) return (gi == 8 && r < ST_G8_R) ? ST_G8_M : (gi == 7 && r >= ST_G7_R0 && r < ST_G7_R1) ? ST_G7_M : 1.0f;   // cc_stal
#endif
#if VF_E
    if (id == 110) return vf_::mult(gi, r);   // cc_vfirst (Eight)
#endif
    if (NX_PK && id == 1 && r < NX_PK_R && gold == 0) return NX_PK_M;
    if (NX_GFB && id == 2 && gems == 36 && gi < 31 && ((NX_GFB_G >> gi) & 1) && r >= NX_GFB_R && gc34_::side_on(NX_GFB)) return NX_GFB_M;   // cc_gfb
    if (NX_GF && id == 2 && (gi == 10 || gi == 11) && gems == 36 && sd <= 40 && r >= NX_GF_R) return NX_GF_M;
    if (NX_FM && id == 3 && (gi == 0 || gi == 1) && gems == 50 && sd <= 66 && r >= NX_FM_R) return NX_FM_M;
    if (id >= 20 && id <= 22) return a3_::mult(id, gi, gems, gold, sd, r);   // cc_3a
#if GC3A
    if (id == 10 && gc3a_g(gi, gems, r)) return GC3A_M;   // cc_gcfam GC3 side A (before r GC3_R)
#endif
    if (id == 10) return gc34_::group_mult(id, gi, gems, sd, r);   // cc_pin32 (cc_gc34 GC3)
    if (id == 14 && gc1a_g(gi, r)) return GC1A_M;   // cc_gc1 GC1 side A (window GC1A_R..GC1A_E)
    if (id == 14) return nx29_::group_mult25(id, gi, sd, r);   // cc_pin32 (cc_next29r GC1)
    if (id == 25) return (IGB_IL && r >= IGB_IL_R && gi < 31 && ((IGB_IL_G >> gi) & 1) && gc34_::side_on(IGB_IL)) ? IGB_IL_M : 1.0f;   // cc_igb
    if (id == 26) return (IGB_GX && r >= IGB_GX_R && gi < 31 && ((IGB_GX_G >> gi) & 1) && gc34_::side_on(IGB_GX)) ? IGB_GX_M : 1.0f;   // cc_igb
#if GRC_CO
    if (id == 29) return (gi == 0 && r < GRC_CO_R) ? GRC_CO_M : 1.0f;   // cc_grc: Corridors monolith
#endif
    if (id == 24) return (WK_OA && r >= WK_R && (gi == 6 || gi == 7) && gems == 36) ? WK_M : 1.0f;   // cc_pin34 (cc_weak2 oasis)
#if WS_AV
    if (id == 32) return wst_::av_mult(gi, r);   // cc_worst Anvil
#endif
#if TB3_SIDES || TBAG_SIDES
    if (id == 16 && (TBAG_SIDES & 1) && gc34_::side_on(1))   // cc_tbagold: side A
        return (r >= TBAG_R && r < TBAG_RE && gi < 31 && ((TBAG_G >> gi) & 1)) ? TBAG_M : 1.0f;
    if (id == 16) return (r >= TB3_GM_R && gi < 31 && ((TB3_GM_G >> gi) & 1)) ? TB3_GM_M
                         : (r < TB3_AV_R && gi < 31 && ((TB3_AV_G >> gi) & 1)) ? TB3_AV_M : 1.0f;   // cc_tbb3
#endif
#if GC5B
    if (id == 27) return gc5b_g0(gi, gems, r) ? GC5B_M : 1.0f;   // cc_gcfam GC5 side B
#endif
#if CG_QY || CG_SP || CG_GR
    if (id == 33) return cg_::qy(gems, gold, sd, r);   // cc_cheapgold Quarry
    if (id == 34) return cg_::sp(gems, gold, sd, r);   // cc_cheapgold Stampede
    if (id == 13) return sb7_::mult(id, gems, sd, r) * cg_::gr(gems, gold, r);   // cc_cheapgold Gold Rush (x cc_sideb7)
#endif
#if SW_RW || SW_EA
    if (id == 35) return sw_::rw_mult(gi, gems, r);   // cc_sweep Rows
    if (id == 36) return sw_::ea(gi, gems, gold, sd, r);   // cc_sweep Elk Arena
#endif
#if CTO_SIDES
    if (id == 37) return cto_free(id, gems, gold, sd, r) ? CTO_M : 1.0f;   // cc_contest
#endif
#if LN_SIDES
    if (id == 38) return ln_::mult(gi, gems, gold, r);   // cc_line
#endif
#if SCB_SP || SCB_CR
    if (id == 42 || id == 43) return scb_::mult(id, gi, r);   // cc_scb
#endif
#if VF_SIDES
    if (id == 111) return vf_::mult(id, gi, r);   // cc_vfirst
#endif
#if HCO_HK || HCO_CO || HCO_OA
    if (id >= 140 && id <= 142) return hco_::mult(id, gi, r);   // cc_hco
#endif
    if (id >= 30) return em_::mult(id, gi, r);   // cc_emi
    if (id == 8) return nx27_::group_mult25(id, gi, gems, gold, r) * em_::mult(id, gi, r);   // cc_emi x cc_next27 (NX_ER off)
    if (id >= 11) return sb7_::mult(id, gems, sd, r);   // cc_sideb7
    if (id >= 7) return nx27_::group_mult25(id, gi, gems, gold, r);   // cc_next27
    return 1.0f;
}
}  // namespace nx25_
#if BAG_FD
// cc_bag, 02fields row only (row 80, side mask BAG_FD): a carrier with >= 60 gems routes to the own-half altar while
// that route is at most 20 steps longer than the nearest-altar route (optimized.cpp / z68 do_bank; field built on turn 6).
#ifndef BAG_TURN
#define BAG_TURN 6
#endif
namespace bag_ {
struct Row { int id, w, h, n, groups; std::uint64_t sig; int mask, min, slack, ax, ay, bx, by; };
inline constexpr Row rows[] = {
    { 80, 51, 51, 5, 22, 9454141637299587625ULL, BAG_FD, 60, 20, 17, 25, 33, 25 },   // 02fields
};
template <class G> inline const Row* detect(int W, int H, const G& groups) {
    bool dim = false;
    for (const auto& r : rows) if (r.w == W && r.h == H) dim = true;
    if (!dim) return nullptr;
    std::uint64_t h = 1469598103934665603ULL;
    for (const auto& g : groups) { nx25_::mix(h, static_cast<std::uint64_t>(g.gems)); nx25_::mix(h, static_cast<std::uint64_t>(g.gold)); nx25_::mix(h, g.camps.size()); }
    const int n = static_cast<int>(cambc::draft::team_size()), ng = static_cast<int>(groups.size());
    for (const auto& r : rows)
        if (r.w == W && r.h == H && r.n == n && r.groups == ng && r.sig == h) return (r.mask && gc34_::side_on(r.mask)) ? &r : nullptr;
    return nullptr;
}
}  // namespace bag_
#endif
// cc_gf (30 Oct; docs/claude-gf-2026-10-30.md). SIDE A only (side masks via gc34_::side_on), own exact-layout rows
// 60/61 (same FNV group signature as nx25 rows 7/26), separate from nx25_::rows so no other lever sees them.
// Evidence: research/claude-gf/ (ranked games since 27 Oct; side-A winners from the live map lists).
//   GF_FD  02fields A (1W8L with [11,1,6,2,0]): we lose 10 of 17 big rewards (142/256 gems from g13/g9) on the way to
//          the enemy-side altar (33,25) or the centre one; side-A winners bank 13 of 15 big bags at their own altar
//          (17,25) and lose none on side A. Lever: a carrier with >= GF_FD_MIN gems routes to the altar nearest our
//          spawn while that route is at most GF_FD_SLACK steps longer than the nearest altar (optimized.cpp do_bank).
//   GF_GX  00galaxy A (4W10L with C&I): losses die 8-23 times in r300-1000 with 0-1 kills (opponent gold 1100-2000
//          by r600, mostly kill gold); side-A winners die 1-6 times there. Lever: from GF_GX_R (300; R0 lost 2 local wins) the flee ratio
//          (g_mm_flee, the cc_lwb hook) becomes GF_GX_FLEE.
#ifndef GF_FD
#define GF_FD 1
#endif
#ifndef GF_FD_MIN
#define GF_FD_MIN 60
#endif
#ifndef GF_FD_SLACK
#define GF_FD_SLACK 20
#endif
#ifndef GF_FD_TURN
#define GF_FD_TURN 6
#endif
#ifndef GF_GX
#define GF_GX 1
#endif
#ifndef GF_GX_R
#define GF_GX_R 300
#endif
#ifndef GF_GX_FLEE
#define GF_GX_FLEE 1.3f
#endif
#if GF_FD || GF_GX
namespace gf_ {
struct Row { int id, w, h, n, groups; std::uint64_t sig; };
inline constexpr Row rows[] = {
    { 60, 51, 51, 5, 22, 9454141637299587625ULL },   // 02fields (= nx25 row 7)
    { 61, 93, 93, 5, 9, 17500354938131429551ULL },   // 00galaxy (= nx25 row 26 / ld row 2)
};
template <class G> inline int detect(int W, int H, const G& groups) {
    std::uint64_t h = 1469598103934665603ULL;
    for (const auto& g : groups) { nx25_::mix(h, static_cast<std::uint64_t>(g.gems)); nx25_::mix(h, static_cast<std::uint64_t>(g.gold)); nx25_::mix(h, g.camps.size()); }
    const int n = static_cast<int>(cambc::draft::team_size()), ng = static_cast<int>(groups.size());
    for (const auto& r : rows) if (r.w == W && r.h == H && r.n == n && r.groups == ng && r.sig == h) {
        if (r.id == 60) return (GF_FD && gc34_::side_on(GF_FD)) ? 60 : 0;
        if (r.id == 61) return (GF_GX && gc34_::side_on(GF_GX)) ? 61 : 0;
    }
    return 0;
}
}  // namespace gf_
#endif
