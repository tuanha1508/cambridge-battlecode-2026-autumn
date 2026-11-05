#pragma once
// cc_pin47 (30 Oct; docs/claude-pin47-2026-10-30.md): cc_pin46 (v775) + E2_EA 1 (cc_early2 01elk-arena SIDE A only: skip the
//   centre elk group 0 before r100; platform G347 +1/0 on A). E2_EA side B (G349 0/-1) and E2_EI (00eight B) FAILED: B off, E2_EI 0.
// cc_pin45 (30 Oct; docs/claude-pin45-2026-10-30.md): cc_pin44 (v771) + two platform-passed side-B levers ON (G327, cc_batchB3):
//   HCO_OA 2 (cc_hco 00oasis B, nx25 row 142; +1/0) and N5_GC2 2 (cc_next5 adgato-gemcombat2 B, n5_gate.h; +1/0).
//   HCO_HK, HCO_CO and SW_EA (also in cc_batchB3) did not pass and stay 0. N5_RR / N5_GF stay 0 (z113 hook not ported).
// cc_pin44 (30 Oct; docs/claude-pin44-2026-10-30.md): cc_pin43 (v767) + three platform-passed levers ON:
//   ST_SIDES 1 (cc_stal 02stalactites A, nx25 row 290; batch A6 G341 +3/-1), FLW_FR 1 (cc_flw 00fragment A, z357 hook;
//   G341 +1/0), N6_AR 2 (cc_next6 02arena B, nx25 row 200, n6_gate.h; batch B4 G329 +1/0). FLW_LW and PKS_FL not ported.
// cc_pin43 (30 Oct; docs/claude-pin43-2026-10-30.md): cc_pin42b (v764) + CTO_SIDES 1 (cc_contest 00contest A, platform
// PASSED via batch A2 in G317, +1/0) + VF_SIDES 2 (cc_vfirst 00dunes B, PASSED via batch B2 in G319, +1/0). VF_E
// (00eight B, failed) stays 0. Every other value is unchanged from cc_pin42b.
// cc_pin42 (30 Oct; docs/claude-pin42-2026-10-30.md): cc_pin41 + the two batch-A levers that passed on the platform
// (G313): LN_SIDES 1 (00the-line A) and N3_SA 1 (01starvation A). Every other value is unchanged from cc_pin41.
// cc_pin41 (30 Oct; docs/claude-pin41-2026-10-30.md): cc_pin40b (v757) + two platform-passed side-A levers ON:
//   GRC_CO 1   (cc_grc 01corridors A: monolith group 0 valued x0 before r1500; nx25 row 29)
//   TBAG_SIDES 1 + TBAG_RE 600 (cc_tba2 02treasure-box A: own 200/300-gold groups 25/27 x2.5 before r600; nx25 row 16)
// It also hosts every queued batch lever (cc_batchA / cc_batchB / cc_batchA2 / cc_batchB2) behind its own switch, ALL 0
// here (code compiled out). A later pin = flip the passed switches to their "cand value" below, touch the .cpp, re-verify.
// cc_grc's Gold Rush A flee lever (GRC_GR, ld row 28) FAILED on the platform and is NOT ported.
// cc_pin40 (30 Oct; docs/claude-pin40-2026-10-30.md): cc_merge_all with FUEL_AMBUSH 1 and FUEL_PILGRIM 1, all other switches 0.
// cc_merge_all (30 Oct; docs/claude-merge-all-2026-10-30.md). cc_pin39 (v750) + 12 map/side candidates, each behind its
// own switch. EVERY switch defaults to 0 here, so this folder plays exactly like cc_pin39 (verified by finalInput).
// To build a pin: set the switches of the levers that passed to the "cand" value in the comment (one-line edits here),
// then touch the .cpp files and re-verify. Other tuning macros keep each candidate's own defaults in its gate header.
//   switch          cand value  candidate     target map / side                     where
#ifndef WS_EA
#define WS_EA 0          // 1    cc_worst      01elk-arena A                          wst_gate.h, z61 hook
#endif
#ifndef WS_AV
#define WS_AV 0          // 1    cc_worst      00anvil A (row 20 -> id 32)            wst_gate.h, nx25
#endif
#ifndef CG_GR
#define CG_GR 0          // 2    cc_cheapgold  01gold-rush B (row 13)                 cg_gate.h, nx25
#endif
#ifndef CG_QY
#define CG_QY 0          // 2    cc_cheapgold  00quarry B (row 33)                    cg_gate.h, nx25
#endif
#ifndef CG_SP
#define CG_SP 0          // 1    cc_cheapgold  00stampede A (row 34)                  cg_gate.h, nx25, z69 hook
#endif
#ifndef CG_EA
#define CG_EA 0          // 0    cc_cheapgold  01elk-arena B (off in the candidate)   cg_gate.h, z61 hook
#endif
#ifndef GC5B
#define GC5B 0           // 2    cc_gcfam      adgato-gemcombat5 B (row 27)           nx25
#endif
#ifndef GC3A
#define GC3A 0           // 1    cc_gcfam      adgato-gemcombat3 A (row 10)           nx25
#endif
#ifndef BC_LB_SIDES
#define BC_LB_SIDES 0    // 3    cc_bread      00breadcrumbs A+B                      bgi_switch.h, optimized.cpp
#endif
#ifndef LN_SIDES
#define LN_SIDES 1       // 1    cc_line       00the-line A (row 38)                  nx25 (ON in cc_pin42)
#endif
#ifndef SW_RW
#define SW_RW 0          // 2    cc_sweep      00rows B (row 35)                      sw_gate.h, nx25
#endif
#ifndef SW_EA
#define SW_EA 0          // 2    cc_sweep      01elk-arena B (row 36)                 sw_gate.h, nx25, z61 hook
#endif
#ifndef CTO_SIDES
#define CTO_SIDES 1      // 3    cc_contest    00contest A+B (row 37; was 27)         nx25, optimized.cpp (1 = A ON in cc_pin43)
#endif
#ifndef CC_CRA_MODE
#define CC_CRA_MODE 0    // 6    cc_crossa     02crossing A                           cc_switch.h, optimized.cpp
#endif
#ifndef LWA_SIDES
#define LWA_SIDES 0      // 1    cc_lwa        00lost-world A (ld id 3)               nx25, optimized.cpp
#endif
#ifndef TBA_DET
#define TBA_DET 0        // 1    cc_tba        02treasure-box A (wards)               ward.h
#endif
#ifndef SCB_SP
#define SCB_SP 0         // 1    cc_scb        00split B (row 42; was 27)             nx25, z68 hook
#endif
#ifndef SCB_CR
#define SCB_CR 0         // 1    cc_scb        02crossing B (row 43; was 28)          nx25, optimized.cpp
#endif
#ifndef TBAG_SIDES
#define TBAG_SIDES 1     // 1    cc_tbagold    02treasure-box A (row 16 groups 25/27)  nx25 (shares row 16 / ld 5 with TB3)
#endif
#ifndef TB3_SIDES
#define TB3_SIDES 2      // 2    cc_tbb3       02treasure-box B (row 16, ld id 5)     nx25, optimized.cpp
#endif
// cc_fuel5 fuel fixes, per map (fu_gate.h). Platform: Ambush A +3/0 and Pilgrimage A +1/0 passed; Harvest A, Big World A failed.
#ifndef FUEL_AMBUSH
#define FUEL_AMBUSH 1    // 1    cc_fuel5      00ambush A (+ side-B pin replay)       legacy.cpp
#endif
#ifndef FUEL_PILGRIM
#define FUEL_PILGRIM 1   // 1    cc_fuel5      00pilgrimage (z195 50x65 detector)     z195_layout.h
#endif
#ifndef FUEL_HARVEST
#define FUEL_HARVEST 0   // 1    cc_fuel5      02harvest (failed on platform)         legacy.cpp
#endif
#ifndef FUEL_BIGWORLD
#define FUEL_BIGWORLD 0  // 1    cc_fuel5      01big-world (failed on platform)       z78_m1.cpp
#endif
// ---- cc_batchA / cc_batchB (30 Oct; docs/claude-batches-2026-10-30.md): queued map/side levers ported onto cc_pin40b,
// each behind its own switch (0 = compiled out = cc_pin40b). Side masks: 1 = side A, 2 = side B (gc34_::side_on).
//   switch          cand value  candidate     target map / side                     where
#ifndef N2_SF
#define N2_SF 0          // 1    cc_next2      01starfish A: bank 8 near altar from r600 (ld row 70)  nx25, z70 bank
#endif
#ifndef N2_SF_BANK
#define N2_SF_BANK 8
#endif
#ifndef N2_SF_BR
#define N2_SF_BR 600
#endif
#ifndef N2_G4
#define N2_G4 0          // 2    cc_next2      adgato-gemcombat4 B: flee 1.5 (ld row 71)            nx25, optimized.cpp init
#endif
#ifndef N2_G4_FLEE
#define N2_G4_FLEE 1.5f
#endif
#ifndef SWF_SIDES
#define SWF_SIDES 0      // 1    cc_swirla     00swirl A: flee 1.3 (ld row 55)                       nx25, optimized.cpp init
#endif
#ifndef SWF_FLEE
#define SWF_FLEE 1.3f
#endif
#ifndef N3_SA
#define N3_SA 1          // 1    cc_next3b     01starvation A: bank 8 near altar from r600 (n3 row 90) nx25, optimized.cpp bank (ON in cc_pin42)
#endif
#ifndef N3_SA_BANK
#define N3_SA_BANK 8
#endif
#ifndef N3_SA_BR
#define N3_SA_BR 600
#endif
#ifndef SVB_SIDES
#define SVB_SIDES 0      // 2    cc_starvb     01starvation B: groups 1/9 x2, centre group 2 co-op from r600 (row 100)  nx25
#endif
#ifndef VF_E
#define VF_E 0           // 2    cc_vfirst     00eight B only: own gold groups 1/5 x1.5 (row 110)    nx25
#endif
#ifndef BAG_FD
#define BAG_FD 2         // 2    cc_bag        02fields B only: 60+ gem bags to altar (33,25) (bag row 80)  nx25, optimized/z68 do_bank
#endif
#ifndef TBAG_RE
#define TBAG_RE 600      // 600  cc_tba2       02treasure-box A: TBAG_SIDES lever ends at this round   nx25 (ON in cc_pin41)
#endif
// cc_pilgrim (docs/claude-pilgrim-2026-10-30.md), ported in cc_pilgrim41: 00pilgrimage side mask (1 = A, 2 = B). Vesper skips
// groups whose first camp has y >= PGA_Y (the herd) before PGA_R. Side A is nx25 id 28 (row 12; 27 is cc_gcfam GC5).
#ifndef PGA
#define PGA 0            // 1    cc_pilgrim    00pilgrimage A (row 12 -> id 28)       nx25, optimized.cpp
#endif
// Batch2 ports (docs/claude-batches2-2026-10-30.md). Code is compiled out while the switch is 0.
#ifndef GF_FD
#define GF_FD 0          // 1    cc_gf         02fields A (gf_ row 60)                nx25 gf_, optimized.cpp
#endif
#ifndef GF_GX
#define GF_GX 0          // 1    cc_gf         00galaxy A (gf_ row 61, flee 1.3 r300)   nx25 gf_, optimized.cpp
#endif
#ifndef N4_GW_SIDES
#define N4_GW_SIDES 0    // 1    cc_next4      00great-wall A (n4 row 122, flee 1.3)  n4_gate.h, 4 policy files
#endif
#ifndef N4_BB_SIDES
#define N4_BB_SIDES 0    // 2    cc_next4      01boss-battle B (n4 row 121, bank 20 r900) n4_gate.h, 4 policy files
#endif
#ifndef N4_FA_SIDES
#define N4_FA_SIDES 0    // (1)  cc_next4      01the-farm A (not in either batch)     n4_gate.h
#endif
#ifndef VF_SIDES
#define VF_SIDES 2       // 2    cc_vfirst     00dunes B only (nx25 + ld row 111)     nx25 vf_, optimized.cpp (ON in cc_pin43)
#endif
// ---- cc_grc Corridors (docs/claude-grc-2026-10-30.md; platform PASSED): 01corridors side A, nx25 row 29.
// Group 0 (the 428-gem monolith, camps 0/1) is valued x GRC_CO_M before round GRC_CO_R (value only, helpers 0).
#ifndef GRC_CO
#define GRC_CO 1         // 1    cc_grc        01corridors A (nx25 row 29)            nx25 (ON in cc_pin41)
#endif
#ifndef GRC_CO_R
#define GRC_CO_R 1500
#endif
#ifndef GRC_CO_M
#define GRC_CO_M 0.0f
#endif
// ---- cc_hco (docs/claude-hco-2026-10-30.md; on cc_pin40b): side-B value levers, own nx25 rows 140/141/142 (ld rows 140/142).
// cand value 2 (side B). Tuning macros (HCO_*_M / _G / _R / _FLEE / _BANK) keep cc_hco's defaults in nx25_gate.h.
#ifndef HCO_HK
#define HCO_HK 0         // 2    cc_hco        01hooks B: own groups 3/5 x2.0 (row 140)               nx25
#endif
#ifndef HCO_CO
#define HCO_CO 0         // 2    cc_hco        00collision B: own gold groups 1/3/5 x1.3 (row 141)    nx25
#endif
#ifndef HCO_OA
#define HCO_OA 2         // 2    cc_hco        00oasis B: groups 7/8 x1.5 (row 142, via row 24 sig)   nx25, optimized.cpp flee chain (ON in cc_pin45)
#endif
// ---- cc_amb (docs/claude-amb-2026-10-30.md; on cc_pin40b): side-A levers, own nx25 rows 150/151 (placed first in nx25_::rows).
// cand value 1 (side A). Rows are compiled out while 0 (row 150 shares row 20's Anvil signature). Do not combine AMB_AN with WS_AV.
#ifndef AMB_AN
#define AMB_AN 0         // 1    cc_amb        00anvil A: crystal groups 8/9 x1.5 from r600 (row 150)  nx25
#endif
#ifndef AMB_BC
#define AMB_BC 0         // 1    cc_amb        01broken-curve A: boss g0 counts 2 helpers from r600 (row 151)  nx25
#endif
// ---- cc_stal (30 Oct; docs/claude-stal-2026-10-30.md): 02stalactites side mask (1 = A, 2 = B), nx25 row 290
// (30x30, 4 picks, 15 groups). Off (0) = cc_pin42b. Value/hold levers only; tuning in nx25_gate.h (st_ namespace).
#ifndef ST_SIDES
#define ST_SIDES 1       // 1    cc_stal       02stalactites A (nx25 row 290)         nx25, optimized.cpp rs_hold hook
#endif
#ifndef ST_NOHOLD
#define ST_NOHOLD 1      // 1 = the cc_rs altar hold (RS_ST_HOLD) is skipped on the ST_SIDES side (reuses g_bgi == 3)
#endif
#ifndef ST_G8_M
#define ST_G8_M 0.0f     // group 8 (centre 168-gem monolith) value multiplier before ST_G8_R (screen: x0 to r300)
#endif
#ifndef ST_G8_R
#define ST_G8_R 300
#endif
#ifndef ST_G7_M
#define ST_G7_M 1.0f     // group 7 (far-side 77-gem camp) value multiplier in [ST_G7_R0, ST_G7_R1)
#endif
#ifndef ST_G7_R0
#define ST_G7_R0 300
#endif
#ifndef ST_G7_R1
#define ST_G7_R1 600
#endif

// ---- cc_flw (30 Oct; docs/claude-flw-2026-10-30.md; on cc_pin42b): side-A levers for atnaM's 08:50 losses. Side mask 1 = A.
//   (FLW_LW, 00lost-world A, was in cc_batchA6 but is NOT ported: no platform change.)
//   FLW_FR  00fragment A: our own 26-gem centre group (spawn_dist <= FLW_FR_SD, g0) joins the z357 objective order with
//           FLW_FR_N healthy bodies near (pin: 3) before FLW_FR_R (120; 300 lost the local pin win, 60 nearly inert). Losses let the opponent take g0/g1 first.
#ifndef FLW_FR
#define FLW_FR 1
#endif
#ifndef FLW_FR_R
#define FLW_FR_R 120
#endif
#ifndef FLW_FR_N
#define FLW_FR_N 2
#endif
#ifndef FLW_FR_SD
#define FLW_FR_SD 10
#endif
// ---- cc_next5 port (from cc_batchB3; docs/claude-batches3-2026-10-30.md). Only the GC2 B hook (optimized.cpp turn-3 g_cargo)
// is ported into cc_pin45; the Road A z113 hooks (N5_RR) and Gem Fields A (N5_GF) are not, so both stay 0.
#ifndef N5_RR
#define N5_RR 0          // 1    cc_next5      00road-of-riches A: flee 1.3 from r600 (lever 162)    n5_gate.h, z113 hook NOT ported
#endif
#ifndef N5_GC2
#define N5_GC2 2         // 2    cc_next5      adgato-gemcombat2 B: flee 1.15 (lever 161)           n5_gate.h, optimized.cpp (ON in cc_pin45)
#endif
#ifndef N5_GF
#define N5_GF 0          // (1)  cc_next5      00gem-fields A: not ported (no hook)                   n5_gate.h
#endif
// ---- cc_early2 (docs/claude-early2-2026-10-30.md): first-death levers, e2_gate.h. Side masks (1 = A, 2 = B).
#if __has_include("e2_over.h")
#include "e2_over.h"   // local test variants only (not in the shipped folder)
#endif
#ifndef E2_EA
#define E2_EA 1          // 1    cc_early2     01elk-arena A only: skip centre group 0 before r100 (mode 2)  z61 (nx27 id 1) (ON in cc_pin47; B failed G349)
#endif
#ifndef E2_EI
#define E2_EI 0          // (2)  cc_early2     00eight B: FAILED on platform; code not ported (stays 0)
#endif
// ---- cc_firstdeath (docs/claude-firstdeath-2026-10-30.md): skip group X before r100 where our first death in ranked
// losses repeats at the same camp group; fd_gate.h. Side masks (1 = A, 2 = B); 0 = cc_pin47 behaviour.
#if __has_include("fd_over.h")
#include "fd_over.h"   // local test variants only (not in the shipped folder)
#endif
#ifndef FD_QD
#define FD_QD 1          // 1    cc_firstdeath 00quadrants A: skip group 5 before r100       z68_golem_optimized.cpp (45x45)
#endif
#ifndef FD_VA
#define FD_VA 0          // 2    cc_firstdeath 00vault B: skip vault group 0 before r100       optimized.cpp (lv id 4). Platform FAILED, OFF in cc_pin48
#endif
#ifndef FD_CO
#define FD_CO 2          // 2    cc_firstdeath 01corridors B: Heart of Ruin skips group 1 before r100  optimized.cpp (lv id 3)
#endif
#ifndef FD_BB
#define FD_BB 0          // 2    cc_firstdeath 01boss-battle B: skip group 9 before r100       interim_optimized.cpp (nx27 id 3). Platform FAILED, OFF in cc_pin48
#endif
#ifndef FD_AN
#define FD_AN 0          // 1    cc_firstdeath 00anvil A: skip crystal group 9 before r100     optimized.cpp (nx25 id 20). Local: lost 3 pin-class wins, OFF
#endif
#ifndef FD_CO_WHO
#define FD_CO_WHO 1      // 1    cc_firstdeath 01corridors: Heart of Ruin only (all bodies lost 2 local wins)
#endif

// ---- cc_late (30 Oct; docs/claude-late-2026-10-30.md): late-collapse levers, rows 360-363 (lt_gate.h, n4_gate.h) ----
// switch           value  target                      lever
#ifndef LT_BB
#define LT_BB 0          // 1    01boss-battle A: bank LT_BB_BANK within LT_BB_NEAR of the altar from LT_BB_R (row 360, n4 row 121). Platform FAILED, OFF in cc_pin49
#endif
#ifndef LT_BB_BANK
#define LT_BB_BANK 20
#endif
#ifndef LT_BB_R
#define LT_BB_R 600
#endif
#ifndef LT_BB_NEAR
#define LT_BB_NEAR 40
#endif
#ifndef LT_HV
#define LT_HV 0          // 1    02harvest A: enemy-side 29-gem group 15 x LT_HV_M from LT_HV_R (row 361, legacy.cpp). Platform no change, OFF in cc_pin49
#endif
#ifndef LT_HV_M
#define LT_HV_M 0.0f
#endif
#ifndef LT_HV_R
#define LT_HV_R 900
#endif
#ifndef LT_MO
#define LT_MO 0          // 2    00monolith B: gem-less centre groups 12-15 x LT_MO_M from LT_MO_R (row 362, optimized.cpp). Platform no change, OFF in cc_pin49
#endif
#ifndef LT_MO_M
#define LT_MO_M 0.0f
#endif
#ifndef LT_MO_R
#define LT_MO_R 600
#endif
#ifndef LT_MO_BANK
#define LT_MO_BANK 15     // > 0: on LT_MO's side also bank at LT_MO_BANK gems within LD_NEAR of an altar from LT_MO_R
#endif
#ifndef LT_BW
#define LT_BW 1          // 1    01big-world A: 228-gem monolith group 17 x LT_BW_M from LT_BW_R (row 363, z78_m1.cpp). Platform PASSED G361, ON in cc_pin49
#endif
#ifndef LT_BW_M
#define LT_BW_M 0.5f
#endif
#ifndef LT_BW_R
#define LT_BW_R 600
#endif
