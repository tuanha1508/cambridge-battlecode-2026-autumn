#include "ward.h"
#include "n4_gate.h"   // cc_next4
#include "cb3g_gate.h"
#include "ccx_layout.h"
#include "gwc_switch.h"
#include "z362_switch.h"
#line 1 "z114_policy.cpp"
#include "z327_switch.h"
#if Z327_ON
#include "z327_layout.h"
static bool z327_expanse=false;
#endif
#line 1 "z319_extension_z114_policy.cpp"
#include "z319_switch.h"
#line 1 "z114_policy.cpp"
#include "z316_switch.h"
#include "z180_switch.h"
#include "z181_switch.h"
#include "z170_switch.h"
#include "z163_switch.h"
#include "z144_switch.h"
#include "z114_switch.h"
#if Z114_MAPS
// Economy-first bot: cached camp layout, claim-based target choice, ability
// use, batched banking and shopping. One wasm instance runs per champion.
#include <cambc.h>
#include "cxv_switches.h"
#include "cxy_switches.h"
#include "cxw_switches.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <optional>
#include <variant>
#include <vector>

#include "z59_switches.h"
#if Z59_MAP1 || Z59_MAP2 || Z59_MAP3
#include "z59_draft.h"
#endif

using namespace cambc;
namespace z114_policy {

#if __has_include("optimized_params.h")
#include "optimized_params.h"

#ifndef CXK2_GATE
#define CXK2_GATE 1
#endif
static bool cxk2_keeper=true;
#ifndef CXK_PROD
#define CXK_PROD 1
#endif
#ifndef CXK_MAPS
#define CXK_MAPS 0
#endif
#ifndef CXK_SW
#define CXK_SW 7
#endif
#ifndef CXK_GEMS
#define CXK_GEMS 44
#endif
#ifndef CXK_NEAR
#define CXK_NEAR 6
#endif
// ---- sf_hunter knobs (test opponent only) ----
// ---- fd (fight discipline, 17 Oct): all default off = us_shop3 ----
#ifndef FD_TETH
#define FD_TETH 0       // (a) Issac walks back to Cassi when farther than this (farm mode, not capturing)
#endif
#ifndef FD_TETHC
#define FD_TETHC 0      // (a) 1: Cassi also tethers to Issac
#endif
#ifndef FD_BTOG
#define FD_BTOG 0       // (a) partner banks too when the other banks and it carries >= this many gems
#endif
#ifndef FD_BTOGR
#define FD_BTOGR 12     // ... and is within this distance
#endif
#ifndef FD_PAIRALL
#define FD_PAIRALL 0    // (a) P_PAIR target weighting on every map size
#endif
#ifndef FD_CG
#define FD_CG 0         // (b) carrier with >= FD_CG gems, enemy within FD_CV and no ally within FD_CR: flee
#endif
#ifndef FD_CV
#define FD_CV 6
#endif
#ifndef FD_CR
#define FD_CR 4
#endif
#ifndef FD_CBANK
#define FD_CBANK 0      // (b) 1: such a carrier also switches to bank mode
#endif
#ifndef FD_NUM
#define FD_NUM 0        // (c) flee when visible enemies within FD_NUMR exceed allies (incl me) within FD_NUMA + FD_NUMS
#endif
#ifndef FD_NUMR
#define FD_NUMR 6
#endif
#ifndef FD_NUMA
#define FD_NUMA 5
#endif
#ifndef FD_NUMS
#define FD_NUMS 0
#endif
#ifndef FD_NUMP
#define FD_NUMP 1e9f    // (c) ... unless our power is at least FD_NUMP x theirs
#endif
#ifndef FD_RG
#define FD_RG 0         // (d) carriers with >= FD_RG gems recall below FD_RHP
#endif
#ifndef FD_RHP
#define FD_RHP 0.45f
#endif
#ifndef FD_WHOMAPS
#define FD_WHOMAPS 511  // (e) SF_WHO applies only on these sf_map() bits
#endif
#ifndef FD_WHO2
#define FD_WHO2 0       // (e) extra SF_WHO bits on FD_WHOMAPS2 maps
#endif
#ifndef FD_WHOMAPS2
#define FD_WHOMAPS2 0
#endif
#ifndef FD_MCXMAPS
#define FD_MCXMAPS 511  // (e) SF_MCX applies only on these sf_map() bits
#endif
#ifndef FD_RGMAPS
#define FD_RGMAPS 511   // (d) maps for FD_RG
#endif
#ifndef FD_CWHO
#define FD_CWHO 4095    // (b) champion_type bits the carrier rule applies to (cassi 8, issac 16)
#endif
#ifndef FD_NWHO
#define FD_NWHO 4095    // (c) champion_type bits the numbers rule applies to
#endif
#ifndef FD_NUMC
#define FD_NUMC 0       // (c) 1: when outnumbered only refuse to chase (no forced flee)
#endif
#ifndef FD_MAPS
#define FD_MAPS 511     // sf_map() bits on which fd rules apply (0 elk,1 coll,2 il,3 eight,4 pilg,5 stamp,6 exp,7 rows,8 other)
#endif
#ifndef SF_ON
#define SF_ON 0
#endif
#ifndef SF_HUNTR
#define SF_HUNTR 9      // enemies considered (and hunted) within this distance
#endif
#ifndef SF_HUNT
#define SF_HUNT 1.3f    // hunt a target only when our power near it is this many times theirs
#endif
#ifndef SF_FLEE
#define SF_FLEE 0.8f    // leave when our local power is below this fraction of theirs
#endif
#ifndef SF_BACK
#define SF_BACK 0.45f   // leave any fight below this HP fraction
#endif
#ifndef SF_KEEP
#define SF_KEEP 4       // ranged: keep enemy melee at this distance
#endif
#ifndef SF_ISO
#define SF_ISO 500.0f   // target bonus for an enemy with no ally within 4
#endif
#ifndef SF_CARRY
#define SF_CARRY 30     // our carrier (gems) leaves fights unless clearly ahead
#endif
#ifndef SF_CARRYR
#define SF_CARRYR 1.6f
#endif
#ifndef SF_CALL
#define SF_CALL 1       // answer team hunt calls
#endif
#ifndef SF_CALLR
#define SF_CALLR 10
#endif
#ifndef SF_CALLHP
#define SF_CALLHP 0.6f
#endif
#ifndef SF_CALLG
#define SF_CALLG 30
#endif
#ifndef SF_RAID
#define SF_RAID 0       // 1: Ren+Stellar raid the enemy half from SF_RAID0; 2: + Varg; 3: + Issac
#endif
#ifndef SF_RAID0
#define SF_RAID0 150
#endif
#ifndef SF_RAIDHP
#define SF_RAIDHP 0.6f
#endif
#ifndef SF_RAIDG
#define SF_RAIDG 30
#endif
#ifndef SF_RAIDF
#define SF_RAIDF 12     // enemy-half group: dist_enemy * SF_RAIDF < dist_own * 10
#endif
#ifndef SF_OWNR
#define SF_OWNR 5
#endif
#ifndef SF_RAIDC
#define SF_RAIDC -0.33f
#endif
#ifndef SF_FMAP
#define SF_FMAP 3
#endif
#ifndef SF_LAN
#define SF_LAN 0
#endif
#ifndef SF_TRADE
#define SF_TRADE 0.0f
#endif
#ifndef S2_ON
#define S2_ON 0
#endif
#ifndef S2_SHOP5
#define S2_SHOP5 0
#endif
#ifndef SF_SEED
#define SF_SEED 0
#endif
#ifndef SF_NZ
#define SF_NZ 0.15f
#endif
#ifndef SF_SHOP
#define SF_SHOP 0
#endif
#ifndef SF_MHIT
#define SF_MHIT 0
#endif
#ifndef SF_BANKV
#define SF_BANKV 0
#endif
#ifndef SF_LEAVED
#define SF_LEAVED 5
#endif
#ifndef SF_WHO
#define SF_WHO 15
#endif
#ifndef SF_MCX
#define SF_MCX 0
#endif
#ifndef SF_CHR
#define SF_CHR 4
#endif
#ifndef SR_NOSHOP
#define SR_NOSHOP 0     // sr (17 Oct): 1 = v311 builds on swirl (45x30) and rushbait_v2 (30x80)
#endif
#ifndef SF_SHOPM
#define SF_SHOPM 511
#endif
#ifndef SF_WHOM
#define SF_WHOM 0
#endif
#ifndef SF_LOG
#define SF_LOG 0
#endif
#ifndef SF_SHM
#define SF_SHM 6
#endif
#endif
#ifndef DBG
#define DBG 0
#endif
// ---- mc (16 Oct platform map change) knobs, all default off ----
#ifndef MC_CHUB
#define MC_CHUB 0
#endif
#ifndef MC_CHUBR
#define MC_CHUBR 3
#endif
#ifndef MC_CHUBD
#define MC_CHUBD 60
#endif
#ifndef MC_CPHUB
#define MC_CPHUB 0
#endif
#ifndef MC_LHUB
#define MC_LHUB 0
#endif
#ifndef CT_BANK
#define CT_BANK 0   // >0: vs Stellar+Tavi drafts, bank at this many carried gems on every map
#endif
static bool g_ct = false;
static int g_ek3 = 0;  // ek3: 1 = elkring (see EK3_ON), 2 = sides computed
// mm2: Memory?-style draft (Varg + Cassi&Issac + Stellar, no Tavi/Prism/Willow) -> per-map gated knobs.
#ifndef MM_BANK_LN
#define MM_BANK_LN 0
#endif
#ifndef MM_BANK_EK
#define MM_BANK_EK 0
#endif
#ifndef MM_BANK_PK
#define MM_BANK_PK 0
#endif
#ifndef MM_BANK_IL
#define MM_BANK_IL 0
#endif
#ifndef MM_FLEE_LN
#define MM_FLEE_LN 0.0f
#endif
#ifndef MM_FLEE_EK
#define MM_FLEE_EK 0.0f
#endif
#ifndef MM_FLEE_PK
#define MM_FLEE_PK 0.0f
#endif
#ifndef MM_FLEE_IL
#define MM_FLEE_IL 0.0f
#endif
#ifndef MM_LOG
#define MM_LOG 0
#endif
static bool g_mm = false;
#ifndef MM_LN_OWNHALF
#define MM_LN_OWNHALF 0
#endif
#ifndef MM_EK_MAGES
#define MM_EK_MAGES 0
#endif
// os: opponent-signature table generalising the Memory? gate. A signature is the enemy's first three picks
// in pick order, champions that must be absent, and the maps it is valid on; it selects a knob set.
// Map index: 0 lanes 90x30, 1 elkring 30x30, 2 packs 31x31, 3 interlock 40x40, 4 collision 41x41,
// 5 corners 71x71, 6 monolith 50x50, 7 rows 50x50 (two altars), 8 expanse 80x80, 9 any other map.
// Knob sets: 0 Memory? (MM_* macros, unchanged), 1 Mackey, 2 test, 3 Gem_alert, 4 Boost.
// Per-set macros (10 values, map order above): OS_<SET>_BANK ints, OS_<SET>_FLEE floats; OS_<SET>_OWNHALF, OS_<SET>_MAGES.
#ifndef OS_MACK_BANK
#define OS_MACK_BANK 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
#endif
#ifndef OS_MACK_FLEE
#define OS_MACK_FLEE 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
#endif
#ifndef OS_MACK_OWNHALF
#define OS_MACK_OWNHALF 0
#endif
#ifndef OS_MACK_MAGES
#define OS_MACK_MAGES 0
#endif
#ifndef OS_TEST_BANK
#define OS_TEST_BANK 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
#endif
#ifndef OS_TEST_FLEE
#define OS_TEST_FLEE 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
#endif
#ifndef OS_TEST_OWNHALF
#define OS_TEST_OWNHALF 0
#endif
#ifndef OS_TEST_MAGES
#define OS_TEST_MAGES 0
#endif
#ifndef OS_GEM_BANK
#define OS_GEM_BANK 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
#endif
#ifndef OS_GEM_FLEE
#define OS_GEM_FLEE 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
#endif
#ifndef OS_GEM_OWNHALF
#define OS_GEM_OWNHALF 0
#endif
#ifndef OS_GEM_MAGES
#define OS_GEM_MAGES 0
#endif
#ifndef OS_BOOST_BANK
#define OS_BOOST_BANK 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
#endif
#ifndef OS_BOOST_FLEE
#define OS_BOOST_FLEE 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
#endif
#ifndef OS_BOOST_OWNHALF
#define OS_BOOST_OWNHALF 0
#endif
#ifndef OS_BOOST_MAGES
#define OS_BOOST_MAGES 0
#endif
constexpr int OS_NMAP = 10, OS_NSET = 5;
// Knob values live in volatile globals so the off-gate instruction stream is the same for every knob setting.
static volatile int os_k_bank[OS_NSET][OS_NMAP] = {
    { MM_BANK_LN, MM_BANK_EK, MM_BANK_PK, MM_BANK_IL, 0, 0, 0, 0, 0, 0 },
    { OS_MACK_BANK }, { OS_TEST_BANK }, { OS_GEM_BANK }, { OS_BOOST_BANK } };
static volatile float os_k_flee[OS_NSET][OS_NMAP] = {
    { MM_FLEE_LN, MM_FLEE_EK, MM_FLEE_PK, MM_FLEE_IL, 0, 0, 0, 0, 0, 0 },
    { OS_MACK_FLEE }, { OS_TEST_FLEE }, { OS_GEM_FLEE }, { OS_BOOST_FLEE } };
static volatile int os_k_ownhalf[OS_NSET] = { MM_LN_OWNHALF, OS_MACK_OWNHALF, OS_TEST_OWNHALF, OS_GEM_OWNHALF, OS_BOOST_OWNHALF };
static volatile int os_k_mages[OS_NSET] = { MM_EK_MAGES, OS_MACK_MAGES, OS_TEST_MAGES, OS_GEM_MAGES, OS_BOOST_MAGES };
struct OsSig { std::int8_t p0, p1, p2; std::uint16_t excl, maps; std::int8_t set; };
#define OS_B(c) (1u << static_cast<int>(champion_draft::c))
#define OS_C(c) static_cast<std::int8_t>(champion_draft::c)
// First match wins. Observed in decoded platform replays (15 Oct); several are shared by other teams (Mackey: Conley, hello, Gem_alert, many C&I,Re,St drafts).
static constexpr OsSig os_sigs[] = {
    // Memory?: Varg, C&I, Stellar[, Ren, Ryth] on every map, no Tavi/Prism/Willow (the v210 gate, unchanged).
    { OS_C(varg), OS_C(cassi_and_issac), OS_C(stellar), OS_B(tavi) | OS_B(prism) | OS_B(willow), 0x3FF, 0 },
    // Mackey: elkring C&I,Ry,St,Ta,Va (= Conley); interlock C&I,Re,St,Ta (= Gem_alert, hello, many);
    // collision/expanse C&I,Re,St (very common); lanes/corners Ry,St,Pr,Re,Ta (= hello).
    { OS_C(cassi_and_issac), OS_C(ryth), OS_C(stellar), 0, 1u << 1, 1 },
    { OS_C(cassi_and_issac), OS_C(ren), OS_C(stellar), 0, (1u << 3) | (1u << 4) | (1u << 8), 1 },
    { OS_C(ryth), OS_C(stellar), OS_C(prism), 0, (1u << 0) | (1u << 5), 1 },
    // test: collision Ta,St,Vd; interlock Re,St,Ta,Pr; lanes Ry,Re,St,Pr,Ta; monolith Va,Ry,Re,St,Ta.
    { OS_C(tavi), OS_C(stellar), OS_C(vendetta), 0, 1u << 4, 2 },
    { OS_C(ren), OS_C(stellar), OS_C(tavi), 0, 1u << 3, 2 },
    { OS_C(ryth), OS_C(ren), OS_C(stellar), 0, 1u << 0, 2 },
    { OS_C(varg), OS_C(ryth), OS_C(ren), 0, 1u << 6, 2 },
    // Gem_alert: elkring C&I,St,Ta,Re,Va; lanes C&I,St,Ry,Va,Vd; expanse C&I,Re,Vd; rows Va,C&I,Ry,Vd (interlock = Mackey's entry).
    { OS_C(cassi_and_issac), OS_C(stellar), OS_C(tavi), 0, 1u << 1, 3 },
    { OS_C(cassi_and_issac), OS_C(stellar), OS_C(ryth), 0, 1u << 0, 3 },
    { OS_C(cassi_and_issac), OS_C(ren), OS_C(vendetta), 0, 1u << 8, 3 },
    { OS_C(varg), OS_C(cassi_and_issac), OS_C(ryth), 0, 1u << 7, 3 },
    // Boost: collision St,Re,Ta; elkring/interlock St,Pr,Ta; monolith C&I,Vd,St,Ve,Ta; packs C&I,St,Ta.
    { OS_C(stellar), OS_C(ren), OS_C(tavi), 0, 1u << 4, 4 },
    { OS_C(stellar), OS_C(prism), OS_C(tavi), 0, (1u << 1) | (1u << 3), 4 },
    { OS_C(cassi_and_issac), OS_C(vendetta), OS_C(stellar), 0, 1u << 6, 4 },
    { OS_C(cassi_and_issac), OS_C(stellar), OS_C(tavi), 0, 1u << 2, 4 },
};
#undef OS_B
#undef OS_C
static int g_os = -1;   // matched knob set, -1 none
static int g_mm_bank = 0;
static bool g_conley_packs = false;
static float g_mm_flee = 0.0f;
// dp (port of dg DG_DIS/DG_BANK): gated on enemy draft with Stellar, Tavi or Prism, computed once in init.
// DG_DIS >0: disengage below DG_DIS% HP when an enemy Stellar/Tavi/Prism is within DG_DISR (spell range 4).
// DG_BANK >0: on collision/interlock bank at DG_BANK carried gems.
#ifndef DG_DIS
#define DG_DIS 0
#endif
#ifndef DG_DISR
#define DG_DISR 4
#endif
#ifndef DG_BANK
#define DG_BANK 0
#endif
// DP_J: interlock-only target stickiness jitter for padded ensembles (3 = baseline, compiled out).
#ifndef DP_J
#define DP_J 3
#endif
static bool g_dg = false;   // enemy caster draft (DG_DIS gate)
static bool g_dgb = false;  // g_dg on collision/interlock (DG_BANK gate)
static bool g_dgd = false;  // DG_DIS gate: g_dg on collision only
#ifndef P_BANK
#define P_BANK 60
#endif
#ifndef P_SHOP
#define P_SHOP 150
#endif
#ifndef P_GOLDW
#define P_GOLDW 0.4f
#endif
#ifndef P_RECALL_HP
#define P_RECALL_HP 0.30f
#endif
#ifndef P_REST_HP
#define P_REST_HP 0.50f
#endif
#ifndef P_FLEE
#define P_FLEE 1.0f
#endif
#ifndef P_CHASE
#define P_CHASE 1.0f
#endif
#ifndef P_ENDGAME
#define P_ENDGAME 300
#endif
#ifndef P_MONO
#define P_MONO 0.14f
#endif
#ifndef P_ALARM
#define P_ALARM 0
#endif
#ifndef P_CARRY_FLEE
#define P_CARRY_FLEE 9999
#endif
#ifndef P_CARRY_RATIO
#define P_CARRY_RATIO 1.5f
#endif
#ifndef P_LOG
#define P_LOG 1
#endif
#ifndef P_BANKFLEE
#define P_BANKFLEE 1.0f
#endif
#ifndef P_IDLEBANK
#define P_IDLEBANK 10
#endif
#ifndef P_IDLESHOP
#define P_IDLESHOP 60
#endif
#ifndef FM_RQ
#define FM_RQ 0          // >0: recall after FM_RQ rounds unhit with the nearest enemy >= FM_RQD
#endif
#ifndef FM_RQD
#define FM_RQD 5
#endif
#ifndef P_RECALL_QUIET
#define P_RECALL_QUIET 6
#endif
#ifndef P_LOCK
#define P_LOCK 1
#endif
#ifndef P_LOCKMAX
#define P_LOCKMAX 4
#endif

// ra (18 Oct): abort memory after a cancelled recall. RA_ON bit 1: no recall restart for RA_K rounds after a cancel;
// bit 2: during that window retreat with rc_flee (away from visible enemies, then home) before any PvP.
// RA_MAPS: nxm() bits where ra applies (the rc recheck also runs there). Default RA_ON 0 compiles to the old code.
#ifndef RA_ON
#define RA_ON 0
#endif
#ifndef RA_K
#define RA_K 4
#endif
#ifndef RA_MAPS
#define RA_MAPS 0x12004
#endif
// rc (17 Oct): an active recall rechecks visible enemies each turn (NotSoLonely death: Stellar recalling with one tick left, Issac 2 tiles away).
#ifndef RC_ON
#define RC_ON 0          // 1: recheck threats while recalling; cancel and retreat when one can hit us before completion
#endif
#ifndef RC_MAPS
#define RC_MAPS 0x3FFFF  // nxm() bit mask of maps where the recheck runs
#endif
// bl (18 Oct, death ledger): 1 = burst leave gate in pvp_t/sf_pvp_inner, 2 = safe recall (walk out of spell reach before
// channelling, rc recheck + rc_flee on BL_MAPS). BL_ON 0 compiles to the old code.
#ifndef BL_ON
#define BL_ON 0
#endif
#ifndef BL_MAPS
#define BL_MAPS 0x782E   // nxm() bits: collision interlock eight stampede contest swirl rushbait corners
#endif
#ifndef BL_FRAC
#define BL_FRAC 0.5f     // leave when the enemies' two-turn damage reaches this fraction of our HP
#endif
#ifndef BL_HOLD
#define BL_HOLD 6        // rounds without re-engaging after a burst leave
#endif
#ifndef BL_FLOOR
#define BL_FLOOR 0.3f    // also leave when two turns of enemy damage would take us below this fraction of max HP
#endif
#ifndef BL_API
#define BL_API 1         // 1: read enemy ability cooldown/damage through the SDK (kit model after an SDK error)
#endif
#ifndef BL_LMAPS
#define BL_LMAPS BL_MAPS // per-switch masks (leave gate / safe recall)
#endif
#ifndef BL_RMAPS
#define BL_RMAPS BL_MAPS
#endif
#define BL_L ((BL_ON & 1) && ((BL_LMAPS >> nxm()) & 1))
#define BL_R ((BL_ON & 2) && ((BL_RMAPS >> nxm()) & 1))
// ---- tf (18 Oct): PvP fight-or-disengage controller for pvp_t champions, compile-time TF_ON, nxm() bitmask TF_MAPS.
// Once in an exchange (enemy within 4, or hurt in the last 2 rounds with an enemy within 6) we fight only when our local
// power (hp x dps within TF_R of us, Cassi & Issac counted once) is >= TF_FIGHT x theirs and we are above TF_LOWHP;
// otherwise we step away first (dash first) and attack from the new tile, do not chase for TF_HOLD rounds unless the
// power turns favourable, recall below TF_LOWHP once clear, and never channel with an enemy within TF_RECD (casters
// TF_RECDC = spell range 4 + radius 1 + 1). Camp farming is untouched without an enemy within 6. TF_ON 0 = old code.
// rr (19 Oct; contest-winning-spec item 1, geese dossier counters 2-3). On RR_MASK maps (nxm bits: 0x800 contest,
// 0x2 collision) bodies in RR_WHO (1 Stellar, 2 Tavi, 4 Ren, 8 Ryth, 16 other) decide retreat BEFORE the optional
// attack/cast in pvp_t: a ranged attack ends movement, so the base bot never leaves while an enemy is in its range
// (geese contest: Tavi r18-26, Stellar r20-29 attack every turn and die without moving).
// A: leave condition = the existing outmatched/lethal/rotate test (sf_rflee formula) plus, RR_INC > 0, incoming
//    (enemies within their range + 1: strength + burst) >= RR_INC x hp while allies within 3 < those enemies.
//    Leaving = step out first, then attack only if still in range.
// B (RR_B): enemies within 4 > allies within 4 + 1 -> no optional attack unless the lowest-hp target in range dies
//    this turn (hp <= strength + burst); the body steps out instead.
#ifndef RR_MASK
#define RR_MASK 0
#endif
#ifndef RR_WHO
#define RR_WHO 3
#endif
#ifndef RR_INC
#define RR_INC 0.0f
#endif
#ifndef RR_B
#define RR_B 0
#endif
#ifndef RR_COND
#define RR_COND 31      // leave terms: 1 outmatched, 2 lethal burst, 4 low-hp rotate, 8 incoming (RR_INC), 16 outnumbered (RR_B)
#endif
#ifndef RR_KITE
#define RR_KITE 0       // 1: a leaving ranged body prefers a step that keeps an enemy within its range, then attacks
#endif
// ctx (19 Oct): CTX_DR gates the rr switches on the enemy draft (imit_geese reproduces the real geese contest loss):
// 1 = enemy drafted Cassi&Issac; 2 = enemy draft is exactly {Varg, Stellar, C&I, Ren, Ryth} (geese); 3 = C&I and no Tavi.
#ifndef CTX_DR
#define CTX_DR 0
#endif
static bool g_ctx_dr = CTX_DR == 0;
#ifndef RR_LOG
#define RR_LOG 0
#endif
#ifndef TF_ON
#define TF_ON 0
#endif
#ifndef TF_MAPS
#define TF_MAPS 0
#endif
#ifndef TF_FIGHT
#define TF_FIGHT 1.2f
#endif
#ifndef TF_LOWHP
#define TF_LOWHP 0.35f
#endif
#ifndef TF_HOLD
#define TF_HOLD 6
#endif
#ifndef TF_RECD
#define TF_RECD 6
#endif
#ifndef TF_RECDC
#define TF_RECDC 7
#endif
#ifndef TF_R
#define TF_R 6
#endif
#ifndef TF_LOG
#define TF_LOG 0
#endif
// tf2 (18 Oct): the controller split into parts. TF_DIS = (b) power-based disengage, hold, post-PvP recall threshold;
// TF_REC = (c) no channel with an enemy within TF_RECD / caster within TF_RECDC. The ranged step-then-attack micro (a)
// is always on under TF_ON: with TF_DIS 0 it fires on the old flee decision (outmatched/lethal/rotate/fd_force).
#ifndef TF_DIS
#define TF_DIS 1
#endif
#ifndef TF_REC
#define TF_REC 1
#endif
// tf2_fin: each part has its own nxm() map mask (defaults: every map when the part flag is set).
#ifndef TF_DISMAPS
#define TF_DISMAPS (TF_DIS ? 0x3FFFF : 0)
#endif
#ifndef TF_RECMAPS
#define TF_RECMAPS (TF_REC ? 0x3FFFF : 0)
#endif
#ifndef RC_TURNS
#define RC_TURNS 0       // enemy turns considered beyond the next one (horizon = min(left, 1 + RC_TURNS))
#endif
#ifndef RC_DASH
#define RC_DASH 0        // extra reach assumed for melee enemies (dash)
#endif
#ifndef RC_SPR
#define RC_SPR 4         // ranged/spell reach without moving
#endif
#ifndef RC_LEFT
#define RC_LEFT 9        // only recheck when remaining ticks <= RC_LEFT
#endif
#ifndef RC_HP
#define RC_HP 1.0f       // only cancel when hp < RC_HP * maxhp
#endif
#ifndef RC_DMG
#define RC_DMG 0.0f      // only cancel when predicted damage >= RC_DMG * hp (0: any reachable threat)
#endif
#ifndef RC_FT
#define RC_FT 1          // 1: forced retreat after cancelling; 0: fall through to normal turn logic
#endif
#ifndef RC_LOG
#define RC_LOG 0
#endif
#ifndef P_SAFEBANK
#define P_SAFEBANK 1
#endif
#ifndef P_EDPS
#define P_EDPS 1.3f
#endif
#ifndef P_BANKSTALL
#define P_BANKSTALL 20
#endif
// dr_* (15 Oct): caster draft variants. DR_HOT/DR_FOUR/DR_BIG/DR_TINY/DR_SMALL override the draft orders
// when DR_*G gate passes (0 always, 1 only when the enemy already has Stellar, 2 only when it has Stellar or Tavi).
#ifndef DR_BIGSEL
#define DR_BIGSEL 15
#endif
#ifndef DR_PAD
#define DR_PAD 0
#endif
#ifndef DR_J
#define DR_J 0
#endif
#ifndef DR_LOG
#define DR_LOG 0
#endif
#ifndef DR_HOTG
#define DR_HOTG 0
#endif
#ifndef DR_FOURG
#define DR_FOURG 0
#endif
#ifndef DR_BIGG
#define DR_BIGG 0
#endif
#ifndef DR_TINYG
#define DR_TINYG 0
#endif
#ifndef P_SMALL_TEAM
#define P_SMALL_TEAM 3
#endif
#ifndef P_SMALL_DRAFT
#define P_SMALL_DRAFT 9, 2, 0, 10, 8, 3, 6, 7, 1, 4, 5
#endif
#ifndef P_SMALL_AREA
#define P_SMALL_AREA 3600
#endif
#ifndef P_BURSTW
#define P_BURSTW 1.0f
#endif
#ifndef P_LETHAL
#define P_LETHAL 0.8f
#endif
#ifndef P_HOME
#define P_HOME 0
#endif
#ifndef P_HOME_HP
#define P_HOME_HP 0.4f
#endif
#ifndef P_BANK_GIVEUP
#define P_BANK_GIVEUP 60
#endif
#ifndef P_BANK_COOL
#define P_BANK_COOL 40
#endif
#ifndef P_PRESS
#define P_PRESS 4
#endif
// ed (19 Oct): carried-gem protection late in close races. All masks are nxm() bits (>= 512); 0 = off (default).
// ED_MASK: enemy bank >= ED_EB and own bag >= ED_BAG -> persistent delivery intent before pvp_t (bank mode, no
//   engaged()/cooldown/give-up gate, no bank-stall press chase) until the bag is gone.
// ED_MASK2: same with ED_EB2 / ED_BAG2 (narrow rushbait/ambush deadline: bank now even mid-capture).
// ED_HOLD: held-altar avoidance: once per bank run, when first within ED_HV of the nearest altar, skip it if >= ED_HN
//   visible enemies stand within ED_HR of it (or an enemy altar lock is on it); and skip it after ED_STALL bank rounds
//   spent within ED_SR of it without delivering (counts rounds, not distance gains).
#ifndef ED_MASK
#define ED_MASK 0
#endif
#ifndef ED_EB
#define ED_EB 750
#endif
#ifndef ED_BAG
#define ED_BAG 44
#endif
#ifndef ED_MASK2
#define ED_MASK2 0
#endif
#ifndef ED_EB2
#define ED_EB2 850
#endif
#ifndef ED_BAG2
#define ED_BAG2 60
#endif
#ifndef ED_HOLD
#define ED_HOLD 0
#endif
#ifndef ED_HN
#define ED_HN 3
#endif
#ifndef ED_HR
#define ED_HR 4
#endif
#ifndef ED_HV
#define ED_HV 9
#endif
#ifndef ED_SR
#define ED_SR 8
#endif
#ifndef ED_STALL
#define ED_STALL 12
#endif
#ifndef ED_SE
#define ED_SE 0   // 1: only rounds with an enemy seen within 6 last round count toward ED_STALL
#endif
#ifndef ED_LOG
#define ED_LOG 0
#endif
#ifndef P_DASHFLEE
#define P_DASHFLEE 1
#endif
#ifndef P_PAIR
#define P_PAIR 2
#endif
#ifndef P_PAIR_W
#define P_PAIR_W 3.0f
#endif
#ifndef P_PAIR_AREA
#define P_PAIR_AREA 1000
#endif
// il5 (19 Oct): IL5_PAIR 1 = the P_PAIR partner-target weighting also on interlock (40x40 four-pick), which P_PAIR_AREA 1000
// excludes (1600 tiles); only the il choose_target instantiation reads it, so every other map compiles to the same code.
// IL5_PAIR 1 = Cassi weights Issac's target only; 2 = both directions (as P_PAIR 2 does on elkring).
#ifndef IL5_PAIR
#define IL5_PAIR 0
#endif
// il5 (19 Oct): IL5_LSST 1 = on the collision lane-sweep plan (g_cplan 2) Stellar is exempt from ls_allow(), so she farms
// own-side rewards/camps from round 1 instead of trailing the melee sweep (v400 losses: Stellar 240-430 gold vs ~1000).
#ifndef IL5_LSST
#define IL5_LSST 0
#endif
// il5 (19 Oct): IL5_STOWN 1 = on the collision lane-sweep plan Stellar does not start enemy-half camps on her own (altar-adjacent
// groups and groups an ally already claims stay allowed): the logged v400-style losses show her walking alone into the enemy
// half after the sweep, dying or recalling, and earning 240-430 gold.
#ifndef IL5_STOWN
#define IL5_STOWN 0
#endif
#ifndef P_FUNNEL
#define P_FUNNEL 1
#endif
#ifndef P_HUBW
#define P_HUBW 1.8f
#endif
#ifndef P_SPREAD
#define P_SPREAD 0.3f
#endif
// rx_rows1 (15 Oct, rows only = two altars on 50x50): fast rows teams (SSSA, tip, Ely2sh) bank 16+ camps by r370.
// RX_ROWS 1: no lock mission to the enemy altar (it cost one champion r65-r180 and opponents break the lock in 3 turns; we still drop one on our own altar and now clear enemy locks there),
// Monster strength then Power items (faster camp kills; SSSA's build), and no CT_BANK 36 small bags vs Stellar+Tavi.
#ifndef RX_ROWS
#define RX_ROWS 0
#endif
#ifndef P_ROWS_BANK
#define P_ROWS_BANK 90
#endif
#ifndef P_HUBMIN
#define P_HUBMIN 20
#endif
#ifndef P_HUBW_SMALL
#define P_HUBW_SMALL 2.0f
#endif
#ifndef P_FOCUS
#define P_FOCUS 1
#endif
#ifndef P_BUILD
#define P_BUILD 11
#endif
// Collision (41x41) only: favour the two wolves beside the altar, unless we recently fled there.
#ifndef P_CO_HOT
#define P_CO_HOT 2.0f
#endif
#ifndef P_CO_DANGER
#define P_CO_DANGER 15
#endif
#ifndef P_CO_DANGERW
#define P_CO_DANGERW 0.5f
#endif

// Interlock (40x40, four picks) only.
#ifndef IL_VGOLD
#define IL_VGOLD 1.0f
#endif
#ifndef IL_VSPLASH
#define IL_VSPLASH 99
#endif
#ifndef IL_RAID
#define IL_RAID 0
#endif
#ifndef IL_RAIDN
#define IL_RAIDN 2
#endif
#ifndef IL_NEARR
#define IL_NEARR 5
#endif
#ifndef IL_DRAFTCI
#define IL_DRAFTCI 0
#endif
#ifndef IL_KEEPPAIR
#define IL_KEEPPAIR 0
#endif
#ifndef FF_ROT
#define FF_ROT 0.3f
#endif
#ifndef FF_ROT_I
#define FF_ROT_I 0.6f
#endif
// Corners gold-first (71x71, no Prism draft). The hub crystals pay gems only, so v190 earned almost no gold there and
// lost the late fights against teams carrying ten or more items. Until a champion's gold (held plus spent on items)
// reaches FG_GOLD, and before round FG_UNTIL, gold counts FG_GW per coin and the hub loses its bonus.
#ifndef FG_ON
#define FG_ON 1
#endif
#ifndef FG_GOLD
#define FG_GOLD 300
#endif
#ifndef FG_GOLD_SUP
#define FG_GOLD_SUP FG_GOLD
#endif
#ifndef FG_GW
#define FG_GW 1.0f
#endif
#ifndef FG_HUBE
#define FG_HUBE 1.0f
#endif
#ifndef FG_UNTIL
#define FG_UNTIL 400
#endif
#ifndef FG_NOLOCK
#define FG_NOLOCK 1
#endif
#ifndef FG_BUILD
#define FG_BUILD 0
#endif
#ifndef FG_PAD
#define FG_PAD 0
#endif
// fh (v193 candidate): the corners economy rule on two more maps, accepted per map by padded ensembles + holdout.
// Packs (31x31): health-first build, no altar locks, gold-first to FH_PACKS_GOLD. Monolith (50x50, not two-altar rows):
// health-first build and no altar locks only (gold-first lost its gain there). Elkring, interlock, collision, lanes,
// rows and expanse were tested and keep v192 play.
#ifndef FH_PACKS
#define FH_PACKS 1
#endif
#ifndef FH_PACKS_GOLD
#define FH_PACKS_GOLD 150
#endif
#ifndef FH_MONO
#define FH_MONO 1
#endif
#ifndef FH_PAD
#define FH_PAD 0
#endif
// pk (16 Oct, packs 31x31 only): crystal-hub plans from decoded platform games (runs/research-packs-winners.md).
// PK_ON 0 keeps ca_sel2 play exactly. PK_HUBW multiplies crystal-hub groups (harmless, 40+ gems); PK_OPENR/PK_OPENW:
// before round PK_OPENR non-Varg champions weight the hubs by PK_OPENW instead (winners take both hubs by R30-50).
// PK_SAFE: skip a hub an enemy was seen within PK_HUBR of in the last PK_HUBMEM rounds unless PK_HUBN allies are
// within 4 of us; PK_FAR: skip the enemy's two home-edge wolf pairs. PK_MIMIC (proxy only): Cassi, Issac, Stellar
// and Ren camp the hubs and Varg/Vendetta farm the own half, as Ely2sh, Conley and AlphaSlop do.
#ifndef PK_ON
#define PK_ON 0
#endif
// ek (16 Oct, eight 41x30 one altar / elkring 30x30): map-gated target weights from decoded platform games
// (scratchpad/ek_agent/notes.md). Other maps never instantiate this code path.
#ifndef E9_ON
#define E9_ON 0
#endif
#ifndef E9_R
#define E9_R 0
#endif
#ifndef E9_WHO
#define E9_WHO 4095
#endif
#ifndef E9_TAVI
#define E9_TAVI 0
#endif
#ifndef EI_ON
#define EI_ON 0
#endif
#ifndef EI_R
#define EI_R 150       // opening window (rounds)
#endif
#ifndef EI_W72
#define EI_W72 1.0f    // own-side 72-gem elk pair, during the opening
#endif
#ifndef EI_W72L
#define EI_W72L 1.0f   // own-side 72-gem elk pair, after the opening
#endif
#ifndef EI_W44
#define EI_W44 1.0f    // centre 44-gem wolf pairs, during the opening
#endif
#ifndef EI_W44L
#define EI_W44L 1.0f   // centre 44-gem wolf pairs, after the opening
#endif
#ifndef EI_WFAR
#define EI_WFAR 1.0f   // enemy-side groups (spawn_dist > EI_FARD)
#endif
#ifndef EI_FARD
#define EI_FARD 24
#endif
#ifndef EI_BANK
#define EI_BANK 0     // eight: bank at this many carried gems (0 = P_BANK)
#endif
#ifndef EK_WMID
#define EK_WMID 1.0f  // elkring: middle-distance groups (between EK_NEARD and EK_FARD)
#endif
#ifndef EK_NEEDCI
#define EK_NEEDCI 0
#endif
#ifndef EK_ON
#define EK_ON 0
#endif
#ifndef EK_WFAR
#define EK_WFAR 1.0f   // elkring: groups nearer the enemy spawn (spawn_dist >= EK_FARD)
#endif
#ifndef EK_FARD
#define EK_FARD 24
#endif
#ifndef EK_WNEAR
#define EK_WNEAR 1.0f  // elkring: own-corner groups (spawn_dist <= EK_NEARD)
#endif
#ifndef EK_NEARD
#define EK_NEARD 10
#endif
#ifndef PK_HUBW
#define PK_HUBW 1.0f
#endif
#ifndef PK_OPENR
#define PK_OPENR 0
#endif
#ifndef PK_OPENW
#define PK_OPENW 1.0f
#endif
#ifndef PK_SAFE
#define PK_SAFE 0
#endif
#ifndef PK_HUBR
#define PK_HUBR 6
#endif
#ifndef PK_HUBMEM
#define PK_HUBMEM 10
#endif
#ifndef PK_HUBN
#define PK_HUBN 1
#endif
#ifndef PK_FAR
#define PK_FAR 0
#endif
#ifndef PK_SAFESEL
#define PK_SAFESEL 0
#endif
#ifndef PK_MIMIC
#define PK_MIMIC 0
#endif
#ifndef PK_NOGOLD
#define PK_NOGOLD 0
#endif
#ifndef PK_LOG
#define PK_LOG 0
#endif
// fb (BottleBots farming race): all default off = fhr behaviour.
// FB_COMMIT: skip the low-health recall while finishing a camp we can kill before it kills us (no enemy seen for FB_CQ rounds).
// FB_RB: in bank mode, recall home and bank from spawn when walking is more than recall + spawn-to-altar + FB_RB turns.
#ifndef FB_COMMIT
#define FB_COMMIT 0
#endif
#ifndef FB_CMARGIN
#define FB_CMARGIN 0.05f
#endif
#ifndef FB_CQ
#define FB_CQ 6
#endif
#ifndef FB_RB
#define FB_RB 0
#endif
#ifndef Q9_LNSHOP
#define Q9_LNSHOP 0     // q9: lanes shopping recall gold threshold (0 = off, P_SHOP)
#endif
#ifndef FB_RBMAP
#define FB_RBMAP 0
#endif
#ifndef FB_PAD
#define FB_PAD 0
#endif
#ifndef FB_ELK
#define FB_ELK 0
#endif
#ifndef FB_ELKHP
#define FB_ELKHP 1300.0f
#endif
#ifndef FB_CHASE
#define FB_CHASE 0
#endif
#ifndef FB_CHASEMAP
#define FB_CHASEMAP 0
#endif
#ifndef FB_LOG
#define FB_LOG 0
#endif

namespace {

constexpr std::uint16_t UNREACH = std::numeric_limits<std::uint16_t>::max();
constexpr std::array<vec2, 8> DIRS{ vec2{0, 1}, vec2{1, 1}, vec2{1, 0}, vec2{1, -1}, vec2{0, -1}, vec2{-1, -1}, vec2{-1, 0}, vec2{-1, 1} };

int cheb(vec2 a, vec2 b) { return static_cast<int>(a.chebyshev_distance(b)); }

// cc_crystal (28 Oct): on exact ccx_ layouts, a carrier already banking hops over a wall/hole with one
// Teleport Crystal onto a free altar neighbour when walking there would take >= CCX_MINWALK steps.
#ifndef CCX_MASK
#define CCX_MASK 0x2
#endif
#ifndef CCX_MINWALK
#define CCX_MINWALK 10
#endif
#ifndef CCX_RMASK
#define CCX_RMASK 0x2
#endif
#ifndef CCX_RWALK
#define CCX_RWALK 40
#endif
#ifndef CCX_RGEMS
#define CCX_RGEMS 40
#endif
static int ccx_map = 0;
static bool g_gwc = false;   // cc_crystal2: exact Great Wall (ccx_map 2) and GWC_ON, set on turn 3
inline bool ccx_on() { return ccx_map > 0 && ((CCX_MASK >> ccx_map) & 1); }
void end_mission();
// ---- fu (18 Oct): fuel fixes, decision-neutral. FU_ELOGF: print the E fight log only with at least this much fuel left
// (the iostream line costs ~27k fuel on corners). FU_SNAP: pvp_t keeps one snapshot of visible enemy positions for the
// flee loops instead of three host calls per enemy per candidate tile (re-read after a dash). FU_DFME: choose_target
// recomputes the from-me distance field only when the position changed (terrain is static). All 0 = old code.
#ifndef FU_ELOGF
#define FU_ELOGF 0
#endif
#ifndef FU_SNAP
#define FU_SNAP 0
#endif
#ifndef FU_DFME
#define FU_DFME 0
#endif

struct Camp {
    cambc::camp h;
    vec2 pos;
    float hp, maxhp, str;
    int range;
    // Static adjacency only; health is always read fresh from the current Camp state.
    mutable std::array<std::uint16_t, 9> adjacent{};
    mutable int adjacent_count = -1;
    explicit Camp(cambc::camp handle) : h(handle) {}
};

struct Group {
    handle_camp_group_t h;
    std::span<Camp> camps;
    // Immutable group tiles, materialized only when a caller first needs them.
    mutable std::optional<bitboard> tile_cache;
    const bitboard& tiles() const {
        if (!tile_cache) {
            tile_cache.emplace(uninitialized);
            tile_cache->clear();
            for (const auto& c : camps) tile_cache->set(c.pos);
        }
        return *tile_cache;
    }
    int gems, gold, respawn;
    int st = 0;            // 0 alive, 1 reward available, 2 respawning
    int respawn_round = 0; // round at which a respawning group is alive again
    int spawn_dist = 0;
    int splash = 1;        // most camps hit by a radius-one effect
    float mdps = 0;        // summed strength of the group's attackers
    int max_range = 3;
    int seen = -1;
    bool hub = false;      // part of a cluster of three or more harmless gem groups
    float hp() const { float s = 0; for (auto& c : camps) s += c.hp; return s; }
    float alive_dps() const { float s = 0; for (auto& c : camps) if (c.hp > 0) s += c.str; return s; }
};

int alive_neighbours(const Group& g, const Camp& c) {
    if (c.adjacent_count < 0) {
        c.adjacent_count = 0;
        for (std::size_t j = 0; j < g.camps.size(); ++j) {
            const int dx = c.pos.x - g.camps[j].pos.x;
            if (dx < -1 || dx > 1) continue;
            const int dy = c.pos.y - g.camps[j].pos.y;
            if (dy < -1 || dy > 1) continue;
            if (c.adjacent_count < static_cast<int>(c.adjacent.size()))
                c.adjacent[c.adjacent_count] = static_cast<std::uint16_t>(j);
            ++c.adjacent_count;
        }
    }
    int n = 0;
    if (c.adjacent_count <= static_cast<int>(c.adjacent.size())) {
        for (int i = 0; i < c.adjacent_count; ++i) if (g.camps[c.adjacent[i]].hp > 0) ++n;
    } else {
        // Preserve the general count even for overlapping camp positions.
        for (const auto& o : g.camps) if (o.hp > 0 && cheb(o.pos, c.pos) <= 1) ++n;
    }
    return n;
}

enum class Mode { farm, bank, recall };

struct State {
    // Reserved once for the complete layout; every Group span stays valid.
    std::vector<Camp> camp_storage;
    std::vector<Group> groups;
    bitboard terrain, vis, occ, goals, me_bb, visited, tmp;
    distance_field df_spawn, df_altar, df_me, df_go;
#if FU_DFME
    vec2 fu_dfpos{ -1, -1 };
#endif
    std::vector<vec2> altars;
    int idx = 0;
    champion_type type{};
    bool ranged = false;
    bool rows_like = false; // two altars on a 50x50 map: an uncontested farming race
    int range = 1;
    int target = -1;
    Mode mode = Mode::farm;
    std::array<std::uint8_t, 8> shm{};
    int round = 0;
    bool moved = false, cast = false;
    int W = 0, H = 0;
    int idle = 0, attacks = 0, casts = 0, moves = 0, turns = 0, channel = 0, late = 0;
    int last_round = -1;
    bool was_dead = false;
    int bank_unsafe_round = -9;
    int mission = -1;
    int enemy_round = -99;
#if RA_ON
    int ra_round = -999;
#endif
#if BL_ON
    int bl_hold = -1;
#endif
#if TF_ON
    int tf_dis = -99, tf_pvp = -99;   // tf: disengaging until this round; last round in a PvP exchange
#endif
    bool fb_logged = false;
    int fb_chase = 0;
    int fb_eng_tgt = -1, fb_eng_r0 = 0;
    float fb_eng_hp0 = 0;
    int spawn_altar = 1 << 20; // walking distance from our spawn to the nearest altar tile
    int last_hp = 0, last_gems = 0;
    int bank_turns = 0, bank_skip = -1, bank_best = 1 << 30;
    int bank_age = 0, bank_cool_until = -1;
#if ED_MASK || ED_MASK2 || ED_HOLD
    int ed_on = 0, ed_dec = -1, ed_near = 0;
#endif
    int seen_hp = -1, hurt_round = -99;
    int t_recall = 0, t_bank = 0, t_pvp = 0, t_heal = 0, t_wait = 0, t_cap = 0, t_fight = 0, t_walk = 0, n_recall = 0;
    float jungle = 1.0f;
    bool old_elkring_mages = false;
    bool mite_elkring_roster = false;
    bool mite_elkring_finish = false;
    bool interlock_keeper = false;
    bool corners_hello_safe100 = false;
    bool corners_cap = false; // corners vs Stellar drafts: at most three champions take the hub bonus
    bool fg_on = false;    // corners (71x71) without Prism: gold-first opening, no altar locks
    bool fg_shop = false;  // health-first build, no altar locks: corners with fg_on, packs, monolith
    bool fg_route = false; // gold-first targeting: corners with fg_on, packs
    bool pk = false;       // packs crystal-hub plan (PK_ON)
    bool ekmap = false;    // ek target weights (EI_ON eight / EK_ON elkring)
    int fg_target = FG_GOLD;
    bool lanes_ownhalf = false;
    bool lanes_current_left = false;
    bool spawn_right = false;
    bool coll = false;     // collision (41x41)
    bool il = false;       // interlock (40x40, four picks)
    bool il_nolock = false;
    bool il_west = false;
};

// Collision-only state, kept outside State so other maps run the same code and memory layout.
struct CollisionState {
    int hot_flee_round = -999;
    std::array<std::uint16_t, 64> altar_dist{};
};
CollisionState CO;

// Collision bag discipline (41x41 map only; every hook is behind CB.on).
#ifndef CB_A
#define CB_A 0      // (a) carried gems at which far/enemy-side camps are not started; 0 off
#endif
#ifndef CB_AD
#define CB_AD 19    // far camp: altar distance at least this
#endif
#ifndef CB_SD
#define CB_SD 28    // enemy-side camp: spawn distance at least this
#endif
#ifndef CB_B
#define CB_B 0      // (b) carried gems for the escort wait after a failed bank; 0 off
#endif
#ifndef CB_BR
#define CB_BR 4     // ally radius that ends the wait
#endif
#ifndef CB_BMAX
#define CB_BMAX 60  // longest wait
#endif
#ifndef CB_BMODE
#define CB_BMODE 1  // 0 hold away from the altar, 1 walk to the nearest ally
#endif
#ifndef CB_HOLD
#define CB_HOLD 7
#endif
#ifndef CB_C
#define CB_C 0      // (c) carried gems that make a must-bank bag (also banked + carried >= win); 0 off
#endif
#ifndef CB_CR
#define CB_CR 10    // escort radius
#endif
#ifndef CB_D
#define CB_D 0      // (d) longest carry without a bank, in rounds; 0 off
#endif
struct CollBag {
    bool on = false;
    bool prev_bank = false;
    bool wait = false;
    int zero_round = 0;
    int wait_since = 0;
};
CollBag CB;

// ---- cx: collision "pack and hill" mode (41x41 only, every hook behind CX.on) ----
// One shared altar decides collision. The team farms its own lane as a pack (shared target), concedes the
// enemy lane, banks small bags near the altar, escorts a banking ally, regroups before contesting a held altar,
// and never chases alone.
#ifndef CX_ON
#define CX_ON 0
#endif
#ifndef CX_PACKW
#define CX_PACKW 2.0f   // target score multiplier per ally already heading to the same camp group
#endif
#ifndef CX_TERR
#define CX_TERR 1       // skip camp groups on the enemy's half unless within CX_TERRD of the altar
#endif
#ifndef CX_TERRD
#define CX_TERRD 4
#endif
#ifndef CX_BANKN
#define CX_BANKN 22     // bank at this many gems when within CX_BANKD of the altar
#endif
#ifndef CX_BANKD
#define CX_BANKD 9
#endif
#ifndef CX_BANKB
#define CX_BANKB 44     // bank at this many gems anywhere
#endif
#ifndef CX_ESC
#define CX_ESC 12       // join a banking ally within this radius (0 off)
#endif
#ifndef CX_HOLD
#define CX_HOLD 1       // regroup before entering a held altar
#endif
#ifndef CX_HOLDD
#define CX_HOLDD 8
#endif
#ifndef CX_HOLDMAX
#define CX_HOLDMAX 30
#endif
#ifndef CX_NOCHASE
#define CX_NOCHASE 1    // no chase without an ally within 3
#endif
#ifndef CX_RAID
#define CX_RAID 0.0f   // proxy only: score multiplier for enemy-side groups within 9 of the altar
#endif
#ifndef CX_LEASH
#define CX_LEASH 0      // >0: while farming, rejoin the nearest field ally farther than this
#endif
struct CxState {
    bool on = false;
    bool north = false;
    int hold_since = -1;
    int bank_slot = -1;
};
CxState CX;
// ---- ca: adaptive collision plan (41x41 only), chosen once in init() from the enemy draft.
// 0 = rx_rows1 (v268), 1 = CX territory (cxr2), 2 = LS lane sweep (cl_sweep). Each path is the tested source.
#ifndef CA_LOG
#define CA_LOG 0
#endif
#ifndef CA_FORCE
#define CA_FORCE -1     // >=0: always use this plan (equivalence checks)
#endif
// cz2 (17 Oct): collision plan vs Cassi&Issac + Ren + Stellar drafts without Varg/Vendetta (0-8 on the platform with plan 0:
// 天使, Valenki, NotSoLonely, VeryGood, invisibleAgain, WHIM, Memory?). -1 = off (q9_fix play). CZ2_PRRS / CZ2_CSTA: same
// for Prism+Ren+Stellar (geese) and C&I+Stellar+Tavi without Ren (Cabbage) drafts.
#ifndef CZ2_CIRS
#define CZ2_CIRS -1
#endif
#ifndef CZ2_PRRS
#define CZ2_PRRS -1
#endif
#ifndef CZ2_CSTA
#define CZ2_CSTA -1
#endif
#ifndef CZ2_SRRY
#define CZ2_SRRY -1     // mirror Stellar+Ren+Ryth without C&I/Varg/Vendetta (剑桥夜爬者 286:1046)
#endif
int g_cplan = 0;
int ca_select();
bool cx_skip(int gi);
float cx_pack_mult(int gi);
bool cx_chase_ok(vec2 pos);
bool cx_bank_now(std::uint32_t gems, vec2 pos);
bool cx_post(std::uint32_t gems);
// ---- ls: collision lane sweep (41x41 only, behind CB.on). Every winning collision team opens by killing its
// own lane wolves as one group in spawn order (W1 (10,5) ~R16, W2 (15,9) ~R27, W3 (19,13) ~R40) before
// contesting the altar; we rushed the altar wolf alone (hot weight) and died there. Until LS_END only the
// front lane wolf (nearest living own-side wolf group to our spawn) and capturable rewards are targets.
#ifndef LS_ON
#define LS_ON 0
#endif
#ifndef LS_END
#define LS_END 75
#endif
#ifndef LS_BANKN
#define LS_BANKN 0     // >0: on collision bank at this many gems within LS_BANKD of the altar
#endif
#ifndef LS_BANKD
#define LS_BANKD 9
#endif
#ifndef LS_ELK
#define LS_ELK 0       // 1: own-side elk groups also count as lane camps
#endif
int ls_round = -1, ls_f = -1, ls_north = -1;

// Corners vs Prism drafts. Only reached through the corners_hello_safe100 check, so other games run unchanged code.
#ifndef CP_SAFE
#define CP_SAFE 100
#endif
#ifndef CP_MARGIN
#define CP_MARGIN 4
#endif
#ifndef CP_HOTN
#define CP_HOTN 2
#endif
#ifndef CP_BMEM
#define CP_BMEM 5
#endif
#ifndef CP_HUBR
#define CP_HUBR 8
#endif
struct CornersPrism {
    bool ready = false;
    bool west = false;
    vec2 hub{};
    int hot_round = -999;
    int centre = -1;
    int turn = -1;
};
CornersPrism CP;

std::optional<State> S;

int draft_id_of(champion_type t) {
    switch (t) {
    case champion_type::ryth: return 0;
    case champion_type::vesper_vale: return 1;
    case champion_type::vendetta: return 2;
    case champion_type::cassi: return 3;
    case champion_type::issac: return 3;
    case champion_type::stellar: return 4;
    case champion_type::prism: return 5;
    case champion_type::marcella: return 6;
    case champion_type::willow: return 7;
    case champion_type::ren: return 8;
    case champion_type::varg: return 9;
    case champion_type::tavi: return 10;
    case champion_type::heart_of_ruin: return 11;
    }
    return 0;
}

__attribute__((noinline)) void collision_init() {
    auto& s = *S;
    for (std::size_t gi = 0; gi < s.groups.size() && gi < CO.altar_dist.size(); ++gi) {
        int d = UNREACH;
        for (auto& c : s.groups[gi].camps) d = std::min<int>(d, s.df_altar.get(c.pos));
        CO.altar_dist[gi] = static_cast<std::uint16_t>(d);
    }
}

__attribute__((noinline)) float collision_hot_weight() {
    auto& s = *S;
    return s.round - CO.hot_flee_round <= P_CO_DANGER ? P_CO_DANGERW : P_CO_HOT;
}

// New-pool (16 Oct 2026) map identity: exact size, altar set and camp-group count.
// 1 contest (50x50, Monolith-only gems), 2 stampede (60x50, one elk herd). 0 = any other map.
#ifndef NM_CT_MONO
#define NM_CT_MONO 0.0f // contest: Monolith allowed once 200 dps <= NM_CT_MONO x max health (0 = always)
#endif
// ct (17 Oct, contest only, g_nm == 1 inside the big choose_target instantiation): platform losses decoded in
// scratchpad/ct_agent/notes.md. P_GOLDW decays to 0 at round 400, so from then on the Monolith is our only target:
// base-stat champions walk into it one at a time, die, or kill it and feed the 200-gem bag to the centre holder.
// gd (17 Oct): the P_GOLDW decay (0 from round 400) on the big-map branch, per map (GD_MASK = nxm() bits):
// gold weight = P_GOLDW x max(GD_FLOOR, 1 - round / GD_R). GD_MASK 0 compiles to ct_fix's code.
#ifndef GD_MASK
#define GD_MASK 0
#endif
// tr (19 Oct): gold audit switches (docs/fuel-gold-items-audit-2026-10-19.md). All default 0 = cb_fin's code.
#ifndef TR_TMASK
#define TR_TMASK 0      // nxm() map bits: the TR_TWHO champions buy one Treasure emblem (TR_TPOS 1 = first, 2 = second emblem)
#endif
#ifndef TR_TWHO
#define TR_TWHO 3076    // champion_type bits: vendetta 4, varg 1024, tavi 2048
#endif
#ifndef TR_TPOS
#define TR_TPOS 1
#endif
#ifndef TR_TR
#define TR_TR 400       // no Treasure shard purchase from this round on (a late emblem returns too little)
#endif
#ifndef TR_CMASK
#define TR_CMASK 0      // nxm() map bits: Cassi may share ready rewards (P_FUNNEL skip lifted) while her gold < TR_CGOLD
#endif
#ifndef TR_CGOLD
#define TR_CGOLD 150
#endif
#ifndef GO_MASK
#define GO_MASK 0       // go: nxm() map bits with a gold weight floor
#endif
#ifndef GO_FLOOR
#define GO_FLOOR 0.0f
#endif
#ifndef GD_FLOOR
#define GD_FLOOR 0.0f
#endif
#ifndef GD_R
#define GD_R 400.0f
#endif
#ifndef GD_MASK2
#define GD_MASK2 0      // second map set with its own floor GD_FLOOR2
#endif
#ifndef GD_FLOOR2
#define GD_FLOOR2 0.0f
#endif
#ifndef CT_ON
#define CT_ON 0
#endif
#ifndef CT_GW
#define CT_GW 0.0f      // contest: gold weight floor for the whole game (0 = P_GOLDW decay)
#endif
#ifndef CT_OPEN
#define CT_OPEN 0       // contest: before this round skip the lone 35-gold camps (move to the centre as a team)
#endif
#ifndef CT_GROUP
#define CT_GROUP 0.0f   // contest: score x (1 + CT_GROUP x allies already on the group) before round CT_GR
#endif
#ifndef CT_GR
#define CT_GR 0
#endif
#ifndef NM_CT_ROWSOFF
#define NM_CT_ROWSOFF 1
#endif
#ifndef NM_BIG
#define NM_BIG 1
#endif
#ifndef FR_GROUP
#define FR_GROUP 0.0f // fragment: score x (1 + FR_GROUP x allies already on the group): move as a pack
#endif
#ifndef FR_GMDPS
#define FR_GMDPS 0 // pack weight only on groups with at least this monster dps (80 = the centre elks)
#endif
#ifndef FR_GROUP_ON
#define FR_GROUP_ON 0
#endif
#ifndef FR_ON
#define FR_ON 0 // fragment: 1 = skip enemy-side wolf camps, 2 = also the far centre elk
#endif
#include "cxt2_switches.h"
int g_nm = -1;
#if CXT2_COLLISION_BANK18
__attribute__((noinline)) bool cxt2_collision(){static const bool yes=[](){const auto& s=*S;
if(s.W!=41||s.H!=41||s.altars.size()!=2||s.groups.size()!=20)return false;
if(std::find(s.altars.begin(),s.altars.end(),vec2{27,20})==s.altars.end())return false;
if(std::find(s.altars.begin(),s.altars.end(),vec2{39,20})==s.altars.end())return false;
if(s.groups[0].camps.size()!=2)return false;
if(std::none_of(s.groups[0].camps.begin(),s.groups[0].camps.end(),[](const Camp& c){return c.pos==vec2{10,5}&&static_cast<int>(c.h.kind())==0;}))return false;
if(std::none_of(s.groups[0].camps.begin(),s.groups[0].camps.end(),[](const Camp& c){return c.pos==vec2{10,4}&&static_cast<int>(c.h.kind())==3;}))return false;
if(s.groups[1].camps.size()!=2)return false;
if(std::none_of(s.groups[1].camps.begin(),s.groups[1].camps.end(),[](const Camp& c){return c.pos==vec2{10,35}&&static_cast<int>(c.h.kind())==0;}))return false;
if(std::none_of(s.groups[1].camps.begin(),s.groups[1].camps.end(),[](const Camp& c){return c.pos==vec2{10,36}&&static_cast<int>(c.h.kind())==3;}))return false;
if(s.groups[2].camps.size()!=3)return false;
if(std::none_of(s.groups[2].camps.begin(),s.groups[2].camps.end(),[](const Camp& c){return c.pos==vec2{15,9}&&static_cast<int>(c.h.kind())==0;}))return false;
if(std::none_of(s.groups[2].camps.begin(),s.groups[2].camps.end(),[](const Camp& c){return c.pos==vec2{14,9}&&static_cast<int>(c.h.kind())==3;}))return false;
if(std::none_of(s.groups[2].camps.begin(),s.groups[2].camps.end(),[](const Camp& c){return c.pos==vec2{15,10}&&static_cast<int>(c.h.kind())==3;}))return false;
if(s.groups[3].camps.size()!=3)return false;
if(std::none_of(s.groups[3].camps.begin(),s.groups[3].camps.end(),[](const Camp& c){return c.pos==vec2{15,31}&&static_cast<int>(c.h.kind())==0;}))return false;
if(std::none_of(s.groups[3].camps.begin(),s.groups[3].camps.end(),[](const Camp& c){return c.pos==vec2{14,31}&&static_cast<int>(c.h.kind())==3;}))return false;
if(std::none_of(s.groups[3].camps.begin(),s.groups[3].camps.end(),[](const Camp& c){return c.pos==vec2{15,30}&&static_cast<int>(c.h.kind())==3;}))return false;
if(s.groups[4].camps.size()!=3)return false;
if(std::none_of(s.groups[4].camps.begin(),s.groups[4].camps.end(),[](const Camp& c){return c.pos==vec2{19,13}&&static_cast<int>(c.h.kind())==0;}))return false;
if(std::none_of(s.groups[4].camps.begin(),s.groups[4].camps.end(),[](const Camp& c){return c.pos==vec2{20,13}&&static_cast<int>(c.h.kind())==3;}))return false;
if(std::none_of(s.groups[4].camps.begin(),s.groups[4].camps.end(),[](const Camp& c){return c.pos==vec2{19,14}&&static_cast<int>(c.h.kind())==2;}))return false;
if(s.groups[5].camps.size()!=3)return false;
if(std::none_of(s.groups[5].camps.begin(),s.groups[5].camps.end(),[](const Camp& c){return c.pos==vec2{19,27}&&static_cast<int>(c.h.kind())==0;}))return false;
if(std::none_of(s.groups[5].camps.begin(),s.groups[5].camps.end(),[](const Camp& c){return c.pos==vec2{20,27}&&static_cast<int>(c.h.kind())==3;}))return false;
if(std::none_of(s.groups[5].camps.begin(),s.groups[5].camps.end(),[](const Camp& c){return c.pos==vec2{19,26}&&static_cast<int>(c.h.kind())==2;}))return false;
if(s.groups[6].camps.size()!=3)return false;
if(std::none_of(s.groups[6].camps.begin(),s.groups[6].camps.end(),[](const Camp& c){return c.pos==vec2{24,18}&&static_cast<int>(c.h.kind())==0;}))return false;
if(std::none_of(s.groups[6].camps.begin(),s.groups[6].camps.end(),[](const Camp& c){return c.pos==vec2{24,19}&&static_cast<int>(c.h.kind())==2;}))return false;
if(std::none_of(s.groups[6].camps.begin(),s.groups[6].camps.end(),[](const Camp& c){return c.pos==vec2{25,18}&&static_cast<int>(c.h.kind())==2;}))return false;
if(s.groups[7].camps.size()!=3)return false;
if(std::none_of(s.groups[7].camps.begin(),s.groups[7].camps.end(),[](const Camp& c){return c.pos==vec2{24,22}&&static_cast<int>(c.h.kind())==0;}))return false;
if(std::none_of(s.groups[7].camps.begin(),s.groups[7].camps.end(),[](const Camp& c){return c.pos==vec2{24,21}&&static_cast<int>(c.h.kind())==2;}))return false;
if(std::none_of(s.groups[7].camps.begin(),s.groups[7].camps.end(),[](const Camp& c){return c.pos==vec2{25,22}&&static_cast<int>(c.h.kind())==2;}))return false;
if(s.groups[8].camps.size()!=2)return false;
if(std::none_of(s.groups[8].camps.begin(),s.groups[8].camps.end(),[](const Camp& c){return c.pos==vec2{12,7}&&static_cast<int>(c.h.kind())==3;}))return false;
if(std::none_of(s.groups[8].camps.begin(),s.groups[8].camps.end(),[](const Camp& c){return c.pos==vec2{12,8}&&static_cast<int>(c.h.kind())==3;}))return false;
if(s.groups[9].camps.size()!=2)return false;
if(std::none_of(s.groups[9].camps.begin(),s.groups[9].camps.end(),[](const Camp& c){return c.pos==vec2{12,33}&&static_cast<int>(c.h.kind())==3;}))return false;
if(std::none_of(s.groups[9].camps.begin(),s.groups[9].camps.end(),[](const Camp& c){return c.pos==vec2{12,32}&&static_cast<int>(c.h.kind())==3;}))return false;
if(s.groups[10].camps.size()!=2)return false;
if(std::none_of(s.groups[10].camps.begin(),s.groups[10].camps.end(),[](const Camp& c){return c.pos==vec2{17,11}&&static_cast<int>(c.h.kind())==3;}))return false;
if(std::none_of(s.groups[10].camps.begin(),s.groups[10].camps.end(),[](const Camp& c){return c.pos==vec2{17,10}&&static_cast<int>(c.h.kind())==3;}))return false;
if(s.groups[11].camps.size()!=2)return false;
if(std::none_of(s.groups[11].camps.begin(),s.groups[11].camps.end(),[](const Camp& c){return c.pos==vec2{17,29}&&static_cast<int>(c.h.kind())==3;}))return false;
if(std::none_of(s.groups[11].camps.begin(),s.groups[11].camps.end(),[](const Camp& c){return c.pos==vec2{17,30}&&static_cast<int>(c.h.kind())==3;}))return false;
if(s.groups[12].camps.size()!=2)return false;
if(std::none_of(s.groups[12].camps.begin(),s.groups[12].camps.end(),[](const Camp& c){return c.pos==vec2{22,16}&&static_cast<int>(c.h.kind())==3;}))return false;
if(std::none_of(s.groups[12].camps.begin(),s.groups[12].camps.end(),[](const Camp& c){return c.pos==vec2{22,15}&&static_cast<int>(c.h.kind())==3;}))return false;
if(s.groups[13].camps.size()!=2)return false;
if(std::none_of(s.groups[13].camps.begin(),s.groups[13].camps.end(),[](const Camp& c){return c.pos==vec2{22,24}&&static_cast<int>(c.h.kind())==3;}))return false;
if(std::none_of(s.groups[13].camps.begin(),s.groups[13].camps.end(),[](const Camp& c){return c.pos==vec2{22,25}&&static_cast<int>(c.h.kind())==3;}))return false;
if(s.groups[14].camps.size()!=2)return false;
if(std::none_of(s.groups[14].camps.begin(),s.groups[14].camps.end(),[](const Camp& c){return c.pos==vec2{21,3}&&static_cast<int>(c.h.kind())==1;}))return false;
if(std::none_of(s.groups[14].camps.begin(),s.groups[14].camps.end(),[](const Camp& c){return c.pos==vec2{22,3}&&static_cast<int>(c.h.kind())==3;}))return false;
if(s.groups[15].camps.size()!=2)return false;
if(std::none_of(s.groups[15].camps.begin(),s.groups[15].camps.end(),[](const Camp& c){return c.pos==vec2{21,37}&&static_cast<int>(c.h.kind())==1;}))return false;
if(std::none_of(s.groups[15].camps.begin(),s.groups[15].camps.end(),[](const Camp& c){return c.pos==vec2{22,37}&&static_cast<int>(c.h.kind())==3;}))return false;
if(s.groups[16].camps.size()!=3)return false;
if(std::none_of(s.groups[16].camps.begin(),s.groups[16].camps.end(),[](const Camp& c){return c.pos==vec2{29,3}&&static_cast<int>(c.h.kind())==1;}))return false;
if(std::none_of(s.groups[16].camps.begin(),s.groups[16].camps.end(),[](const Camp& c){return c.pos==vec2{30,3}&&static_cast<int>(c.h.kind())==3;}))return false;
if(std::none_of(s.groups[16].camps.begin(),s.groups[16].camps.end(),[](const Camp& c){return c.pos==vec2{28,3}&&static_cast<int>(c.h.kind())==2;}))return false;
if(s.groups[17].camps.size()!=3)return false;
if(std::none_of(s.groups[17].camps.begin(),s.groups[17].camps.end(),[](const Camp& c){return c.pos==vec2{29,37}&&static_cast<int>(c.h.kind())==1;}))return false;
if(std::none_of(s.groups[17].camps.begin(),s.groups[17].camps.end(),[](const Camp& c){return c.pos==vec2{30,37}&&static_cast<int>(c.h.kind())==3;}))return false;
if(std::none_of(s.groups[17].camps.begin(),s.groups[17].camps.end(),[](const Camp& c){return c.pos==vec2{28,37}&&static_cast<int>(c.h.kind())==2;}))return false;
if(s.groups[18].camps.size()!=6)return false;
if(std::none_of(s.groups[18].camps.begin(),s.groups[18].camps.end(),[](const Camp& c){return c.pos==vec2{37,3}&&static_cast<int>(c.h.kind())==1;}))return false;
if(std::none_of(s.groups[18].camps.begin(),s.groups[18].camps.end(),[](const Camp& c){return c.pos==vec2{37,4}&&static_cast<int>(c.h.kind())==1;}))return false;
if(std::none_of(s.groups[18].camps.begin(),s.groups[18].camps.end(),[](const Camp& c){return c.pos==vec2{37,2}&&static_cast<int>(c.h.kind())==1;}))return false;
if(std::none_of(s.groups[18].camps.begin(),s.groups[18].camps.end(),[](const Camp& c){return c.pos==vec2{36,2}&&static_cast<int>(c.h.kind())==2;}))return false;
if(std::none_of(s.groups[18].camps.begin(),s.groups[18].camps.end(),[](const Camp& c){return c.pos==vec2{36,4}&&static_cast<int>(c.h.kind())==2;}))return false;
if(std::none_of(s.groups[18].camps.begin(),s.groups[18].camps.end(),[](const Camp& c){return c.pos==vec2{36,3}&&static_cast<int>(c.h.kind())==2;}))return false;
if(s.groups[19].camps.size()!=6)return false;
if(std::none_of(s.groups[19].camps.begin(),s.groups[19].camps.end(),[](const Camp& c){return c.pos==vec2{37,37}&&static_cast<int>(c.h.kind())==1;}))return false;
if(std::none_of(s.groups[19].camps.begin(),s.groups[19].camps.end(),[](const Camp& c){return c.pos==vec2{37,36}&&static_cast<int>(c.h.kind())==1;}))return false;
if(std::none_of(s.groups[19].camps.begin(),s.groups[19].camps.end(),[](const Camp& c){return c.pos==vec2{37,38}&&static_cast<int>(c.h.kind())==1;}))return false;
if(std::none_of(s.groups[19].camps.begin(),s.groups[19].camps.end(),[](const Camp& c){return c.pos==vec2{36,38}&&static_cast<int>(c.h.kind())==2;}))return false;
if(std::none_of(s.groups[19].camps.begin(),s.groups[19].camps.end(),[](const Camp& c){return c.pos==vec2{36,36}&&static_cast<int>(c.h.kind())==2;}))return false;
if(std::none_of(s.groups[19].camps.begin(),s.groups[19].camps.end(),[](const Camp& c){return c.pos==vec2{36,37}&&static_cast<int>(c.h.kind())==2;}))return false;
return true;}();return yes;}
#endif
#if CXT2_INTERLOCK_BANK19
__attribute__((noinline)) bool cxt2_interlock(){static const bool yes=[](){const auto& s=*S;
if(s.W!=40||s.H!=40||s.altars.size()!=4||s.groups.size()!=14)return false;
if(std::find(s.altars.begin(),s.altars.end(),vec2{3,27})==s.altars.end())return false;
if(std::find(s.altars.begin(),s.altars.end(),vec2{19,16})==s.altars.end())return false;
if(std::find(s.altars.begin(),s.altars.end(),vec2{20,23})==s.altars.end())return false;
if(std::find(s.altars.begin(),s.altars.end(),vec2{36,12})==s.altars.end())return false;
if(s.groups[0].camps.size()!=4)return false;
if(std::none_of(s.groups[0].camps.begin(),s.groups[0].camps.end(),[](const Camp& c){return c.pos==vec2{28,3}&&static_cast<int>(c.h.kind())==1;}))return false;
if(std::none_of(s.groups[0].camps.begin(),s.groups[0].camps.end(),[](const Camp& c){return c.pos==vec2{29,3}&&static_cast<int>(c.h.kind())==1;}))return false;
if(std::none_of(s.groups[0].camps.begin(),s.groups[0].camps.end(),[](const Camp& c){return c.pos==vec2{29,2}&&static_cast<int>(c.h.kind())==2;}))return false;
if(std::none_of(s.groups[0].camps.begin(),s.groups[0].camps.end(),[](const Camp& c){return c.pos==vec2{29,4}&&static_cast<int>(c.h.kind())==3;}))return false;
if(s.groups[1].camps.size()!=4)return false;
if(std::none_of(s.groups[1].camps.begin(),s.groups[1].camps.end(),[](const Camp& c){return c.pos==vec2{11,36}&&static_cast<int>(c.h.kind())==1;}))return false;
if(std::none_of(s.groups[1].camps.begin(),s.groups[1].camps.end(),[](const Camp& c){return c.pos==vec2{10,36}&&static_cast<int>(c.h.kind())==1;}))return false;
if(std::none_of(s.groups[1].camps.begin(),s.groups[1].camps.end(),[](const Camp& c){return c.pos==vec2{10,37}&&static_cast<int>(c.h.kind())==2;}))return false;
if(std::none_of(s.groups[1].camps.begin(),s.groups[1].camps.end(),[](const Camp& c){return c.pos==vec2{10,35}&&static_cast<int>(c.h.kind())==3;}))return false;
if(s.groups[2].camps.size()!=4)return false;
if(std::none_of(s.groups[2].camps.begin(),s.groups[2].camps.end(),[](const Camp& c){return c.pos==vec2{31,10}&&static_cast<int>(c.h.kind())==1;}))return false;
if(std::none_of(s.groups[2].camps.begin(),s.groups[2].camps.end(),[](const Camp& c){return c.pos==vec2{32,10}&&static_cast<int>(c.h.kind())==1;}))return false;
if(std::none_of(s.groups[2].camps.begin(),s.groups[2].camps.end(),[](const Camp& c){return c.pos==vec2{32,11}&&static_cast<int>(c.h.kind())==2;}))return false;
if(std::none_of(s.groups[2].camps.begin(),s.groups[2].camps.end(),[](const Camp& c){return c.pos==vec2{32,9}&&static_cast<int>(c.h.kind())==2;}))return false;
if(s.groups[3].camps.size()!=4)return false;
if(std::none_of(s.groups[3].camps.begin(),s.groups[3].camps.end(),[](const Camp& c){return c.pos==vec2{8,29}&&static_cast<int>(c.h.kind())==1;}))return false;
if(std::none_of(s.groups[3].camps.begin(),s.groups[3].camps.end(),[](const Camp& c){return c.pos==vec2{7,29}&&static_cast<int>(c.h.kind())==1;}))return false;
if(std::none_of(s.groups[3].camps.begin(),s.groups[3].camps.end(),[](const Camp& c){return c.pos==vec2{7,28}&&static_cast<int>(c.h.kind())==2;}))return false;
if(std::none_of(s.groups[3].camps.begin(),s.groups[3].camps.end(),[](const Camp& c){return c.pos==vec2{7,30}&&static_cast<int>(c.h.kind())==2;}))return false;
if(s.groups[4].camps.size()!=2)return false;
if(std::none_of(s.groups[4].camps.begin(),s.groups[4].camps.end(),[](const Camp& c){return c.pos==vec2{5,21}&&static_cast<int>(c.h.kind())==0;}))return false;
if(std::none_of(s.groups[4].camps.begin(),s.groups[4].camps.end(),[](const Camp& c){return c.pos==vec2{4,21}&&static_cast<int>(c.h.kind())==3;}))return false;
if(s.groups[5].camps.size()!=2)return false;
if(std::none_of(s.groups[5].camps.begin(),s.groups[5].camps.end(),[](const Camp& c){return c.pos==vec2{34,18}&&static_cast<int>(c.h.kind())==0;}))return false;
if(std::none_of(s.groups[5].camps.begin(),s.groups[5].camps.end(),[](const Camp& c){return c.pos==vec2{35,18}&&static_cast<int>(c.h.kind())==3;}))return false;
if(s.groups[6].camps.size()!=2)return false;
if(std::none_of(s.groups[6].camps.begin(),s.groups[6].camps.end(),[](const Camp& c){return c.pos==vec2{19,2}&&static_cast<int>(c.h.kind())==3;}))return false;
if(std::none_of(s.groups[6].camps.begin(),s.groups[6].camps.end(),[](const Camp& c){return c.pos==vec2{19,3}&&static_cast<int>(c.h.kind())==3;}))return false;
if(s.groups[7].camps.size()!=2)return false;
if(std::none_of(s.groups[7].camps.begin(),s.groups[7].camps.end(),[](const Camp& c){return c.pos==vec2{20,37}&&static_cast<int>(c.h.kind())==3;}))return false;
if(std::none_of(s.groups[7].camps.begin(),s.groups[7].camps.end(),[](const Camp& c){return c.pos==vec2{20,36}&&static_cast<int>(c.h.kind())==3;}))return false;
if(s.groups[8].camps.size()!=2)return false;
if(std::none_of(s.groups[8].camps.begin(),s.groups[8].camps.end(),[](const Camp& c){return c.pos==vec2{36,2}&&static_cast<int>(c.h.kind())==3;}))return false;
if(std::none_of(s.groups[8].camps.begin(),s.groups[8].camps.end(),[](const Camp& c){return c.pos==vec2{36,3}&&static_cast<int>(c.h.kind())==3;}))return false;
if(s.groups[9].camps.size()!=2)return false;
if(std::none_of(s.groups[9].camps.begin(),s.groups[9].camps.end(),[](const Camp& c){return c.pos==vec2{3,37}&&static_cast<int>(c.h.kind())==3;}))return false;
if(std::none_of(s.groups[9].camps.begin(),s.groups[9].camps.end(),[](const Camp& c){return c.pos==vec2{3,36}&&static_cast<int>(c.h.kind())==3;}))return false;
if(s.groups[10].camps.size()!=1)return false;
if(std::none_of(s.groups[10].camps.begin(),s.groups[10].camps.end(),[](const Camp& c){return c.pos==vec2{5,8}&&static_cast<int>(c.h.kind())==0;}))return false;
if(s.groups[11].camps.size()!=1)return false;
if(std::none_of(s.groups[11].camps.begin(),s.groups[11].camps.end(),[](const Camp& c){return c.pos==vec2{34,31}&&static_cast<int>(c.h.kind())==0;}))return false;
if(s.groups[12].camps.size()!=2)return false;
if(std::none_of(s.groups[12].camps.begin(),s.groups[12].camps.end(),[](const Camp& c){return c.pos==vec2{5,14}&&static_cast<int>(c.h.kind())==0;}))return false;
if(std::none_of(s.groups[12].camps.begin(),s.groups[12].camps.end(),[](const Camp& c){return c.pos==vec2{4,14}&&static_cast<int>(c.h.kind())==2;}))return false;
if(s.groups[13].camps.size()!=2)return false;
if(std::none_of(s.groups[13].camps.begin(),s.groups[13].camps.end(),[](const Camp& c){return c.pos==vec2{34,25}&&static_cast<int>(c.h.kind())==0;}))return false;
if(std::none_of(s.groups[13].camps.begin(),s.groups[13].camps.end(),[](const Camp& c){return c.pos==vec2{35,25}&&static_cast<int>(c.h.kind())==2;}))return false;
return true;}();return yes;}
#endif
#if CXT2_COLLISION_BANK18 || CXT2_INTERLOCK_BANK19
__attribute__((noinline)) bool cxt2_engaged(vec2 pos){
    const auto& s=*S;if(s.target<0)return false;const auto& g=s.groups[s.target];
    if(g.st==1)return true;if(g.st!=0)return false;
    for(const auto& c:g.camps)if(c.hp>0 && cheb(pos,c.pos)<=s.range+1)return true;
    return false;
}
#endif

#if CXY_ELK_ISSAC
__attribute__((noinline)) bool cxy_exact_elk(){static const bool exact=[](){const auto& s=*S;
if(s.W!=30 || s.H!=30 || s.altars.size()!=2 || s.groups.size()!=10)return false;
if(std::find(s.altars.begin(),s.altars.end(),vec2{3,26})==s.altars.end())return false;
if(std::find(s.altars.begin(),s.altars.end(),vec2{26,3})==s.altars.end())return false;
if(s.groups[0].camps.size()!=3)return false;
if(s.groups[0].camps[0].pos!=vec2{3,6} || static_cast<int>(s.groups[0].camps[0].h.kind())!=1)return false;
if(s.groups[0].camps[1].pos!=vec2{3,7} || static_cast<int>(s.groups[0].camps[1].h.kind())!=3)return false;
if(s.groups[0].camps[2].pos!=vec2{4,6} || static_cast<int>(s.groups[0].camps[2].h.kind())!=2)return false;
if(s.groups[1].camps.size()!=3)return false;
if(s.groups[1].camps[0].pos!=vec2{26,23} || static_cast<int>(s.groups[1].camps[0].h.kind())!=1)return false;
if(s.groups[1].camps[1].pos!=vec2{26,22} || static_cast<int>(s.groups[1].camps[1].h.kind())!=3)return false;
if(s.groups[1].camps[2].pos!=vec2{25,23} || static_cast<int>(s.groups[1].camps[2].h.kind())!=2)return false;
if(s.groups[2].camps.size()!=3)return false;
if(s.groups[2].camps[0].pos!=vec2{9,2} || static_cast<int>(s.groups[2].camps[0].h.kind())!=1)return false;
if(s.groups[2].camps[1].pos!=vec2{10,2} || static_cast<int>(s.groups[2].camps[1].h.kind())!=3)return false;
if(s.groups[2].camps[2].pos!=vec2{9,3} || static_cast<int>(s.groups[2].camps[2].h.kind())!=2)return false;
if(s.groups[3].camps.size()!=3)return false;
if(s.groups[3].camps[0].pos!=vec2{20,27} || static_cast<int>(s.groups[3].camps[0].h.kind())!=1)return false;
if(s.groups[3].camps[1].pos!=vec2{19,27} || static_cast<int>(s.groups[3].camps[1].h.kind())!=3)return false;
if(s.groups[3].camps[2].pos!=vec2{20,26} || static_cast<int>(s.groups[3].camps[2].h.kind())!=2)return false;
if(s.groups[4].camps.size()!=3)return false;
if(s.groups[4].camps[0].pos!=vec2{3,12} || static_cast<int>(s.groups[4].camps[0].h.kind())!=1)return false;
if(s.groups[4].camps[1].pos!=vec2{3,13} || static_cast<int>(s.groups[4].camps[1].h.kind())!=3)return false;
if(s.groups[4].camps[2].pos!=vec2{3,11} || static_cast<int>(s.groups[4].camps[2].h.kind())!=2)return false;
if(s.groups[5].camps.size()!=3)return false;
if(s.groups[5].camps[0].pos!=vec2{26,17} || static_cast<int>(s.groups[5].camps[0].h.kind())!=1)return false;
if(s.groups[5].camps[1].pos!=vec2{26,16} || static_cast<int>(s.groups[5].camps[1].h.kind())!=3)return false;
if(s.groups[5].camps[2].pos!=vec2{26,18} || static_cast<int>(s.groups[5].camps[2].h.kind())!=2)return false;
if(s.groups[6].camps.size()!=3)return false;
if(s.groups[6].camps[0].pos!=vec2{19,2} || static_cast<int>(s.groups[6].camps[0].h.kind())!=1)return false;
if(s.groups[6].camps[1].pos!=vec2{20,2} || static_cast<int>(s.groups[6].camps[1].h.kind())!=3)return false;
if(s.groups[6].camps[2].pos!=vec2{20,3} || static_cast<int>(s.groups[6].camps[2].h.kind())!=2)return false;
if(s.groups[7].camps.size()!=3)return false;
if(s.groups[7].camps[0].pos!=vec2{10,27} || static_cast<int>(s.groups[7].camps[0].h.kind())!=1)return false;
if(s.groups[7].camps[1].pos!=vec2{9,27} || static_cast<int>(s.groups[7].camps[1].h.kind())!=3)return false;
if(s.groups[7].camps[2].pos!=vec2{9,26} || static_cast<int>(s.groups[7].camps[2].h.kind())!=2)return false;
if(s.groups[8].camps.size()!=3)return false;
if(s.groups[8].camps[0].pos!=vec2{26,9} || static_cast<int>(s.groups[8].camps[0].h.kind())!=1)return false;
if(s.groups[8].camps[1].pos!=vec2{26,10} || static_cast<int>(s.groups[8].camps[1].h.kind())!=3)return false;
if(s.groups[8].camps[2].pos!=vec2{26,8} || static_cast<int>(s.groups[8].camps[2].h.kind())!=2)return false;
if(s.groups[9].camps.size()!=3)return false;
if(s.groups[9].camps[0].pos!=vec2{3,20} || static_cast<int>(s.groups[9].camps[0].h.kind())!=1)return false;
if(s.groups[9].camps[1].pos!=vec2{3,19} || static_cast<int>(s.groups[9].camps[1].h.kind())!=3)return false;
if(s.groups[9].camps[2].pos!=vec2{3,21} || static_cast<int>(s.groups[9].camps[2].h.kind())!=2)return false;
return true;}();return exact;}
#endif
#if CXY_BOSS_DEFER
__attribute__((noinline)) bool cxy_exact_boss(){static const bool exact=[](){const auto& s=*S;
if(s.W!=50 || s.H!=41 || s.altars.size()!=1 || s.groups.size()!=31)return false;
if(std::find(s.altars.begin(),s.altars.end(),vec2{25,20})==s.altars.end())return false;
if(s.groups[0].camps.size()!=7)return false;
if(s.groups[0].camps[0].pos!=vec2{42,1} || static_cast<int>(s.groups[0].camps[0].h.kind())!=1)return false;
if(s.groups[0].camps[1].pos!=vec2{43,2} || static_cast<int>(s.groups[0].camps[1].h.kind())!=1)return false;
if(s.groups[0].camps[2].pos!=vec2{44,1} || static_cast<int>(s.groups[0].camps[2].h.kind())!=0)return false;
if(s.groups[0].camps[3].pos!=vec2{44,2} || static_cast<int>(s.groups[0].camps[3].h.kind())!=3)return false;
if(s.groups[0].camps[4].pos!=vec2{43,1} || static_cast<int>(s.groups[0].camps[4].h.kind())!=3)return false;
if(s.groups[0].camps[5].pos!=vec2{42,2} || static_cast<int>(s.groups[0].camps[5].h.kind())!=1)return false;
if(s.groups[0].camps[6].pos!=vec2{43,3} || static_cast<int>(s.groups[0].camps[6].h.kind())!=3)return false;
if(s.groups[1].camps.size()!=7)return false;
if(s.groups[1].camps[0].pos!=vec2{42,39} || static_cast<int>(s.groups[1].camps[0].h.kind())!=1)return false;
if(s.groups[1].camps[1].pos!=vec2{43,38} || static_cast<int>(s.groups[1].camps[1].h.kind())!=1)return false;
if(s.groups[1].camps[2].pos!=vec2{44,39} || static_cast<int>(s.groups[1].camps[2].h.kind())!=0)return false;
if(s.groups[1].camps[3].pos!=vec2{44,38} || static_cast<int>(s.groups[1].camps[3].h.kind())!=3)return false;
if(s.groups[1].camps[4].pos!=vec2{43,39} || static_cast<int>(s.groups[1].camps[4].h.kind())!=3)return false;
if(s.groups[1].camps[5].pos!=vec2{42,38} || static_cast<int>(s.groups[1].camps[5].h.kind())!=1)return false;
if(s.groups[1].camps[6].pos!=vec2{43,37} || static_cast<int>(s.groups[1].camps[6].h.kind())!=3)return false;
if(s.groups[2].camps.size()!=8)return false;
if(s.groups[2].camps[0].pos!=vec2{5,15} || static_cast<int>(s.groups[2].camps[0].h.kind())!=0)return false;
if(s.groups[2].camps[1].pos!=vec2{4,14} || static_cast<int>(s.groups[2].camps[1].h.kind())!=0)return false;
if(s.groups[2].camps[2].pos!=vec2{3,14} || static_cast<int>(s.groups[2].camps[2].h.kind())!=3)return false;
if(s.groups[2].camps[3].pos!=vec2{4,15} || static_cast<int>(s.groups[2].camps[3].h.kind())!=1)return false;
if(s.groups[2].camps[4].pos!=vec2{3,15} || static_cast<int>(s.groups[2].camps[4].h.kind())!=3)return false;
if(s.groups[2].camps[5].pos!=vec2{5,14} || static_cast<int>(s.groups[2].camps[5].h.kind())!=1)return false;
if(s.groups[2].camps[6].pos!=vec2{4,13} || static_cast<int>(s.groups[2].camps[6].h.kind())!=3)return false;
if(s.groups[2].camps[7].pos!=vec2{5,13} || static_cast<int>(s.groups[2].camps[7].h.kind())!=3)return false;
if(s.groups[3].camps.size()!=8)return false;
if(s.groups[3].camps[0].pos!=vec2{5,25} || static_cast<int>(s.groups[3].camps[0].h.kind())!=0)return false;
if(s.groups[3].camps[1].pos!=vec2{4,26} || static_cast<int>(s.groups[3].camps[1].h.kind())!=0)return false;
if(s.groups[3].camps[2].pos!=vec2{3,26} || static_cast<int>(s.groups[3].camps[2].h.kind())!=3)return false;
if(s.groups[3].camps[3].pos!=vec2{4,25} || static_cast<int>(s.groups[3].camps[3].h.kind())!=1)return false;
if(s.groups[3].camps[4].pos!=vec2{3,25} || static_cast<int>(s.groups[3].camps[4].h.kind())!=3)return false;
if(s.groups[3].camps[5].pos!=vec2{5,26} || static_cast<int>(s.groups[3].camps[5].h.kind())!=1)return false;
if(s.groups[3].camps[6].pos!=vec2{4,27} || static_cast<int>(s.groups[3].camps[6].h.kind())!=3)return false;
if(s.groups[3].camps[7].pos!=vec2{5,27} || static_cast<int>(s.groups[3].camps[7].h.kind())!=3)return false;
if(s.groups[4].camps.size()!=4)return false;
if(s.groups[4].camps[0].pos!=vec2{8,17} || static_cast<int>(s.groups[4].camps[0].h.kind())!=0)return false;
if(s.groups[4].camps[1].pos!=vec2{9,17} || static_cast<int>(s.groups[4].camps[1].h.kind())!=0)return false;
if(s.groups[4].camps[2].pos!=vec2{8,18} || static_cast<int>(s.groups[4].camps[2].h.kind())!=3)return false;
if(s.groups[4].camps[3].pos!=vec2{9,18} || static_cast<int>(s.groups[4].camps[3].h.kind())!=3)return false;
if(s.groups[5].camps.size()!=4)return false;
if(s.groups[5].camps[0].pos!=vec2{8,23} || static_cast<int>(s.groups[5].camps[0].h.kind())!=0)return false;
if(s.groups[5].camps[1].pos!=vec2{9,23} || static_cast<int>(s.groups[5].camps[1].h.kind())!=0)return false;
if(s.groups[5].camps[2].pos!=vec2{8,22} || static_cast<int>(s.groups[5].camps[2].h.kind())!=3)return false;
if(s.groups[5].camps[3].pos!=vec2{9,22} || static_cast<int>(s.groups[5].camps[3].h.kind())!=3)return false;
if(s.groups[6].camps.size()!=4)return false;
if(s.groups[6].camps[0].pos!=vec2{46,6} || static_cast<int>(s.groups[6].camps[0].h.kind())!=0)return false;
if(s.groups[6].camps[1].pos!=vec2{47,6} || static_cast<int>(s.groups[6].camps[1].h.kind())!=1)return false;
if(s.groups[6].camps[2].pos!=vec2{46,7} || static_cast<int>(s.groups[6].camps[2].h.kind())!=3)return false;
if(s.groups[6].camps[3].pos!=vec2{47,7} || static_cast<int>(s.groups[6].camps[3].h.kind())!=3)return false;
if(s.groups[7].camps.size()!=4)return false;
if(s.groups[7].camps[0].pos!=vec2{46,34} || static_cast<int>(s.groups[7].camps[0].h.kind())!=0)return false;
if(s.groups[7].camps[1].pos!=vec2{47,34} || static_cast<int>(s.groups[7].camps[1].h.kind())!=1)return false;
if(s.groups[7].camps[2].pos!=vec2{46,33} || static_cast<int>(s.groups[7].camps[2].h.kind())!=3)return false;
if(s.groups[7].camps[3].pos!=vec2{47,33} || static_cast<int>(s.groups[7].camps[3].h.kind())!=3)return false;
if(s.groups[8].camps.size()!=4)return false;
if(s.groups[8].camps[0].pos!=vec2{28,13} || static_cast<int>(s.groups[8].camps[0].h.kind())!=1)return false;
if(s.groups[8].camps[1].pos!=vec2{28,12} || static_cast<int>(s.groups[8].camps[1].h.kind())!=3)return false;
if(s.groups[8].camps[2].pos!=vec2{29,12} || static_cast<int>(s.groups[8].camps[2].h.kind())!=1)return false;
if(s.groups[8].camps[3].pos!=vec2{29,13} || static_cast<int>(s.groups[8].camps[3].h.kind())!=3)return false;
if(s.groups[9].camps.size()!=4)return false;
if(s.groups[9].camps[0].pos!=vec2{28,27} || static_cast<int>(s.groups[9].camps[0].h.kind())!=1)return false;
if(s.groups[9].camps[1].pos!=vec2{28,28} || static_cast<int>(s.groups[9].camps[1].h.kind())!=3)return false;
if(s.groups[9].camps[2].pos!=vec2{29,28} || static_cast<int>(s.groups[9].camps[2].h.kind())!=1)return false;
if(s.groups[9].camps[3].pos!=vec2{29,27} || static_cast<int>(s.groups[9].camps[3].h.kind())!=3)return false;
if(s.groups[10].camps.size()!=7)return false;
if(s.groups[10].camps[0].pos!=vec2{8,1} || static_cast<int>(s.groups[10].camps[0].h.kind())!=0)return false;
if(s.groups[10].camps[1].pos!=vec2{8,0} || static_cast<int>(s.groups[10].camps[1].h.kind())!=3)return false;
if(s.groups[10].camps[2].pos!=vec2{7,1} || static_cast<int>(s.groups[10].camps[2].h.kind())!=1)return false;
if(s.groups[10].camps[3].pos!=vec2{7,0} || static_cast<int>(s.groups[10].camps[3].h.kind())!=1)return false;
if(s.groups[10].camps[4].pos!=vec2{9,1} || static_cast<int>(s.groups[10].camps[4].h.kind())!=1)return false;
if(s.groups[10].camps[5].pos!=vec2{8,2} || static_cast<int>(s.groups[10].camps[5].h.kind())!=3)return false;
if(s.groups[10].camps[6].pos!=vec2{9,2} || static_cast<int>(s.groups[10].camps[6].h.kind())!=3)return false;
if(s.groups[11].camps.size()!=7)return false;
if(s.groups[11].camps[0].pos!=vec2{8,39} || static_cast<int>(s.groups[11].camps[0].h.kind())!=0)return false;
if(s.groups[11].camps[1].pos!=vec2{8,40} || static_cast<int>(s.groups[11].camps[1].h.kind())!=3)return false;
if(s.groups[11].camps[2].pos!=vec2{7,39} || static_cast<int>(s.groups[11].camps[2].h.kind())!=1)return false;
if(s.groups[11].camps[3].pos!=vec2{7,40} || static_cast<int>(s.groups[11].camps[3].h.kind())!=1)return false;
if(s.groups[11].camps[4].pos!=vec2{9,39} || static_cast<int>(s.groups[11].camps[4].h.kind())!=1)return false;
if(s.groups[11].camps[5].pos!=vec2{8,38} || static_cast<int>(s.groups[11].camps[5].h.kind())!=3)return false;
if(s.groups[11].camps[6].pos!=vec2{9,38} || static_cast<int>(s.groups[11].camps[6].h.kind())!=3)return false;
if(s.groups[12].camps.size()!=3)return false;
if(s.groups[12].camps[0].pos!=vec2{45,17} || static_cast<int>(s.groups[12].camps[0].h.kind())!=3)return false;
if(s.groups[12].camps[1].pos!=vec2{44,17} || static_cast<int>(s.groups[12].camps[1].h.kind())!=3)return false;
if(s.groups[12].camps[2].pos!=vec2{44,18} || static_cast<int>(s.groups[12].camps[2].h.kind())!=3)return false;
if(s.groups[13].camps.size()!=3)return false;
if(s.groups[13].camps[0].pos!=vec2{45,23} || static_cast<int>(s.groups[13].camps[0].h.kind())!=3)return false;
if(s.groups[13].camps[1].pos!=vec2{44,23} || static_cast<int>(s.groups[13].camps[1].h.kind())!=3)return false;
if(s.groups[13].camps[2].pos!=vec2{44,22} || static_cast<int>(s.groups[13].camps[2].h.kind())!=3)return false;
if(s.groups[14].camps.size()!=5)return false;
if(s.groups[14].camps[0].pos!=vec2{8,6} || static_cast<int>(s.groups[14].camps[0].h.kind())!=0)return false;
if(s.groups[14].camps[1].pos!=vec2{7,6} || static_cast<int>(s.groups[14].camps[1].h.kind())!=1)return false;
if(s.groups[14].camps[2].pos!=vec2{7,7} || static_cast<int>(s.groups[14].camps[2].h.kind())!=1)return false;
if(s.groups[14].camps[3].pos!=vec2{8,7} || static_cast<int>(s.groups[14].camps[3].h.kind())!=3)return false;
if(s.groups[14].camps[4].pos!=vec2{9,6} || static_cast<int>(s.groups[14].camps[4].h.kind())!=3)return false;
if(s.groups[15].camps.size()!=5)return false;
if(s.groups[15].camps[0].pos!=vec2{8,34} || static_cast<int>(s.groups[15].camps[0].h.kind())!=0)return false;
if(s.groups[15].camps[1].pos!=vec2{7,34} || static_cast<int>(s.groups[15].camps[1].h.kind())!=1)return false;
if(s.groups[15].camps[2].pos!=vec2{7,33} || static_cast<int>(s.groups[15].camps[2].h.kind())!=1)return false;
if(s.groups[15].camps[3].pos!=vec2{8,33} || static_cast<int>(s.groups[15].camps[3].h.kind())!=3)return false;
if(s.groups[15].camps[4].pos!=vec2{9,34} || static_cast<int>(s.groups[15].camps[4].h.kind())!=3)return false;
if(s.groups[16].camps.size()!=2)return false;
if(s.groups[16].camps[0].pos!=vec2{24,20} || static_cast<int>(s.groups[16].camps[0].h.kind())!=4)return false;
if(s.groups[16].camps[1].pos!=vec2{23,20} || static_cast<int>(s.groups[16].camps[1].h.kind())!=4)return false;
if(s.groups[17].camps.size()!=4)return false;
if(s.groups[17].camps[0].pos!=vec2{27,17} || static_cast<int>(s.groups[17].camps[0].h.kind())!=3)return false;
if(s.groups[17].camps[1].pos!=vec2{28,16} || static_cast<int>(s.groups[17].camps[1].h.kind())!=1)return false;
if(s.groups[17].camps[2].pos!=vec2{28,17} || static_cast<int>(s.groups[17].camps[2].h.kind())!=1)return false;
if(s.groups[17].camps[3].pos!=vec2{29,16} || static_cast<int>(s.groups[17].camps[3].h.kind())!=3)return false;
if(s.groups[18].camps.size()!=4)return false;
if(s.groups[18].camps[0].pos!=vec2{27,23} || static_cast<int>(s.groups[18].camps[0].h.kind())!=3)return false;
if(s.groups[18].camps[1].pos!=vec2{28,24} || static_cast<int>(s.groups[18].camps[1].h.kind())!=1)return false;
if(s.groups[18].camps[2].pos!=vec2{28,23} || static_cast<int>(s.groups[18].camps[2].h.kind())!=1)return false;
if(s.groups[18].camps[3].pos!=vec2{29,24} || static_cast<int>(s.groups[18].camps[3].h.kind())!=3)return false;
if(s.groups[19].camps.size()!=10)return false;
if(s.groups[19].camps[0].pos!=vec2{49,15} || static_cast<int>(s.groups[19].camps[0].h.kind())!=0)return false;
if(s.groups[19].camps[1].pos!=vec2{48,15} || static_cast<int>(s.groups[19].camps[1].h.kind())!=0)return false;
if(s.groups[19].camps[2].pos!=vec2{47,15} || static_cast<int>(s.groups[19].camps[2].h.kind())!=1)return false;
if(s.groups[19].camps[3].pos!=vec2{48,16} || static_cast<int>(s.groups[19].camps[3].h.kind())!=0)return false;
if(s.groups[19].camps[4].pos!=vec2{47,16} || static_cast<int>(s.groups[19].camps[4].h.kind())!=0)return false;
if(s.groups[19].camps[5].pos!=vec2{47,14} || static_cast<int>(s.groups[19].camps[5].h.kind())!=3)return false;
if(s.groups[19].camps[6].pos!=vec2{48,14} || static_cast<int>(s.groups[19].camps[6].h.kind())!=3)return false;
if(s.groups[19].camps[7].pos!=vec2{46,15} || static_cast<int>(s.groups[19].camps[7].h.kind())!=3)return false;
if(s.groups[19].camps[8].pos!=vec2{48,17} || static_cast<int>(s.groups[19].camps[8].h.kind())!=3)return false;
if(s.groups[19].camps[9].pos!=vec2{49,16} || static_cast<int>(s.groups[19].camps[9].h.kind())!=3)return false;
if(s.groups[20].camps.size()!=10)return false;
if(s.groups[20].camps[0].pos!=vec2{49,25} || static_cast<int>(s.groups[20].camps[0].h.kind())!=0)return false;
if(s.groups[20].camps[1].pos!=vec2{48,25} || static_cast<int>(s.groups[20].camps[1].h.kind())!=0)return false;
if(s.groups[20].camps[2].pos!=vec2{47,25} || static_cast<int>(s.groups[20].camps[2].h.kind())!=1)return false;
if(s.groups[20].camps[3].pos!=vec2{48,24} || static_cast<int>(s.groups[20].camps[3].h.kind())!=0)return false;
if(s.groups[20].camps[4].pos!=vec2{47,24} || static_cast<int>(s.groups[20].camps[4].h.kind())!=0)return false;
if(s.groups[20].camps[5].pos!=vec2{47,26} || static_cast<int>(s.groups[20].camps[5].h.kind())!=3)return false;
if(s.groups[20].camps[6].pos!=vec2{48,26} || static_cast<int>(s.groups[20].camps[6].h.kind())!=3)return false;
if(s.groups[20].camps[7].pos!=vec2{46,25} || static_cast<int>(s.groups[20].camps[7].h.kind())!=3)return false;
if(s.groups[20].camps[8].pos!=vec2{48,23} || static_cast<int>(s.groups[20].camps[8].h.kind())!=3)return false;
if(s.groups[20].camps[9].pos!=vec2{49,24} || static_cast<int>(s.groups[20].camps[9].h.kind())!=3)return false;
if(s.groups[21].camps.size()!=4)return false;
if(s.groups[21].camps[0].pos!=vec2{16,12} || static_cast<int>(s.groups[21].camps[0].h.kind())!=3)return false;
if(s.groups[21].camps[1].pos!=vec2{17,12} || static_cast<int>(s.groups[21].camps[1].h.kind())!=1)return false;
if(s.groups[21].camps[2].pos!=vec2{17,13} || static_cast<int>(s.groups[21].camps[2].h.kind())!=0)return false;
if(s.groups[21].camps[3].pos!=vec2{16,13} || static_cast<int>(s.groups[21].camps[3].h.kind())!=3)return false;
if(s.groups[22].camps.size()!=4)return false;
if(s.groups[22].camps[0].pos!=vec2{16,28} || static_cast<int>(s.groups[22].camps[0].h.kind())!=3)return false;
if(s.groups[22].camps[1].pos!=vec2{17,28} || static_cast<int>(s.groups[22].camps[1].h.kind())!=1)return false;
if(s.groups[22].camps[2].pos!=vec2{17,27} || static_cast<int>(s.groups[22].camps[2].h.kind())!=0)return false;
if(s.groups[22].camps[3].pos!=vec2{16,27} || static_cast<int>(s.groups[22].camps[3].h.kind())!=3)return false;
if(s.groups[23].camps.size()!=3)return false;
if(s.groups[23].camps[0].pos!=vec2{37,15} || static_cast<int>(s.groups[23].camps[0].h.kind())!=1)return false;
if(s.groups[23].camps[1].pos!=vec2{36,15} || static_cast<int>(s.groups[23].camps[1].h.kind())!=3)return false;
if(s.groups[23].camps[2].pos!=vec2{37,16} || static_cast<int>(s.groups[23].camps[2].h.kind())!=3)return false;
if(s.groups[24].camps.size()!=3)return false;
if(s.groups[24].camps[0].pos!=vec2{37,25} || static_cast<int>(s.groups[24].camps[0].h.kind())!=1)return false;
if(s.groups[24].camps[1].pos!=vec2{36,25} || static_cast<int>(s.groups[24].camps[1].h.kind())!=3)return false;
if(s.groups[24].camps[2].pos!=vec2{37,24} || static_cast<int>(s.groups[24].camps[2].h.kind())!=3)return false;
if(s.groups[25].camps.size()!=2)return false;
if(s.groups[25].camps[0].pos!=vec2{0,3} || static_cast<int>(s.groups[25].camps[0].h.kind())!=3)return false;
if(s.groups[25].camps[1].pos!=vec2{0,2} || static_cast<int>(s.groups[25].camps[1].h.kind())!=3)return false;
if(s.groups[26].camps.size()!=2)return false;
if(s.groups[26].camps[0].pos!=vec2{0,37} || static_cast<int>(s.groups[26].camps[0].h.kind())!=3)return false;
if(s.groups[26].camps[1].pos!=vec2{0,38} || static_cast<int>(s.groups[26].camps[1].h.kind())!=3)return false;
if(s.groups[27].camps.size()!=7)return false;
if(s.groups[27].camps[0].pos!=vec2{37,0} || static_cast<int>(s.groups[27].camps[0].h.kind())!=1)return false;
if(s.groups[27].camps[1].pos!=vec2{36,0} || static_cast<int>(s.groups[27].camps[1].h.kind())!=1)return false;
if(s.groups[27].camps[2].pos!=vec2{38,0} || static_cast<int>(s.groups[27].camps[2].h.kind())!=1)return false;
if(s.groups[27].camps[3].pos!=vec2{36,1} || static_cast<int>(s.groups[27].camps[3].h.kind())!=3)return false;
if(s.groups[27].camps[4].pos!=vec2{37,1} || static_cast<int>(s.groups[27].camps[4].h.kind())!=3)return false;
if(s.groups[27].camps[5].pos!=vec2{38,1} || static_cast<int>(s.groups[27].camps[5].h.kind())!=3)return false;
if(s.groups[27].camps[6].pos!=vec2{37,2} || static_cast<int>(s.groups[27].camps[6].h.kind())!=3)return false;
if(s.groups[28].camps.size()!=7)return false;
if(s.groups[28].camps[0].pos!=vec2{37,40} || static_cast<int>(s.groups[28].camps[0].h.kind())!=1)return false;
if(s.groups[28].camps[1].pos!=vec2{36,40} || static_cast<int>(s.groups[28].camps[1].h.kind())!=1)return false;
if(s.groups[28].camps[2].pos!=vec2{38,40} || static_cast<int>(s.groups[28].camps[2].h.kind())!=1)return false;
if(s.groups[28].camps[3].pos!=vec2{36,39} || static_cast<int>(s.groups[28].camps[3].h.kind())!=3)return false;
if(s.groups[28].camps[4].pos!=vec2{37,39} || static_cast<int>(s.groups[28].camps[4].h.kind())!=3)return false;
if(s.groups[28].camps[5].pos!=vec2{38,39} || static_cast<int>(s.groups[28].camps[5].h.kind())!=3)return false;
if(s.groups[28].camps[6].pos!=vec2{37,38} || static_cast<int>(s.groups[28].camps[6].h.kind())!=3)return false;
if(s.groups[29].camps.size()!=6)return false;
if(s.groups[29].camps[0].pos!=vec2{38,9} || static_cast<int>(s.groups[29].camps[0].h.kind())!=0)return false;
if(s.groups[29].camps[1].pos!=vec2{38,8} || static_cast<int>(s.groups[29].camps[1].h.kind())!=0)return false;
if(s.groups[29].camps[2].pos!=vec2{39,8} || static_cast<int>(s.groups[29].camps[2].h.kind())!=1)return false;
if(s.groups[29].camps[3].pos!=vec2{39,9} || static_cast<int>(s.groups[29].camps[3].h.kind())!=3)return false;
if(s.groups[29].camps[4].pos!=vec2{38,10} || static_cast<int>(s.groups[29].camps[4].h.kind())!=3)return false;
if(s.groups[29].camps[5].pos!=vec2{37,9} || static_cast<int>(s.groups[29].camps[5].h.kind())!=3)return false;
if(s.groups[30].camps.size()!=6)return false;
if(s.groups[30].camps[0].pos!=vec2{38,31} || static_cast<int>(s.groups[30].camps[0].h.kind())!=0)return false;
if(s.groups[30].camps[1].pos!=vec2{38,32} || static_cast<int>(s.groups[30].camps[1].h.kind())!=0)return false;
if(s.groups[30].camps[2].pos!=vec2{39,32} || static_cast<int>(s.groups[30].camps[2].h.kind())!=1)return false;
if(s.groups[30].camps[3].pos!=vec2{39,31} || static_cast<int>(s.groups[30].camps[3].h.kind())!=3)return false;
if(s.groups[30].camps[4].pos!=vec2{38,30} || static_cast<int>(s.groups[30].camps[4].h.kind())!=3)return false;
if(s.groups[30].camps[5].pos!=vec2{37,31} || static_cast<int>(s.groups[30].camps[5].h.kind())!=3)return false;
return true;}();return exact;}
#endif


#if CXW_CONTEST_GOLD
// Lazily initialized only inside the existing g_nm==1 branch. No other map pays for this check.
__attribute__((noinline)) bool cxw_exact_contest() {
    static const bool exact = [] {
        const auto& s = *S;
        if (s.W != 50 || s.H != 50 || s.altars.size() != 2 || s.groups.size() != 14) return false;
        if (std::find(s.altars.begin(), s.altars.end(), vec2{24,25}) == s.altars.end()
            || std::find(s.altars.begin(), s.altars.end(), vec2{25,24}) == s.altars.end()) return false;
        if (s.groups[0].camps.size() != 1 || s.groups[0].gold != 300 || s.groups[0].gems != 200) return false;
        if (std::none_of(s.groups[0].camps.begin(), s.groups[0].camps.end(), [](const Camp& c) { return c.pos.x == 17 && c.pos.y == 31 && static_cast<int>(c.h.kind()) == 4; })) return false;
        if (s.groups[1].camps.size() != 1 || s.groups[1].gold != 300 || s.groups[1].gems != 200) return false;
        if (std::none_of(s.groups[1].camps.begin(), s.groups[1].camps.end(), [](const Camp& c) { return c.pos.x == 32 && c.pos.y == 18 && static_cast<int>(c.h.kind()) == 4; })) return false;
        if (s.groups[2].camps.size() != 4 || s.groups[2].gold != 80 || s.groups[2].gems != 0) return false;
        if (std::none_of(s.groups[2].camps.begin(), s.groups[2].camps.end(), [](const Camp& c) { return c.pos.x == 22 && c.pos.y == 31 && static_cast<int>(c.h.kind()) == 3; })) return false;
        if (std::none_of(s.groups[2].camps.begin(), s.groups[2].camps.end(), [](const Camp& c) { return c.pos.x == 22 && c.pos.y == 32 && static_cast<int>(c.h.kind()) == 3; })) return false;
        if (std::none_of(s.groups[2].camps.begin(), s.groups[2].camps.end(), [](const Camp& c) { return c.pos.x == 22 && c.pos.y == 30 && static_cast<int>(c.h.kind()) == 3; })) return false;
        if (std::none_of(s.groups[2].camps.begin(), s.groups[2].camps.end(), [](const Camp& c) { return c.pos.x == 23 && c.pos.y == 31 && static_cast<int>(c.h.kind()) == 3; })) return false;
        if (s.groups[3].camps.size() != 4 || s.groups[3].gold != 80 || s.groups[3].gems != 0) return false;
        if (std::none_of(s.groups[3].camps.begin(), s.groups[3].camps.end(), [](const Camp& c) { return c.pos.x == 27 && c.pos.y == 18 && static_cast<int>(c.h.kind()) == 3; })) return false;
        if (std::none_of(s.groups[3].camps.begin(), s.groups[3].camps.end(), [](const Camp& c) { return c.pos.x == 27 && c.pos.y == 17 && static_cast<int>(c.h.kind()) == 3; })) return false;
        if (std::none_of(s.groups[3].camps.begin(), s.groups[3].camps.end(), [](const Camp& c) { return c.pos.x == 27 && c.pos.y == 19 && static_cast<int>(c.h.kind()) == 3; })) return false;
        if (std::none_of(s.groups[3].camps.begin(), s.groups[3].camps.end(), [](const Camp& c) { return c.pos.x == 26 && c.pos.y == 18 && static_cast<int>(c.h.kind()) == 3; })) return false;
        if (s.groups[4].camps.size() != 4 || s.groups[4].gold != 80 || s.groups[4].gems != 0) return false;
        if (std::none_of(s.groups[4].camps.begin(), s.groups[4].camps.end(), [](const Camp& c) { return c.pos.x == 18 && c.pos.y == 27 && static_cast<int>(c.h.kind()) == 3; })) return false;
        if (std::none_of(s.groups[4].camps.begin(), s.groups[4].camps.end(), [](const Camp& c) { return c.pos.x == 17 && c.pos.y == 27 && static_cast<int>(c.h.kind()) == 3; })) return false;
        if (std::none_of(s.groups[4].camps.begin(), s.groups[4].camps.end(), [](const Camp& c) { return c.pos.x == 19 && c.pos.y == 27 && static_cast<int>(c.h.kind()) == 3; })) return false;
        if (std::none_of(s.groups[4].camps.begin(), s.groups[4].camps.end(), [](const Camp& c) { return c.pos.x == 18 && c.pos.y == 26 && static_cast<int>(c.h.kind()) == 3; })) return false;
        if (s.groups[5].camps.size() != 4 || s.groups[5].gold != 80 || s.groups[5].gems != 0) return false;
        if (std::none_of(s.groups[5].camps.begin(), s.groups[5].camps.end(), [](const Camp& c) { return c.pos.x == 31 && c.pos.y == 22 && static_cast<int>(c.h.kind()) == 3; })) return false;
        if (std::none_of(s.groups[5].camps.begin(), s.groups[5].camps.end(), [](const Camp& c) { return c.pos.x == 32 && c.pos.y == 22 && static_cast<int>(c.h.kind()) == 3; })) return false;
        if (std::none_of(s.groups[5].camps.begin(), s.groups[5].camps.end(), [](const Camp& c) { return c.pos.x == 30 && c.pos.y == 22 && static_cast<int>(c.h.kind()) == 3; })) return false;
        if (std::none_of(s.groups[5].camps.begin(), s.groups[5].camps.end(), [](const Camp& c) { return c.pos.x == 31 && c.pos.y == 23 && static_cast<int>(c.h.kind()) == 3; })) return false;
        if (s.groups[6].camps.size() != 2 || s.groups[6].gold != 40 || s.groups[6].gems != 0) return false;
        if (std::none_of(s.groups[6].camps.begin(), s.groups[6].camps.end(), [](const Camp& c) { return c.pos.x == 19 && c.pos.y == 3 && static_cast<int>(c.h.kind()) == 3; })) return false;
        if (std::none_of(s.groups[6].camps.begin(), s.groups[6].camps.end(), [](const Camp& c) { return c.pos.x == 19 && c.pos.y == 4 && static_cast<int>(c.h.kind()) == 3; })) return false;
        if (s.groups[7].camps.size() != 2 || s.groups[7].gold != 40 || s.groups[7].gems != 0) return false;
        if (std::none_of(s.groups[7].camps.begin(), s.groups[7].camps.end(), [](const Camp& c) { return c.pos.x == 30 && c.pos.y == 46 && static_cast<int>(c.h.kind()) == 3; })) return false;
        if (std::none_of(s.groups[7].camps.begin(), s.groups[7].camps.end(), [](const Camp& c) { return c.pos.x == 30 && c.pos.y == 45 && static_cast<int>(c.h.kind()) == 3; })) return false;
        if (s.groups[8].camps.size() != 2 || s.groups[8].gold != 40 || s.groups[8].gems != 0) return false;
        if (std::none_of(s.groups[8].camps.begin(), s.groups[8].camps.end(), [](const Camp& c) { return c.pos.x == 5 && c.pos.y == 33 && static_cast<int>(c.h.kind()) == 3; })) return false;
        if (std::none_of(s.groups[8].camps.begin(), s.groups[8].camps.end(), [](const Camp& c) { return c.pos.x == 5 && c.pos.y == 34 && static_cast<int>(c.h.kind()) == 3; })) return false;
        if (s.groups[9].camps.size() != 2 || s.groups[9].gold != 40 || s.groups[9].gems != 0) return false;
        if (std::none_of(s.groups[9].camps.begin(), s.groups[9].camps.end(), [](const Camp& c) { return c.pos.x == 44 && c.pos.y == 16 && static_cast<int>(c.h.kind()) == 3; })) return false;
        if (std::none_of(s.groups[9].camps.begin(), s.groups[9].camps.end(), [](const Camp& c) { return c.pos.x == 44 && c.pos.y == 15 && static_cast<int>(c.h.kind()) == 3; })) return false;
        if (s.groups[10].camps.size() != 2 || s.groups[10].gold != 40 || s.groups[10].gems != 0) return false;
        if (std::none_of(s.groups[10].camps.begin(), s.groups[10].camps.end(), [](const Camp& c) { return c.pos.x == 36 && c.pos.y == 9 && static_cast<int>(c.h.kind()) == 3; })) return false;
        if (std::none_of(s.groups[10].camps.begin(), s.groups[10].camps.end(), [](const Camp& c) { return c.pos.x == 37 && c.pos.y == 9 && static_cast<int>(c.h.kind()) == 3; })) return false;
        if (s.groups[11].camps.size() != 2 || s.groups[11].gold != 40 || s.groups[11].gems != 0) return false;
        if (std::none_of(s.groups[11].camps.begin(), s.groups[11].camps.end(), [](const Camp& c) { return c.pos.x == 13 && c.pos.y == 40 && static_cast<int>(c.h.kind()) == 3; })) return false;
        if (std::none_of(s.groups[11].camps.begin(), s.groups[11].camps.end(), [](const Camp& c) { return c.pos.x == 12 && c.pos.y == 40 && static_cast<int>(c.h.kind()) == 3; })) return false;
        if (s.groups[12].camps.size() != 2 || s.groups[12].gold != 40 || s.groups[12].gems != 0) return false;
        if (std::none_of(s.groups[12].camps.begin(), s.groups[12].camps.end(), [](const Camp& c) { return c.pos.x == 5 && c.pos.y == 18 && static_cast<int>(c.h.kind()) == 3; })) return false;
        if (std::none_of(s.groups[12].camps.begin(), s.groups[12].camps.end(), [](const Camp& c) { return c.pos.x == 6 && c.pos.y == 18 && static_cast<int>(c.h.kind()) == 3; })) return false;
        if (s.groups[13].camps.size() != 2 || s.groups[13].gold != 40 || s.groups[13].gems != 0) return false;
        if (std::none_of(s.groups[13].camps.begin(), s.groups[13].camps.end(), [](const Camp& c) { return c.pos.x == 44 && c.pos.y == 31 && static_cast<int>(c.h.kind()) == 3; })) return false;
        if (std::none_of(s.groups[13].camps.begin(), s.groups[13].camps.end(), [](const Camp& c) { return c.pos.x == 43 && c.pos.y == 31 && static_cast<int>(c.h.kind()) == 3; })) return false;
        return true;
    }();
    return exact;
}
#endif

// fx2 (16 Oct, fragment only, g_nm == 3): second-stage fixes after v305's pack weight. Decoded post-fix losses
// (meow, Gem Simons, BrendanBurgess; scratchpad/fx2_agent/notes.md): the pack forms, but walks into the first
// contest at R37-58 with 0 items (enemy 3-4), sits on the 125-gold camp reward below P_SHOP, and loses
// 3-for-0 fights near the centre elks and the north/south altars.
#ifndef FX_SHOP
#define FX_SHOP 0 // fragment: recall to shop at this much gold (P_SHOP elsewhere)
#endif
#ifndef FX_SHOPU
#define FX_SHOPU 100000 // FX_SHOP applies before this round only
#endif
#ifndef FX_RHP
#define FX_RHP 0.0f // fragment: recall below this hp fraction (P_RECALL_HP elsewhere)
#endif
#ifndef FX_FARU
#define FX_FARU 0 // fragment: before this round skip enemy-side groups (FX_FARE 2: also the far centre elk)
#endif
#ifndef FX_FARE
#define FX_FARE 1
#endif
#ifndef FX_DR
#define FX_DR 0 // fragment draft: 1 = C&I, Stellar, Ren
#endif
#ifndef FX_FUNI
#define FX_FUNI 0 // fragment: Issac also leaves claimed rewards (and their gold) to Ryth/Ren
#endif
#ifndef FX_FUNIK
#define FX_FUNIK 0 // FX_FUNI only once Issac owns at least this many items (0 = always)
#endif
#ifndef FX_CAS
#define FX_CAS 0 // fragment: Cassi captures rewards too (1) and buys Durability health first (2: health only first)
#endif
unsigned long long g_fxfar = 0;
#if SH_ON && defined(SH_MAPS)
static bool g_shpk = false;  // sh on packs (map id 4) without the PK plan: s.pk is set to reach the dispatch at zero cost elsewhere
#endif
// Uses only data init() already holds (no extra camp_groups() fuel). Evaluated lazily at the default target
// dispatch (not in init) so the fuel-tight first turn on corners keeps its exact instruction count.
int nm_map_id(int W, int H, const std::vector<vec2>& alt, std::size_t ng) {
    if (g_nm >= 0) return g_nm;
    g_nm = 0;
    if (W != 50 && W != 60) return g_nm;
    auto has_alt = [&](int x, int y) { for (auto a : alt) if (a.x == x && a.y == y) return true; return false; };
    if (W == 50 && H == 50 && alt.size() == 2 && has_alt(24, 25) && has_alt(25, 24) && ng == 14) g_nm = 1;
    else if (W == 60 && H == 50 && alt.size() == 2 && has_alt(31, 23) && has_alt(31, 26) && ng == 13) g_nm = 2;
    return g_nm;
}

#ifndef IX_ON
#define IX_ON 0
#endif
void ix_init();
#ifndef PH_ON
#define PH_ON 0
#endif
#if PH_ON
static bool g_pg = false;       // ph: pilgrimage 50x65 (set once in init)
static bool g_phright = false;  // ph: our spawn is on the east half
#endif
static bool cxh_present=false;
void init() {
    S.emplace();
    auto& s = *S;
    auto& me = self();
    s.W = map::width();
    s.H = map::height();
#if PH_ON
    g_pg = s.W == 50 && s.H == 65;
    g_phright = me.position().x >= s.W / 2;
#endif
    if (s.W == 71 && s.H == 71) {
        // Any Prism draft (hello's, 剑桥夜爬者's): safe opening on either side, corner-altar banking while the hub is held.
        const auto foe = draft::current_draft(team::enemy);
        s.corners_hello_safe100 = std::find(foe.begin(), foe.end(), champion_draft::prism) != foe.end();
    }
    if (s.W == 71 && s.H == 71) {
        const auto foe = draft::current_draft(team::enemy);
        s.corners_cap = std::find(foe.begin(), foe.end(), champion_draft::stellar) != foe.end();
        s.fg_on = FG_ON && !s.corners_hello_safe100;
    }
    if (s.W == 40 && s.H == 40) {
        const auto foe = draft::current_draft(team::enemy);
        auto has = [&](champion_draft c) { return std::find(foe.begin(), foe.end(), c) != foe.end(); };
        s.interlock_keeper = has(champion_draft::stellar) && has(champion_draft::tavi)
            && !has(champion_draft::prism) && !has(champion_draft::willow);
    }
    s.coll = s.W == 41 && s.H == 41;
    s.il = s.W == 40 && s.H == 40 && draft::team_size() == 4;
    if (s.il) {
        const auto foe = draft::current_draft(team::enemy);
        s.il_nolock = std::find(foe.begin(), foe.end(), champion_draft::cassi_and_issac) == foe.end();
    }
    if (s.il) s.il_west = me.position().x + me.position().y < s.W - 1;
    if (s.coll) {
        // Mite-style Varg + Cassi&Issac collision drafts beat the altar-farm rule (v181 lost Mite's collision win): keep v177 play there.
        const auto foe = draft::current_draft(team::enemy);
        const bool varg = std::find(foe.begin(), foe.end(), champion_draft::varg) != foe.end();
        const bool ci = std::find(foe.begin(), foe.end(), champion_draft::cassi_and_issac) != foe.end();
        if (varg && ci) s.coll = false;
    }
    if (s.W == 30 && s.H == 30) {
        const auto enemy_picks = draft::current_draft(team::enemy);
        s.old_elkring_mages = std::find(enemy_picks.begin(), enemy_picks.end(), champion_draft::stellar) != enemy_picks.end()
            && (std::find(enemy_picks.begin(), enemy_picks.end(), champion_draft::prism) != enemy_picks.end()
                || (std::find(enemy_picks.begin(), enemy_picks.end(), champion_draft::tavi) != enemy_picks.end()
                    && std::find(enemy_picks.begin(), enemy_picks.end(), champion_draft::varg) == enemy_picks.end()));
        const auto has = [&](champion_draft pick) {
            return std::find(enemy_picks.begin(), enemy_picks.end(), pick) != enemy_picks.end();
        };
        s.mite_elkring_roster = enemy_picks.size() == 5
            && has(champion_draft::varg) && has(champion_draft::stellar)
            && has(champion_draft::cassi_and_issac) && has(champion_draft::tavi)
            && has(champion_draft::ren);
        s.mite_elkring_finish = me.position().x < s.W / 2 && s.mite_elkring_roster;
#if FR_GROUP_ON
        // fragment (30x30, 3 picks, altars (14,5)/(15,24)): g_nm = 3 routes the default target choice to the
        // pack-weighted instantiation. Only 30x30 maps execute this; the per-turn dispatch tests g_nm == 3 only
        // on maps where nm_map_id() > 0 (contest, stampede, fragment) or old_elkring_mages holds (30x30 only).
        if (draft::team_size() == 3) {
            const auto al = map::altars().positions();
            bool a1 = false, a2 = false;
            for (auto a : al) { if (a.x == 14 && a.y == 5) a1 = true; if (a.x == 15 && a.y == 24) a2 = true; }
            if (al.size() == 2 && a1 && a2) g_nm = 3;
        }
#endif
    }
    {
        const auto ep = draft::current_draft(team::enemy);
        cxh_present = std::find(ep.begin(),ep.end(),champion_draft::heart_of_ruin)!=ep.end();
        const auto own=draft::current_draft(team::ally);
        cxh_present |= std::find(own.begin(),own.end(),champion_draft::heart_of_ruin)!=own.end();
        const auto eh = [&](champion_draft pick) { return std::find(ep.begin(), ep.end(), pick) != ep.end(); };
        // Memory?'s pick order (Varg, C&I, Stellar[, Ren, Ryth]); Varg/Stellar/C&I-ordered packs drafts (Mite, Nyx, Univ...) stay untouched.
        // os: the Memory? test above generalised to a signature table (first match wins).
        g_os = -1;
        if (ep.size() >= 3) {
            const int W = s.W, H = s.H;
            const int mi = (W == 90 && H == 30) ? 0 : (W == 30 && H == 30) ? 1 : (W == 31 && H == 31) ? 2 : (W == 40 && H == 40) ? 3
                : (W == 41 && H == 41) ? 4 : (W == 71 && H == 71) ? 5 : (W == 50 && H == 50) ? (map::altars().positions().size() == 2 ? 7 : 6)
                : (W == 80 && H == 80) ? 8 : 9;
            for (const auto& sg : os_sigs) {
                if (static_cast<int>(ep[0]) != sg.p0 || static_cast<int>(ep[1]) != sg.p1 || static_cast<int>(ep[2]) != sg.p2) continue;
                if (!((sg.maps >> mi) & 1u)) continue;
                bool excluded = false;
                for (auto c : ep) if ((sg.excl >> static_cast<int>(c)) & 1u) excluded = true;
                if (excluded) continue;
                g_os = sg.set;
                break;
            }
            if (g_os >= 0) { g_mm_bank = os_k_bank[g_os][mi]; g_mm_flee = os_k_flee[g_os][mi]; }
            // Cassi's 44-gem Conley bag needs to leave before the enemy
            // reaches the central Monolith altar. Keep other champions and
            // drafts on the exact v229 threshold.
            if (mi == 6 && me.champion_name() == champion_type::cassi
                && ep.size() >= 5 && ep[0] == champion_draft::ryth
                && ep[1] == champion_draft::cassi_and_issac
                && ep[2] == champion_draft::stellar
                && ep[3] == champion_draft::ren
                && ep[4] == champion_draft::varg)
                g_mm_bank = 44;
            // In Conley's Packs game Vendetta carried 48 near a left altar,
            // stayed for another camp, then died with 92. Gate the late bag.
            g_conley_packs = mi == 2 && me.champion_name() == champion_type::vendetta
                && ep.size() >= 3 && ep[0] == champion_draft::cassi_and_issac
                && ep[1] == champion_draft::stellar
                && ep[2] == champion_draft::varg;
        }
        g_mm = g_os >= 0;
#if CTX_DR
        {
            const bool ci = eh(champion_draft::cassi_and_issac);
            if (CTX_DR == 1) g_ctx_dr = ci;
            else if (CTX_DR == 2) g_ctx_dr = ci && ep.size() == 5 && eh(champion_draft::varg) && eh(champion_draft::stellar) && eh(champion_draft::ren) && eh(champion_draft::ryth);
            else if (CTX_DR == 3) g_ctx_dr = ci && !eh(champion_draft::tavi);
        }
#endif
        (void)eh;
#if MM_LOG
        std::cout << "MMGATE " << g_mm << " set " << g_os << " bank " << g_mm_bank << " flee " << g_mm_flee << "\n";
#endif
    }
    if (CT_BANK > 0) {
        const auto ep = draft::current_draft(team::enemy);
        const auto eh = [&](champion_draft pick) { return std::find(ep.begin(), ep.end(), pick) != ep.end(); };
        g_ct = eh(champion_draft::stellar) && eh(champion_draft::tavi);
    }
    if (DG_DIS > 0 || DG_BANK > 0) {
        const auto ep = draft::current_draft(team::enemy);
        const auto eh = [&](champion_draft pick) { return std::find(ep.begin(), ep.end(), pick) != ep.end(); };
        g_dg = eh(champion_draft::stellar) || eh(champion_draft::tavi) || eh(champion_draft::prism);
        g_dgb = DG_BANK > 0 && g_dg && ((s.W == 41 && s.H == 41) || (s.W == 40 && s.H == 40));
        g_dgd = DG_DIS > 0 && g_dg && s.W == 41 && s.H == 41;
    }
    if (s.W == 90 && s.H == 30) {
        const auto enemy_picks = draft::current_draft(team::enemy);
        s.spawn_right = me.position().x >= s.W / 2;
        const auto has = [&](champion_draft pick) {
            return std::find(enemy_picks.begin(), enemy_picks.end(), pick) != enemy_picks.end();
        };
        const bool mite_stellar_left = !s.spawn_right && has(champion_draft::varg)
            && has(champion_draft::stellar) && has(champion_draft::cassi_and_issac)
            && has(champion_draft::tavi) && has(champion_draft::ren);
        s.lanes_current_left = mite_stellar_left || (!s.spawn_right && has(champion_draft::varg)
            && has(champion_draft::vendetta) && has(champion_draft::cassi_and_issac)
            && has(champion_draft::willow) && has(champion_draft::ren));
        s.lanes_ownhalf = s.lanes_current_left;
        if (g_os >= 0 && os_k_ownhalf[g_os] && !s.lanes_ownhalf) { s.lanes_ownhalf = true; s.lanes_current_left = false; }
    }
    if (g_os >= 0 && os_k_mages[g_os] && s.W == 30 && s.H == 30) s.old_elkring_mages = true;
#if EK3_ON
    if (s.W == 30 && s.H == 30 && draft::team_size() == 5) {
        const auto al = map::altars().positions();
        bool a1 = false, a2 = false;
        for (auto a : al) { if (a.x == 3 && a.y == 26) a1 = true; if (a.x == 26 && a.y == 3) a2 = true; }
        if (al.size() == 2 && a1 && a2) g_ek3 = 1;
    }
#endif
    s.type = me.champion_name();
    s.range = me.attack_range();
    s.ranged = s.range > 1;
    s.terrain = map::walls() | map::holes() | map::altars() | map::spawn_region(team::enemy);
    s.altars = map::altars().positions();
    s.ekmap = (EI_ON && s.W == 41 && s.H == 30 && s.altars.size() == 1) || (EK_ON && s.W == 30 && s.H == 30 && s.altars.size() == 2
            && ((s.altars[0].x == 26 && s.altars[0].y == 3) || (s.altars[0].x == 3 && s.altars[0].y == 26))
            && ((s.altars[1].x == 26 && s.altars[1].y == 3) || (s.altars[1].x == 3 && s.altars[1].y == 26)));  // elkring only, not fragment
    if (EK_NEEDCI && s.ekmap && s.W == 30) {
        // EK_NEEDCI: elkring weights only against a Cassi & Issac draft (ひななー's St/Re/Ry/Va/Ta proxy regressed).
        const auto foe = draft::current_draft(team::enemy);
        s.ekmap = std::find(foe.begin(), foe.end(), champion_draft::cassi_and_issac) != foe.end();
    }

    for (vec2 z114_p : {vec2{24,19}, vec2{24,20}, vec2{25,19}, vec2{25,20}}) s.terrain.draw_chebyshev(z114_p, 5.0f);
    s.df_spawn.compute(s.terrain, map::spawn_region(team::ally), s.visited);
    s.goals.clear();
    for (auto a : s.altars) s.goals.draw_chebyshev(a, static_cast<float>(constants::submit_gems_range));
    s.goals &= ~s.terrain;
    s.df_altar.compute(s.terrain, s.goals, s.visited);
    for (auto p : map::spawn_region(team::ally).positions()) {
        const int d = s.df_altar.get(p);
        if (d != UNREACH) s.spawn_altar = std::min(s.spawn_altar, d);
    }

    // SDK 2.2.5 detail::load_camp_groups layout and order, constructed directly
    // into stable bot records instead of allocating and copying SDK vectors.
    std::uint32_t word_count = 0;
    auto words = std::make_unique_for_overwrite<std::uint32_t[]>(256);
    detail::check(detail::sdk_map_camp_layout(words.get(), 256, &word_count));
    if (word_count > 256) {
        words = std::make_unique_for_overwrite<std::uint32_t[]>(word_count);
        detail::check(detail::sdk_map_camp_layout(words.get(), word_count, &word_count));
    }
    std::size_t group_count = 0, camp_count = 0;
    for (std::size_t i = 0; i + 5 <= word_count;) {
        ++group_count;
        camp_count += words[i + 4];
        i += 5 + 4 * words[i + 4];
    }
    s.groups.reserve(group_count);
    s.camp_storage.reserve(camp_count);
    for (std::size_t i = 0; i + 5 <= word_count;) {
        const auto group = words[i];
        Group out{ static_cast<handle_camp_group_t>(group) };
        out.gold = static_cast<int>(words[i + 1]);
        out.gems = static_cast<int>(words[i + 2]);
        out.respawn = static_cast<std::int32_t>(words[i + 3]);
        const auto members = words[i + 4];
        i += 5;
        out.spawn_dist = UNREACH;
        const auto camp_begin = s.camp_storage.size();
        for (std::uint32_t member = 0; member < members && i + 4 <= word_count; ++member, i += 4) {
            const vec2 pos{ static_cast<std::int32_t>(words[i + 2]), static_cast<std::int32_t>(words[i + 3]) };
            const auto kind = static_cast<camp_type>(words[i + 1]);
            auto& k = s.camp_storage.emplace_back(detail::handle_access::make_camp(static_cast<handle_camp_t>(words[i]), out.h, kind, pos));
            k.pos = pos;
            switch (kind) {
            case camp_type::wolf: k.maxhp = 1350; k.str = 20; k.range = 3; break;
            case camp_type::elk: k.maxhp = 2400; k.str = 60; k.range = 3; break;
            case camp_type::monolith: k.maxhp = 20000; k.str = 200; k.range = 5; break;
            default: k.maxhp = 500; k.str = 0; k.range = 0; break;
            }
            if (kind == camp_type::golem) {
                cambc::detail::camp_design design{};
                cambc::detail::check(cambc::detail::sdk_camp_design(static_cast<unsigned>(camp_type::golem), &design));
                k.maxhp = design.base_stats.base_health;
                k.str = design.base_stats.base_strength;
                k.range = design.base_stats.base_attack_range;
            }
            k.hp = k.maxhp;
            out.mdps += k.str;
            out.max_range = std::max(out.max_range, k.range);
            out.spawn_dist = std::min<int>(out.spawn_dist, s.df_spawn.get(k.pos));
        }
        out.camps = std::span<Camp>(s.camp_storage.data() + camp_begin, s.camp_storage.size() - camp_begin);
        // Only Varg needs the static splash score. Other champions cache
        // adjacency lazily when a ground-targeted spell needs a neighbor count.
        if (s.type == champion_type::varg) for (auto& a : out.camps) {
            a.adjacent_count = 0;
            for (std::size_t j = 0; j < out.camps.size(); ++j) {
                const auto& b = out.camps[j];
                // Tile coordinates are bounded by the map; reject x before testing y.
                const int dx = a.pos.x - b.pos.x;
                if (dx < -1 || dx > 1) continue;
                const int dy = a.pos.y - b.pos.y;
                if (dy < -1 || dy > 1) continue;
                if (a.adjacent_count < static_cast<int>(a.adjacent.size()))
                    a.adjacent[a.adjacent_count] = static_cast<std::uint16_t>(j);
                ++a.adjacent_count;
            }
            out.splash = std::max(out.splash, a.adjacent_count);
        }
        s.groups.push_back(std::move(out));
    }
#if N4_GW_SIDES || N4_BB_SIDES
    if (g_os < 0 && s.W == 50) { const float n4f = n4_::flee(n4_::detect(s.W, s.H, s.groups)); if (n4f > 0.0f) g_mm_flee = n4f; }   // cc_next4 (n4_gate.h)
#endif
#if FR_ON
    // fr (16 Oct, fragment only: 30x30, 3 picks, altars (14,5)/(15,24)). Decoded platform losses: Cassi and Ryth
    // raid the enemy's wolf camps at R40-90 and die alone (15-28 deaths a game); winners (Mite, Nyx) keep to
    // their half and the near centre elk. Zeroing the value of enemy-side groups makes choose_target skip them
    // (value <= 0) without touching per-turn code on any other map.
    if (s.W == 30 && s.H == 30 && s.altars.size() == 2 && draft::team_size() == 3) {
        bool a1 = false, a2 = false;
        for (auto a : s.altars) { if (a.x == 14 && a.y == 5) a1 = true; if (a.x == 15 && a.y == 24) a2 = true; }
        if (a1 && a2) {
            distance_field de;
            de.compute(map::walls() | map::holes() | map::altars(), map::spawn_region(team::enemy), s.visited);
            for (auto& g : s.groups) {
                int own = UNREACH, foe = UNREACH;
                for (auto& c : g.camps) { own = std::min<int>(own, s.df_spawn.get(c.pos)); foe = std::min<int>(foe, de.get(c.pos)); }
                const bool far = foe < own;
                if (FR_ON && far && (FR_ON == 2 || g.mdps <= 20)) { g.gems = 0; g.gold = 0; }
            }
        }
    }
#endif
#if FX_FARU
    if (g_nm == 3) {
        distance_field de;
        de.compute(map::walls() | map::holes() | map::altars(), map::spawn_region(team::enemy), s.visited);
        for (std::size_t gi = 0; gi < s.groups.size() && gi < 64; ++gi) {
            auto& g = s.groups[gi];
            int own = UNREACH, foe = UNREACH;
            for (auto& c : g.camps) { own = std::min<int>(own, s.df_spawn.get(c.pos)); foe = std::min<int>(foe, de.get(c.pos)); }
            if (foe < own && (FX_FARE == 2 || g.mdps <= 20)) g_fxfar |= 1ull << gi;
        }
    }
#endif
    if (s.coll) collision_init();
    if (s.W == 41 && s.H == 41) { CB.on = true; if (!s.coll) collision_init(); }
    if (s.W == 41 && s.H == 41) g_cplan = ca_select();
    if (CA_LOG && s.W == 41 && s.H == 41) std::cout << "CA plan " << g_cplan << " type " << static_cast<int>(s.type) << "\n";
    if (g_cplan == 1 && s.W == 41 && s.H == 41) { CX.on = true; CX.north = me.position().y < s.H / 2; }
    s.rows_like = s.altars.size() == 2 && s.W * s.H >= 2500;
    s.fg_shop = s.fg_on;
    s.fg_route = s.fg_on;
    if (FH_PACKS && s.W == 31 && s.H == 31) { s.fg_shop = s.fg_route = true; s.fg_target = FH_PACKS_GOLD; }
    if (PK_ON && s.W == 31 && s.H == 31) {
        // Platform one-match tests (16 Oct): early crystal grab beat EMPEROR Tung (Vd/Va/C&I) 1032:864 but lost
        // AlphaSlop (Re/C&I/St) and Conley (C&I/St/Va); only Vendetta+Varg+C&I drafts get it.
        const auto foe = draft::current_draft(team::enemy);
        auto has = [&](champion_draft c) { return std::find(foe.begin(), foe.end(), c) != foe.end(); };
        if (has(champion_draft::vendetta) && has(champion_draft::varg) && has(champion_draft::cassi_and_issac)) {
            s.pk = true; if (PK_NOGOLD) s.fg_route = false;
        }
#if SH_ON && defined(SH_MAPS)
        else if ((SH_MAPS >> 4) & 1) { s.pk = true; g_shpk = true; }
#endif
    }
#if CXK_MAPS
    if ((CXK_MAPS & 0x200) && s.W==31 && s.H==31 && (CXK_SW & 8)) {
        const auto foe=draft::current_draft(team::enemy);
#if CXK2_GATE
        // Two independently observed gains: Ren-first economy and C&I/Stellar/Varg.
        cxk2_keeper = foe.size()==3 &&
            ((foe[0]==champion_draft::ren && foe[1]==champion_draft::stellar && foe[2]==champion_draft::cassi_and_issac) ||
             (foe[0]==champion_draft::cassi_and_issac && foe[1]==champion_draft::stellar && foe[2]==champion_draft::varg));
#endif
        if(foe.size()==3 && foe[0]==champion_draft::ren && foe[1]==champion_draft::stellar && foe[2]==champion_draft::cassi_and_issac) {
            s.pk=true; if(PK_NOGOLD)s.fg_route=false;
        }
    }
#endif
    if (s.W == 50 && s.H == 50) {
        // contest (new pool) has two altars like rows but plays like monolith: health-first build, monolith recall rules.
        if (NM_CT_ROWSOFF && nm_map_id(s.W, s.H, s.altars, s.groups.size()) == 1) s.rows_like = false;
        if (FH_MONO && !s.rows_like) s.fg_shop = true;
    }
    // Hub: three or more harmless gem groups packed together (corners' centre) are the map's richest source.
    for (auto& a : s.groups) {
        if (a.mdps > 0 || a.gems < P_HUBMIN) continue;
        int n = 0;
        for (auto& b : s.groups)
            if (b.mdps == 0 && b.gems >= P_HUBMIN && cheb(a.camps[0].pos, b.camps[0].pos) <= 6) ++n;
        a.hub = n >= 3;
    }
#if MC_CHUB
    // mc (16 Oct map change): corners' centre crystal hub became 2 elk + 2 wolves (36/22 gems, with gold). With no crystal
    // hub left, treat the groups within MC_CHUBR of the centre altar as the hub so the corners hub plans keep their target.
    if (s.W == 71 && s.H == 71) {
        bool any = false;
        for (auto& a : s.groups) any = any || a.hub;
        if (!any) {
            int ci = -1, best = 1 << 20;
            for (std::size_t i = 0; i < s.altars.size(); ++i) {
                const int d = std::abs(2 * s.altars[i].x - (s.W - 1)) + std::abs(2 * s.altars[i].y - (s.H - 1));
                if (d < best) { best = d; ci = static_cast<int>(i); }
            }
            if (ci >= 0)
                for (auto& a : s.groups)
                    if (a.mdps <= MC_CHUBD && cheb(a.camps[0].pos, s.altars[ci]) <= MC_CHUBR) a.hub = true;
        }
    }
#endif

#if IX_DUMP
    if (s.W == 40 && s.H == 40) {
        std::cout << "IXD spawn " << me.position().x << "," << me.position().y << " altars";
        for (auto a : s.altars) std::cout << " " << a.x << "," << a.y;
        std::cout << " cap " << constants::capture_time << " sleep " << constants::camp_sleep_time << " resp " << constants::respawn_time << " recall " << constants::recall_time << "\n";
        for (std::size_t i = 0; i < s.groups.size(); ++i) { auto& g = s.groups[i];
            std::cout << "IXD g" << i << " gems " << g.gems << " gold " << g.gold << " respawn " << g.respawn << " mdps " << g.mdps << " splash " << g.splash << " sd " << g.spawn_dist << " altar " << s.df_altar.get(g.camps[0].pos) << " camps";
            for (auto& c : g.camps) std::cout << " " << c.pos.x << "," << c.pos.y << ":" << c.maxhp;
            std::cout << "\n"; }
    }
#endif
#if MC_LHUB
    // mc: lanes' twelve 24-gem crystal singles (x = 36 / W-37, upper two rows, a hub before 16 Oct) became 22-gem wolves.
    if (s.W == 90 && s.H == 30) {
        for (auto& a : s.groups) {
            const vec2 c = a.camps[0].pos;
            if (a.camps.size() == 1 && a.gems >= 20 && a.mdps <= 20 && (c.x == 36 || c.x == s.W - 37) && c.y <= 16) a.hub = true;
        }
    }
#endif
    if (IX_ON) ix_init();
    // Shared-memory slot: position of this champion's pick in the draft plan.
    const int want = draft_id_of(s.type);
    s.idx = want % 5;
    for (std::size_t i = 0; i < draft_memory.size() && i < 5; ++i)
        if (static_cast<int>(draft_memory[i]) == want) s.idx = static_cast<int>(i);
    if (s.type == champion_type::issac) s.idx = 5;
}

// ---- shared memory: one byte per body, [state:2][group+1:6] ----
void read_shm() {
    for (int i = 0; i < 8; ++i) S->shm[i] = static_cast<std::uint8_t>(shared_memory[i]);
}
void publish(int group, int state) {
    const std::uint8_t v = group < 0 ? 0 : static_cast<std::uint8_t>(((state & 3) << 6) | ((group + 1) & 63));
    if (S->shm[S->idx] != v) {
        shared_memory[S->idx] = static_cast<std::byte>(v);
        S->shm[S->idx] = v;
    }
}
int claims_on(int group, bool capturing_only) {
    int n = 0;
    for (int i = 0; i < 6; ++i) {
        if (i == S->idx) continue;
        const auto v = S->shm[i];
        if ((v & 63) != group + 1) continue;
        if (capturing_only && (v >> 6) != 2) continue;
        ++n;
    }
    return n;
}

// ---- observation ----
void observe() {
    auto& s = *S;
    vision::visible_tiles_into(s.vis);
    for (int gi = 0; gi < static_cast<int>(s.groups.size()); ++gi) {
        auto& g = s.groups[gi];
        bool any = false;
        for (auto& c : g.camps) if (s.vis.is_set(c.pos)) { any = true; break; }
        if (!any) {
            if (g.st == 2 && s.round >= g.respawn_round) {
                g.st = 0;
                for (auto& c : g.camps) c.hp = c.maxhp;
            }
            // An ally standing on the reward starts the respawn clock soon.
            if (g.st != 2 && claims_on(gi, true) > 0) {
                g.st = 2;
                g.respawn_round = s.round + constants::capture_time + g.respawn;
            }
            continue;
        }
        g.seen = s.round;
        int st = 0, timer = 0;
        try {
            // The visible-tile snapshot above already proved a visible member, as
            // SDK camp_group::state does. Keep the SDK's exact state/error mapping.
            std::uint32_t state = 0;
            detail::check(detail::sdk_camp_group_state(static_cast<std::uint32_t>(g.h), &state));
            switch (state) {
            case 1: break;
            case 2: st = 1; break;
            case 3:
                st = 2;
                detail::check(detail::sdk_camp_group_cooldown(static_cast<std::uint32_t>(g.h), &timer));
                break;
            case 0: throw cambc::error{ error_code::api_not_in_vision, "camp group state is unknown" };
            default: throw std::runtime_error("unknown engine camp group state");
            }
        } catch (const cambc::error&) {
            continue;
        }
        g.st = st;
        if (st == 2) {
            g.respawn_round = s.round + timer;
            for (auto& c : g.camps) c.hp = c.maxhp;
        } else if (st == 1) {
            for (auto& c : g.camps) c.hp = 0;
        } else {
            for (auto& c : g.camps) {
                if (!s.vis.is_set(c.pos)) continue;
                c.hp = c.h.is_visible() ? c.h.health() : 0.0f;
            }
        }
    }
}

// ---- shopping ----
#if SF_ON
// sf map index: 0 elkring, 1 collision, 2 interlock, 3 eight, 4 pilgrimage, 5 stampede, 6 expanse, 7 rows, 8 other
__attribute__((noinline)) int sf_map() {
    static int m = -1;
    if (m >= 0) return m;
    const int W = map::width(), H = map::height(), n = static_cast<int>(draft::team_size());
    m = (W == 30 && H == 30 && n == 5) ? 0 : (W == 41 && H == 41) ? 1 : (W == 40 && H == 40) ? 2 : (W == 41 && H == 30) ? 3
      : (W == 50 && H == 65) ? 4 : (W == 60 && H == 50) ? 5 : (W == 80 && H == 80) ? 6 : (W == 50 && H == 50 && n == 4) ? 7 : 8;
    return m;
}
// nx2 (17 Oct): full map index. 0..7 = sf_map; 8 lanes, 9 packs, 10 monolith, 11 contest, 12 swirl, 13 rushbait,
// 14 corners, 15 ambush, 16 fragment, 17 other. Masks below 512 keep the sf_map meaning (bit 8 = every other map);
// all nx2 terms are compile-time guarded so the default build compiles to mx3's code.
__attribute__((noinline)) int nxm() {
    static int m = -1;
    if (m >= 0) return m;
    const int sm = sf_map();
    if (sm <= 7) return m = sm;
    const int W = map::width(), H = map::height(), n = static_cast<int>(draft::team_size());
    m = (W == 90 && H == 30) ? 8 : (W == 31 && H == 31) ? 9 : (W == 50 && H == 50 && n == 5) ? (map::altars().positions().size() != 2 ? 10 : 11)
      : (W == 45 && H == 30) ? 12 : (W == 30 && H == 80) ? 13 : (W == 71 && H == 71) ? 14 : (W == 100 && H == 100) ? 15 : (W == 30 && H == 30) ? 16 : 17;
    return m;
}
#define NXB(M) ((M) < 512 ? (((M) >> sf_map()) & 1) : (((M) >> nxm()) & 1))

#if RR_MASK
inline bool rr_on() { static int m = -1; if (m < 0) m = ((RR_MASK >> nxm()) & 1) && g_ctx_dr; return m == 1; }
inline int rr_bit(champion_type t) { return t == champion_type::stellar ? 1 : t == champion_type::tavi ? 2 : t == champion_type::ren ? 4 : t == champion_type::ryth ? 8 : 16; }
#endif
#if TF_ON
inline bool tf_on() { static int m = -1; if (m < 0) m = (TF_MAPS >> nxm()) & 1; return m == 1; }
inline bool tf_dis() { static int m = -1; if (m < 0) m = (TF_DISMAPS >> nxm()) & 1; return m == 1; }
inline bool tf_rec() { static int m = -1; if (m < 0) m = (TF_RECMAPS >> nxm()) & 1; return m == 1; }
inline bool tf_caster(champion_type t) {
    return t == champion_type::stellar || t == champion_type::tavi || t == champion_type::prism || t == champion_type::cassi || t == champion_type::marcella;
}
// Cassi & Issac share their fate: the pair counts once as min(hp) x summed dps.
inline float tf_pair(float hc, float dc, float hi, float di) {
    if (hc < 0.0f) return hi < 0.0f ? 0.0f : hi * di;
    if (hi < 0.0f) return hc * dc;
    return std::min(hc, hi) * (dc + di);
}
#endif
#ifndef NX_FD
#define NX_FD 0         // nx2: extra nxm() bits treated as fd maps (SF_MCX key A; B also needs FD_MCXMAPS)
#endif
#ifndef NX_CPX
#define NX_CPX 0        // nx2: extra nxm() bits with the cp caster layers (Stellar/Tavi sf_pvp, chase key, recall CP_RECHP)
#endif
#ifndef FD_RHP2
#define FD_RHP2 0.45f
#endif
#ifndef FD_RHP2MAPS
#define FD_RHP2MAPS 0   // nx2: map bits where FD_RG carriers recall below FD_RHP2 instead of FD_RHP
#endif
#ifndef FD_RGWHO
#define FD_RGWHO 4095   // nx2: champion_type bits for the FD_RG recall (cassi 8, issac 16)
#endif
#ifndef FD_RGI
#define FD_RGI 0.0f     // nx2: Issac carriers (>= FD_RG gems) recall below this on FD_RGIMAPS (0 = off)
#endif
#ifndef FD_RGIMAPS
#define FD_RGIMAPS 0
#endif
#ifndef CP_RECHP2
#define CP_RECHP2 0.45f
#endif
#ifndef CP_RECM2
#define CP_RECM2 0      // nx2: cp_map bits where the cp recall uses CP_RECHP2
#endif
#endif
// ---- cp (17 Oct): caster play gated per map. Map index: 0 elkring, 1 collision, 2 interlock, 3 eight, 4 fragment,
// 5 lanes, 6 packs, 7 monolith, 8 other. Masks: CP_WHOM (Stellar/Tavi use sf_pvp), CP_MCXM (chase key), CP_RECM (recall
// CP_RECHP). All masks 0 = us_shop3 behaviour.
#ifndef CP_WHOM
#define CP_WHOM 0
#endif
#ifndef CP_WHO
#define CP_WHO 3
#endif
#ifndef CP_MCXM
#define CP_MCXM 0
#endif
#ifndef CP_RECM
#define CP_RECM 0
#endif
#ifndef CP_RECHP
#define CP_RECHP 0.45f
#endif
__attribute__((noinline)) int cp_map() {
    static int m = -1;
    if (m >= 0) return m;
    const int W = map::width(), H = map::height(), n = static_cast<int>(draft::team_size());
    m = (W == 30 && H == 30 && n == 5) ? 0 : (W == 41 && H == 41) ? 1 : (W == 40 && H == 40) ? 2 : (W == 41 && H == 30) ? 3
      : (W == 30 && H == 30 && n == 3) ? 4 : (W == 90 && H == 30) ? 5 : (W == 31 && H == 31) ? 6
      : (W == 50 && H == 50 && map::altars().positions().size() != 2) ? 7 : 8;
    return m;
}
#ifndef CP_SHOPM
#define CP_SHOPM 0      // maps where the health shop also runs on fragment/corners (us_shop3 skips them)
#endif
static bool g_cp_mcx = false;
inline bool cp_mcx() { return g_cp_mcx; }
inline void cp_init() { static bool done = false; if (!done) { done = true; g_cp_mcx = ((CP_MCXM >> cp_map()) & 1) || (NX_CPX && ((NX_CPX >> nxm()) & 1)); } }
// ---- mx (17 Oct): per-map owner. cp maps (cp_map 1 collision, 3 eight, 4 fragment, 5 lanes) use cp_fix play;
// sf_map 0..7 non-cp maps use fd_fix play; all remaining maps use us_shop3 play.
#ifndef MX_CPMAPS
#define MX_CPMAPS 58    // cp_map bits owned by cp_fix
#endif
__attribute__((noinline)) bool mx_cpmap_f() { return ((MX_CPMAPS >> cp_map()) & 1) != 0 || (NX_CPX && ((NX_CPX >> nxm()) & 1)); }
__attribute__((noinline)) bool mx_fdmap_f() { return !mx_cpmap_f() && (sf_map() <= 7 || (NX_FD && ((NX_FD >> nxm()) & 1))); }
// Cached once per game (mx_init) so hot loops pay a global load, not a call.
static bool g_mx_cp = false, g_mx_a = false, g_mx_b = false;
#if GD_MASK || GD_MASK2
static float g_gdf = -1.0f;   // gd: gold weight floor on this map (-1 = ct_fix decay), set once in mx_init
#endif
inline bool mx_cpmap() { return g_mx_cp; }
// A: spell-target key (fd_fix: SF_MCX on every map). B: chase key and ranged step-first (fd_fix: SF_MCX on FD_MCXMAPS).
inline bool mx_mcxA() { return g_mx_a; }
inline bool mx_mcxB() { return g_mx_b; }
inline void mx_init() {
    static bool done = false; if (done) return; done = true;
    cp_init();
    g_mx_cp = mx_cpmap_f();
#if GD_MASK || GD_MASK2
    g_gdf = ((GD_MASK >> nxm()) & 1) ? GD_FLOOR : ((GD_MASK2 >> nxm()) & 1) ? GD_FLOOR2 : -1.0f;
#endif
    g_mx_a = g_mx_cp ? cp_mcx() : (SF_MCX && mx_fdmap_f());
    g_mx_b = g_mx_cp ? cp_mcx() : (SF_MCX && mx_fdmap_f() && NXB(FD_MCXMAPS));
    if (map::width() == 45 && map::height() == 30) {
        const auto foe = draft::current_draft(team::enemy);
        if (foe.size() == 4 && foe[0] == champion_draft::varg
            && foe[1] == champion_draft::ryth
            && foe[2] == champion_draft::cassi_and_issac
            && foe[3] == champion_draft::stellar) g_mx_a = g_mx_b = true;
    }
}
inline float cp_recall() { return (CP_RECM2 && ((CP_RECM2 >> cp_map()) & 1)) ? CP_RECHP2 : (((CP_RECM >> cp_map()) & 1) || (NX_CPX && ((NX_CPX >> nxm()) & 1))) ? CP_RECHP : 0.30f; }
#if Z181_ON
__attribute__((noinline)) bool z181_exact_fragment() {
    static const bool exact = []() { const auto& s = *S;
        return s.W == 30 && s.H == 30 && s.altars.size() == 2
            && std::find(s.altars.begin(), s.altars.end(), vec2{14,5}) != s.altars.end()
            && std::find(s.altars.begin(), s.altars.end(), vec2{15,24}) != s.altars.end();
    }();
    return exact;
}
#endif

void shop() {
    auto& me = self();
    std::uint32_t gold = me.gold_count();
    if ((ccx_on() || g_gwc) && me.consumables()[consumable_type::tp_crystal].count() == 0 && gold >= 10
        && me.can_buy(consumable_type::tp_crystal)) {
        try { me.buy(consumable_type::tp_crystal); } catch (const cambc::error&) {}
        gold = me.gold_count();
    }
#if P_LOCK
    if (FG_NOLOCK && (S->fg_shop || S->il_nolock)) {} else
    if (gold >= 10 && me.consumables()[consumable_type::altar_lock].count() == 0 && me.can_buy(consumable_type::altar_lock)) {
        me.buy(consumable_type::altar_lock);
        gold -= 10;
    }
#endif
    if (gold < 30) return;
#if Z181_ON
    auto owned = me.item_count();
#else
    const auto owned = me.item_count();
#endif
    using it = item_type;
    struct Step { it shard; it emblem; };
#include "cb3_hor.h"
#if Z181_ON
    // Z181: after the opening emblem, finish one Bruiser Health emblem before
    // further purchases. Keep the opening build and existing shop-return timing.
    std::uint32_t z181_emblems = 0;
    for (int z181_i = 1; z181_i < 42; z181_i += 2)
        z181_emblems += owned[static_cast<it::enum_type>(z181_i)];
    if (z181_exact_fragment() && z181_emblems > 0 && owned[it::bruiser_emblem_of_health] == 0) {
        const std::uint32_t z181_need = 5 - std::min<std::uint32_t>(5, owned[it::bruiser_shard_of_health]);
        const std::uint32_t z181_n = std::min(z181_need, gold / 30);
        if (z181_n == 0 || !me.can_buy(it::bruiser_shard_of_health, z181_n)) return;
        me.buy(it::bruiser_shard_of_health, z181_n);
        gold -= z181_n * 30;
        owned = me.item_count();
        if (z181_n < z181_need) return;
    }
#endif

#if Z144_MARCELLA
    if (S->type == champion_type::marcella) {
        // Observed focus-first winner order: Treasure F, Bruiser F, Durability H,
        // Bruiser H, then repeat Bruiser F. Five shards form each emblem.
        const std::array<Step, 4> z144_plan{
            Step{ it::treasure_shard_of_focus, it::treasure_emblem_of_focus },
            Step{ it::bruiser_shard_of_focus, it::bruiser_emblem_of_focus },
            Step{ it::durability_shard_of_health, it::durability_emblem_of_health },
            Step{ it::bruiser_shard_of_health, it::bruiser_emblem_of_health } };
        for (const auto& z144_step : z144_plan) {
            if (gold < 30) return;
            if (owned[z144_step.emblem] > 0) continue;
            const std::uint32_t z144_need = 5 - std::min<std::uint32_t>(5, owned[z144_step.shard]);
            const std::uint32_t z144_n = std::min(z144_need, gold / 30);
            if (z144_n == 0 || !me.can_buy(z144_step.shard, z144_n)) return;
            me.buy(z144_step.shard, z144_n);
            gold -= z144_n * 30;
            if (z144_n < z144_need) return;
        }
        const std::uint32_t z144_n = gold / 30;
        if (z144_n && me.can_buy(it::bruiser_shard_of_focus, z144_n)) me.buy(it::bruiser_shard_of_focus, z144_n);
        return;
    }
#endif
#if TR_TMASK
    if (((TR_TMASK >> nxm()) & 1) && ((TR_TWHO >> static_cast<int>(S->type)) & 1) && S->round < TR_TR) {
        // tr: one Treasure emblem (+20% gold income) for the main gold earners, as the first (TR_TPOS 1) or second emblem.
        const bool foc = S->type == champion_type::tavi || S->type == champion_type::cassi || S->type == champion_type::stellar || S->type == champion_type::prism;
        const it tsh = foc ? it::treasure_shard_of_focus : it::treasure_shard_of_strength;
        const it tem = foc ? it::treasure_emblem_of_focus : it::treasure_emblem_of_strength;
        const bool have = owned[it::treasure_emblem_of_strength] + owned[it::treasure_emblem_of_focus] + owned[it::treasure_emblem_of_health] > 0;
        std::uint32_t nemb = 0;
        for (it e : { it::jungle_emblem_of_strength, it::monster_emblem_of_strength, it::power_emblem_of_strength, it::durability_emblem_of_strength,
                      it::bruiser_emblem_of_health, it::durability_emblem_of_health, it::monster_emblem_of_focus, it::power_emblem_of_focus }) nemb += owned[e];
        if (!have && owned[tem] == 0 && (TR_TPOS <= 1 || nemb >= TR_TPOS - 1)) {
            const std::uint32_t need = 5 - std::min<std::uint32_t>(5, owned[tsh]);
            const std::uint32_t n = std::min(need, gold / 30);
            if (n == 0 || !me.can_buy(tsh, n)) return;
            me.buy(tsh, n);
            gold -= n * 30;
            if (n < need) return;
        }
    }
#endif
#if S2_SHOP5
    if ((S2_SHOP5 >> nxm()) & 1) {   // s2 (test opponent; S2_SHOP5 = nxm() map bits): Intercontinental Fish builds from 17 Oct replays, on every map.
        // Casters Power Focus, Durability Health, Power Focus; Ren Bruiser Health, Durability Health, Power Focus;
        // Issac/Varg Power Strength, Durability Health, Bruiser Health; others Bruiser Health, Durability Health, Power Strength.
        const auto t = S->type;
        const bool cas = t == champion_type::cassi || t == champion_type::tavi || t == champion_type::stellar || t == champion_type::prism;
        const bool pw = t == champion_type::issac || t == champion_type::varg;
        const bool ren = t == champion_type::ren;
        const std::array<Step, 3> sp = cas
            ? std::array<Step, 3>{ Step{ it::power_shard_of_focus, it::power_emblem_of_focus }, Step{ it::durability_shard_of_health, it::durability_emblem_of_health }, Step{ it::monster_shard_of_focus, it::monster_emblem_of_focus } }
            : pw ? std::array<Step, 3>{ Step{ it::power_shard_of_strength, it::power_emblem_of_strength }, Step{ it::durability_shard_of_health, it::durability_emblem_of_health }, Step{ it::bruiser_shard_of_health, it::bruiser_emblem_of_health } }
            : ren ? std::array<Step, 3>{ Step{ it::bruiser_shard_of_health, it::bruiser_emblem_of_health }, Step{ it::durability_shard_of_health, it::durability_emblem_of_health }, Step{ it::power_shard_of_focus, it::power_emblem_of_focus } }
            : std::array<Step, 3>{ Step{ it::bruiser_shard_of_health, it::bruiser_emblem_of_health }, Step{ it::durability_shard_of_health, it::durability_emblem_of_health }, Step{ it::power_shard_of_strength, it::power_emblem_of_strength } };
        for (auto& step : sp) {
            if (gold < 30) return;
            if (owned[step.emblem] > 0) continue;
            const std::uint32_t need = 5 - std::min<std::uint32_t>(5, owned[step.shard]);
            const std::uint32_t n = std::min(need, gold / 30);
            if (n == 0 || !me.can_buy(step.shard, n)) return;
            me.buy(step.shard, n);
            gold -= n * 30;
            if (n < need) return;
        }
        const it extra = (cas || ren) ? it::power_shard_of_focus : it::power_shard_of_strength;
        const std::uint32_t n = gold / 30;
        if (n && me.can_buy(extra, n)) me.buy(extra, n);
        return;
    }
#endif
#if SF_SHOP
    if (((SF_SHOPM >> sf_map()) & 1) && !(SR_NOSHOP && ((map::width() == 45 && map::height() == 30) || (map::width() == 30 && map::height() == 80))) && !(map::width() == 71 && map::height() == 71) && (!(map::width() == 30 && map::height() == 30 && draft::team_size() != 5) || ((CP_SHOPM >> cp_map()) & 1))) {  // not corners/fragment   // sf: top-team builds. Casters: Power Focus, Durability Health, then Power Focus. Melee: Jungle, Durability
        // Health, Bruiser Health, then Power Strength (their max HP is 200-400 above ours by round 300).
        const bool cas = S->type == champion_type::cassi || S->type == champion_type::tavi || S->type == champion_type::stellar;
        const std::array<Step, 3> sp = cas
            ? std::array<Step, 3>{ Step{ it::power_shard_of_focus, it::power_emblem_of_focus }, Step{ it::durability_shard_of_health, it::durability_emblem_of_health },
                                   Step{ it::monster_shard_of_focus, it::monster_emblem_of_focus } }
            : (SF_SHOP == 2 ? std::array<Step, 3>{ Step{ it::durability_shard_of_health, it::durability_emblem_of_health }, Step{ it::bruiser_shard_of_health, it::bruiser_emblem_of_health },
                                   Step{ it::jungle_shard_of_strength, it::jungle_emblem_of_strength } }
             : SF_SHOP == 3 ? std::array<Step, 3>{ Step{ it::jungle_shard_of_strength, it::jungle_emblem_of_strength }, Step{ it::bruiser_shard_of_health, it::bruiser_emblem_of_health },
                                   Step{ it::durability_shard_of_health, it::durability_emblem_of_health } }
             : SF_SHOP == 4 ? std::array<Step, 3>{ Step{ it::monster_shard_of_strength, it::monster_emblem_of_strength }, Step{ it::durability_shard_of_health, it::durability_emblem_of_health },
                                   Step{ it::bruiser_shard_of_health, it::bruiser_emblem_of_health } }
             : std::array<Step, 3>{ Step{ it::jungle_shard_of_strength, it::jungle_emblem_of_strength }, Step{ it::durability_shard_of_health, it::durability_emblem_of_health },
                                   Step{ it::bruiser_shard_of_health, it::bruiser_emblem_of_health } });
        for (auto& step : sp) {
            if (gold < 30) return;
            if (owned[step.emblem] > 0) continue;
            const std::uint32_t need = 5 - std::min<std::uint32_t>(5, owned[step.shard]);
            const std::uint32_t n = std::min(need, gold / 30);
            if (n == 0 || !me.can_buy(step.shard, n)) return;
            me.buy(step.shard, n);
            gold -= n * 30;
            if (n < need) return;
        }
        const it extra = cas ? it::power_shard_of_focus : it::power_shard_of_strength;
        const std::uint32_t n = gold / 30;
        if (n && me.can_buy(extra, n)) me.buy(extra, n);
        return;
    }
#endif
#if P_BUILD == 10
    // Mite's observed order: Jungle, Durability, Bruiser (health) emblems, then Power.
    const std::array<Step, 3> plan{ Step{ it::jungle_shard_of_strength, it::jungle_emblem_of_strength },
                                    Step{ it::durability_shard_of_strength, it::durability_emblem_of_strength },
                                    Step{ it::bruiser_shard_of_health, it::bruiser_emblem_of_health } };
#elif P_BUILD == 11
    // Restore the older strength build against Stellar and Prism together on Elkring.
    const std::array<Step, 2> plan = S->old_elkring_mages
        ? std::array<Step, 2>{ Step{ it::monster_shard_of_strength, it::monster_emblem_of_strength },
                               Step{ it::jungle_shard_of_strength, it::jungle_emblem_of_strength } }
        : std::array<Step, 2>{ Step{ it::monster_shard_of_strength, it::monster_emblem_of_strength },
                               Step{ it::durability_shard_of_health, it::durability_emblem_of_health } };
#elif P_BUILD == 0
    const std::array<Step, 2> plan{ Step{ it::jungle_shard_of_strength, it::jungle_emblem_of_strength },
                                    Step{ it::monster_shard_of_strength, it::monster_emblem_of_strength } };
#elif P_BUILD == 1
    const std::array<Step, 2> plan{ Step{ it::monster_shard_of_strength, it::monster_emblem_of_strength },
                                    Step{ it::jungle_shard_of_strength, it::jungle_emblem_of_strength } };
#elif P_BUILD == 2
    const std::array<Step, 1> plan{ Step{ it::monster_shard_of_strength, it::monster_emblem_of_strength } };
#else
    const std::array<Step, 1> plan{ Step{ it::jungle_shard_of_strength, it::jungle_emblem_of_strength } };
#endif
#if P_FOCUS
    // Casters scale with Focus: Monster (focus), Durability (health), then Power (focus).
    const bool caster = S->type == champion_type::cassi || S->type == champion_type::tavi || S->type == champion_type::stellar;
    const std::array<Step, 2> fplan = (FX_CAS && g_nm == 3 && S->type == champion_type::cassi)
        ? std::array<Step, 2>{ Step{ it::durability_shard_of_health, it::durability_emblem_of_health },
                               Step{ it::monster_shard_of_focus, it::monster_emblem_of_focus } }
        : std::array<Step, 2>{ Step{ it::monster_shard_of_focus, it::monster_emblem_of_focus },
                               Step{ it::durability_shard_of_health, it::durability_emblem_of_health } };
    if (caster && !S->old_elkring_mages) {
        for (auto& step : fplan) {
            if (gold < 30) return;
            if (owned[step.emblem] > 0) continue;
            const std::uint32_t need = 5 - std::min<std::uint32_t>(5, owned[step.shard]);
            const std::uint32_t n = std::min(need, gold / 30);
            if (n == 0 || !me.can_buy(step.shard, n)) return;
            me.buy(step.shard, n);
            gold -= n * 30;
            if (n < need) return;
        }
        const std::uint32_t n = gold / 30;
        if (n && me.can_buy(it::power_shard_of_focus, n)) me.buy(it::power_shard_of_focus, n);
        return;
    }
#endif
#if FG_BUILD
    if (S->fg_shop) {
#if FG_BUILD == 1
        const std::array<Step, 2> cplan{ Step{ it::durability_shard_of_health, it::durability_emblem_of_health },
                                         Step{ it::monster_shard_of_strength, it::monster_emblem_of_strength } };
#else
        const std::array<Step, 2> cplan{ Step{ it::bruiser_shard_of_health, it::bruiser_emblem_of_health },
                                         Step{ it::monster_shard_of_strength, it::monster_emblem_of_strength } };
#endif
        for (auto& step : cplan) {
            if (gold < 30) return;
            if (owned[step.emblem] > 0) continue;
            const std::uint32_t need = 5 - std::min<std::uint32_t>(5, owned[step.shard]);
            const std::uint32_t n = std::min(need, gold / 30);
            if (n == 0 || !me.can_buy(step.shard, n)) return;
            me.buy(step.shard, n);
            gold -= n * 30;
            if (n < need) return;
        }
        const std::uint32_t n = gold / 30;
        if (n && me.can_buy(it::power_shard_of_strength, n)) me.buy(it::power_shard_of_strength, n);
        return;
    }
#endif
    if (S->rows_like && S->W == 50 && S->H == 50) {
#if RX_ROWS
        const std::array<Step, 1> rows_plan{ Step{ it::monster_shard_of_strength, it::monster_emblem_of_strength } };
#else
        const std::array<Step, 3> rows_plan{ Step{ it::jungle_shard_of_strength, it::jungle_emblem_of_strength },
                                             Step{ it::durability_shard_of_strength, it::durability_emblem_of_strength },
                                             Step{ it::bruiser_shard_of_health, it::bruiser_emblem_of_health } };
#endif
        for (auto& step : rows_plan) {
            if (gold < 30) return;
            if (owned[step.emblem] > 0) continue;
            const std::uint32_t need = 5 - std::min<std::uint32_t>(5, owned[step.shard]);
            const std::uint32_t n = std::min(need, gold / 30);
            if (n == 0 || !me.can_buy(step.shard, n)) return;
            me.buy(step.shard, n);
            gold -= n * 30;
            if (n < need) return;
        }
        const std::uint32_t n = gold / 30;
        if (n && me.can_buy(it::power_shard_of_strength, n)) me.buy(it::power_shard_of_strength, n);
        return;
    }
    for (auto& step : plan) {
        if (gold < 30) return;
        if (owned[step.emblem] > 0) continue;
        const std::uint32_t need = 5 - std::min<std::uint32_t>(5, owned[step.shard]);
        const std::uint32_t n = std::min(need, gold / 30);
        if (n == 0 || !me.can_buy(step.shard, n)) return;
        me.buy(step.shard, n);
        gold -= n * 30;
        if (n < need) return;
    }
    const std::uint32_t n = gold / 30;
    if (n && me.can_buy(it::power_shard_of_strength, n)) me.buy(it::power_shard_of_strength, n);
}

// ---- movement ----
void refresh_occ() {
    auto& s = *S;
    vision::champion_tiles_into(s.occ);
    vision::camp_tiles_into(s.tmp);
    s.occ |= s.tmp;
    s.occ |= s.terrain;
}

bool in_bounds(vec2 p) { return p.x >= 0 && p.y >= 0 && p.x < S->W && p.y < S->H; }

// Step down a distance field, avoiding visible occupants. Optionally dash first.
bool step_field(const distance_field& f, bool allow_dash) {
    auto& s = *S;
    auto& me = self();
    if (!me.can_move()) return false;
    vec2 pos = me.position();
    std::uint16_t cur = f.get(pos);
    if (cur == 0 || cur == UNREACH) return false;

    if (allow_dash && cur >= 4 && !s.cast && me.can_cast()) {
        const auto abilities = me.abilities();
        for (auto& ab : abilities) {
            if (!ab.is_dash() || !ab.is_ready()) continue;
            vec2 best = pos;
            std::uint16_t bv = cur - 1;
            for (int dx = -2; dx <= 2; ++dx) for (int dy = -2; dy <= 2; ++dy) {
                vec2 p{ pos.x + dx, pos.y + dy };
                if (!in_bounds(p) || s.occ.is_set(p)) continue;
                const auto v = f.get(p);
                if (v < bv) { bv = v; best = p; }
            }
            if (best != pos) {
                try {
                    if (ab.use(best)) { s.cast = true; ++s.casts; pos = me.position(); cur = f.get(pos); }
                } catch (const cambc::error&) {}
            }
            break;
        }
        if (cur == 0 || !me.can_move()) return true;
    }

    std::array<std::pair<std::uint16_t, vec2>, 8> opts;
    int n = 0;
    for (auto d : DIRS) {
        vec2 p{ pos.x + d.x, pos.y + d.y };
        if (!in_bounds(p) || s.occ.is_set(p)) continue;
        const auto v = f.get(p);
        if (v < cur) opts[n++] = { v, p };
    }
    std::sort(opts.begin(), opts.begin() + n, [](auto& a, auto& b) { return a.first < b.first; });
    for (int i = 0; i < n; ++i) {
        try {
            if (me.move_absolute(opts[i].second)) { s.moved = true; ++s.moves; return true; }
        } catch (const cambc::error&) {}
    }
    return false;
}

// Path to the nearest goal tile around visible occupants.
bool step_to_goals(bool allow_dash) {
    auto& s = *S;
    auto& me = self();
    if (!me.can_move()) return false;
    const vec2 pos = me.position();
    refresh_occ();
    s.occ.set(pos, false);
    s.goals &= ~s.occ;
    if (s.goals.is_empty() || s.goals.is_set(pos)) return false;
    s.me_bb.clear();
    s.me_bb.set(pos);
    s.df_go.compute_to(s.occ, s.goals, s.me_bb, s.visited);
    if (s.df_go.get(pos) == UNREACH) {
        // Units block the way: path over terrain only and wait for a gap.
        s.df_go.compute_to(s.terrain, s.goals, s.me_bb, s.visited);
    }
    return step_field(s.df_go, allow_dash);
}

// Heart of Ruin: strength is the unmodified stat; the passive scales outgoing damage.
float cxh_attack(const champion& c) {
    const float a=c.strength();
    if(!cxh_present || c.champion_name()!=champion_type::heart_of_ruin)return a;
    const float missing=std::clamp(1.0f-c.health()/std::max(1.0f,c.max_health()),0.0f,1.0f);
    return a*(1.0f+constants::passive_abilities::heart_of_ruin_missing_health_damage_max_bonus*missing);
}
float cxh_health(const champion& c) {
    const float hp=c.health();
    if(!cxh_present || c.champion_name()!=champion_type::heart_of_ruin)return hp;
    const auto buffs=c.buffs();
    float shield=buffs.has_shield()?buffs.get<buff_shield>().value:0.0f;
    for(const auto& ab:c.abilities())if(ab.is_shield()&&ab.is_ready())shield+=ab.shield_amount();
    return hp+shield;
}
float cxh_burst(const champion& c) {
    float hit=0;
    for(const auto& ab:c.abilities())if(ab.is_dash())hit=std::max(hit,ab.damage_amount());
    return hit; // Engine already includes the missing-health multiplier.
}

// ---- combat ----
struct Spell { int slot; int kind; float dmg; bool ground; bool excl; };

void attack_camp(Camp& c) {
    auto& me = self();
    me.attack(c.h);
    ++S->attacks;
    c.hp = c.h.is_visible() ? c.h.health() : 0.0f;
}

// bm (18 Oct, compile-time BM_MASK = nxm() map bits; default 0 compiles to tv_ar393's code): Intercontinental Fish's
// big-camp protocol decoded from stampede replay c532c184 (scratchpad/bm_agent/big-monster-playbook.md). Every body
// stays at Chebyshev distance >= 4 from every living monster with strength > 0 (camp attack range is 3, monsters do
// not chase, their HP does not regenerate), casts its range-4 damage spell whenever it is off cooldown, ranged bodies
// basic-attack the strength-0 clusters reachable from that ring, nobody walks in before the last attacker is dead.
// Champions without a range-4 camp spell (Varg, Issac, Vesper, Vendetta, Willow) skip the group while it lives.
#ifndef BM_MASK
#define BM_MASK 0
#endif
#ifndef BM_MDPS
#define BM_MDPS 200.0f
#endif
#ifndef BM_CASTERS
#define BM_CASTERS 2793   // ryth cassi stellar prism marcella ren tavi
#endif
#ifndef BM_INDPS
#define BM_INDPS 60.0f  // walk in (normal fight code) once the living attackers' summed strength is at most this
#endif
#ifndef BM_LOG
#define BM_LOG 0
#endif
#if BM_MASK
inline bool bm_on() { return ((BM_MASK >> nxm()) & 1) != 0; }
inline bool bm_caster() { return ((BM_CASTERS >> static_cast<int>(S->type)) & 1) != 0; }
inline bool bm_group(const Group& g) { return g.mdps >= BM_MDPS; }
// True when the turn was spent on the poke protocol (cast, safe basic attack, reposition or hold).
__attribute__((noinline)) bool bm_poke(Group& g) {
    auto& s = *S;
    auto& me = self();
    const vec2 pos = me.position();
    int dmin = 99;
    for (auto& c : g.camps) if (c.hp > 0 && c.str > 0) dmin = std::min(dmin, cheb(pos, c.pos));
    if (dmin == 99) return false;  // only harmless clusters left: the normal fight code finishes them
    if (g.alive_dps() <= BM_INDPS) return false;  // last attacker: walk in so we stand on the tiles when it dies
    bool acted = false;
    if (dmin >= 4 && !s.moved && !s.cast && me.can_cast()) {
        const auto abilities = me.abilities();
        int pick = -1; float pv = 0; vec2 tile{}; Camp* camp = nullptr;
        for (int i = 0; i < 2; ++i) {
            const auto& ab = abilities[i];
            if (!ab.is_ready() || ab.is_dash() || ab.is_attack_reset() || ab.is_shield() || ab.is_heal()) continue;
            const float dmg = ab.damage_amount();
            if (dmg <= 0) continue;
            if (ab.can_target_ground()) {
                int most = 0; vec2 t{};
                for (auto& c : g.camps) {
                    if (c.hp <= 0 || cheb(pos, c.pos) > 4) continue;
                    int n = 0;
                    n = alive_neighbours(g, c);
                    if (n > most) { most = n; t = c.pos; }
                }
                if (most > 0 && dmg * most > pv) { pv = dmg * most; pick = i; tile = t; camp = nullptr; }
            } else if (ab.can_target_camps()) {
                Camp* b = nullptr; float bk = -1e18f;
                for (auto& c : g.camps) {
                    if (c.hp <= 0 || cheb(pos, c.pos) > 4 || !c.h.is_visible()) continue;
                    const float key = (c.str > 0 ? 1e6f : 0.0f) - c.hp;
                    if (key > bk) { bk = key; b = &c; }
                }
                if (b && std::min(dmg, b->hp) > pv) { pv = std::min(dmg, b->hp); pick = i; camp = b; }
            }
        }
        if (pick >= 0) {
            try {
                const bool ok = camp ? abilities[pick].use(camp->h) : abilities[pick].use(tile);
                if (ok) { s.cast = true; ++s.casts; acted = true; }
            } catch (const cambc::error&) {}
            for (auto& c : g.camps) if (c.hp > 0 && s.vis.is_set(c.pos)) c.hp = c.h.is_visible() ? c.h.health() : 0.0f;
        }
    }
    if (dmin >= 4 && s.ranged && me.can_attack()) {
        Camp* b = nullptr;
        for (auto& c : g.camps) if (c.hp > 0 && c.str <= 0 && cheb(pos, c.pos) <= s.range && c.h.is_visible() && (!b || c.hp < b->hp)) b = &c;
        if (b) { try { attack_camp(*b); acted = true; } catch (const cambc::error&) {} }
    }
#if BM_LOG
    if (s.round % 10 == 0 || dmin <= 3) std::cout << "BM r" << s.round << " t" << static_cast<int>(s.type) << " @" << pos.x << "," << pos.y << " dmin " << dmin << " ghp " << g.hp() << (acted ? " act" : "") << "\n";
#endif
    if (acted) { ++s.t_fight; return true; }
    if (dmin == 4) { ++s.idle; ++s.t_wait; return true; }  // in position, spells on cooldown: hold
    // Reposition onto the ring: distance exactly 4 from the nearest living attacker.
    bitboard danger; danger.clear();
    for (auto& c : g.camps) if (c.hp > 0 && c.str > 0) danger.draw_chebyshev(c.pos, 3.0f);
    s.goals.clear();
    for (auto& c : g.camps) if (c.hp > 0 && c.str > 0) s.goals.draw_chebyshev(c.pos, 4.0f);
    s.goals &= ~danger; s.goals &= ~s.terrain; s.goals &= ~g.tiles();
    bool moved;
    if (dmin >= 5) {   // approaching: never path through the attackers' reach
        const bitboard saved = s.terrain;
        s.terrain |= danger;
        moved = step_to_goals(false);
        s.terrain = saved;
    } else moved = step_to_goals(false);   // inside: shortest way out
    if (!moved) ++s.idle;
    ++s.t_walk;
    return true;
}
#endif

// st4 (19 Oct; docs/stampede-contested-herd-spec-2026-10-19.md): stampede herd occupation controller. ST4_MASK = nxm() map
// bits (32 = stampede); 0 = off (default, byte-identical behaviour). ST4_SW bits pick the parts (see st4_pre / bm_poke).
#ifndef ST4_MASK
#define ST4_MASK 0
#endif
#ifndef ST4_SW
#define ST4_SW 31
#endif
#ifndef ST4_LOG
#define ST4_LOG 0
#endif
#if ST4_MASK || ST4_LOG
inline bool st4_on() { static int m = -1; if (m < 0) m = ((ST4_MASK >> nxm()) & 1); return m == 1; }
int st4_herd() {
    static int h = -2;
    if (h != -2) return h;
    h = -1; float best = 0;
    for (int i = 0; i < static_cast<int>(S->groups.size()); ++i) if (S->groups[i].mdps > best) { best = S->groups[i].mdps; h = i; }
    if (best < BM_MDPS) h = -1;
    return h;
}
int st4_tdist(const Group& g, vec2 p) {
    int d = 99;
    for (auto& c : g.camps) d = std::min(d, cheb(p, c.pos));
    return d;
}
#endif
#if ST4_LOG
void st4_log(const char* tag) {
    auto& s = *S;
    if (nxm() != 5) return;
    const int h = st4_herd(); if (h < 0) return;
    auto& g = s.groups[h]; auto& me = self(); const vec2 pos = me.position();
    if (st4_tdist(g, pos) > 8) return;
    std::cout << "S4 " << tag << " r" << s.round << " t" << static_cast<int>(s.type) << " @" << pos.x << "," << pos.y << " hp " << static_cast<int>(me.health()) << "/" << static_cast<int>(me.max_health())
              << " st " << g.st << " adps " << static_cast<int>(g.alive_dps()) << " ghp " << static_cast<int>(g.hp()) << " tgt " << s.target << " m " << static_cast<int>(s.mode) << " E:";
    for (auto& e : vision::champions(team::enemy)) if (e.is_visible() && e.is_alive() && cheb(e.position(), pos) <= 8)
        std::cout << " " << static_cast<int>(e.champion_name()) << "@" << e.position().x << "," << e.position().y << ":" << static_cast<int>(e.health()) << (g.tiles().is_set(e.position()) ? "T" : "");
    std::cout << " A:";
    for (auto& a : vision::champions(team::ally)) if (a.is_visible() && a.is_alive() && a.position() != pos && cheb(a.position(), pos) <= 8)
        std::cout << " " << static_cast<int>(a.champion_name()) << "@" << a.position().x << "," << a.position().y << ":" << static_cast<int>(a.health()) << (g.tiles().is_set(a.position()) ? "T" : "");
    std::cout << "\n";
}
#define ST4L(t) st4_log(t)
#else
#define ST4L(t) ((void)0)
#endif

bool cxh_own_dash(vec2 target) {
    auto& s=*S;auto& me=self();
    if(s.type!=champion_type::heart_of_ruin || s.cast || s.moved || !me.can_cast())return false;
    for(const auto& ab:me.abilities()) {
        if(!ab.is_dash() || !ab.is_ready())continue;
        const auto q=ab.get<ability_dash>();const auto pos=me.position();
        if(cheb(pos,target)>q.range+q.radius)return false;
        if(me.health()<.55f*me.max_health())for(const auto& shield:me.abilities())if(shield.is_shield()&&shield.is_ready())return false;
        refresh_occ();vec2 best=pos;int value=999;
        for(int dx=-static_cast<int>(q.range);dx<=q.range;++dx)for(int dy=-static_cast<int>(q.range);dy<=q.range;++dy){
            const vec2 p{pos.x+dx,pos.y+dy};
            if(p==pos||!in_bounds(p)||s.occ.is_set(p)||cheb(p,target)>q.radius)continue;
            const int v=10*cheb(p,target)+cheb(pos,p);if(v<value){value=v;best=p;}
        }
        if(best==pos)return false;
        try{if(ab.use(best)){s.cast=true;s.moved=true;++s.casts;return true;}}catch(const cambc::error&){}
    }
    return false;
}

// Fight the group: returns true if any offensive action happened.
bool fight(Group& g) {
    auto& s = *S;
    auto& me = self();
    const vec2 pos = me.position();
    const bool varg = s.type == champion_type::varg;

    auto neighbours = [&](const Camp& c) {
        int n = 0;
        n = alive_neighbours(g, c);
        return n;
    };
    Camp* best = nullptr;
    float best_key = -1e18f;
    for (auto& c : g.camps) {
        if (c.hp <= 0) continue;
        if (cheb(pos, c.pos) > s.range || !c.h.is_visible()) continue;
        const float key = (varg ? 10000.0f * neighbours(c) : 0.0f) - c.hp + (c.str > 0 ? 1.0f : 0.0f);
        if (key > best_key) { best_key = key; best = &c; }
    }
    if (!best) return false;
    if(cxh_own_dash(best->pos))return true;

    // One cast per turn: pick the most valuable ready ability.
    if (!s.cast && me.can_cast()) {
        const auto abilities = me.abilities();
        int pick = -1;
        float pick_value = 0;
        vec2 pick_tile{};
        Camp* pick_camp = nullptr;
        const bool hurting = g.alive_dps() > 0;
        for (int i = 0; i < 2; ++i) {
            const auto& ab = abilities[i];
            if (ab.is_dash() || !ab.is_ready()) continue;
            float value = 0;
            vec2 tile{};
            Camp* camp = nullptr;
            if (ab.is_attack_reset()) {
                value = cxh_attack(me) * (varg ? neighbours(*best) : 1);
            } else if (ab.is_shield()) {
                if (hurting) value = 1000;
            } else if (ab.is_heal()) {
                continue;
            } else if (ab.can_target_ground()) {
                if (s.moved) continue;
                const float dmg = ab.damage_amount();
                int most = 0;
                for (auto& c : g.camps) {
                    if (c.hp <= 0 || cheb(pos, c.pos) > 4) continue;
                    const int n = neighbours(c);
                    if (n > most) { most = n; tile = c.pos; }
                }
                value = dmg * most;
            } else if (ab.can_target_camps()) {
                if (s.moved) {
                    // Walking excludes most unit spells; stay safe and skip.
                    continue;
                }
                const float dmg = ab.damage_amount();
                camp = best;
                value = dmg > 0 ? std::min(dmg, best->hp) : 0.3f * best->str;
            }
            if (value > pick_value) { pick_value = value; pick = i; pick_tile = tile; pick_camp = camp; }
        }
        if (pick >= 0) {
            const auto& ab = abilities[pick];
            try {
                bool ok = false;
                if (ab.is_attack_reset()) {
                    if (me.can_attack()) attack_camp(*best);
                    if (best->hp > 0 || true) ok = ab.use();
                } else if (ab.is_shield()) ok = ab.use();
                else if (pick_camp) ok = ab.use(pick_camp->h);
                else ok = ab.use(pick_tile);
                if (ok) { s.cast = true; ++s.casts; }
            } catch (const cambc::error& e) {
                if (DBG) std::cout << "cast err " << e.what() << "\n";
            }
            for (auto& c : g.camps)
                if (c.hp > 0 && s.vis.is_set(c.pos)) c.hp = c.h.is_visible() ? c.h.health() : 0.0f;
        }
    }

    if (me.can_attack()) {
        if (best->hp <= 0) {
            best = nullptr;
            for (auto& c : g.camps)
                if (c.hp > 0 && cheb(pos, c.pos) <= s.range && c.h.is_visible() && (!best || c.hp < best->hp)) best = &c;
        }
        if (best) {
            try { attack_camp(*best); } catch (const cambc::error& e) { if (DBG) std::cout << "atk err " << e.what() << "\n"; }
        }
    }
    return true;
}

// Attack an enemy champion that is already in range.
// ---- sr (17 Oct): rushbait_v2 (30x80) / swirl (45x30) spawns sit a few tiles apart. Top teams park a champion on its
// own spawn (healed every round) next to ours; our pvp kept attacking it for 100s of rounds (v316 lost 5/5 there).
// SR_SPN 1: on rushbait_v2 ignore enemy champions standing on the enemy spawn region (no target, no fight trigger).
#ifndef SR_SPNSW
#define SR_SPNSW 0      // swirl too (local vs sf_hunter: swirl A 15 -> 11 of 18, so off)
#endif
#ifndef SR_SPN
#define SR_SPN 1
#endif
inline bool sr_map() { static int m = -1; if (m < 0) { const int W = map::width(), H = map::height(); m = (W == 30 && H == 80) || (SR_SPNSW && W == 45 && H == 30); } return m == 1; }
#ifndef SR_SPNHP
#define SR_SPNHP 0.0f   // ... only while it is above this health fraction
#endif
inline bool sr_skip(const champion& e) { return SR_SPN && map::spawn_region(team::enemy).is_set(e.position()) && e.health() >= SR_SPNHP * e.max_health(); }
#if Z316_ON
// Match fight_enemy_t's basic-attack target ordering, including its special map bias.
template <bool SR> bool z316_priority(float hp, float maxhp) {
#if Z327_ON
    if (z327_expanse) return false;
#endif
    auto& s = *S; auto& me = self();
    static constexpr bool z316_active[13] = {
        false, Z316_CHAMP == 1, false, false, false,
        Z316_CHAMP == 5, Z316_CHAMP == 6, false, false, false, false, false, false
    };
    if (!s.ranged || !z316_active[static_cast<int>(s.type)]
        || hp * 100 >= Z316_HP * maxhp || !me.can_move() || !me.can_attack()
        || s.mode == Mode::bank) return false;
    const vec2 pos = me.position();
    bool z316_threat = false;
    std::optional<champion> z316_target;
    float z316_score = 1e18f;
    for (auto& e : vision::champions(team::enemy)) {
        if (!e.is_visible() || !e.is_alive()) continue;
        const int d = cheb(pos, e.position());
        // One normal step plus the enemy's basic attack range; spells are not forecast.
#line 1 "z319_extension_z114_policy.cpp"
#if Z319_VESPER_ON
        if (d <= e.attack_range() + (s.type == champion_type::vesper_vale ? Z319_VESPER_REACH : 1)) z316_threat = true;
#else
#line 3277 "z114_policy.cpp"
        if (d <= e.attack_range() + 1) z316_threat = true;
#line 1 "z319_extension_z114_policy.cpp"
#endif
#line 3278 "z114_policy.cpp"
        if ((SR && sr_skip(e)) || d > 4 || d > s.range) continue;
        float k = cxh_health(e);
        if (mx_mcxA()) {
            int g = 0; try { g = static_cast<int>(e.gem_count()); } catch (const cambc::error&) {}
            k = k * (e.champion_name() == champion_type::cassi ? 0.7f : 1.0f) - 8.0f * std::min(g, 60);
        }
        if (k < z316_score) { z316_score = k; z316_target = e; }
    }
    if (!z316_threat || !z316_target) return false;
    const auto z316_buffs = z316_target->buffs();
    const float z316_shield = z316_buffs.has_shield() ? z316_buffs.get<buff_shield>().value : 0.0f;
    return z316_target->health() + z316_shield > cxh_attack(me);
#line 1 "z319_extension_z114_policy.cpp"
}
#endif
#if Z319_ANY
template <bool SR> bool z319_priority(float hp, float maxhp) {
    auto& s = *S; auto& me = self();
    const int z319_type = static_cast<int>(s.type);
    static constexpr int z319_hp[13] = {Z319_RYTH_ON*Z319_RYTH_HP,0,Z319_VENDETTA_ON*Z319_VENDETTA_HP,Z319_CASSI_ON*Z319_CASSI_HP,0,Z319_STELLAR_ON*Z319_STELLAR_HP,Z319_PRISM_ON*Z319_PRISM_HP,Z319_MARCELLA_ON*Z319_MARCELLA_HP,0,0,0,0,Z319_HOR_ON*Z319_HOR_HP};
    if (!z319_hp[z319_type] || hp * 100 >= z319_hp[z319_type] * maxhp
        || !me.can_move() || !me.can_attack() || s.mode == Mode::bank) return false;
    const bool z319_ranged = z319_type == 5 || z319_type == 6;
    int z319_enemies = 0;
    bool z319_lethal = false;
    const auto z319_own_buffs = me.buffs();
    const float z319_own_shield = z319_own_buffs.has_shield() ? z319_own_buffs.get<buff_shield>().value : 0.0f;
    const vec2 pos = me.position();
    bool z319_threat = false;
    std::optional<champion> z319_target;
    float z319_score = 1e18f;
    for (auto& e : vision::champions(team::enemy)) {
        if (!e.is_visible() || !e.is_alive()) continue;
        const int d = cheb(pos, e.position());
        if (d <= 6) ++z319_enemies;
        // One-step basic-hit envelope, including active shields; no ability forecast.
        if (d <= e.attack_range() + 1) {
            z319_threat = true;
            if (cxh_attack(e) >= hp + z319_own_shield) z319_lethal = true;
        }
        if ((SR && sr_skip(e)) || d > 4 || d > s.range) continue;
        float k = cxh_health(e);
        if (mx_mcxA()) {
            int g = 0; try { g = static_cast<int>(e.gem_count()); } catch (const cambc::error&) {}
            k = k * (e.champion_name() == champion_type::cassi ? 0.7f : 1.0f) - 8.0f * std::min(g, 60);
        }
        if (k < z319_score) { z319_score = k; z319_target = e; }
    }
    if (!z319_threat || !z319_target) return false;
    if (!z319_ranged && !z319_lethal) return false;
#if Z319_OUTNUMBERED
    if (z319_ranged) {
        int z319_allies = 1;
        for (auto& a : vision::champions(team::ally)) {
            if (a.position() == pos || !a.is_visible() || !a.is_alive()) continue;
            if (cheb(pos, a.position()) <= 6) ++z319_allies;
        }
        if (z319_enemies <= z319_allies) return false;
    }
#endif
    const auto z319_buffs = z319_target->buffs();
    const float z319_shield = z319_buffs.has_shield() ? z319_buffs.get<buff_shield>().value : 0.0f;
    return z319_target->health() + z319_shield > cxh_attack(me);
#line 3290 "z114_policy.cpp"
}
#endif
template <bool SR> bool fight_enemy_t() {
    auto& s = *S;
    auto& me = self();
    const vec2 pos = me.position();
    std::optional<champion> best, spell_target;
    float best_hp = 1e18f, spell_hp = 1e18f;
    for (auto& e : vision::champions(team::enemy)) {
        if (!e.is_visible() || !e.is_alive() || (SR && sr_skip(e))) continue;
        const int d = cheb(pos, e.position());
        if (d > 4) continue;
        const float hp = cxh_health(e);
        if (hp < spell_hp) { spell_hp = hp; spell_target = e; }
        if (mx_mcxA()) {
            int g = 0;
            try { g = static_cast<int>(e.gem_count()); } catch (const cambc::error&) {}
            const float k = hp * (e.champion_name() == champion_type::cassi ? 0.7f : 1.0f) - 8.0f * std::min(g, 60);
            if (d <= s.range && k < best_hp) { best_hp = k; best = e; }
        } else
        if (d <= s.range && hp < best_hp) { best_hp = hp; best = e; }
    }
    if (!spell_target) return false;
    if(cxh_own_dash(spell_target->position()))return true;
    bool acted = false;
    try {
        if (!s.cast && me.can_cast()) {
            const auto abilities = me.abilities();
            for (auto& ab : abilities) {
                if (!ab.is_ready() || ab.is_dash() || ab.is_heal() || ab.is_attack_reset()) continue;
                bool ok = false;
                if (ab.is_shield()) ok = ab.use();
                else if (s.moved) continue;
                else if (ab.can_target_ground()) ok = ab.use(spell_target->position());
                else if (ab.can_target_enemies()) ok = ab.use(*spell_target);
                if (ok) { s.cast = true; ++s.casts; acted = true; break; }
            }
        }
        if (best && best->is_visible() && best->is_alive() && me.can_attack()) { me.attack(*best); ++s.attacks; acted = true; }
        if (best && !s.cast && me.can_cast() && best->is_visible() && best->is_alive()) {
            const auto abilities = me.abilities();
            for (auto& ab : abilities)
                if (ab.is_attack_reset() && ab.is_ready() && ab.use()) {
                    s.cast = true; ++s.casts;
                    if (me.can_attack()) { me.attack(*best); ++s.attacks; }
                    break;
                }
        }
    } catch (const cambc::error& e) {
        if (DBG) std::cout << "enemy err " << e.what() << "\n";
    }
    return acted;
}

// ---- alarm: bytes 6-7 hold [round:6][y/4:5][x/4:5] of a sighted enemy ----
void raise_alarm(vec2 p) {
    const std::uint16_t v = static_cast<std::uint16_t>(((S->round & 63) << 10) | (((p.y >> 2) & 31) << 5) | ((p.x >> 2) & 31));
    shared_memory[6] = static_cast<std::byte>(v & 255);
    shared_memory[7] = static_cast<std::byte>(v >> 8);
}
bool alarm_near(vec2 pos, vec2& where) {
    const std::uint16_t v = static_cast<std::uint16_t>(S->shm[6] | (S->shm[7] << 8));
    if (v == 0) return false;
    const int age = ((S->round & 63) - (v >> 10) + 64) & 63;
    if (age > 2) return false;
    where = vec2{ std::min<int>(S->W - 1, static_cast<int>(v & 31) * 4 + 2), std::min<int>(S->H - 1, static_cast<int>((v >> 5) & 31) * 4 + 2) };
    return cheb(pos, where) <= P_ALARM;
}

// Largest single spell hit a champion can land, from its kit and current focus.
float champ_burst(const champion& c) {
    float k = 0;
    switch (c.champion_name()) {
    case champion_type::heart_of_ruin: return cxh_burst(c);
    case champion_type::ryth: k = 1.0f; break;
    case champion_type::cassi: k = 2.0f; break;
    case champion_type::stellar: k = 2.0f; break;
    case champion_type::prism: k = 1.5f; break;
    case champion_type::marcella: k = 2.0f; break;
    case champion_type::ren: k = 1.25f; break;
    case champion_type::tavi: k = CXV_TAVI_BURST; break;
    default: break;
    }
    return k * c.focus();
}
// Sustained damage per turn: basic attacks plus spell damage spread over the cooldown.
float champ_dps(const champion& c) {
    if(cxh_present && c.champion_name()==champion_type::heart_of_ruin)
        return 1.1f*cxh_attack(c)+P_BURSTW*cxh_burst(c)/8.0f;
    float extra = 0;
    if (c.champion_name() == champion_type::stellar) extra = 1.4f * c.focus();
    if (c.champion_name() == champion_type::prism) extra = 1.0f * c.focus();
    return c.strength() * 1.1f + P_BURSTW * (champ_burst(c) + extra) / 8.0f;
}

#if BL_ON
// bl: damage a visible enemy at Chebyshev d can deal to us over the next two turns (autos it can reach plus a spell
// that is ready within a turn); spell_reach returns its spell range + splash radius for the safe-recall test.
static int g_bl_api = BL_API;
__attribute__((noinline)) float bl_threat(const champion& e, const champion& me, int d, int& spell_reach) {
    const bool melee = e.attack_range() <= 1;
    float dmg = 0;
    if (d <= (melee ? 3 : e.attack_range() + 2)) dmg += 2.0f * cxh_attack(e) * (melee ? 1.5f : 1.0f);
    const float kit = champ_burst(e);
    float burst = kit;
    spell_reach = 5;
    if (g_bl_api) {
        try {
            float api = -1.0f; bool seen = false;
            for (const auto& ab : e.abilities()) {
                const auto& inf = ab.info();
                if (!inf) continue;
                float range = 0, radius = 0;
                if (const auto* q = std::get_if<ability_damage>(&*inf)) range = q->range;
                else if (const auto* q = std::get_if<ability_aoe_damage>(&*inf)) { range = q->range; radius = q->radius; }
                else if(cxh_present && e.champion_name()==champion_type::heart_of_ruin) {
                    if(const auto* q=std::get_if<ability_dash>(&*inf)){range=q->range;radius=q->radius;}else continue;
                }else continue;
                seen = true;
                spell_reach = std::max(spell_reach, static_cast<int>(range + radius + 0.5f));
                if (ab.cooldown() > 1) continue;
                api = std::max(api, ab.damage_amount(me));
            }
            if (seen) burst = api >= 0.0f ? api : 0.0f;
        } catch (...) {
            g_bl_api = 0; burst = kit; spell_reach = 5;
#if P_LOG
            std::cout << "BLAPI off\n";
#endif
        }
    }
    if (d <= spell_reach + 1) dmg += burst;
    return dmg;
}
#endif

// React to visible enemy champions. Returns true when the turn was spent on them.
__attribute__((noinline)) bool early_packs_stellar_threat(float hp, float maxhp) {
    auto& s = *S;
    if (s.round > 50 || s.type != champion_type::vendetta || hp >= 0.5f * maxhp) return false;
    const vec2 pos = self().position();
    for (auto& e : vision::champions(team::enemy)) {
        if (e.is_visible() && e.is_alive()
            && e.champion_name() == champion_type::stellar
            && cheb(pos, e.position()) <= 2) return true;
    }
    return false;
}

#if DG_DIS > 0
// Low on health with an enemy caster (Stellar/Tavi/Prism) within its spell range: disengage.
__attribute__((noinline)) bool dg_disengage(float hp, float maxhp) {
    if (hp >= (DG_DIS / 100.0f) * maxhp) return false;
    const vec2 pos = self().position();
    for (auto& e : vision::champions(team::enemy)) {
        if (!e.is_visible() || !e.is_alive() || cheb(pos, e.position()) > DG_DISR) continue;
        const auto t = e.champion_name();
        if (t == champion_type::stellar || t == champion_type::tavi || t == champion_type::prism) return true;
    }
    return false;
}
#endif

#if SF_ON
bool sf_pvp(float hp, float maxhp);
#endif
// ---- fd helpers ----
// Shared memory is 8 bytes: partner state comes from its body byte (Cassi: cx_idx, Issac: 5).
__attribute__((noinline)) bool fd_on() { return NXB(FD_MAPS) != 0; }
int cx_idx(champion_type t);
// Our Cassi/Issac partner: visible position (vis=true), else the first camp of its published target group.
__attribute__((noinline)) bool fd_partner(vec2& out, bool& vis) {
    const auto t = S->type;
    vis = false;
    if (t != champion_type::cassi && t != champion_type::issac) return false;
    const auto want = t == champion_type::cassi ? champion_type::issac : champion_type::cassi;
    for (auto& a : vision::champions(team::ally)) {
        if (!a.is_visible() || !a.is_alive()) continue;
        if (a.champion_name() == want) { out = a.position(); vis = true; return true; }
    }
    const int v = S->shm[cx_idx(want)];
    const int g = (v & 63) - 1;
    if (g < 0 || g >= static_cast<int>(S->groups.size()) || S->groups[g].camps.empty()) return false;
    out = S->groups[g].camps[0].pos;
    return true;
}
__attribute__((noinline)) bool fd_partner_banking() {
    const auto t = S->type;
    if (t != champion_type::cassi && t != champion_type::issac) return false;
    const auto want = t == champion_type::cassi ? champion_type::issac : champion_type::cassi;
    return S->shm[cx_idx(want)] == static_cast<std::uint8_t>((3u << 6) | 63u);
}

#if ST4_MASK
// st4 hook, before pvp_t (play_turn): herd capture window on ST4_MASK maps.
// SW 1: reward available (st 1) -> a body within ST4_R of the herd that is on a herd tile holds it (no kite/chase; leaves only
//       on the lethal check); a body off the tiles enters while the region is contested (enemy on a tile) or while fewer than
//       ST4_N allies hold it, else it supports through the normal pvp_t. Holders and entrants attack from where they stand.
// SW 2: the hook's attack and spell pick an enemy standing on a herd tile first (then lowest hp).
// SW 4: the same entry while the herd is down to its last attacker (alive_dps <= BM_INDPS) with <= ST4_PREHP hp left
//       (free tiles of dead clusters / dead elks; living camps block their own tiles).
// SW 16: Cassi never holds/enters (her death also kills Issac). SW 32: hook off while outnumbered around the herd
//        (visible enemies within ST4_OR of the tiles > allies within ST4_OR, me included, + ST4_OM).
// SW 8: while the reward is available, farming bodies within ST4_R2 (not only ST4_R) come back and enter/hold.
#ifndef ST4_R
#define ST4_R 4
#endif
#ifndef ST4_R2
#define ST4_R2 12       // SW 8: radius while the reward is available and we are farming (come back to an abandoned herd)
#endif
#ifndef ST4_OR
#define ST4_OR 5        // SW 32: radius (from the herd tiles) of the head count
#endif
#ifndef ST4_OM
#define ST4_OM 0        // SW 32: hook off while enemies near the herd > allies near the herd + ST4_OM
#endif
#ifndef ST4_N
#define ST4_N 1
#endif
#ifndef ST4_EHP
#define ST4_EHP 0.4f
#endif
#ifndef ST4_LHP
#define ST4_LHP 0.35f
#endif
#ifndef ST4_PREHP
#define ST4_PREHP 400.0f
#endif
bool st4_pre(float hp, float maxhp) {
    if (!st4_on()) return false;
    auto& s = *S;
    if (s.mode == Mode::recall) return false;
    const int h = st4_herd(); if (h < 0) return false;
    auto& g = s.groups[h];
    auto& me = self();
    const vec2 pos = me.position();
    if (st4_tdist(g, pos) > (((ST4_SW & 8) && g.st == 1 && s.mode == Mode::farm) ? ST4_R2 : ST4_R)) return false;
    bool cap = (ST4_SW & 1) && g.st == 1;
    if (!cap && (ST4_SW & 4) && g.st == 0 && g.alive_dps() > 0 && g.alive_dps() <= BM_INDPS) {
        float ahp = 0; for (auto& c : g.camps) if (c.hp > 0 && c.str > 0) ahp += c.hp;
        cap = ahp <= ST4_PREHP;
    }
    if (!cap) return false;
    const bool on = g.tiles().is_set(pos);
    if ((ST4_SW & 16) && s.type == champion_type::cassi) return false;   // the linked pair never holds (Cassi dies -> Issac dies)
    int eT = 0, aT = 0, eN = 0, aN = 1; float inc = 0;
    for (auto& e : vision::champions(team::enemy)) {
        if (!e.is_visible() || !e.is_alive()) continue;
        const vec2 p = e.position();
        if (st4_tdist(g, p) <= ST4_OR) ++eN;
        if (g.tiles().is_set(p)) ++eT;
        if (cheb(p, pos) <= static_cast<int>(e.attack_range()) + 1) inc += cxh_attack(e) + champ_burst(e);
    }
    for (auto& a : vision::champions(team::ally)) {
        if (!a.is_visible() || !a.is_alive() || a.position() == pos) continue;
        if (g.tiles().is_set(a.position())) ++aT;
        if (st4_tdist(g, a.position()) <= ST4_OR) ++aN;
    }
    if ((ST4_SW & 32) && eN > aN + ST4_OM) return false;   // outnumbered around the herd: the normal controller decides
    if (inc >= hp || (hp < ST4_LHP * maxhp && inc > 0)) return false;   // lethal / low: the normal controller leaves
    if (!on) {
        if (hp < ST4_EHP * maxhp) return false;
        if (eT == 0 && aT >= ST4_N) return false;   // support: normal pvp_t / capture-wait code
        if (me.can_move()) {
            s.goals = g.tiles(); s.goals &= ~s.terrain;
            step_to_goals(true);
        }
    }
    s.target = h;
    const bool now_on = g.tiles().is_set(me.position());
    publish(h, now_on ? 2 : 1);
    if (now_on) ++s.channel;
    // attack from where we stand
    const vec2 p2 = me.position();
    std::optional<champion> best, spell_target;
    float bk = 1e18f, sk = 1e18f;
    for (auto& e : vision::champions(team::enemy)) {
        if (!e.is_visible() || !e.is_alive()) continue;
        const int d = cheb(p2, e.position());
        if (d > 4) continue;
        const float k = e.health() - (((ST4_SW & 2) && g.tiles().is_set(e.position())) ? 1e5f : 0.0f);
        if (k < sk) { sk = k; spell_target = e; }
        if (d <= s.range && k < bk) { bk = k; best = e; }
    }
    try {
        if (spell_target && !s.cast && me.can_cast()) {
            for (auto& ab : me.abilities()) {
                if (!ab.is_ready() || ab.is_dash() || ab.is_heal() || ab.is_attack_reset()) continue;
                bool ok = false;
                if (ab.is_shield()) ok = ab.use();
                else if (ab.can_target_ground()) ok = ab.use(spell_target->position());
                else if (ab.can_target_enemies()) ok = ab.use(*spell_target);
                if (ok) { s.cast = true; ++s.casts; break; }
            }
        }
        if (best && best->is_visible() && best->is_alive() && me.can_attack()) { me.attack(*best); ++s.attacks; }
        else if (!best && me.can_attack()) {   // no enemy in range: hit a living camp in range (pre-entry)
            Camp* b = nullptr;
            for (auto& c : g.camps) if (c.hp > 0 && cheb(p2, c.pos) <= s.range && c.h.is_visible() && (!b || c.hp < b->hp)) b = &c;
            if (b) attack_camp(*b);
        }
    } catch (const cambc::error&) {}
    ++s.t_cap;
    return true;
}
#endif

template <bool SR> bool pvp_t(float hp, float maxhp) {
#if CXK_MAPS
    if ((CXK_MAPS & 0x200) && nxm()==9) {
        auto& k=*S;auto& me=self();
        const bool loaded=static_cast<int>(me.gem_count())>=CXK_GEMS;
        if ((CXK_SW & 2) && loaded && (k.mode==Mode::bank || k.mode==Mode::recall)) return false;
        if ((CXK_SW & 1) && cxk2_keeper && k.mode==Mode::farm && k.target>=0 && hp>=.65f*maxhp) {
            const auto& g=k.groups[k.target];bool working=false;
            for(const auto& c:g.camps)if(cheb(me.position(),c.pos)<=k.range+1)working=true;
            int near=99;for(const auto& e:vision::champions(team::enemy))if(e.is_visible() && e.is_alive())near=std::min(near,cheb(me.position(),e.position()));
            const bool empty_worker=CXK_PROD==1 || me.gem_count()==0;
            const bool living_target=CXK_PROD<3 || g.st==0;
            const bool own_loop=CXK_PROD!=5 || g.spawn_dist<=18;
            if(working && g.st!=2 && near>(CXK_PROD==4?4:3) && empty_worker && living_target && own_loop)return false;
        }
    }
#endif

#if SF_ON
    {
        const auto t = S->type;
        const int bit = t == champion_type::stellar ? 1 : t == champion_type::tavi ? 2 : t == champion_type::cassi ? 4 : 8;
        if (((((SF_WHOM >> sf_map()) & 1) ? 15 : (g_mx_cp ? 0 : ((NXB(FD_WHOMAPS) ? SF_WHO : 0) | (NXB(FD_WHOMAPS2) ? FD_WHO2 : 0)))) | ((((CP_WHOM >> cp_map()) & 1) || (NX_CPX && ((NX_CPX >> nxm()) & 1))) ? CP_WHO : 0)) & bit) return sf_pvp(hp, maxhp);
    }
#endif
    auto& s = *S;
    auto& me = self();
    const vec2 pos = me.position();
    const auto enemies = vision::champions(team::enemy);
    float foe = 0, worst_burst = 0;
#if TF_ON
    // tf: power sums and enemy positions gathered inside the existing loops (no extra host calls: fuel).
    const bool tfon = tf_on();
    float tf_foe = 0.0f, tf_ours = 0.0f, eh_c = -1.0f, ed_c = 0.0f, eh_i = -1.0f, ed_i = 0.0f, ah_c = -1.0f, ad_c = 0.0f, ah_i = -1.0f, ad_i = 0.0f;
    std::array<vec2, 8> tf_ep{}; int tf_ne = 0;
#endif
    int nearest = 99;
    vec2 nearest_pos{};
    float weakest = 1e18f;
    vec2 weakest_pos{};
#if BL_ON
    float bl_dmg = 0;
#endif
#if FU_SNAP
    std::array<vec2, 16> fu_ep{}; int fu_ne = 0;
    auto fu_snap = [&] { fu_ne = 0; for (auto& e : enemies) if (e.is_visible() && e.is_alive() && fu_ne < 16) fu_ep[fu_ne++] = e.position(); };
#endif
    for (auto& e : enemies) {
#if FU_SNAP
        if (!e.is_visible() || !e.is_alive()) continue;
        const vec2 p = e.position();
        if (fu_ne < 16) fu_ep[fu_ne++] = p;
        if (SR && sr_skip(e)) continue;
#else
        if (!e.is_visible() || !e.is_alive() || (SR && sr_skip(e))) continue;
        const vec2 p = e.position();
#endif
        const int d = cheb(pos, p);
        if (d > 6) continue;
#if BL_ON
        if (BL_L) { int sr = 0; bl_dmg += bl_threat(e, me, d, sr); }
#endif
        const float ehp = cxh_health(e);
#if TF_ON
        const float edps = champ_dps(e);
        foe += ehp * edps;
        if (tfon) {
            if (tf_ne < 8) tf_ep[tf_ne++] = p;
            const auto et = e.champion_name();
            if (et == champion_type::cassi) { eh_c = ehp; ed_c = edps; }
            else if (et == champion_type::issac) { eh_i = ehp; ed_i = edps; }
            else tf_foe += ehp * edps;
        }
#else
        foe += ehp * champ_dps(e);
#endif
        if (d <= 5) worst_burst = std::max(worst_burst, champ_burst(e));
        if (d < nearest) { nearest = d; nearest_pos = p; }
        if (mx_mcxB()) {   // sf: chase key prefers carriers, isolated enemies and Cassi (top teams kill the pair through her).
            int iso = 0;
            for (auto& o : enemies) if (o.is_visible() && o.is_alive() && o.position() != p && cheb(o.position(), p) <= 4) ++iso;
            int g = 0;
            try { g = static_cast<int>(e.gem_count()); } catch (const cambc::error&) {}
            const float key = ehp - 10.0f * std::min(g, 60) - (iso == 0 ? SF_ISO * 0.6f : 0.0f) - (e.champion_name() == champion_type::cassi ? 300.0f : 0.0f);
            if (d <= SF_CHR && key < weakest) { weakest = key; weakest_pos = p; }
        } else
        if (d <= 4 && ehp < weakest) { weakest = ehp; weakest_pos = p; }
    }
    if (nearest == 99) {
#if FB_CHASE
        s.fb_chase = 0;
#endif
        return false;
    }
    s.enemy_round = s.round;
#if BL_ON
    bool bl_leave = false;
    if (BL_L) {
        if (bl_dmg >= BL_FRAC * hp || hp - bl_dmg < BL_FLOOR * maxhp) { bl_leave = true; s.bl_hold = s.round + BL_HOLD; }
        else if (s.round < s.bl_hold) bl_leave = true;
#if P_LOG
        if (bl_leave) std::cout << "BL " << s.round << " " << static_cast<int>(s.type) << " hp " << static_cast<int>(hp) << " dmg " << static_cast<int>(bl_dmg) << " near " << nearest << "\n";
#endif
    }
#else
    const bool bl_leave = false;
#endif
#if TF_ON
    const float mydps = champ_dps(me);
    float ours = cxh_health(me) * mydps;
    if (tfon) { if (s.type == champion_type::cassi) { ah_c = hp; ad_c = mydps; } else if (s.type == champion_type::issac) { ah_i = hp; ad_i = mydps; } else tf_ours = hp * mydps; }
    for (auto& a : vision::champions(team::ally)) {
        if (!a.is_visible() || !a.is_alive()) continue;
        const vec2 p = a.position();
        if (p == pos) continue;
        const bool near_fight = cheb(p, nearest_pos) <= 5, tf_in = tfon && cheb(p, pos) <= TF_R;
        if (!near_fight && !tf_in) continue;
        const float ah = cxh_health(a), ad = champ_dps(a);
        if (near_fight) ours += ah * ad;
        if (tf_in) {
            const auto at = a.champion_name();
            if (at == champion_type::cassi) { ah_c = ah; ad_c = ad; }
            else if (at == champion_type::issac) { ah_i = ah; ad_i = ad; }
            else tf_ours += ah * ad;
        }
    }
#else
    float ours = cxh_health(me) * champ_dps(me);
    for (auto& a : vision::champions(team::ally)) {
        if (!a.is_visible() || !a.is_alive()) continue;
        const vec2 p = a.position();
        if (p == pos || cheb(p, nearest_pos) > 5) continue;
        ours += cxh_health(a) * champ_dps(a);
    }
#endif
    (void)nearest_pos;
    bool fd_force = false, fd_nochase = false;
#if FD_NUM || FD_CG
    if (fd_on()) {
        int ne_n = 0, na_n = 1; bool alone = true;
#if FU_SNAP
        for (int i = 0; i < fu_ne; ++i) if (cheb(pos, fu_ep[i]) <= FD_NUMR) ++ne_n;
#else
        for (auto& e : enemies) if (e.is_visible() && e.is_alive() && cheb(pos, e.position()) <= FD_NUMR) ++ne_n;
#endif
        for (auto& a : vision::champions(team::ally)) {
            if (!a.is_visible() || !a.is_alive()) continue;
            const vec2 p = a.position();
            if (p == pos) continue;
            const int d = cheb(pos, p);
            if (d <= FD_NUMA) ++na_n;
            if (d <= FD_CR) alone = false;
        }
        const int hd0 = s.df_spawn.get(pos);
        const bool at_home = P_HOME > 0 && hd0 != UNREACH && hd0 <= P_HOME;
        const int tb = 1 << static_cast<int>(s.type);
        if (FD_NUM && (FD_NWHO & tb) && !at_home && ne_n > na_n + FD_NUMS && ours < FD_NUMP * foe) { if (FD_NUMC) fd_nochase = true; else fd_force = true; }
        if (FD_CG > 0 && (FD_CWHO & tb) && static_cast<int>(me.gem_count()) >= FD_CG && alone && nearest <= FD_CV && ours < 2.0f * foe) {
            fd_force = true;
            if (FD_CBANK && s.mode == Mode::farm) s.mode = Mode::bank;
        }
    }
#endif
#if TF_ON
    bool tf_leave = false, tf_weak = false;
    if (tfon) {
        tf_foe += tf_pair(eh_c, ed_c, eh_i, ed_i);
        tf_ours += tf_pair(ah_c, ad_c, ah_i, ad_i);
        const bool in_pvp = nearest <= 4 || s.round - s.hurt_round <= 2;
        const bool tf_low = hp < TF_LOWHP * maxhp;
        tf_weak = tf_ours < TF_FIGHT * tf_foe;
        if (in_pvp) {
            s.tf_pvp = s.round;
            if (tf_weak || tf_low) { s.tf_dis = s.round + TF_HOLD; tf_leave = true; }
        } else if (s.round < s.tf_dis && (tf_weak || tf_low)) tf_leave = true;
#if TF_LOG
        std::cout << "TF " << s.round << " " << static_cast<int>(s.type) << " hp " << static_cast<int>(hp) << " near " << nearest << " ours " << static_cast<int>(tf_ours / 1000)
                  << " foe " << static_cast<int>(tf_foe / 1000) << " pvp " << in_pvp << " leave " << tf_leave << " hold " << (s.round < s.tf_dis) << "\n";
#endif
    }
#endif
#if P_LOG
#if FU_ELOGF
    if (fuel_remaining() >= static_cast<std::uint64_t>(FU_ELOGF)) {
#endif
    std::cout << "E " << s.round << " " << static_cast<int>(s.type) << " hp " << static_cast<int>(hp) << "/" << static_cast<int>(maxhp) << " g " << me.gem_count()
              << " at " << pos.x << "," << pos.y << " m " << static_cast<int>(s.mode) << " t " << s.target << " ours " << static_cast<int>(ours / 1000) << " foe " << static_cast<int>(foe / 1000) << " |";
    for (auto& e : enemies) {
        if (!e.is_visible() || !e.is_alive()) continue;
        std::cout << " " << static_cast<int>(e.champion_name()) << ":" << static_cast<int>(e.health()) << ":" << static_cast<int>(cxh_attack(e)) << "/" << static_cast<int>(e.focus()) << ":" << cheb(pos, e.position());
    }
    std::cout << "\n";
#if FU_ELOGF
    }
#endif
#endif
    bool sf_rflee = false;
    if (mx_mcxB() && s.ranged) {
        // A ranged attack ends movement: when we are about to leave, step first and attack from the new tile.
        const float fr = (s.mode == Mode::bank && s.bank_unsafe_round < s.round - 1) ? P_BANKFLEE : (g_mm_flee > 0.0f ? g_mm_flee : P_FLEE);
        const bool cr = me.gem_count() >= P_CARRY_FLEE && ours < P_CARRY_RATIO * foe;
        const int hd = s.df_spawn.get(pos);
        const bool hm = P_HOME > 0 && hd != UNREACH && hd <= P_HOME;
        const bool om = hm ? hp < P_HOME_HP * maxhp : (ours < fr * foe || cr);
        const bool lt = P_LETHAL > 0 && worst_burst >= P_LETHAL * hp;
        const bool rt = nearest <= 3 && hp < (s.type == champion_type::issac ? FF_ROT_I : FF_ROT) * maxhp;
        sf_rflee = (om || lt || rt || fd_force || bl_leave) && (nearest <= 5 || bl_leave) && me.can_move();
    }
#if TF_ON
    // tf: a disengaging champion steps first (a ranged attack would end its movement) and attacks from the new tile.
    if (tfon && s.ranged && nearest <= 5 && me.can_move()) {   // melee keeps hitting while it steps
        const float tf_fr = (s.mode == Mode::bank && s.bank_unsafe_round < s.round - 1) ? P_BANKFLEE : (g_mm_flee > 0.0f ? g_mm_flee : P_FLEE);
        const bool tf_cr = me.gem_count() >= P_CARRY_FLEE && ours < P_CARRY_RATIO * foe;
        const int tf_hd = s.df_spawn.get(pos);
        const bool tf_hm = P_HOME > 0 && tf_hd != UNREACH && tf_hd <= P_HOME;
        const bool tf_lv = tf_dis() ? tf_leave : tf_hm ? hp < P_HOME_HP * maxhp : (ours < tf_fr * foe || tf_cr);
        const bool tf_lt = P_LETHAL > 0 && worst_burst >= P_LETHAL * hp;
        const bool tf_rt = nearest <= 3 && hp < (s.type == champion_type::issac ? FF_ROT_I : FF_ROT) * maxhp;
        if (tf_lv || tf_lt || tf_rt || fd_force) sf_rflee = true;
    }
    (void)tf_leave; (void)tf_weak;
#endif
#if RR_MASK
    bool rr_lv = false, rr_post = true;
    if (rr_on() && (RR_WHO & rr_bit(s.type)) && nearest <= 5 && me.can_move()) {
        int en4 = 0, al4 = 0, al3 = 0, enr = 0; float inc = 0.0f, kill_hp = 1e18f;
        for (auto& e : enemies) {
            if (!e.is_visible() || !e.is_alive() || (SR && sr_skip(e))) continue;
            const int d = cheb(pos, e.position());
            if (d <= 4) ++en4;
            if (d <= static_cast<int>(e.attack_range()) + 1) { ++enr; inc += cxh_attack(e) + champ_burst(e); }
            if (d <= s.range && cxh_health(e) < kill_hp) kill_hp = cxh_health(e);
        }
        for (auto& a : vision::champions(team::ally)) {
            if (!a.is_visible() || !a.is_alive()) continue;
            const vec2 p = a.position();
            if (p == pos) continue;
            const int d = cheb(pos, p);
            if (d <= 4) ++al4;
            if (d <= 3) ++al3;
        }
        const float fr = (s.mode == Mode::bank && s.bank_unsafe_round < s.round - 1) ? P_BANKFLEE : (g_mm_flee > 0.0f ? g_mm_flee : P_FLEE);
        const bool cr = me.gem_count() >= P_CARRY_FLEE && ours < P_CARRY_RATIO * foe;
        const int hd = s.df_spawn.get(pos);
        const bool hm = P_HOME > 0 && hd != UNREACH && hd <= P_HOME;
        const bool om = hm ? hp < P_HOME_HP * maxhp : (ours < fr * foe || cr);
        const bool lt = P_LETHAL > 0 && worst_burst >= P_LETHAL * hp;
        const bool rt = nearest <= 3 && hp < (s.type == champion_type::issac ? FF_ROT_I : FF_ROT) * maxhp;
        const bool ib = RR_INC > 0.0f && inc >= RR_INC * hp && al3 < enr;
        const bool outn = RR_B && en4 > al4 + 1;
        const bool killable = kill_hp <= cxh_attack(me) + champ_burst(me);
        rr_lv = ((RR_COND & 1) && om) || ((RR_COND & 2) && lt) || ((RR_COND & 4) && rt) || ((RR_COND & 8) && ib) || ((RR_COND & 16) && outn && !killable);
        if (rr_lv) { sf_rflee = true; rr_post = !outn || killable; }
#if RR_LOG
        std::cout << "RR " << s.round << " " << static_cast<int>(s.type) << " hp " << static_cast<int>(hp) << " en4 " << en4 << " al4 " << al4 << " enr " << enr
                  << " inc " << static_cast<int>(inc) << " om " << om << " lt " << lt << " rt " << rt << " ib " << ib << " outn " << outn << " kill " << killable << " lv " << rr_lv << "\n";
#endif
    }
#endif
    bool fish_swirl_stellar_retreat = false;
    if (s.W == 45 && s.H == 30 && s.type == champion_type::stellar
        && hp < 0.32f * maxhp && nearest <= 4 && me.can_move()) {
        const auto ep = draft::current_draft(team::enemy);
        fish_swirl_stellar_retreat = ep.size() >= 3 && ep[0] == champion_draft::ren
            && ep[1] == champion_draft::stellar
            && ep[2] == champion_draft::cassi_and_issac;
    }
#if Z316_ON
    auto z316_escape = [&](bool z316_early) -> bool {
        if (z316_early) {
            refresh_occ();
        const vec2 here = me.position();
        const int z316_initial = nearest;
        vec2 best = here;
        int best_d = 99;
        #if TF_ON
        if (tfon) { for (int k = 0; k < tf_ne; ++k) best_d = std::min(best_d, cheb(here, tf_ep[k])); } else
#endif
#if FU_SNAP
        for (int i = 0; i < fu_ne; ++i) best_d = std::min(best_d, cheb(here, fu_ep[i]));
#else
        for (auto& e : enemies) if (e.is_visible() && e.is_alive()) best_d = std::min(best_d, cheb(here, e.position()));
#endif
        int best_home = s.df_spawn.get(here);
        for (auto d : DIRS) {
            vec2 p{ here.x + d.x, here.y + d.y };
            if (!in_bounds(p) || s.occ.is_set(p)) continue;
            int nd = 99;
            #if TF_ON
            if (tfon) { for (int k = 0; k < tf_ne; ++k) nd = std::min(nd, cheb(p, tf_ep[k])); } else
#endif
#if FU_SNAP
            for (int i = 0; i < fu_ne; ++i) nd = std::min(nd, cheb(p, fu_ep[i]));
#else
            for (auto& e : enemies) if (e.is_visible() && e.is_alive()) nd = std::min(nd, cheb(p, e.position()));
#endif
            const int home = s.df_spawn.get(p);
            if (nd > best_d || (nd == best_d && home < best_home)) { best = p; best_d = nd; best_home = home; }
        }
        if (best == here || best_d <= z316_initial || s.terrain.is_set(best)) return false;
        }
        // Outmatched: step away, preferring the way home. Dash first when we have one.
        if (s.coll && s.df_altar.get(pos) <= 6) CO.hot_flee_round = s.round;
        refresh_occ();
#if P_DASHFLEE
        if (!s.cast && me.can_cast()) {
            const auto abilities = me.abilities();
            for (auto& ab : abilities) {
                if (!ab.is_dash() || !ab.is_ready()) continue;
                vec2 dbest = pos;
                int dbd = nearest, dhome = s.df_spawn.get(pos);
                for (int dx = -2; dx <= 2; ++dx) for (int dy = -2; dy <= 2; ++dy) {
                    vec2 q{ pos.x + dx, pos.y + dy };
                    if (!in_bounds(q) || s.occ.is_set(q)) continue;
                    const int home = s.df_spawn.get(q);
                    if (home == UNREACH) continue;
                    int nd = 99;
                    #if TF_ON
                    if (tfon) { for (int k = 0; k < tf_ne; ++k) nd = std::min(nd, cheb(q, tf_ep[k])); } else
#endif
#if FU_SNAP
                    for (int i = 0; i < fu_ne; ++i) nd = std::min(nd, cheb(q, fu_ep[i]));
#else
                    for (auto& e : enemies) if (e.is_visible() && e.is_alive()) nd = std::min(nd, cheb(q, e.position()));
#endif
                    if (nd > dbd || (nd == dbd && home < dhome)) { dbest = q; dbd = nd; dhome = home; }
                }
                if (dbest != pos) {
#if FU_SNAP
                    try { if (ab.use(dbest)) { s.cast = true; ++s.casts; fu_snap(); } } catch (const cambc::error&) {}
#else
                    try { if (ab.use(dbest)) { s.cast = true; ++s.casts; } } catch (const cambc::error&) {}
#endif
                }
                break;
            }
        }
#endif
#if RR_MASK && RR_KITE
        if (rr_lv && s.ranged && me.can_move()) {
            const vec2 h0 = me.position();
            vec2 kb = h0; int kd = -1, kh = 1 << 30;
            for (auto d : DIRS) {
                vec2 p{ h0.x + d.x, h0.y + d.y };
                if (!in_bounds(p) || s.occ.is_set(p)) continue;
                int nd = 99; bool inr = false;
                for (auto& e : enemies) if (e.is_visible() && e.is_alive()) { const int q = cheb(p, e.position()); nd = std::min(nd, q); if (q <= s.range) inr = true; }
                if (!inr) continue;
                const int hm = s.df_spawn.get(p);
                if (nd > kd || (nd == kd && hm < kh)) { kb = p; kd = nd; kh = hm; }
            }
            if (kb != h0 && kd >= nearest) { try { if (me.move_absolute(kb)) { s.moved = true; ++s.moves; } } catch (const cambc::error&) {} }
        }
#endif
        const vec2 here = me.position();
        vec2 best = here;
        int best_d = 99;
        #if TF_ON
        if (tfon) { for (int k = 0; k < tf_ne; ++k) best_d = std::min(best_d, cheb(here, tf_ep[k])); } else
#endif
#if FU_SNAP
        for (int i = 0; i < fu_ne; ++i) best_d = std::min(best_d, cheb(here, fu_ep[i]));
#else
        for (auto& e : enemies) if (e.is_visible() && e.is_alive()) best_d = std::min(best_d, cheb(here, e.position()));
#endif
        int best_home = s.df_spawn.get(here);
        for (auto d : DIRS) {
            vec2 p{ here.x + d.x, here.y + d.y };
            if (!in_bounds(p) || s.occ.is_set(p)) continue;
            int nd = 99;
            #if TF_ON
            if (tfon) { for (int k = 0; k < tf_ne; ++k) nd = std::min(nd, cheb(p, tf_ep[k])); } else
#endif
#if FU_SNAP
            for (int i = 0; i < fu_ne; ++i) nd = std::min(nd, cheb(p, fu_ep[i]));
#else
            for (auto& e : enemies) if (e.is_visible() && e.is_alive()) nd = std::min(nd, cheb(p, e.position()));
#endif
            const int home = s.df_spawn.get(p);
            if (nd > best_d || (nd == best_d && home < best_home)) { best = p; best_d = nd; best_home = home; }
        }
        if (best != here && me.can_move()) {
            try { if (me.move_absolute(best)) { s.moved = true; ++s.moves; } } catch (const cambc::error&) {}
        }
#if RR_MASK
        if (sf_rflee && rr_post) fight_enemy_t<SR>();
#else
        if (sf_rflee) fight_enemy_t<SR>();
#endif
        if (z316_early && me.position() == pos) return false;
        publish(-1, 0);
        return true;
    };
#line 1 "z319_extension_z114_policy.cpp"
#if Z319_ANY
    if ((z316_priority<SR>(hp, maxhp) || z319_priority<SR>(hp, maxhp)) && z316_escape(true)) return true;
#else
#line 3990 "z114_policy.cpp"
    if (z316_priority<SR>(hp, maxhp) && z316_escape(true)) return true;
#line 1 "z319_extension_z114_policy.cpp"
#endif
#line 3991 "z114_policy.cpp"
    const bool acted = (s.mode == Mode::bank && s.ranged) || sf_rflee
        || fish_swirl_stellar_retreat ? false : fight_enemy_t<SR>();

    const bool carrying = me.gem_count() >= P_CARRY_FLEE && ours < P_CARRY_RATIO * foe;
    const float flee_ratio = (s.mode == Mode::bank && s.bank_unsafe_round < s.round - 1) ? P_BANKFLEE : (g_mm_flee > 0.0f ? g_mm_flee : P_FLEE);
    // A spell that would nearly kill us outright is a threat whatever the health totals say.
    const bool lethal = P_LETHAL > 0 && worst_burst >= P_LETHAL * hp;
    // Next to our own spawn we can always step back and heal, so we hold our ground there.
    const int home_d = s.df_spawn.get(pos);
    const bool home = P_HOME > 0 && home_d != UNREACH && home_d <= P_HOME;
#if TF_ON
    const bool outmatched = (tfon && tf_dis()) ? tf_leave : home ? hp < P_HOME_HP * maxhp : (ours < flee_ratio * foe || carrying);
#else
    const bool outmatched = home ? hp < P_HOME_HP * maxhp : (ours < flee_ratio * foe || carrying);
#endif
    // Rotate out: a body low on health leaves the front before it is burst (Issac earlier, Cassi dies with him).
    const bool rotate = nearest <= 3 && hp < (s.type == champion_type::issac ? FF_ROT_I : FF_ROT) * maxhp;
    if ((outmatched || lethal || rotate || fd_force || fish_swirl_stellar_retreat || bl_leave
#if RR_MASK
         || rr_lv
#endif
#if DG_DIS > 0
         || (g_dgd && dg_disengage(hp, maxhp))
#endif
         || (s.W == 31 && s.H == 31 && early_packs_stellar_threat(hp, maxhp)))
        && (nearest <= 5 || fd_force || bl_leave) && me.can_move()) return z316_escape(false);

#else
    const bool acted = (s.mode == Mode::bank && s.ranged) || sf_rflee
        || fish_swirl_stellar_retreat ? false : fight_enemy_t<SR>();
    const bool carrying = me.gem_count() >= P_CARRY_FLEE && ours < P_CARRY_RATIO * foe;
    const float flee_ratio = (s.mode == Mode::bank && s.bank_unsafe_round < s.round - 1) ? P_BANKFLEE : (g_mm_flee > 0.0f ? g_mm_flee : P_FLEE);
    // A spell that would nearly kill us outright is a threat whatever the health totals say.
    const bool lethal = P_LETHAL > 0 && worst_burst >= P_LETHAL * hp;
    // Next to our own spawn we can always step back and heal, so we hold our ground there.
    const int home_d = s.df_spawn.get(pos);
    const bool home = P_HOME > 0 && home_d != UNREACH && home_d <= P_HOME;
#if TF_ON
    const bool outmatched = (tfon && tf_dis()) ? tf_leave : home ? hp < P_HOME_HP * maxhp : (ours < flee_ratio * foe || carrying);
#else
    const bool outmatched = home ? hp < P_HOME_HP * maxhp : (ours < flee_ratio * foe || carrying);
#endif
    // Rotate out: a body low on health leaves the front before it is burst (Issac earlier, Cassi dies with him).
    const bool rotate = nearest <= 3 && hp < (s.type == champion_type::issac ? FF_ROT_I : FF_ROT) * maxhp;
    if ((outmatched || lethal || rotate || fd_force || fish_swirl_stellar_retreat || bl_leave
#if RR_MASK
         || rr_lv
#endif
#if DG_DIS > 0
         || (g_dgd && dg_disengage(hp, maxhp))
#endif
         || (s.W == 31 && s.H == 31 && early_packs_stellar_threat(hp, maxhp)))
        && (nearest <= 5 || fd_force || bl_leave) && me.can_move()) {
        // Outmatched: step away, preferring the way home. Dash first when we have one.
        if (s.coll && s.df_altar.get(pos) <= 6) CO.hot_flee_round = s.round;
        refresh_occ();
#if P_DASHFLEE
        if (!s.cast && me.can_cast()) {
            const auto abilities = me.abilities();
            for (auto& ab : abilities) {
                if (!ab.is_dash() || !ab.is_ready()) continue;
                vec2 dbest = pos;
                int dbd = nearest, dhome = s.df_spawn.get(pos);
                for (int dx = -2; dx <= 2; ++dx) for (int dy = -2; dy <= 2; ++dy) {
                    vec2 q{ pos.x + dx, pos.y + dy };
                    if (!in_bounds(q) || s.occ.is_set(q)) continue;
                    const int home = s.df_spawn.get(q);
                    if (home == UNREACH) continue;
                    int nd = 99;
                    #if TF_ON
                    if (tfon) { for (int k = 0; k < tf_ne; ++k) nd = std::min(nd, cheb(q, tf_ep[k])); } else
#endif
#if FU_SNAP
                    for (int i = 0; i < fu_ne; ++i) nd = std::min(nd, cheb(q, fu_ep[i]));
#else
                    for (auto& e : enemies) if (e.is_visible() && e.is_alive()) nd = std::min(nd, cheb(q, e.position()));
#endif
                    if (nd > dbd || (nd == dbd && home < dhome)) { dbest = q; dbd = nd; dhome = home; }
                }
                if (dbest != pos) {
#if FU_SNAP
                    try { if (ab.use(dbest)) { s.cast = true; ++s.casts; fu_snap(); } } catch (const cambc::error&) {}
#else
                    try { if (ab.use(dbest)) { s.cast = true; ++s.casts; } } catch (const cambc::error&) {}
#endif
                }
                break;
            }
        }
#endif
#if RR_MASK && RR_KITE
        if (rr_lv && s.ranged && me.can_move()) {
            const vec2 h0 = me.position();
            vec2 kb = h0; int kd = -1, kh = 1 << 30;
            for (auto d : DIRS) {
                vec2 p{ h0.x + d.x, h0.y + d.y };
                if (!in_bounds(p) || s.occ.is_set(p)) continue;
                int nd = 99; bool inr = false;
                for (auto& e : enemies) if (e.is_visible() && e.is_alive()) { const int q = cheb(p, e.position()); nd = std::min(nd, q); if (q <= s.range) inr = true; }
                if (!inr) continue;
                const int hm = s.df_spawn.get(p);
                if (nd > kd || (nd == kd && hm < kh)) { kb = p; kd = nd; kh = hm; }
            }
            if (kb != h0 && kd >= nearest) { try { if (me.move_absolute(kb)) { s.moved = true; ++s.moves; } } catch (const cambc::error&) {} }
        }
#endif
        const vec2 here = me.position();
        vec2 best = here;
        int best_d = 99;
        #if TF_ON
        if (tfon) { for (int k = 0; k < tf_ne; ++k) best_d = std::min(best_d, cheb(here, tf_ep[k])); } else
#endif
#if FU_SNAP
        for (int i = 0; i < fu_ne; ++i) best_d = std::min(best_d, cheb(here, fu_ep[i]));
#else
        for (auto& e : enemies) if (e.is_visible() && e.is_alive()) best_d = std::min(best_d, cheb(here, e.position()));
#endif
        int best_home = s.df_spawn.get(here);
        for (auto d : DIRS) {
            vec2 p{ here.x + d.x, here.y + d.y };
            if (!in_bounds(p) || s.occ.is_set(p)) continue;
            int nd = 99;
            #if TF_ON
            if (tfon) { for (int k = 0; k < tf_ne; ++k) nd = std::min(nd, cheb(p, tf_ep[k])); } else
#endif
#if FU_SNAP
            for (int i = 0; i < fu_ne; ++i) nd = std::min(nd, cheb(p, fu_ep[i]));
#else
            for (auto& e : enemies) if (e.is_visible() && e.is_alive()) nd = std::min(nd, cheb(p, e.position()));
#endif
            const int home = s.df_spawn.get(p);
            if (nd > best_d || (nd == best_d && home < best_home)) { best = p; best_d = nd; best_home = home; }
        }
        if (best != here && me.can_move()) {
            try { if (me.move_absolute(best)) { s.moved = true; ++s.moves; } } catch (const cambc::error&) {}
        }
#if RR_MASK
        if (sf_rflee && rr_post) fight_enemy_t<SR>();
#else
        if (sf_rflee) fight_enemy_t<SR>();
#endif
        publish(-1, 0);
        return true;
    }
#endif
    // Press the attack on whoever pins us at home or blocks a stalled bank run.
#if ED_MASK || ED_MASK2
    const bool press = home || (P_PRESS > 0 && s.mode == Mode::bank && s.bank_turns >= P_PRESS && ours >= foe && !s.ed_on);
#else
    const bool press = home || (P_PRESS > 0 && s.mode == Mode::bank && s.bank_turns >= P_PRESS && ours >= foe);
#endif
    if (s.mode == Mode::bank && !press) return false;
#if FB_CHASE
    if (acted) { s.fb_chase = 0; return true; }
    // A chase that has not produced an attack in FB_CHASE turns is a kiting decoy: go back to farming.
    if (!home && !press && s.fb_chase >= FB_CHASE && s.round - s.hurt_round > 1 && (FB_CHASEMAP == 0 || s.W * s.H >= 6400)) return false;
#else
    if (acted) return true;
#endif
    if (CX.on && !press && !cx_chase_ok(pos)) return false;
    if (fd_nochase && !press) return false;
#if TF_ON
    // tf: no chase while the local power is unfavourable or within TF_HOLD rounds of a disengage.
    if (tfon && tf_dis() && !press && (tf_weak || s.round < s.tf_dis)) return false;
#endif
    if ((ours >= P_CHASE * foe || home) && (weakest < 1e17f || press) && me.can_move() && hp > 0.5f * maxhp) {
#if FB_CHASE
        ++s.fb_chase;
#endif
        s.goals.clear();
        s.goals.draw_chebyshev(weakest < 1e17f ? weakest_pos : nearest_pos, static_cast<float>(s.range));
        if (step_to_goals(false)) {
            if (!s.ranged) fight_enemy_t<SR>();
            return true;
        }
    }
    return false;
}

// ---- sf_hunter (17 Oct): test opponent that fights like the top caster teams ----
// Ranged champions kite (move first, then attack: a ranged attack ends movement), everyone focuses one target
// shared through shared memory, hunts isolated or gem-carrying enemies when the local numbers allow it, and
// leaves early (SF_BACK). Only used by the sf_* test opponents, never submitted.
#if SF_ON
static int sf_hunt_r = -99;
__attribute__((noinline)) bool sf_pvp_inner(float hp, float maxhp) {
    auto& s = *S;
    auto& me = self();
    const vec2 pos = me.position();
    struct E { vec2 p; float hp, mhp, dps, burst; int d, gems, iso; bool melee, cassi; };
    std::array<E, 8> es{};
    int ne = 0;
#if BL_ON
    float bl_dmg = 0;
#endif
    for (auto& e : vision::champions(team::enemy)) {
        if (!e.is_visible() || !e.is_alive() || ne >= 8) continue;
        const vec2 p = e.position();
        const int d = cheb(pos, p);
#if BL_ON
        if (BL_L && d <= 6) { int sr = 0; bl_dmg += bl_threat(e, me, d, sr); }
#endif
        if (d > SF_HUNTR) continue;
        int g = 0;
        try { g = static_cast<int>(e.gem_count()); } catch (const cambc::error&) {}
        es[ne++] = E{ p, cxh_health(e), e.max_health(), champ_dps(e), champ_burst(e), d, g, 0, e.attack_range() <= 1,
                      e.champion_name() == champion_type::cassi };
    }
    // Shared hunt call: bytes SF_SHM..+2 = [round][x][y] of the team's focus target.
    // (P_LOCK must be 0: bytes 6-7 then carry [round:6][y/3:5][x/3:5].)
    const unsigned cv = static_cast<unsigned>(static_cast<std::uint8_t>(shared_memory[SF_SHM])) | (static_cast<unsigned>(static_cast<std::uint8_t>(shared_memory[SF_SHM + 1])) << 8);
    const vec2 call{ std::min<int>(s.W - 1, static_cast<int>(cv & 31) * 3 + 1), std::min<int>(s.H - 1, static_cast<int>((cv >> 5) & 31) * 3 + 1) };
    const bool call_fresh = SF_CALL && cv != 0 && ((s.round - static_cast<int>(cv >> 10)) & 63) <= 2;
    if (ne == 0) {
        s.fb_chase = 0;
        // Answer a fresh call when healthy and not carrying much.
        if (SF_CALL && call_fresh && hp > SF_CALLHP * maxhp && s.mode == Mode::farm && static_cast<int>(me.gem_count()) < SF_CALLG
            && cheb(pos, call) <= SF_CALLR && cheb(pos, call) > s.range
            && !(s.target >= 0 && s.groups[s.target].st == 1 && s.groups[s.target].tiles().is_set(pos))) {
            s.goals.clear();
            s.goals.draw_chebyshev(call, static_cast<float>(std::max(1, s.range)));
            s.goals &= ~s.terrain;
            if (step_to_goals(true)) return true;
        }
        return false;
    }
    std::array<vec2, 6> ally{};
    std::array<float, 6> apow{}, ahp{}, adps{};
    int na = 0;
    for (auto& a : vision::champions(team::ally)) {
        if (!a.is_visible() || !a.is_alive() || na >= 6) continue;
        const vec2 p = a.position();
        if (p == pos) continue;
        ally[na] = p; ahp[na] = cxh_health(a); adps[na] = champ_dps(a); apow[na] = ahp[na] * adps[na]; ++na;
    }
    for (int i = 0; i < ne; ++i)
        for (int j = 0; j < ne; ++j) if (j != i && cheb(es[i].p, es[j].p) <= 4) ++es[i].iso;
    const float mypow = cxh_health(me) * champ_dps(me);
    // Power around a point: ours within 5 (incl. me if within 5), theirs within 5.
    auto ours_at = [&](vec2 q) {
#if SF_LAN
        float h = 0, d = 0;
        if (cheb(pos, q) <= SF_OWNR + 1) { h += hp; d += champ_dps(me); }
        for (int k = 0; k < na; ++k) if (cheb(ally[k], q) <= SF_OWNR) { h += ahp[k]; d += adps[k]; }
        return h * d;
#else
        float v = cheb(pos, q) <= SF_OWNR + 1 ? mypow : 0.0f;
        for (int k = 0; k < na; ++k) if (cheb(ally[k], q) <= SF_OWNR) v += apow[k];
        return v;
#endif
    };
    auto foe_at = [&](vec2 q) {
#if SF_LAN
        float h = 0, d = 0;
        for (int i = 0; i < ne; ++i) if (cheb(es[i].p, q) <= 5) { h += es[i].hp; d += es[i].dps; }
        return h * d;
#else
        float v = 0;
        for (int i = 0; i < ne; ++i) if (cheb(es[i].p, q) <= 5) v += es[i].hp * es[i].dps;
        return v;
#endif
    };
    int nearest = 99, nearest_melee = 99;
    float worst_burst = 0;
    for (int i = 0; i < ne; ++i) {
        nearest = std::min(nearest, es[i].d);
        if (es[i].melee) nearest_melee = std::min(nearest_melee, es[i].d);
        if (es[i].d <= 5) worst_burst = std::max(worst_burst, es[i].burst);
    }
    if (nearest <= 6) s.enemy_round = s.round;
    const float ours_here = ours_at(pos), foe_here = foe_at(pos);
    const bool outmatched = nearest <= 5 && ours_here < SF_FLEE * foe_here;
    const bool low = hp < SF_BACK * maxhp;
    const bool lethal = P_LETHAL > 0 && worst_burst >= P_LETHAL * hp;
    const bool carrying = static_cast<int>(me.gem_count()) >= SF_CARRY;

    // Focus target: carriers, isolated, low effective HP, Cassi; only where our local power beats theirs.
    int ti = -1; float tv = -1e18f;
    for (int i = 0; i < ne; ++i) {
        const float ratio = ours_at(es[i].p) / std::max(1.0f, foe_at(es[i].p));
        if (ratio < SF_HUNT) continue;
        float v = 1600.0f - es[i].hp + 12.0f * std::min(es[i].gems, 60) + (es[i].iso == 0 ? SF_ISO : 0.0f)
                + (es[i].cassi ? 250.0f : 0.0f) - 60.0f * es[i].d;
        if (call_fresh && cheb(es[i].p, call) <= 2) v += 300.0f;
        if (v > tv) { tv = v; ti = i; }
    }
    if (ti >= 0 && SF_CALL) {
        const unsigned v = (static_cast<unsigned>(s.round & 63) << 10) | (static_cast<unsigned>(es[ti].p.y / 3) << 5) | static_cast<unsigned>(es[ti].p.x / 3);
        shared_memory[SF_SHM] = static_cast<std::byte>(v & 255);
        shared_memory[SF_SHM + 1] = static_cast<std::byte>(v >> 8);
        sf_hunt_r = s.round;
    }

    auto attack_best = [&]() -> bool {
        if (!me.can_attack()) return false;
        std::optional<champion> best;
        float bk = 1e18f;
        for (auto& e : vision::champions(team::enemy)) {
            if (!e.is_visible() || !e.is_alive() || cheb(me.position(), e.position()) > s.range) continue;
            float k = cxh_health(e) * (e.champion_name() == champion_type::cassi ? 0.7f : 1.0f);
            if (ti >= 0 && e.position() == es[ti].p) k -= 5000.0f;
            if (k < bk) { bk = k; best = e; }
        }
        if (!best) return false;
        try { me.attack(*best); ++s.attacks; } catch (const cambc::error&) { return false; }
        if (!s.cast && me.can_cast() && best->is_visible() && best->is_alive()) {
            for (auto& ab : me.abilities())
                if (ab.is_attack_reset() && ab.is_ready()) {
                    try { if (ab.use()) { s.cast = true; ++s.casts; if (me.can_attack()) { me.attack(*best); ++s.attacks; } } } catch (const cambc::error&) {}
                    break;
                }
        }
        return true;
    };
    // Spell on a kill, then the focus target, then Cassi/low HP, within cast range 4.
    auto do_cast = [&](bool kill_only) -> bool {
        if (s.cast || s.moved || !me.can_cast()) return false;
        const auto abilities = me.abilities();
        int pick = -1; float pv = 0; vec2 tile{}; std::optional<champion> tgt;
        for (int i = 0; i < 2; ++i) {
            const auto& ab = abilities[i];
            if (!ab.is_ready() || ab.is_dash() || ab.is_heal() || ab.is_attack_reset()) continue;
            if (ab.is_shield()) {
                if (!kill_only && nearest <= 2 && s.round - s.hurt_round <= 1 && 300.0f > pv) { pv = 300.0f; pick = i; tgt.reset(); tile = vec2{ -1, -1 }; }
                continue;
            }
            const float dmg = ab.damage_amount();
            if (ab.can_target_ground()) {
                for (int a = 0; a < ne; ++a) {
                    if (es[a].d > 4) continue;
                    int n = 0; bool kills = false;
                    for (int b = 0; b < ne; ++b) if (cheb(es[a].p, es[b].p) <= 1) { ++n; if (es[b].hp <= dmg) kills = true; }
                    if (kill_only && !kills) continue;
                    const float v = dmg * n + (kills ? 2000.0f : 0.0f) + (ti == a ? 200.0f : 0.0f);
                    if (v > pv) { pv = v; pick = i; tile = es[a].p; tgt.reset(); }
                }
            } else if (ab.can_target_enemies()) {
                for (auto& e : vision::champions(team::enemy)) {
                    if (!e.is_visible() || !e.is_alive() || cheb(pos, e.position()) > 4) continue;
                    const bool kills = cxh_health(e) <= std::max(dmg, 1.0f);
                    if (kill_only && !kills) continue;
                    const float v = std::max(dmg, 150.0f) + (kills ? 2000.0f : 0.0f) + (e.champion_name() == champion_type::cassi ? 150.0f : 0.0f)
                                    + (ti >= 0 && e.position() == es[ti].p ? 400.0f : 0.0f) + 0.1f * std::max(0.0f, 1500.0f - e.health());
                    if (v > pv) { pv = v; pick = i; tgt = e; tile = vec2{ 0, 0 }; }
                }
            }
        }
        if (pick < 0) return false;
        try {
            const auto& ab = abilities[pick];
            const bool ok = tgt ? ab.use(*tgt) : (tile.x < 0 ? ab.use() : ab.use(tile));
            if (ok) { s.cast = true; ++s.casts; return true; }
        } catch (const cambc::error&) {}
        return false;
    };
    // One step scored by distance to melee/any enemy, allies, home.
    auto do_step = [&](bool fleeing, int keep) -> bool {
        if (s.moved || !me.can_move()) return false;
        refresh_occ();
        const vec2 here = me.position();
        auto score = [&](vec2 p) {
            int dm = 99, da = 99;
            for (int i = 0; i < ne; ++i) {
                const int d = cheb(p, es[i].p);
                da = std::min(da, d);
                if (es[i].melee) dm = std::min(dm, d);
            }
            float v = 4.0f * std::min(dm, fleeing ? 7 : keep) + 1.5f * std::min(da, fleeing ? 7 : keep);
            if (!fleeing && ti >= 0) { const int dt = cheb(p, es[ti].p); if (dt <= 4 && dt >= 3) v += 2.0f; }
            int dal = 99;
            for (int k = 0; k < na; ++k) dal = std::min(dal, cheb(p, ally[k]));
            if (dal <= 2) v += 1.0f;
            const int home = s.df_spawn.get(p);
            if (home != UNREACH && fleeing) v -= 0.4f * home;
            return v;
        };
        // Dash away when fleeing from melee.
        if (fleeing && nearest <= 3 && !s.cast && me.can_cast()) {
            for (auto& ab : me.abilities()) {
                if (!ab.is_dash() || !ab.is_ready()) continue;
                vec2 db = here; float dv = score(here) + 2.0f;
                for (int dx = -2; dx <= 2; ++dx) for (int dy = -2; dy <= 2; ++dy) {
                    vec2 q{ here.x + dx, here.y + dy };
                    if (!in_bounds(q) || s.occ.is_set(q) || s.df_spawn.get(q) == UNREACH) continue;
                    const float v = score(q);
                    if (v > dv) { dv = v; db = q; }
                }
                if (db != here) { try { if (ab.use(db)) { s.cast = true; ++s.casts; return true; } } catch (const cambc::error&) {} }
                break;
            }
        }
        vec2 best = here;
        float bv = score(here) + 0.01f;
        for (auto d : DIRS) {
            vec2 p{ here.x + d.x, here.y + d.y };
            if (!in_bounds(p) || s.occ.is_set(p) || s.terrain.is_set(p)) continue;
            const float v = score(p);
            if (v > bv) { bv = v; best = p; }
        }
        if (best == here) return false;
        try { if (me.move_absolute(best)) { s.moved = true; ++s.moves; return true; } } catch (const cambc::error&) {}
        return false;
    };

    // v2 (SF_BANKV): carriers keep walking to the altar unless a fight is actually on them.
    const bool leave0 = (SF_BANKV) ? (s.mode == Mode::bank ? (nearest <= 3 && (low || outmatched || lethal))
                                            : (nearest <= SF_LEAVED && (low || outmatched || lethal || (carrying && ours_here < SF_CARRYR * foe_here))))
                                  : (nearest <= 5 && (low || outmatched || lethal || (carrying && ours_here < SF_CARRYR * foe_here)));
#if BL_ON
    bool bl_leave = false;
    if (BL_L && nearest <= 6) {
        if (bl_dmg >= BL_FRAC * hp || hp - bl_dmg < BL_FLOOR * maxhp) { bl_leave = true; s.bl_hold = s.round + BL_HOLD; }
        else if (s.round < s.bl_hold) bl_leave = true;
#if P_LOG
        if (bl_leave) std::cout << "BL " << s.round << " " << static_cast<int>(s.type) << " hp " << static_cast<int>(hp) << " dmg " << static_cast<int>(bl_dmg) << " near " << nearest << " sf\n";
#endif
    }
    const bool leave = leave0 || bl_leave;
#else
    const bool leave = leave0;
#endif
#if SF_LOG
    std::cout << "SF " << s.round << " " << static_cast<int>(s.type) << " hp " << static_cast<int>(hp) << "/" << static_cast<int>(maxhp) << " g " << me.gem_count()
              << " m " << static_cast<int>(s.mode) << " ne " << ne << " near " << nearest << " ours " << static_cast<int>(ours_here / 1000) << " foe " << static_cast<int>(foe_here / 1000)
              << " ti " << ti << (ti >= 0 ? " td " : "") << (ti >= 0 ? es[ti].d : 0) << " lv " << leave << " lo " << low << " om " << outmatched << " |";
    for (int i = 0; i < ne; ++i) std::cout << " " << static_cast<int>(es[i].hp) << "@" << es[i].d << (es[i].melee ? "m" : "r") << "g" << es[i].gems << "i" << es[i].iso;
    std::cout << "\n";
#endif
    if (s.mode == Mode::bank && !leave) {
        // Carrier: step out of melee reach but keep walking otherwise.
        if (SF_MHIT) { if (!s.ranged) attack_best(); }
        if (SF_BANKV) {
        if (s.ranged && nearest <= s.range) attack_best();
        return false;
        }
        if (nearest_melee <= 2 && ours_here < 1.5f * foe_here) { const bool st = do_step(true, 4); if (s.ranged) attack_best(); return st; }
        if (ti < 0 || es[ti].d > s.range) return false;
    }
    if (leave) {
        if (s.coll && s.df_altar.get(pos) <= 6) CO.hot_flee_round = s.round;
        if (s.ranged) {
            if (!do_cast(true)) { if (!do_step(true, 7)) do_cast(false); attack_best(); }
        } else {
            // Melee: finish a kill in reach, else run (attack does not stop a melee step).
            bool killed = false;
            for (int i = 0; i < ne; ++i) if (es[i].d <= s.range && es[i].hp <= cxh_attack(me) + 1.0f) killed = true;
            bool dash_ready = false;
            for (auto& ab : me.abilities()) if (ab.is_dash() && ab.is_ready()) dash_ready = true;
            if (SF_TRADE > 0.0f && !killed && nearest_melee <= 1 && !dash_ready && hp > SF_TRADE * maxhp) {
                do_cast(false); attack_best(); publish(-1, 0); return true;
            }
            if (SF_MHIT) {
            // A melee attack does not end movement: hit whatever is in reach, then step away.
            attack_best();
            do_step(true, 7);
            (void)killed;
            } else {
            if (killed) attack_best();
            if (!do_step(true, 7) && !killed) attack_best();
            }
        }
        publish(-1, 0);
        return true;
    }
    if (s.ranged) {
        bool acted = do_cast(false);
        if (!s.cast && nearest_melee <= SF_KEEP - 1) { if (do_step(false, SF_KEEP)) acted = true; }
        if (attack_best()) acted = true;
        if (acted) { s.fb_chase = 0; return true; }
        // Close to range on the focus target.
        if (ti >= 0 && me.can_move() && hp > 0.55f * maxhp && es[ti].d > s.range) {
            s.goals.clear();
            s.goals.draw_chebyshev(es[ti].p, static_cast<float>(s.range));
            bitboard inner; inner.draw_chebyshev(es[ti].p, static_cast<float>(std::max(0, s.range - 2)));
            s.goals &= ~inner; s.goals &= ~s.terrain;
            if (step_to_goals(false)) { ++s.fb_chase; attack_best(); return true; }
        }
        return false;
    }
    // Melee.
    if (ti >= 0 && es[ti].d <= s.range) {
        do_cast(false);
        attack_best();
        return true;
    }
    if (ti >= 0 && me.can_move() && hp > 0.5f * maxhp) {
        // Dash onto a carrier / low target in dash + range.
        if (!s.cast && me.can_cast() && es[ti].d <= 2 + s.range + 1 && (es[ti].gems >= 10 || es[ti].hp < 0.5f * es[ti].mhp)) {
            for (auto& ab : me.abilities()) {
                if (!ab.is_dash() || !ab.is_ready()) continue;
                refresh_occ();
                vec2 db = pos; int dd = es[ti].d;
                for (int dx = -2; dx <= 2; ++dx) for (int dy = -2; dy <= 2; ++dy) {
                    vec2 q{ pos.x + dx, pos.y + dy };
                    if (!in_bounds(q) || s.occ.is_set(q)) continue;
                    const int d = cheb(q, es[ti].p);
                    if (d < dd && d >= 1) { dd = d; db = q; }
                }
                if (db != pos) { try { if (ab.use(db)) { s.cast = true; ++s.casts; } } catch (const cambc::error&) {} }
                break;
            }
        }
        s.goals.clear();
        s.goals.draw_chebyshev(es[ti].p, static_cast<float>(s.range));
        s.goals &= ~s.terrain;
        if (step_to_goals(false) || s.cast) { ++s.fb_chase; attack_best(); return true; }
    }
    // Something in reach anyway.
    if (nearest <= s.range) { do_cast(false); if (attack_best()) return true; }
    return false;
}
__attribute__((noinline)) bool sf_pvp(float hp, float maxhp) {
    try { return sf_pvp_inner(hp, maxhp); } catch (const cambc::error&) { return true; }
}
#endif

#if SF_ON && SF_RAID
// Raid: from SF_RAID0, Ren/Stellar/Varg farm the enemy half (their home camps and altar walks), where lone
// carriers and farmers are met; pvp() then hunts them.
static std::uint64_t sf_far = 0;
static bool sf_far_init = false;
__attribute__((noinline)) int sf_raid_pick(float hp, float maxhp, int gems) {
    auto& s = *S;
    const auto t = s.type;
    const bool raider = t == champion_type::ren || t == champion_type::stellar || (SF_RAID >= 2 && t == champion_type::varg)
        || (SF_RAID >= 3 && t == champion_type::issac);
    if (!raider || s.round < SF_RAID0 || hp < SF_RAIDHP * maxhp || gems >= SF_RAIDG) return -1;
    if (!sf_far_init) {
        sf_far_init = true;
        distance_field de;
        de.compute(map::walls() | map::holes() | map::altars(), map::spawn_region(team::enemy), s.visited);
        for (std::size_t gi = 0; gi < s.groups.size() && gi < 64; ++gi) {
            int own = UNREACH, foe = UNREACH;
            for (auto& c : s.groups[gi].camps) { own = std::min<int>(own, s.df_spawn.get(c.pos)); foe = std::min<int>(foe, de.get(c.pos)); }
            if (foe != UNREACH && own != UNREACH && foe * SF_RAIDF < own * 10) sf_far |= 1ull << gi;
        }
    }
    int best = -1; float bv = 0;
    for (std::size_t gi = 0; gi < s.groups.size() && gi < 64; ++gi) {
        if (!((sf_far >> gi) & 1ull)) continue;
        const auto& g = s.groups[gi];
        if (g.st == 2 || g.mdps >= 200.0f) continue;
        int d = UNREACH;
        for (auto& c : g.camps) d = std::min<int>(d, s.df_me.get(c.pos));
        if (d == UNREACH) continue;
        const float v = (g.gems + 0.3f * g.gold + 10.0f) / (d + 8.0f) * (g.st == 1 ? 1.5f : 1.0f) * (1.0f + SF_RAIDC * claims_on(static_cast<int>(gi), false));
        if (v > bv) { bv = v; best = static_cast<int>(gi); }
    }
    return best;
}
#endif
// ---- ce (claude-econ, 28 Oct; docs/claude-econ-2026-10-28.md) ----
// Mirror losses: opponents take 40+-gem non-monolith groups 2.5x as often (a third with 2-3 bodies); we take them
// solo 97% of the time. The solo damage penalty counts only published claims, so nobody commits first.
// Co-op: with >=2 healthy allies (me included) within 8 tiles, count them as helpers on such groups.
#include "ce_switch.h"
#ifndef CE_GW
#define CE_GW 0.2f
#endif
#ifndef CE_MING
#define CE_MING 40
#endif
#ifndef CE_W
#define CE_W 2.0f
#endif
static int g_ce = 0, ce_near = 0;
#ifndef CC5_ON
#define CC5_ON 0
#endif
#if CC5_ON
static bool cc5_gw = false;
#endif
static bool ce_job = false;
static void ce_tick() {
    auto& s = *S; auto& me = self();
    if (s.turns == 3) {
        int id = 0;
        if (s.W == 50 && s.H == 40 && draft::team_size() == 5) id = 9;
#if CC5_ON
        // cc_maps5 (28 Oct): exact 00great_wall (17 groups; herds 5/6 = 80 gems, 8 camps, strength 180) -> co-op on.
        cc5_gw = id == 9 && s.groups.size() == 17 && s.groups[5].gems == 80 && s.groups[5].camps.size() == 8
                 && s.groups[6].gems == 80 && s.groups[6].camps.size() == 8;
#endif
        g_ce = id;
    }
    ce_job = false; ce_near = 0;
    if (!g_ce || !(((CE_COOPMAPS >> (g_ce - 1)) & 1)
#if CC5_ON
        || cc5_gw
#endif
        )) return;
    if (!me.is_alive() || s.mode != Mode::farm || me.health() < .70f * me.max_health() || me.gem_count() >= 60) return;
    for (auto& a : vision::champions(team::ally))
        if (a.is_visible() && a.is_alive() && a.health() >= .70f * a.max_health() && cheb(a.position(), me.position()) <= 8) ++ce_near;
    ce_job = ce_near >= 2;
}

// ---- target choice ----
float my_dps(const Group& g) {
    auto& s = *S;
    auto& me = self();
    float d = cxh_attack(me) * 1.1f;
    if (s.type == champion_type::varg) d *= static_cast<float>(g.splash);
    return d;
}

// Once per turn: note enemies at the hub; with gems in hand while they hold it, bank at a corner altar.
// Per group: during the opening, skip groups on the enemy's side (hub included).
__attribute__((noinline)) bool cp_skip(const Group& g) {
    auto& s = *S;
    auto& me = self();
    if (!CP.ready) {
        CP.ready = true;
        CP.west = me.position().x < s.W / 2;
        int hx = 0, hy = 0, hn = 0;
        for (auto& a : s.groups) if (a.hub) { hx += a.camps[0].pos.x; hy += a.camps[0].pos.y; ++hn; }
        if (hn > 0) CP.hub = vec2{ (hx + hn / 2) / hn, (hy + hn / 2) / hn };
        for (std::size_t i = 0; i < s.altars.size(); ++i)
            if (hn > 0 && cheb(s.altars[i], CP.hub) <= 4) CP.centre = static_cast<int>(i);
#if MC_CPHUB
        // mc: no crystal hub (16 Oct corners): the hub is the altar nearest the map centre.
        if (hn == 0) {
            int best = 1 << 20;
            for (std::size_t i = 0; i < s.altars.size(); ++i) {
                const int d = std::abs(2 * s.altars[i].x - (s.W - 1)) + std::abs(2 * s.altars[i].y - (s.H - 1));
                if (d < best) { best = d; CP.centre = static_cast<int>(i); CP.hub = s.altars[i]; }
            }
        }
#endif
    }
    if (CP.turn != s.round) {
        CP.turn = s.round;
        int en = 0;
        for (auto& e : vision::champions(team::enemy))
            if (e.is_visible() && e.is_alive() && cheb(e.position(), CP.hub) <= CP_HUBR) ++en;
        if (en >= CP_HOTN) CP.hot_round = s.round;
        if (CP.centre >= 0 && me.gem_count() > 0) {
            const bool hot = s.round - CP.hot_round <= CP_BMEM;
            if (hot && s.bank_skip < 0 && cheb(me.position(), s.altars[CP.centre]) > constants::submit_gems_range) s.bank_skip = CP.centre;
        }
    }
    const int x = g.camps[0].pos.x;
    return s.round < CP_SAFE && (CP.west ? x > s.W / 2 - CP_MARGIN : x < s.W / 2 + CP_MARGIN);
}

// Interlock: visible living allies within five tiles.
__attribute__((noinline)) int il_allies_near(vec2 pos) {
    int n = 0;
    for (auto& a : vision::champions(team::ally)) {
        if (!a.is_visible() || !a.is_alive()) continue;
        const vec2 p = a.position();
        if (p == pos || cheb(p, pos) > IL_NEARR) continue;
        ++n;
    }
    return n;
}

// ---- ix (16 Oct, platform interlock since ~12:00Z: four 72-gem elk PAIRS, no crystals) ----
// Winners send 3-4 bodies to an elk pair from R35-180; we kept all four on three wolves until R330+.
// IX_ON: squad champions (all but the wolf keeper) value nearby elk pairs as a squad job and rally before engaging.
#ifndef IX_ON
#define IX_ON 0
#endif
#ifndef IX_N
#define IX_N 3        // squad size assumed when scoring an elk pair
#endif
#ifndef IX_W
#define IX_W 1.5f     // elk-pair score multiplier for squad members
#endif
#ifndef IX_SD
#define IX_SD 29      // only elk pairs within this spawn distance (N, M10, M29; not the enemy's F pair)
#endif
#ifndef IX_START
#define IX_START 0    // round from which the squad takes elk pairs
#endif
#ifndef IX_RALLY
#define IX_RALLY 2    // bodies (incl. me) near an untouched pair before engaging; 0 off
#endif
#ifndef IX_KEEP
#define IX_KEEP 2     // wolf keeper champion_type (2 = Vendetta); -1 none
#endif
#ifndef IX_LOG
#define IX_LOG 0
#endif
#ifndef IX_PACK
#define IX_PACK 0.0f  // interlock: score x (1 + IX_PACK x allies claiming) on fighting camps (pack farming)
#endif
#ifndef IX_SQ
#define IX_SQ 0       // squad as a champion_type bitmask (e.g. Varg|Ryth = 1025); 0 = all but IX_KEEP
#endif
static bool g_ix = false;      // platform interlock layout + IX_ON
static bool g_ixsq = false;    // this champion is in the elk squad
static std::uint64_t g_ixelk = 0; // elk-pair group bits
static int ix_wait_tgt = -1, ix_wait0 = 0, ix_cool = 0;
#ifndef IX_WAITMAX
#define IX_WAITMAX 30 // rounds to wait for the squad before dropping elk pairs for IX_COOL rounds
#endif
#ifndef IX_COOL
#define IX_COOL 40
#endif
#ifndef IX_MODE
#define IX_MODE 1     // 2: one squad elk target at a time; hold outside reach whenever fewer than IX_RALLY bodies
#endif
#ifndef IX_JOIN
#define IX_JOIN 2.0f  // IX_MODE 2: score multiplier for the elk pair a squad ally already claims
#endif
#ifndef IX_FIN
#define IX_FIN 0.25f  // IX_MODE 2: below this fraction of pair health, finish it without the rally
#endif
__attribute__((noinline)) void ix_init() {
    auto& s = *S;
    g_ix = false; g_ixsq = false; g_ixelk = 0;
    if (!IX_ON || !(s.W == 40 && s.H == 40 && draft::team_size() == 4)) return;
    int pairs = 0;
    for (std::size_t i = 0; i < s.groups.size() && i < 64; ++i) {
        auto& g = s.groups[i];
        if (g.camps.size() == 2 && g.gems == 72 && g.mdps >= 120 && g.spawn_dist <= IX_SD) { g_ixelk |= 1ull << i; ++pairs; }
    }
    if (pairs == 0) return;
    g_ix = true;
#if IX_LOG
    std::cout << "IXI shm " << constants::shared_memory_bytes << " idx " << s.idx << " type " << static_cast<int>(s.type) << "\n";
#endif
    g_ixsq = IX_SQ > 0 ? ((IX_SQ >> static_cast<int>(s.type)) & 1) != 0 : static_cast<int>(s.type) != IX_KEEP;
    if (IX_KEEP < 0 || !g_ixsq) return;
    // no keeper type in our draft: Issac keeps (Cassi follows him) -- else everyone is squad
}
__attribute__((noinline)) int ix_bodies_near(vec2 c, int r) {
    int n = 0;
    for (auto& a : champions(team::ally)) {
        try { if (!a.is_alive()) continue; if (cheb(a.position(), c) <= r) ++n; } catch (const cambc::error&) {}
    }
    return n;
}

#ifndef IX_OPHP
#define IX_OPHP 75    // IX_MODE 3: squad members at or above this health % count as ready
#endif
#ifndef IX_OPMAX
#define IX_OPMAX 90   // IX_MODE 3: abandon an assault after this many rounds
#endif
#ifndef IX_OPCOOL
#define IX_OPCOOL 30
#endif
#ifndef IX_OPG
#define IX_OPG 60     // IX_MODE 3: members carrying this many gems bank instead
#endif
// IX_MODE 3 shared memory: [8] op group+1, [9] op start round/4, [10+idx] squad member health % (0 dead / not squad).
static int ix_op_cool = 0;
__attribute__((noinline)) void ix_hp_pub(int pct) {
    auto& s = *S;
    if (constants::shared_memory_bytes < 16 || s.idx > 5) return;
    const std::uint8_t v = g_ixsq ? static_cast<std::uint8_t>(std::clamp(pct, 0, 100)) : 0;
    if (static_cast<std::uint8_t>(shared_memory[10 + s.idx]) != v) shared_memory[10 + s.idx] = static_cast<std::byte>(v);
}
__attribute__((noinline)) int ix_op_tick(float hp, float maxhp) {
    auto& s = *S;
    if (constants::shared_memory_bytes < 16 || s.idx > 5) return -1;
    (void)hp; (void)maxhp;
    int op = static_cast<int>(static_cast<std::uint8_t>(shared_memory[8])) - 1;
    // Leader: the lowest-slot living squad member.
    int lead = -1;
    for (int i = 0; i < 6; ++i) if (static_cast<std::uint8_t>(shared_memory[10 + i]) > 0) { lead = i; break; }
    if (lead != s.idx) return op;
    if (op >= 0) {
        const int age = ((s.round / 4) - static_cast<int>(static_cast<std::uint8_t>(shared_memory[9])) + 256) & 255;
        if (s.groups[op].st == 2 || age * 4 > IX_OPMAX) {
            if (s.groups[op].st != 2) ix_op_cool = s.round + IX_OPCOOL;
            shared_memory[8] = std::byte{0}; op = -1;
#if IX_LOG
            std::cout << "IXOP end " << s.round << "\n";
#endif
        }
        return op;
    }
    if (s.round < IX_START || s.round < ix_op_cool) return -1;
    int ready = 0;
    for (int i = 0; i < 6; ++i) if (static_cast<int>(static_cast<std::uint8_t>(shared_memory[10 + i])) >= IX_OPHP) ++ready;
    if (ready < IX_RALLY) return -1;
    int best = -1, bd = 1 << 20;
    for (int gi = 0; gi < static_cast<int>(s.groups.size()) && gi < 64; ++gi) {
        if (!((g_ixelk >> gi) & 1u) || s.groups[gi].st != 0) continue;
        if (s.groups[gi].spawn_dist < bd) { bd = s.groups[gi].spawn_dist; best = gi; }
    }
    if (best < 0) return -1;
    shared_memory[8] = static_cast<std::byte>(best + 1);
    shared_memory[9] = static_cast<std::byte>((s.round / 4) & 255);
#if IX_LOG
    std::cout << "IXOP start " << s.round << " g" << best << " ready " << ready << "\n";
#endif
    return best;
}

// Gold this champion holds or has spent on items.
__attribute__((noinline)) int fg_equity() {
    auto& me = self();
#if FG_PAD
    for (volatile int i = 0; i < FG_PAD; i = i + 1) {}
#endif
#if Z181_ON
    auto owned = me.item_count();
#else
    const auto owned = me.item_count();
#endif
    int v = static_cast<int>(me.gold_count());
    std::size_t k = 0;
    for (auto n : owned) { v += static_cast<int>(n) * ((k & 1) ? 150 : 30); ++k; }
    return v;
}

// Elk-class groups (a 60+ strength camp, not a monolith) take two trips for an unequipped champion:
// leave them until we have a monster emblem or FB_ELKHP max health, unless the group is already half dead.
__attribute__((noinline)) bool fb_elk_skip(const Group& g, float maxhp) {
    float top = 0, full = 0;
    for (auto& c : g.camps) { top = std::max(top, c.str); full += c.maxhp; }
    if (top < 60.0f || g.mdps >= 200.0f) return false;
    if (g.st == 0 && g.hp() < 0.5f * full) return false;
    if (maxhp >= FB_ELKHP) return false;
    const auto items = self().item_count();
    if (items[item_type::monster_emblem_of_strength] + items[item_type::monster_emblem_of_focus] + items[item_type::monster_emblem_of_health] > 0) return false;
    return true;
}

__attribute__((noinline)) int mackey_packs_fast_return(int choice, int banked);

// ---- pk: packs crystal-hub plan ----
struct PkState { bool init = false; bool north = true; bool safe = true; int turn = -1; int allies = -1; int hot[64]; };
static PkState PK;
__attribute__((noinline)) void pk_tick() {
    auto& s = *S;
    if (!PK.init) {
        PK.init = true;
        for (auto& h : PK.hot) h = -1000;
#if PK_SAFESEL
        {
            // Selector: hub safety only against Cassi & Issac + Stellar drafts (Ely2sh, Conley, Palestine, AlphaSlop, ...).
            const auto ep = draft::current_draft(team::enemy);
            const auto has = [&](champion_draft c) { return std::find(ep.begin(), ep.end(), c) != ep.end(); };
            PK.safe = has(champion_draft::cassi_and_issac) && has(champion_draft::stellar);
        }
#endif
        long sy = 0, n = 0;
        for (const vec2 q : map::spawn_region(team::ally).positions()) { sy += q.y; ++n; }
        PK.north = n == 0 || sy < n * (s.H / 2);
#if PK_LOG
        std::cout << "PKN " << n << " " << sy << " north " << PK.north << "\n";
#endif
    }
    if (PK.turn == s.round) return;
    PK.turn = s.round;
#if PK_LOG
    {
        static int last = 0;
        const int gm = static_cast<int>(self().gem_count());
        if (gm > last) std::cout << "PKG R" << s.round << " " << to_string(s.type) << " " << self().position().x << "," << self().position().y << " +" << (gm - last) << "\n";
        last = gm;
    }
#endif
    PK.allies = -1;
    for (auto& e : vision::champions(team::enemy)) {
        if (!e.is_visible() || !e.is_alive()) continue;
        const vec2 p = e.position();
        for (int gi = 0; gi < static_cast<int>(s.groups.size()) && gi < 64; ++gi)
            if (s.groups[gi].mdps == 0 && cheb(p, s.groups[gi].camps[0].pos) <= PK_HUBR) PK.hot[gi] = s.round;
    }
}
// Score multiplier for group gi on packs; 0 skips it.
__attribute__((noinline)) float pk_mult(int gi) {
    auto& s = *S;
    const auto& g = s.groups[gi];
    const vec2 c = g.camps[0].pos;
    const vec2 q = PK.north ? c : vec2{ s.W - 1 - c.x, s.H - 1 - c.y };
    const bool hubg = g.mdps == 0 && g.gems >= 40;
#if PK_MIMIC
    {
        const bool camper = s.type == champion_type::cassi || s.type == champion_type::issac
            || s.type == champion_type::stellar || s.type == champion_type::ren;
        if (camper) return hubg ? 6.0f : 0.4f;
        if (hubg) return 0.0f;
        return q.y >= 20 ? 0.0f : 1.0f;
    }
#endif
#if PK_FAR
    if (PK.safe && !hubg && q.y >= 20 && q.x >= 10) return 0.0f;
#endif
    float m = 1.0f;
    if (hubg) {
#if PK_SAFE
        if (PK.safe && gi < 64 && s.round - PK.hot[gi] <= PK_HUBMEM) {
            if (PK.allies < 0) {
                PK.allies = 0;
                const vec2 pos = self().position();
                for (auto& a : vision::champions(team::ally))
                    if (a.is_visible() && a.is_alive() && a.position() != pos && cheb(a.position(), pos) <= 4) ++PK.allies;
            }
            if (PK.allies < PK_HUBN) return 0.0f;
        }
#endif
        m = (s.round < PK_OPENR && s.type != champion_type::varg) ? PK_OPENW : PK_HUBW;
    }
    return m;
}

#if E9_ON
// e9: E9_TAVI 1 = the elk-pair exemption only against an enemy draft containing Tavi (cached once per game).
__attribute__((noinline)) bool e9_foe_ok() {
    static int ok = -1;
    if (ok < 0) {
        ok = 1;
        if (E9_TAVI) { const auto foe = draft::current_draft(team::enemy); ok = std::find(foe.begin(), foe.end(), champion_draft::tavi) != foe.end(); }
    }
    return ok == 1;
}
#endif
bool ls_allow(int gi);
int ls_front();
// ek3 (18 Oct, elkring only: 30x30, five picks, altars (3,26)/(26,3)). Decoded platform losses: after the opening deathball
// clears the three near elks (r5-38), 3-4 champions wait ~40 rounds for the respawn and the own (3,20)/(19,2) elks are
// never attacked. Nyxius (same draft) splits after the first elk: Ca Re Ry (+St) take (9,2) then (19,2), Is St Va take
// (3,12) then (3,20). Fixed roles by champion type (no communication): EK3_EAST types own the east own-side elks
// (own frame x > y), the others the west ones; the elk nearest the spawn is shared. Before round EK3_R a champion
// assumes EK3_N bodies on its side's unreward elk groups and scales the other side's own elks by EK3_WOFF.
#ifndef EK3_ON
#define EK3_ON 0
#endif
#ifndef EK3_EAST
#define EK3_EAST 521   // Ryth(0) Cassi(3) Ren(9)
#endif
#ifndef EK3_N
#define EK3_N 3
#endif
#ifndef EK3_WOFF
#define EK3_WOFF 0.3f
#endif
#ifndef EK3_WON
#define EK3_WON 1.0f
#endif
#ifndef EK3_R
#define EK3_R 2000
#endif
#ifndef EK3_NOPAIR
#define EK3_NOPAIR 1   // skip the Cassi/Issac partner weight on own elks (the pair is split across sides)
#endif
#ifndef EK3_LOG
#define EK3_LOG 0
#endif
#ifndef EK3_SHN
#define EK3_SHN 6      // opening: assume this many bodies on the shared (nearest) elk; side roles start once it is cleared
#endif
#ifndef EK3_MODE
#define EK3_MODE 1     // 1 fixed side roles, 2 idle pack
#endif
#ifndef EK3_IDLE0
#define EK3_IDLE0 0
#endif
#ifndef EK3_WAIT
#define EK3_WAIT 15    // mode 2: an ally waiting longer than this for its group's respawn counts as idle
#endif
#ifndef EK3_NEAR
#define EK3_NEAR 1     // this many nearest own elks are shared (no side role)
#endif
#ifndef EK3_OPEN
#define EK3_OPEN 40    // opening ends by this round at the latest
#endif
static std::array<std::int8_t, 64> g_ek3side{};  // -1 other, 0 east, 1 west, 2 shared (own frame)
static int g_ek3role = 0;
static int g_ek3near = -1;
static bool g_ek3open = true;  // the opening deathball on the shared elk is still on
__attribute__((noinline)) void ek3_init() {
    auto& s = *S;
    g_ek3side.fill(-1);
    const bool flip = s.df_spawn.get(vec2{ 0, 0 }) > s.df_spawn.get(vec2{ s.W - 1, s.H - 1 });
    int near = -1, nd = 1 << 20;
    for (std::size_t gi = 0; gi < s.groups.size() && gi < 64; ++gi) {
        const auto& g = s.groups[gi];
        bool elk = false;
        for (auto& c : g.camps) if (c.maxhp == 2400) elk = true;
        if (!elk) continue;
        vec2 p = g.camps[0].pos;
        for (auto& c : g.camps) if (c.maxhp == 2400) p = c.pos;
        const int x = flip ? s.W - 1 - p.x : p.x, y = flip ? s.H - 1 - p.y : p.y;
        if (x + y >= s.W - 1) continue;
        g_ek3side[gi] = x > y ? 0 : 1;
        if (g.spawn_dist < nd) { nd = g.spawn_dist; near = static_cast<int>(gi); }
    }
    if (near >= 0) g_ek3side[near] = 2;
    g_ek3near = near;
    for (int k = 1; k < EK3_NEAR; ++k) {  // the next nearest own elks are shared too
        int b = -1, bd = 1 << 20;
        for (std::size_t gi = 0; gi < s.groups.size() && gi < 64; ++gi)
            if (g_ek3side[gi] == 0 || g_ek3side[gi] == 1) if (s.groups[gi].spawn_dist < bd) { bd = s.groups[gi].spawn_dist; b = static_cast<int>(gi); }
        if (b >= 0) g_ek3side[b] = 2;
    }
    g_ek3role = ((EK3_EAST >> static_cast<int>(s.type)) & 1) ? 0 : 1;
}
// EK3_MODE 2 (pack): allies whose claimed group is respawning (or who claim none) count as joining an unclaimed own elk,
// so the deathball walks on to the fourth/fifth elk together instead of waiting at a respawning one (Nyxius: A B C, then D r44).
static int g_ek3ir = -1, g_ek3in = 0;
__attribute__((noinline)) int ek3_idle() {
    auto& s = *S;
    if (g_ek3ir == s.round) return g_ek3in;
    int n = 0;
    for (int i = 0; i < 6; ++i) {
        if (i == s.idx) continue;
        const int cg = (s.shm[i] & 63) - 1;
        if (cg < 0) { if (EK3_IDLE0) ++n; continue; }
        if (cg < static_cast<int>(s.groups.size()) && s.groups[cg].st == 2 && s.groups[cg].respawn_round - s.round > EK3_WAIT) ++n;
    }
    g_ek3ir = s.round; g_ek3in = n;
    return n;
}
__attribute__((noinline)) float ek_mult(int gi) {
    auto& s = *S;
    const auto& g = s.groups[gi];
    if (s.W == 41) {
        const bool open = s.round < EI_R;
        float m = 1.0f;
        if (g.gems == 72 && g.spawn_dist <= 20) m *= open ? EI_W72 : EI_W72L;
        if (g.gems == 44) m *= open ? EI_W44 : EI_W44L;
        if (g.spawn_dist > EI_FARD) m *= EI_WFAR;
        return m;
    }
    float m = 1.0f;
    if (g.spawn_dist >= EK_FARD) m *= EK_WFAR;
    if (g.spawn_dist <= EK_NEARD) m *= EK_WNEAR;
    if (g.spawn_dist > EK_NEARD && g.spawn_dist < EK_FARD) m *= EK_WMID;
    return m;
}

// uc (18 Oct, contest only, compile-time UC_ON, g_nm == 1 inside the big choose_target instantiation): platform
// summaries (v379/v385) show the opponent collects the two central 200-gem Monoliths ~3x per game while we get ~0.3.
// For UC_WHO champion types from round UC_R the Monolith (>= UC_MING gems) assumes >= UC_N-1 assisting allies and
// its score is multiplied by UC_W (x (1 + UC_J x allies already on it)). UC_ON 0 compiles to am_fix.
#ifndef UC_ON
#define UC_ON 0
#endif
#ifndef UC_WHO
#define UC_WHO 2047
#endif
#ifndef UC_N
#define UC_N 1
#endif
#ifndef UC_W
#define UC_W 1.0f
#endif
#ifndef UC_J
#define UC_J 0.0f
#endif
#ifndef UC_R
#define UC_R 0
#endif
#ifndef UC_MING
#define UC_MING 200
#endif
// uc2 (18 Oct, fragment only, compile-time UC2_ON, g_nm == 3 inside the frg instantiation): the 58-gem centre elk
// pair nearer our spawn (spawn_dist <= UC2_SD): UC2_WHO champion types assume >= UC2_N-1 assisting allies, the P_MONO
// rejection divides its dps among them (am-style), score x UC2_W. UC2_ON 0 compiles to am_fix.
#ifndef UC2_ON
#define UC2_ON 0
#endif
#ifndef UC2_WHO
#define UC2_WHO 2047
#endif
#ifndef UC2_N
#define UC2_N 2
#endif
#ifndef UC2_W
#define UC2_W 1.0f
#endif
#ifndef UC2_SD
#define UC2_SD 11
#endif
// am (18 Oct, ambush 100x100 only, compile-time AM_ON): idle champions (AM_WHO champion_type bitmask, default
// Ryth|Vendetta) may farm our own-half elk group (AM_MING..AM_MAXG gems, the 72-gem pair at (30-31,44)). The P_MONO
// rejection divides the group's damage among max(claims, AM_N-1) assisting farmers first; AM_W weights that group.
#ifndef RW_ARR
#define RW_ARR 0
#endif
// at (18 Oct, compile-time AT_MAPS bitmask over sh_map_id()): on those maps a carrier with >= AT_MIN gems (on AT_MMASK maps; else any bag) that just
// stepped into altar range starts the tribute on the same turn (Swirl v389: Ryth arrived 36, started 37, stunned 45
// one tick short, died at 48 with 88 gems). AT_THR > 0 additionally requires enemy dps within AT_THR (on AT_TMASK maps). AT_MAPS 0 = sh_sw.
#ifndef AT_MAPS
#define AT_MAPS 0
#endif
#ifndef AT_MIN
#define AT_MIN 1
#endif
#ifndef AT_THR
#define AT_THR 0
#endif
#ifndef AT_MMASK
#define AT_MMASK 32767
#endif
#ifndef AT_TMASK
#define AT_TMASK 32767
#endif
#ifndef AT_RMAX
#define AT_RMAX 100000
#endif
#if AT_MAPS
#if !SH_ON
#error "AT_MAPS needs SH_ON (sh_map_id)"
#endif
static int sh_map_id();
static int g_atm = -2;
static inline bool at_on() { if (g_atm == -2) g_atm = sh_map_id(); return g_atm >= 0 && ((AT_MAPS >> g_atm) & 1); }
#endif
#ifndef RW_PRIO
#define RW_PRIO 0
#endif
#ifndef RW_PRIO_SLACK
#define RW_PRIO_SLACK 0
#endif
#ifndef AM_ON
#define AM_ON 0
#endif
#ifndef AM_WHO
#define AM_WHO 5
#endif
#ifndef AM_N
#define AM_N 2
#endif
#ifndef AM_W
#define AM_W 1.0f
#endif
#ifndef AM_MING
#define AM_MING 60
#endif
#ifndef AM_MAXG
#define AM_MAXG 80
#endif
#ifndef AM_MONO
#define AM_MONO P_MONO
#endif
#ifndef AM_LOG
#define AM_LOG 0
#endif
#if AM_ON
static int g_amg = -1;
static int g_ams = -1;  // -1 unknown, 0 off, 1 on (lazy: first default-dispatch turn)
#endif
#if PH_ON && defined(PH_V92)
#ifndef PH_V92WHO
#define PH_V92WHO 1024 // PH_V92: champion_type bitmask (1024 = Varg)
#endif
#ifndef PH_V92D
#define PH_V92D 0     // PH_V92: 999 = both 92-gem groups, else only the own-half one
#endif
__attribute__((noinline)) bool ph_squad();
#endif
#if AM_ON
__attribute__((noinline)) bool am_map();
#endif
// jc (19 Oct, compile-time JC_ON, corners only): 剑桥夜爬者 (Tavi/Ryth/Stellar/Ren/Willow) beat v400 168:1000 twice by walking a
// Ryth+Stellar+Ren+Willow stack into our quadrant by round 50 while our five farmers were split over four camps (Stellar idle at
// home r13-58 waiting for the 10,41 elk respawn). Vs a Willow draft on 71x71, before JC_R every champion skips the centre hub and
// any group farther than JC_SD from spawn, and treats the own-quadrant groups as shared by JC_N workers (P_MONO divided by them),
// so the team farms its corner as a stack. JC_ON 0 compiles to cb2.
#ifndef JC_ON
#define JC_ON 0
#endif
#ifndef JC_R
#define JC_R 120
#endif
#ifndef JC_SD
#define JC_SD 18
#endif
#ifndef JC_N
#define JC_N 3
#endif
#if JC_ON
static int g_jc = -1;
__attribute__((noinline)) bool jc_on_f() {
    if (g_jc < 0) {
        g_jc = 0;
        if (map::width() == 71 && map::height() == 71) {
            const auto foe = draft::current_draft(team::enemy);
            g_jc = std::find(foe.begin(), foe.end(), champion_draft::willow) != foe.end() ? 1 : 0;
        }
    }
    return g_jc == 1;
}
static inline bool jc_on() { return jc_on_f(); }
#endif
// sh (18 Oct, compile-time SH_ON, map-gated by SH_MAPS bitmask over sh_map_id()): am/uc2 generalised. For the map's
// SH_WHOS champion types the nearest own-half (or equidistant) group with the map's SH_GEMS gems and mdps > 0 assumes
// >= SH_N-1 assisting allies and the P_MONO rejection divides its dps among them. SH_ON 0 compiles to uf387.
#ifndef SH_ON
#define SH_ON 0
#endif
#ifndef SH_MAPS
#define SH_MAPS 0
#endif
#ifndef SH_WHO
#define SH_WHO 2047
#endif
#ifndef SH_N
#define SH_N 2
#endif
#ifndef SH_LOG
#define SH_LOG 0
#endif
// map ids 0..14: elkring interlock eight corners packs stampede swirl lanes pilgrimage monolith collision rushbait contest expanse rows
#ifndef SH_KEYS
#define SH_KEYS 30302010, 40404014, 41301008, 71715030, 31316008, 60502013, 45302022, 90318042, 50654023, 50506022, 41412020, 30802018, 50502014, 80810024, 50502009
#endif
// per map: the group's gem count (nearest own-half-or-equidistant group with that many gems and mdps > 0)
// per map: champion_type bitmask (default SH_WHO everywhere)
#ifndef SH_WHOS
#define SH_WHOS SH_WHO, SH_WHO, SH_WHO, SH_WHO, SH_WHO, SH_WHO, SH_WHO, SH_WHO, SH_WHO, SH_WHO, SH_WHO, SH_WHO, SH_WHO, SH_WHO, SH_WHO
#endif
#ifndef SH_GEMS
#define SH_GEMS 36, 72, 72, 72, 72, 360, 88, 36, 68, 44, 108, 36, 200, 120, 66
#endif
#if SH_ON
static int g_shg = -2;  // -2 unknown, -1 off, else the selected group
__attribute__((noinline)) void sh_map();
// lazy (first dispatch on the collision/packs/default branches only; never inside init's fuel-tight first turn)
static inline bool sh_on() { if (g_shg == -2) sh_map(); return g_shg >= 0; }
#if !(PH_ON && AM_ON)
#error "SH_ON needs the PH_ON/AM_ON template parameters"
#endif
#endif
template <bool old_mages, bool ownhalf_guard, bool coll = false, bool il = false, bool fg = false, bool cxls = false, bool big = false, bool pk = false, bool ek = false, bool frg = false, bool ek3 = false
#if PH_ON
    , bool pg = false
#endif
#if AM_ON
    , bool am = false
#endif
#if SH_ON
    , bool sh = false
#endif
    > int choose_target(float hp, float maxhp) {
    auto& s = *S;
    auto& me = self();
    const vec2 pos = me.position();
#if CXT2_INTERLOCK_BANK19
    if constexpr(il) {
        if(s.mode==Mode::farm && me.gem_count()>=19 && s.df_altar.get(pos)<=2
            && s.round>=s.bank_cool_until && !cxt2_engaged(pos) && cxt2_interlock())return -1;
    }
#endif

    s.me_bb.clear();
    s.me_bb.set(pos);
#if FU_DFME
    if (s.fu_dfpos != pos) { s.df_me.compute(s.terrain, s.me_bb, s.visited); s.fu_dfpos = pos; }
#else
    s.df_me.compute(s.terrain, s.me_bb, s.visited);
#endif
    // Small contested maps (three- or four-pick, at most 1,700 tiles): gold buys the fights, keep valuing it.
    const bool small_map = draft::team_size() <= 4 && s.W * s.H <= 1700;
    const float goldw0 = small_map ? (s.W == 31 && s.H == 31 ? 0.3f : 0.6f) * std::max(0.3f, 1.0f - s.round / 800.0f)
#if GD_MASK || GD_MASK2
                                  : g_gdf >= 0.0f ? P_GOLDW * std::max(g_gdf, 1.0f - s.round / GD_R)
#endif
                                  : P_GOLDW * std::max(0.0f, 1.0f - s.round / 400.0f);
#if GO_MASK
    // go (19 Oct): 89 official summaries: on eight, collision, pilgrimage and corners opponents out-earn us 1.5-2.5x after
    // round 450 while our gold weight has decayed to 0 (round 400) or 0.18 (small maps), so wolves/elks/gold clusters are
    // valued at their gems only. On GO_MASK maps (nxm() bits) the weight never falls below GO_FLOOR. GO_MASK 0 = tv_ar393.
    const float goldw1 = ((GO_MASK >> nxm()) & 1) ? std::max(goldw0, GO_FLOOR) : goldw0;
#define GO_W goldw1
#else
#define GO_W goldw0
#endif
#if CT_ON
    const float goldw = (big && g_nm == 1) ? std::max(GO_W, CT_GW) : GO_W;
#else
    const float goldw = GO_W;
#endif
    const float ally_dps = 55.0f + 0.2f * s.round;
#if FH_PAD
    for (volatile int i = 0; i < FH_PAD; i = i + 1) {}
#endif
    bool fg_gold = false;
    if constexpr (fg) {
        const bool sup = s.type == champion_type::cassi || s.type == champion_type::issac || s.type == champion_type::ren;
        fg_gold = s.round < FG_UNTIL && fg_equity() < (s.fg_on ? (sup ? FG_GOLD_SUP : FG_GOLD) : s.fg_target);
    }

    // Stellar teams out-equip hub campers (the crystals pay no gold): beyond three hub workers, farm gold.
    bool hub_full = false;
    if (s.corners_cap && s.target >= 0 ? !s.groups[s.target].hub : s.corners_cap) {
        int hub_others = 0;
        for (int gi = 0; gi < static_cast<int>(s.groups.size()); ++gi)
            if (s.groups[gi].hub && s.groups[gi].gems >= 40) hub_others += claims_on(gi, false);
        hub_full = hub_others >= 3;
    }
    int il_near = -1;
    const bool il_varg = il && s.type == champion_type::varg;
    int best = -1;
    float best_score = 0;
    for (int gi = 0; gi < static_cast<int>(s.groups.size()); ++gi) {
        auto& g = s.groups[gi];
#if JC_ON
        if constexpr (fg) { if (jc_on() && s.round < JC_R && (g.hub || g.spawn_dist > JC_SD)) continue; }
#endif
        if constexpr (ownhalf_guard) {
            // The current left-side roster can take three tiles beyond the center line.
            if (s.lanes_current_left && g.camps[0].pos.x >= s.W / 2 + 3) continue;
            const bool target_right = g.camps[0].pos.x >= s.W / 2;
            if (!s.lanes_current_left && target_right != s.spawn_right) continue;
        }
        // Let the southwest team farm its own camps through the opening fight.
        if (s.corners_hello_safe100 && cp_skip(g)) continue;
        if constexpr (il) {
            // Varg stays out of the enemy half unless at least IL_RAIDN allies walk with it.
            if (IL_RAID && il_varg) {
                const int sum = g.camps[0].pos.x + g.camps[0].pos.y;
                const bool enemy_half = s.il_west ? sum > s.W - 1 : sum < s.W - 1;
                if (enemy_half) {
                    if (il_near < 0) il_near = il_allies_near(pos);
                    if (il_near < IL_RAIDN) continue;
                }
            }
        }
        int d = UNREACH;
        for (auto& c : g.camps) d = std::min<int>(d, s.df_me.get(c.pos));
        if (d == UNREACH) continue;
        float value = g.gems + ((g_ce && ((CE_GOLDMAPS >> (g_ce - 1)) & 1)) ? std::min(goldw, CE_GW) : goldw) * g.gold;
        if constexpr (fg) { if (fg_gold) value = g.gems + FG_GW * g.gold; }
        if constexpr (il) { if (il_varg) value = g.gems + IL_VGOLD * goldw * g.gold; }
        if (value <= 0) continue;
        float score = 0;
        if (g.st == 1) {
            if (claims_on(gi, true) > 0) continue;
#if P_FUNNEL
            // Support champions leave rewards (and their gold) to the melee carries.
            if (!old_mages
                && ((s.type == champion_type::cassi && !(FX_CAS && g_nm == 3) && !(TR_CMASK && ((TR_CMASK >> nxm()) & 1) && me.gold_count() < TR_CGOLD)) || s.type == champion_type::willow || s.type == champion_type::tavi
                    || (FX_FUNI && g_nm == 3 && s.type == champion_type::issac
                        && [&] { if (FX_FUNIK <= 0) return true; std::uint32_t n = 0; for (auto v : me.item_count()) n += v; return n >= static_cast<std::uint32_t>(FX_FUNIK); }()))
                && claims_on(gi, false) > 0) continue;
#endif
            score = value / (d + constants::capture_time + 1.0f);
        } else {
            const int wait = g.st == 2 ? std::max(0, g.respawn_round - s.round) : 0;
            const float t_arr = static_cast<float>(std::max(d - 1, wait));
            int others = claims_on(gi, false);
            const bool ce_co = ce_job && g.gems >= CE_MING && g.mdps < 200.0f;
            if (ce_co) others = std::max(others, std::min(ce_near, 3) - 1);
#if JC_ON
            const bool jcg = fg && jc_on() && s.round < JC_R;
            if (jcg) others = std::max(others, JC_N - 1);
#endif
            bool ixe = false;
            if constexpr (il) {
                if (IX_ON && g_ixsq && gi < 64 && ((g_ixelk >> gi) & 1u)) {
                    if (IX_MODE == 3 || s.round < IX_START || s.round < ix_cool) continue;
                    ixe = true;
                    if (IX_MODE >= 2) {
                        bool other_elk = false;
                        for (int i = 0; i < 6; ++i) {
                            if (i == s.idx) continue;
                            const int cg = (s.shm[i] & 63) - 1;
                            if (cg >= 0 && cg != gi && cg < 64 && ((g_ixelk >> cg) & 1u)) other_elk = true;
                        }
                        if (other_elk && others == 0) continue;
                    }
                    others = std::max(others, IX_N - 1);
                }
            }
            if constexpr (ek3) {
                if (s.round < EK3_R && gi < 64) {
                    const int sd = g_ek3side[gi];
                    if (g_ek3open) { if (gi == g_ek3near) others = std::max(others, EK3_SHN - 1); }
                    else if (EK3_MODE == 2) { if (sd >= 0 && g.st == 0) others = std::max(others, std::min(EK3_N - 1, ek3_idle())); }
                    else if (sd == g_ek3role) others = std::max(others, EK3_N - 1);
                }
            }
#if AM_ON
            if constexpr (am) if (static_cast<int>(gi) == g_amg) others = std::max(others, AM_N - 1);
#endif
#if SH_ON
            if constexpr (sh) if (gi == g_shg) others = std::max(others, SH_N - 1);
#endif
#if UC2_ON
            const bool uc2 = frg && (g_nm == 3 && g.gems >= 50 && g.gems <= 70 && g.spawn_dist <= UC2_SD && ((UC2_WHO >> static_cast<int>(s.type)) & 1));
            if (uc2) others = std::max(others, UC2_N - 1);
#endif
#if UC_ON
            if constexpr (big) if (g_nm == 1 && g.gems >= UC_MING && s.round >= UC_R && ((UC_WHO >> static_cast<int>(s.type)) & 1)) others = std::max(others, UC_N - 1);
#endif
            float mine = my_dps(g);
            if constexpr (il) { if (il_varg && g.splash > IL_VSPLASH) mine = cxh_attack(me) * 1.1f * IL_VSPLASH; }
            const float total = mine + others * ally_dps;
            const float ghp = g.st == 2 ? [&] { float t = 0; for (auto& c : g.camps) t += c.maxhp; return t; }() : g.hp();
            const float t_kill = ghp / total;
            float gdps = (g.st == 2 ? g.mdps : g.alive_dps()) * s.jungle;
#if CC5_ON
            // cc_maps5: on Great Wall, divide the herd's damage among co-op helpers for every skip check, then restore.
            const float cc5_gdps = gdps;
            const bool cc5_div = cc5_gw && ce_co && g.gems >= 50 && g.camps.size() >= 6 && g.mdps <= 240.0f;
            if (cc5_div) gdps /= 1.0f + others;
#endif
#if BM_MASK
            if constexpr (big) { if (g_nm == 2 && bm_on() && bm_group(g)) { if (g.st != 1 && !bm_caster()) continue; gdps = 0.0f; } }  // only the stampede/contest instantiation pays for this
#endif
#if JC_ON
            if (jcg) { if (gdps / (1 + others) > P_MONO * maxhp) continue; }
            else
#endif
#if SH_ON
            if (sh && gi == g_shg) { if (gdps / (1 + others) > P_MONO * maxhp) continue; }
            else
#endif
#if AM_ON
            if (am && static_cast<int>(gi) == g_amg) { if (gdps / (1 + others) > AM_MONO * maxhp) continue; }
            else
#endif
#if UC2_ON
            if (uc2) { if (gdps / (1 + others) > P_MONO * maxhp) continue; }
            else
#endif
#if E9_ON
            // e9 (17 Oct, eight only via the ek instantiation): every champion may join the own-side 72-gem elk pair
            // (Stellar/Cassi otherwise skip it on P_MONO); with E9_R > 0 only before round E9_R.
            if constexpr (ek) { if (gdps > P_MONO * maxhp && !ixe && !(s.W == 41 && g.gems == 72 && g.spawn_dist <= 20 && (E9_R <= 0 || s.round < E9_R) && ((E9_WHO >> static_cast<int>(s.type)) & 1) && e9_foe_ok())) continue; }
            else
#endif
            if constexpr (!big) { if (gdps / (ce_co ? 1 + others : 1) > P_MONO * maxhp && !ixe) continue; } // monolith-class damage: not yet
            else { if (g_nm == 1 && NM_CT_MONO > 0 && gdps > NM_CT_MONO * maxhp) continue; } // new-pool maps whose only gems are a Monolith/elk herd
#if FB_ELK
            if (fb_elk_skip(g, maxhp)) continue;
#endif
#if CC5_ON
            if (cc5_div) gdps = cc5_gdps;
#endif
            const float dmg = gdps * t_kill / (1 + others);
            const float excess = dmg - (hp - P_RECALL_HP * maxhp);
            float pen = 0;
            if (excess > 0) pen = (constants::recall_time + g.spawn_dist) * (excess / (0.7f * maxhp));
            const float share = mine / total;
            score = value * share / (t_arr + t_kill + pen + constants::capture_time * share + 1.0f);
            if (P_SPREAD > 0 && s.rows_like && g.st == 2 && others > 0) score /= 1.0f + P_SPREAD * others;
            if (ixe) score *= IX_W * ((IX_MODE >= 2 && claims_on(gi, false) > 0) ? IX_JOIN : 1.0f);
            if (ce_co) score *= CE_W;
        }
        // Cassi follows Issac: they die together, so keep the melee body between her and danger.
        if (P_PAIR > 0 && (s.W * s.H <= P_PAIR_AREA || (IL5_PAIR && il) || (FD_PAIRALL && fd_on())) && (s.type == champion_type::cassi || (P_PAIR > 1 && !(IL5_PAIR == 1 && il) && s.type == champion_type::issac))
            && !(ek3 && EK3_MODE == 1 && EK3_NOPAIR && s.round < EK3_R && gi < 64 && g_ek3side[gi] >= 0)) {
            const int partner = s.type == champion_type::cassi ? 5 : 2;
            const auto v = s.shm[partner];
            if (v != 0 && static_cast<int>(v & 63) == gi + 1) score *= P_PAIR_W;
        }
        if constexpr (fg) {
            if (g.hub && fg_gold) score *= FG_HUBE;
            else if (g.hub && !(hub_full && g.gems >= 40)) score *= g.gems >= 40 ? P_HUBW : P_HUBW_SMALL;
        } else
        if (g.hub && !(hub_full && g.gems >= 40)) score *= g.gems >= 40 ? P_HUBW : P_HUBW_SMALL;
        // Interlock vs Stellar+Tavi teams without Prism/Willow: Varg keeps our crystal group on cooldown.
        if (s.interlock_keeper && s.type == champion_type::varg && g.mdps == 0 && g.gems >= 60 && g.splash >= 3 && g.spawn_dist <= 20) {
            bool keep = true;
            if constexpr (il) {
                if (IL_KEEPPAIR > 0) { if (il_near < 0) il_near = il_allies_near(pos); keep = il_near >= 1; }
            }
            if (keep) score *= 5.0f;
        }
        if constexpr (coll) {
            if (gi < 64 && CO.altar_dist[gi] <= 3) score *= collision_hot_weight();
        }
        if constexpr (cxls) {  // 41x41 instantiations only: other maps keep rx_rows1's exact per-turn fuel
            if (CX.on) { if (cx_skip(gi)) continue; score *= cx_pack_mult(gi); }
            if (g_cplan == 2 && CB.on && !(IL5_LSST && s.type == champion_type::stellar) && !ls_allow(gi)) continue;
#if IL5_STOWN
            if (g_cplan == 2 && CB.on && s.type == champion_type::stellar && !(gi < 64 && CO.altar_dist[gi] <= 3) && claims_on(gi, false) == 0) {
                ls_front();
                bool enemy_side = false;
                for (auto& c : g.camps) if (ls_north ? c.pos.y > s.H / 2 : c.pos.y < s.H / 2) enemy_side = true;
                if (enemy_side) continue;
            }
#endif
        }
        if constexpr (pk) { const float pm = pk_mult(gi); if (pm <= 0.0f) continue; score *= pm; }
        if constexpr (frg) {
            if (FX_FARU > 0 && s.round < FX_FARU && gi < 64 && ((g_fxfar >> gi) & 1ull)) continue;
            if (g.mdps >= FR_GMDPS) score *= 1.0f + FR_GROUP * claims_on(gi, false);
        }
#if CT_ON
        if constexpr (big) {
            if (g_nm == 1) {
                #if CXW_CONTEST_GOLD
                if (s.round < CT_OPEN && g.gems == 0 && g.gold < (cxw_exact_contest() ? 80 : 100)) continue;
#else
                if (s.round < CT_OPEN && g.gems == 0 && g.gold < 100) continue;
#endif
                if (s.round < CT_GR) score *= 1.0f + CT_GROUP * claims_on(gi, false);
            }
        }
#endif
        if constexpr (ek) { const float em = ek_mult(gi); if (em <= 0.0f) continue; score *= em; }
        if constexpr (ek3) {
            if (s.round < EK3_R && gi < 64) {
                const int sd = g_ek3side[gi];
                if (EK3_MODE == 1 && sd >= 0 && sd != 2) score *= sd == g_ek3role ? EK3_WON : EK3_WOFF;
            }
        }
#if PH_ON && defined(PH_V92)
        if constexpr (pg) if (PH_V92 > 0.0f && g.gems == 92 && ((PH_V92WHO >> static_cast<int>(s.type)) & 1) && !ph_squad() && (PH_V92D >= 999 || (g.camps[0].pos.x >= s.W / 2) == g_phright)) score *= PH_V92;
#endif
#if AM_ON
        if constexpr (am) if (static_cast<int>(gi) == g_amg) score *= AM_W;
#endif
#if UC2_ON
        if constexpr (frg) if (g_nm == 3 && g.gems >= 50 && g.gems <= 70 && g.spawn_dist <= UC2_SD && ((UC2_WHO >> static_cast<int>(s.type)) & 1)) score *= UC2_W;
#endif
#if UC_ON
        if constexpr (big) if (g_nm == 1 && g.gems >= UC_MING && s.round >= UC_R && ((UC_WHO >> static_cast<int>(s.type)) & 1)) score *= UC_W * (1.0f + UC_J * claims_on(gi, false));
#endif
        if (gi == s.target) score *= 1.3f + 0.01f * DR_J + ((DP_J != 3 && s.W == 40 && s.H == 40) ? 0.02f * (DP_J - 3) : 0.0f);
#if SF_SEED
        {   // sf: seeded noise so test-opponent builds play genuinely different games (changes every 40 rounds)
            std::uint32_t h = static_cast<std::uint32_t>(SF_SEED) * 2654435761u ^ static_cast<std::uint32_t>(gi) * 40503u ^ static_cast<std::uint32_t>(s.round / 40 + 1) * 2246822519u ^ static_cast<std::uint32_t>(s.idx) * 3266489917u;
            h ^= h >> 15; h *= 2246822519u; h ^= h >> 13;
            score *= 1.0f + SF_NZ * ((static_cast<float>(h & 1023) / 511.5f) - 1.0f);
        }
#endif
        if (score > best_score) { best_score = score; best = gi; }
    }
    if constexpr (!old_mages && !ownhalf_guard && !coll && !il && fg) {
        return mackey_packs_fast_return(best, global_submitted_gems(team::ally));
    }
    return best;
}

// In a late Packs race, include the return to an altar when choosing Varg's
// next camp. Only replace a target whose reward cannot return promptly.
__attribute__((noinline)) int mackey_packs_fast_return(int choice, int banked) {
    auto& s = *S;
    if (s.W != 31 || s.H != 31 || s.type != champion_type::varg
        || s.round < 650 || banked < 900 || self().gem_count() != 0
        || self().position().x > 8 || self().position().y > 22
        || !map::spawn_region(team::ally).is_set(vec2{0, 0})) return choice;
    const auto ep = draft::current_draft(team::enemy);
    if (ep.size() != 3 || ep[0] != champion_draft::cassi_and_issac
        || ep[1] != champion_draft::stellar || ep[2] != champion_draft::varg) return choice;
    auto back_distance = [&](int gi) {
        int d = UNREACH;
        for (const auto& c : s.groups[gi].camps) d = std::min<int>(d, s.df_altar.get(c.pos));
        return d;
    };
    if (choice >= 0 && back_distance(choice) <= 9) return choice;
    int best = -1;
    int best_route = UNREACH;
    for (int gi = 0; gi < static_cast<int>(s.groups.size()); ++gi) {
        const auto& g = s.groups[gi];
        if (g.st != 0 || g.gems < 40 || g.alive_dps() > 40) continue;
        const int back = back_distance(gi);
        if (back > 8) continue;
        int walk = UNREACH;
        for (const auto& c : g.camps) walk = std::min<int>(walk, s.df_me.get(c.pos));
        if (walk == UNREACH) continue;
        const int route = walk + back;
        if (route < best_route) { best_route = route; best = gi; }
    }
    return best >= 0 ? best : choice;
}

// ---- recall ----
bool ev_run(bool requested);
void do_recall() {
    if(S->W==41 && S->H==30 && S->type==champion_type::varg && ev_run(true))return;
    auto& s = *S;
    auto& me = self();
    const vec2 pos = me.position();
#if RA_ON & 1
    if (((RA_MAPS >> nxm()) & 1) && s.round - s.ra_round < RA_K) {
        refresh_occ();
        s.occ.set(pos, false);
        step_field(s.df_spawn, true);
        return;
    }
#endif
#if FM_RQ > 0
    bool quiet = s.round - s.enemy_round >= P_RECALL_QUIET;
    if (!quiet && s.round - s.hurt_round >= FM_RQ) {
        int nd = 99;
        for (auto& e : vision::champions(team::enemy))
            if (e.is_visible() && e.is_alive()) nd = std::min(nd, cheb(pos, e.position()));
        quiet = nd >= FM_RQD;
    }
#if TF_ON
    if (tf_on() && tf_rec() && quiet) {
        // tf: never start a channel with an enemy within TF_RECD, or a caster within its spell reach (TF_RECDC).
        for (auto& e : vision::champions(team::enemy)) {
            if (!e.is_visible() || !e.is_alive()) continue;
            const int d = cheb(pos, e.position());
            if (d < TF_RECD || (d < TF_RECDC && tf_caster(e.champion_name()))) { quiet = false; break; }
        }
    }
#endif
#if CXK_MAPS
    if ((CXK_MAPS & 0x200) && nxm()==9 && (CXK_SW & 4) && me.gem_count()>=CXK_GEMS) {
        if(s.round-s.hurt_round<=3)quiet=false;
        for(const auto& e:vision::champions(team::enemy))if(e.is_visible() && e.is_alive() && cheb(pos,e.position())<=CXK_NEAR)quiet=false;
    }
#endif
    if (s.df_spawn.get(pos) <= 5 || !quiet) {
#else
    if (s.df_spawn.get(pos) <= 5 || s.round - s.enemy_round < P_RECALL_QUIET) {
#endif
        // Close to home, or enemies were just in sight: walk, a channel would be broken.
        refresh_occ();
        s.occ.set(pos, false);
        step_field(s.df_spawn, true);
        return;
    }
    // Leave the reach of awake monsters before channelling.
    #if CXV_RECALL_ELK
    bool exposed_elk = false;
    // Same exact Elkring geometry as the existing EK map detection, excluding Fragment.
    const bool recall_elkring = s.W == 30 && s.H == 30 && s.altars.size() == 2
        && ((s.altars[0].x == 26 && s.altars[0].y == 3) || (s.altars[0].x == 3 && s.altars[0].y == 26))
        && ((s.altars[1].x == 26 && s.altars[1].y == 3) || (s.altars[1].x == 3 && s.altars[1].y == 26));
    #if CXY_ELK_ISSAC
    if (!recall_elkring || (s.type == champion_type::issac && me.gem_count() >= 22 && cxy_exact_elk())) for (auto& group : s.groups) {
#else
    if (!recall_elkring) for (auto& group : s.groups) {
#endif
        if (group.st != 0 || group.mdps <= 0) continue;
        for (auto& camp : group.camps) {
            if (camp.hp <= 0 || camp.h.kind() != camp_type::elk) continue;
            const int distance = cheb(pos, camp.pos);
            if (distance > 4) continue;
            bool closer = false;
            for (auto side : {team::ally, team::enemy})
                for (auto& body : vision::champions(side))
                    if (body.is_visible() && body.is_alive() && body.position() != pos
                        && cheb(body.position(), camp.pos) < distance) closer = true;
            if (!closer) exposed_elk = true;
        }
    }
#endif
    auto threat = [&](vec2 p) {
        int worst = 99;
        for (auto& g : s.groups) {
            if (g.st != 0 || g.mdps <= 0) continue;
            for (auto& c : g.camps) {
                if (c.hp <= 0 || c.str <= 0) continue;
                #if CXV_RECALL_ELK
                const int reach = exposed_elk && c.h.kind() == camp_type::elk ? 4 : c.range;
                worst = std::min(worst, cheb(p, c.pos) - reach);
#else
                worst = std::min(worst, cheb(p, c.pos) - c.range);
#endif
            }
        }
        return worst; // > 0 means outside every attacker's range
    };
    const int here = threat(pos);
    bool on_spawn_tile = false;
    for (auto& g : s.groups) if (g.tiles().is_set(pos)) on_spawn_tile = true;
    if ((here <= 0 || on_spawn_tile) && me.can_move()) {
        refresh_occ();
        vec2 best = pos;
        int best_t = on_spawn_tile ? -99 : here;
        int best_home = s.df_spawn.get(pos);
        for (auto d : DIRS) {
            vec2 p{ pos.x + d.x, pos.y + d.y };
            if (!in_bounds(p) || s.occ.is_set(p)) continue;
            const int t = std::min(threat(p), 1);
            const int home = s.df_spawn.get(p);
            if (t > best_t || (t == best_t && home < best_home)) { best = p; best_t = t; best_home = home; }
        }
        if (best != pos) {
            try { if (me.move_absolute(best)) { s.moved = true; ++s.moves; } } catch (const cambc::error&) {}
            if (threat(me.position()) <= 0) return;
        }
    }
#if BL_ON
    if (BL_R) {
        // bl: never start the channel with an enemy inside its spell reach + 1 or within 6; keep walking home instead.
        const vec2 at = me.position();
        bool unsafe = false;
        for (auto& e : vision::champions(team::enemy)) {
            if (!e.is_visible() || !e.is_alive()) continue;
            const int d = cheb(at, e.position());
            if (d > 9) continue;
            int sr = 5;
            (void)bl_threat(e, me, d, sr);
            if (d <= 6 || d <= sr + 1) { unsafe = true; break; }
        }
        if (unsafe) {
            s.enemy_round = s.round;
            if (me.can_move()) {
                refresh_occ();
                s.occ.set(at, false);
                step_field(s.df_spawn, true);
            }
            return;
        }
    }
#endif
    try { me.recall(); ++s.channel; } catch (const cambc::error& e) { if (DBG) std::cout << "recall err " << e.what() << "\n"; }
}

// Summed basic-attack pressure of visible enemies close enough to reach us soon.
float enemy_dps_near(vec2 pos, int radius) {
    float dps = 0;
    for (auto& e : vision::champions(team::enemy)) {
        if (!e.is_visible() || !e.is_alive()) continue;
        if (cheb(pos, e.position()) > radius) continue;
        dps += cxh_attack(e) * P_EDPS;
    }
    return dps;
}

// ---- altar locks ----
// Drop a carried lock on any altar in reach that does not have ours yet.
void place_lock() {
#if P_LOCK
    auto& s = *S;
    auto& me = self();
    const vec2 pos = me.position();
    if (s.df_altar.get(pos) > 1) return;
    const auto& lock = me.consumables()[consumable_type::altar_lock];
    if (lock.count() == 0) return;
    const auto ours = vision::altar_lock_tiles(team::ally);
    for (std::size_t i = 0; i < s.altars.size(); ++i) {
        const vec2 a = s.altars[i];
        if (cheb(pos, a) > 2) continue;
        if (ours.is_set(a)) {
            if (i < 8 && !(s.shm[6] & (1u << i))) { s.shm[6] |= static_cast<std::uint8_t>(1u << i); shared_memory[6] = static_cast<std::byte>(s.shm[6]); }
            continue;
        }
        try {
            if (lock.use(a)) {
                if (i < 8) { s.shm[6] |= static_cast<std::uint8_t>(1u << i); shared_memory[6] = static_cast<std::byte>(s.shm[6]); }
                return;
            }
        } catch (const cambc::error& e) {
            if (DBG) std::cout << "lock err " << e.what() << "\n";
        }
    }
#endif
}

static int ccx_rc_round = -1000;
bool ccx_teleport() {
    if (!ccx_on()) return false;
    auto& s = *S;
    auto& me = self();
    if (me.gem_count() == 0 || me.is_recalling() || me.is_submitting_gems() || me.is_teleporting()) return false;
    const bool rmap = (CCX_RMASK >> ccx_map) & 1;
    if (s.mode != Mode::bank && !rmap) return false;
    const vec2 pos = me.position();
    const int walk = s.df_altar.get(pos);
    if (walk < CCX_MINWALK) return false;
    const auto& crystal = me.consumables()[consumable_type::tp_crystal];
    if (crystal.count() == 0) return false;
    const auto occupied = vision::champion_tiles() | vision::camp_tiles() | vision::teleport_tiles();
    const auto locked = vision::altar_lock_tiles(team::enemy);
    vec2 best{-1,-1};
    int bestd = 4;
    for (const vec2 a : s.altars) {
        if (cheb(pos, a) > 4 || locked.is_set(a)) continue;
        for (int x = a.x-1; x <= a.x+1; ++x) for (int y = a.y-1; y <= a.y+1; ++y) {
            const vec2 q{x,y};
            const int d = cheb(pos, q);
            if (q == a || !in_bounds(q) || d > 3 || s.terrain.is_set(q) || occupied.is_set(q)) continue;
            if (d < bestd) { best = q; bestd = d; }
        }
    }
    if (best.x >= 0) {
        bool ok = false;
        try { ok = crystal.use(best); } catch (const cambc::error&) { ok = false; }
        if (ok) {
            end_mission(); publish(-1,0); s.target=-1; s.mode = Mode::bank;
            s.bank_turns=0; s.bank_skip=-1; s.bank_best=1<<30; s.bank_age=0;
#ifdef CCX_TRACE
            std::cout << "ccx_tp " << s.round << " " << static_cast<int>(s.type) << " " << me.gem_count() << " walk " << walk
                      << " " << pos.x << "," << pos.y << " " << best.x << "," << best.y << "\n";
#endif
            ++s.t_bank;
            return true;
        }
    }
    if (rmap && best.x < 0 && me.can_move() && map::spawn_region(team::ally).is_set(pos)) {
        vec2 t{-1,-1}; int td = 1 << 30;
        for (const vec2 p : map::spawn_region(team::ally).positions()) {
            bool near = false;
            for (const vec2 a : s.altars) for (int x = a.x-1; x <= a.x+1 && !near; ++x) for (int y = a.y-1; y <= a.y+1; ++y) {
                const vec2 q{x,y};
                if (q == a || !in_bounds(q) || s.terrain.is_set(q) || cheb(p, q) > 3) continue;
                near = true; break;
            }
            if (near && cheb(pos, p) < td) { t = p; td = cheb(pos, p); }
        }
        if (t.x >= 0 && td > 0) {
            bool moved = false;
            try { moved = me.move_towards(t); } catch (const cambc::error&) {}
            if (moved) { s.moved = true; ++s.t_bank; return true; }
        }
    }
    if (rmap && s.mode == Mode::bank && walk >= CCX_RWALK && static_cast<int>(me.gem_count()) >= CCX_RGEMS
        && s.round - ccx_rc_round > 40) {
        bool ok = false;
        try { me.recall(); ok = true; } catch (const cambc::error&) {}
        if (ok) {
            ccx_rc_round = s.round;
            end_mission(); publish(-1,0); s.target=-1; ++s.channel; ++s.t_recall;
#ifdef CCX_TRACE
            std::cout << "ccx_rc " << s.round << " " << static_cast<int>(s.type) << " " << me.gem_count() << " walk " << walk
                      << " " << pos.x << "," << pos.y << "\n";
#endif
            return true;
        }
    }
    return false;
}

void end_mission() {
    auto& s = *S;
    if (s.mission >= 0 && s.shm[7] == s.mission + 1) { shared_memory[7] = std::byte{0}; s.shm[7] = 0; }
    s.mission = -1;
}

// ---- banking ----
#if Z163_ON
// Z163: start an already intended bank channel in the movement turn.
bool z163_submit_arrival() {
    auto& me = self();
    if (me.gem_count() == 0 || me.is_submitting_gems()) return false;
    const vec2 pos = me.position();
    for (std::size_t i = 0; i < S->altars.size(); ++i) {
        const vec2 a = S->altars[i];
        if (static_cast<int>(i) == S->bank_skip || cheb(pos, a) > constants::submit_gems_range) continue;
        if (vision::altar_lock_tiles(team::enemy).is_set(a)) continue;
        try { me.submit_gems(a); ++S->channel; return true; } catch (const cambc::error&) {}
    }
    return false;
}
#endif

// ---- cc_crystal2: Great Wall crystal bank route (gwc_switch.h), same pattern as cc_cargo3 Fracture ----
static bool gwc_ready = false;
static distance_field* gwc_df = nullptr;
inline bool gwc_launch(vec2 p) { return p.x >= 0 && p.y >= 0 && p.x < 50 && p.y < 40 && GWC_LAUNCH[p.y][p.x] == 'L'; }
void gwc_init() {
    if (gwc_ready) return;
    gwc_ready = true;
    auto& s = *S;
    bitboard roots;
    roots.clear();
    for (int y = 0; y < 40; ++y) for (int x = 0; x < 50; ++x)
        if (GWC_LAUNCH[y][x] == 'L' && !s.terrain.is_set(vec2{x,y})) roots.set(vec2{x,y}, true);
    gwc_df = new distance_field();
    gwc_df->compute(s.terrain, roots, s.visited);
}
inline bool gwc_crystal() { return self().consumables()[consumable_type::tp_crystal].count() > 0; }
int gwc_route(vec2 pos) {
    if (!g_gwc || !gwc_ready || !gwc_crystal()) return UNREACH;
    const int d = gwc_df->get(pos);
    return d == UNREACH ? UNREACH : d + GWC_POST;
}
bool gwc_use_route(vec2 pos) {
    if (!g_gwc) return false;
    const int r = gwc_route(pos);
    return r != UNREACH && r + 4 < S->df_altar.get(pos);
}
bool gwc_act() {
    if (!g_gwc) return false;
    auto& s = *S;
    auto& me = self();
    if (me.gem_count() == 0 || s.mode != Mode::bank || me.is_recalling() || me.is_submitting_gems() || me.is_teleporting()) return false;
    const vec2 pos = me.position();
    if (!gwc_launch(pos) || !gwc_crystal()) return false;
    const int walk = s.df_altar.get(pos);
    const auto occupied = vision::champion_tiles() | vision::camp_tiles() | vision::teleport_tiles();
    vec2 best{-1,-1}; int bd = walk - GWC_HOPGAIN - 3;
    for (int dx = -3; dx <= 3; ++dx) for (int dy = -3; dy <= 3; ++dy) {
        const vec2 q{pos.x+dx, pos.y+dy};
        if (!in_bounds(q) || s.terrain.is_set(q) || occupied.is_set(q)) continue;
        const int d = s.df_altar.get(q);
        if (d == UNREACH) continue;
        if (d < bd) { bd = d; best = q; }
    }
    if (best.x < 0) return false;
    bool ok = false;
    try { ok = me.consumables()[consumable_type::tp_crystal].use(best); } catch (const cambc::error&) { ok = false; }
    if (!ok) return false;
    end_mission(); publish(-1,0); s.target=-1; s.mode = Mode::bank;
    s.bank_turns=0; s.bank_skip=-1; s.bank_best=1<<30; s.bank_age=0;
#ifdef GWC_TRACE
    std::cout << "gwc_tp " << s.round << " " << static_cast<int>(s.type) << " g " << me.gem_count() << " walk " << walk
              << " " << pos.x << "," << pos.y << " -> " << best.x << "," << best.y << " d " << bd << "\n";
#endif
    ++s.t_bank;
    return true;
}

bool do_bank() {
    auto& s = *S;
    auto& me = self();
    const vec2 pos = me.position();
    // Count turns without getting closer to an altar.
    {
        int d = s.df_altar.get(pos);
        if (g_gwc && gwc_use_route(pos)) d = std::min(d, gwc_route(pos));
        if (d < s.bank_best) { s.bank_best = d; s.bank_turns = 0; }
        else ++s.bank_turns;
    }
    if (s.bank_turns > P_BANKSTALL && s.bank_skip < 0 && s.altars.size() >= 2) {
        // No tribute after this long: the nearest altar is not working out, try another.
        int near = 0;
        for (std::size_t i = 1; i < s.altars.size(); ++i) if (cheb(pos, s.altars[i]) < cheb(pos, s.altars[near])) near = static_cast<int>(i);
        s.bank_skip = near;
#if P_LOG
        std::cout << "STALL " << s.round << " " << static_cast<int>(s.type) << " at " << pos.x << "," << pos.y << " g " << me.gem_count() << " skip " << near << "\n";
#endif
    }
#if RX_ROWS
    // Rows: clear an enemy lock on the altar we are about to use (our line never attacked locks).
    if (s.rows_like)
        for (const auto& L : vision::altar_locks(team::enemy)) {
            if (cheb(pos, L.position()) > constants::submit_gems_range + 1) continue;
            if (cheb(pos, L.position()) <= s.range && me.can_attack()) { try { me.attack(L); return true; } catch (const cambc::error&) {} }
            s.goals.clear(); s.goals.draw_chebyshev(L.position(), static_cast<float>(s.range)); s.goals &= ~s.terrain;
            if (me.can_move() && step_to_goals(true)) return true;
        }
#endif
    for (std::size_t i = 0; i < s.altars.size(); ++i) {
        const vec2 a = s.altars[i];
        if (static_cast<int>(i) == s.bank_skip || cheb(pos, a) > constants::submit_gems_range) continue;
#if P_SAFEBANK
        {
            const float dps = enemy_dps_near(pos, 5);
            const float turns = std::max(1.0f, (me.gem_count() + 9) / 10.0f);
            if (dps > 0 && s.round - s.hurt_round <= 2 && me.health() / dps < turns + 1.0f) { s.bank_unsafe_round = s.round; return false; }
        }
#endif
        try { me.submit_gems(a); ++s.channel; return true; } catch (const cambc::error& e) { if (DBG) std::cout << "submit err " << e.what() << "\n"; }
    }
    if (s.bank_skip < 0) {
        refresh_occ();
        s.occ.set(pos, false);
        if (g_gwc && gwc_use_route(pos) && step_field(*gwc_df, true)) { ++s.t_bank; return true; }
        if (step_field(s.df_altar, true)) {
#if Z163_ON
            if (z163_submit_arrival()) return true;
#endif
            if (((s.lanes_ownhalf || (RW_ARR && s.rows_like && s.W == 50 && s.H == 50)) && me.gem_count() > 0
                && (global_submitted_gems(team::ally) + me.gem_count() >= constants::winning_gems || (RW_ARR == 2 && s.rows_like && s.W == 50)))
#if AT_MAPS
                || (me.gem_count() > 0 && s.round <= AT_RMAX && at_on() && (me.gem_count() >= AT_MIN || !((AT_MMASK >> g_atm) & 1)) && (AT_THR <= 0 || !((AT_TMASK >> g_atm) & 1) || enemy_dps_near(me.position(), AT_THR) > 0))
#endif
                ) {
                const vec2 arrived = me.position();
                for (const vec2 a : s.altars) {
                    if (cheb(arrived, a) > constants::submit_gems_range) continue;
#if AT_MAPS && P_LOG
                    try { const int g0 = me.gem_count(); me.submit_gems(a); ++s.channel; std::cout << "ATB " << s.round << " " << static_cast<int>(s.type) << " g " << g0 << "\n"; break; } catch (const cambc::error&) {}
#else
                    try { me.submit_gems(a); ++s.channel; break; } catch (const cambc::error&) {}
#endif
                }
            }
            return true;
        }
    }
    // Static route blocked by a unit (or an altar is being skipped): path dynamically.
    s.goals.clear();
    for (std::size_t i = 0; i < s.altars.size(); ++i)
        if (static_cast<int>(i) != s.bank_skip) s.goals.draw_chebyshev(s.altars[i], static_cast<float>(constants::submit_gems_range));
#if Z163_ON
    const bool z163_moved = step_to_goals(s.bank_skip >= 0);
    if (z163_moved) z163_submit_arrival();
    return z163_moved;
#else
    return step_to_goals(s.bank_skip >= 0);
#endif
}

// ---- ca selector ----
// Platform one-match tests (15 Oct, same opponent bots): v268 beat getramramd (Va/C&I/St) but lost Nyx (Vd/C&I/St)
// and SSSA (C&I/Vd/Ry); CX territory beat Nyx and SSSA, lost getramramd; LS sweep beat SSSA best, lost the other two.
// Varg+C&I: v268 (s.coll already off). C&I without Stellar (melee packs that rush our altar wolf): LS.
// Vendetta+Stellar without Ren (Nyx: Stellar camps the altar from R20): CX. Anything else: v268.
__attribute__((noinline)) int ca_select() {
    if (CA_FORCE >= 0) return CA_FORCE;
    const auto foe = draft::current_draft(team::enemy);
    auto has = [&](champion_draft c) { return std::find(foe.begin(), foe.end(), c) != foe.end(); };
    // Varg+C&I without Stellar (EMPEROR Tung: Vd/Va/C&I): own-half territory won 1104:632 where v268 lost 968:1022.
    if (has(champion_draft::varg) && has(champion_draft::cassi_and_issac) && !has(champion_draft::stellar)) return 1;
    if (has(champion_draft::varg) && has(champion_draft::cassi_and_issac)) return 0;
#if CZ2_CIRS >= 0
    if (has(champion_draft::cassi_and_issac) && has(champion_draft::ren) && has(champion_draft::stellar)
        && !has(champion_draft::varg) && !has(champion_draft::vendetta)) return CZ2_CIRS;
#endif
#if CZ2_PRRS >= 0
    if (has(champion_draft::prism) && has(champion_draft::ren) && has(champion_draft::stellar)) return CZ2_PRRS;
#endif
#if CZ2_CSTA >= 0
    if (has(champion_draft::cassi_and_issac) && has(champion_draft::stellar) && has(champion_draft::tavi)
        && !has(champion_draft::ren)) return CZ2_CSTA;
#endif
#if CZ2_SRRY >= 0
    if (has(champion_draft::stellar) && has(champion_draft::ren) && has(champion_draft::ryth) && !has(champion_draft::cassi_and_issac)
        && !has(champion_draft::varg) && !has(champion_draft::vendetta)) return CZ2_SRRY;
#endif
    if (!has(champion_draft::stellar) && has(champion_draft::cassi_and_issac)) return 2;
    if (has(champion_draft::vendetta) && has(champion_draft::stellar) && !has(champion_draft::ren)) return 1;
    return 0;
}

// ---- cx helpers ----
__attribute__((noinline)) bool cx_enemy_side(vec2 p) { return CX.north ? p.y > S->H / 2 : p.y < S->H / 2; }
__attribute__((noinline)) bool cx_skip(int gi) {
    auto& s = *S;
    if (!CX_TERR) return false;
    if (gi < 64 && CO.altar_dist[gi] <= CX_TERRD) return false;
    for (auto& c : s.groups[gi].camps) if (!cx_enemy_side(c.pos)) return false;
    return true;
}
__attribute__((noinline)) float cx_pack_mult(int gi) {
    const int c = claims_on(gi, false);
    float m = 1.0f + CX_PACKW * static_cast<float>(c);
    if (CX_RAID > 0.0f && gi < 64 && CO.altar_dist[gi] <= 9) {
        bool enemy = true;
        for (auto& k : S->groups[gi].camps) if (!cx_enemy_side(k.pos)) enemy = false;
        if (enemy) m *= CX_RAID;
    }
    return m;
}
// Slot of an ally champion in shared memory (same rule as init()).
__attribute__((noinline)) int cx_idx(champion_type t) {
    if (t == champion_type::issac) return 5;
    const int want = draft_id_of(t);
    int idx = want % 5;
    for (std::size_t i = 0; i < draft_memory.size() && i < 5; ++i)
        if (static_cast<int>(draft_memory[i]) == want) idx = static_cast<int>(i);
    return idx;
}
__attribute__((noinline)) bool cx_partner(champion_type a, champion_type b) {
    return (a == champion_type::cassi || a == champion_type::issac) && (b == champion_type::cassi || b == champion_type::issac);
}
__attribute__((noinline)) int cx_allies_within(vec2 pos, int r) {
    auto& s = *S;
    int n = 0;
    for (auto& a : vision::champions(team::ally)) {
        if (!a.is_visible() || !a.is_alive()) continue;
        const vec2 p = a.position();
        if (p == pos || cx_partner(s.type, a.champion_name())) continue;
        if (cheb(p, pos) <= r) ++n;
    }
    return n;
}
__attribute__((noinline)) bool cx_chase_ok(vec2 pos) { return !CX_NOCHASE || cx_allies_within(pos, 3) > 0; }
__attribute__((noinline)) bool cx_bank_now(std::uint32_t gems, vec2 pos) {
    auto& s = *S;
    if (static_cast<int>(gems) >= CX_BANKB) return true;
    return static_cast<int>(gems) >= CX_BANKN && s.df_altar.get(pos) <= CX_BANKD;
}
__attribute__((noinline)) bool cx_post(std::uint32_t gems) {
    auto& s = *S;
    auto& me = self();
    const vec2 pos = me.position();
    if (gems == 0) CX.hold_since = -1;
    if (s.mode == Mode::recall) return false;
    const vec2 alt = s.altars[0];
    if (s.mode == Mode::bank && CX_HOLD && gems > 0) {
        const int d = s.df_altar.get(pos);
        if (d > 0 && d <= CX_HOLDD && !(gems > 0 && global_submitted_gems(team::ally) + gems >= constants::winning_gems)) {
            int en = 0;
            for (auto& e : vision::champions(team::enemy))
                if (e.is_visible() && e.is_alive() && cheb(e.position(), alt) <= 6) ++en;
            const int al = 1 + cx_allies_within(pos, 4);
            if (en > al) {
                if (CX.hold_since < 0) CX.hold_since = s.round;
                if (s.round - CX.hold_since > CX_HOLDMAX) {
                    // Still outnumbered: farm the deep lane and come back later.
                    CX.hold_since = -1;
                    s.mode = Mode::farm; s.bank_turns = 0; s.bank_skip = -1; s.bank_best = 1 << 30; s.bank_age = 0;
                    s.bank_cool_until = s.round + 20;
                    return false;
                }
                publish(62, 3); s.target = -1; ++s.idle;
                if (d < 6) { refresh_occ(); s.occ.set(pos, false); step_field(s.df_spawn, false); }
                return true;
            }
        }
        CX.hold_since = -1;
        return false;
    }
    if (s.mode != Mode::farm) return false;
    if (s.target >= 0 && s.groups[s.target].st == 1 && s.groups[s.target].tiles().is_set(pos)) return false;
    if (CX_ESC > 0) {
        // Join a banking ally: bank too when carrying, otherwise stand guard beside it.
        for (auto& a : vision::champions(team::ally)) {
            if (!a.is_visible() || !a.is_alive()) continue;
            const vec2 p = a.position();
            if (p == pos || cx_partner(s.type, a.champion_name())) continue;
            const int i = cx_idx(a.champion_name());
            if (i < 0 || i > 5 || (s.shm[i] >> 6) != 3) continue;
            if (cheb(pos, p) > CX_ESC || cheb(p, alt) > 10) continue;
            if (gems > 0) { s.mode = Mode::bank; s.bank_turns = 0; s.bank_skip = -1; s.bank_best = 1 << 30; s.bank_age = 0; return false; }
            publish(-1, 0); s.target = -1;
            if (cheb(pos, p) > 2) { s.goals.clear(); s.goals.draw_chebyshev(p, 2.0f); s.goals &= ~s.terrain; step_to_goals(true); }
            ++s.idle;
            return true;
        }
    }
    if (CX_LEASH > 0) {
        int best = 99; vec2 w{};
        for (auto& a : vision::champions(team::ally)) {
            if (!a.is_visible() || !a.is_alive()) continue;
            const vec2 p = a.position();
            if (p == pos || cx_partner(s.type, a.champion_name())) continue;
            if (s.df_spawn.get(p) <= 4) continue;
            const int d = cheb(pos, p);
            if (d < best) { best = d; w = p; }
        }
        if (best < 99 && best > CX_LEASH && !(s.target >= 0 && s.groups[s.target].st == 0 && [&] { for (auto& c : s.groups[s.target].camps) if (c.hp > 0 && cheb(pos, c.pos) <= s.range + 1) return true; return false; }())) {
            s.goals.clear(); s.goals.draw_chebyshev(w, 2.0f); s.goals &= ~s.terrain;
            if (step_to_goals(true)) { ++s.idle; return true; }
        }
    }
    return false;
}

// ---- lane sweep helpers ----
__attribute__((noinline)) int ls_front() {
    auto& s = *S;
    if (ls_round == s.round) return ls_f;
    if (ls_north < 0) {
        int sy = 0, n = 0;
        for (auto p : map::spawn_region(team::ally).positions()) { sy += p.y; ++n; }
        ls_north = (n > 0 && sy < n * (s.H / 2)) ? 1 : 0;
    }
    ls_round = s.round; ls_f = -1;
    int best = 1 << 20;
    for (int gi = 0; gi < static_cast<int>(s.groups.size()); ++gi) {
        const auto& g = s.groups[gi];
        if (g.st != 0 || g.camps.empty()) continue;
        bool ok = true;
        for (auto& c : g.camps) {
            const bool lane = (c.str == 20.0f && c.maxhp == 1350.0f) || (LS_ELK && c.str == 60.0f && c.maxhp == 2400.0f);
            const bool enemy_side = ls_north ? c.pos.y > s.H / 2 : c.pos.y < s.H / 2;
            if (!lane || enemy_side) ok = false;
        }
        if (ok && g.spawn_dist < best) { best = g.spawn_dist; ls_f = gi; }
    }
    return ls_f;
}
__attribute__((noinline)) bool ls_allow(int gi) {
    auto& s = *S;
    if (s.round > LS_END) return true;
    if (s.groups[gi].st == 1) return true;
    const int f = ls_front();
    return f < 0 || gi == f;
}

// ---- collision bag discipline ----
__attribute__((noinline)) void coll_pre(std::uint32_t gems) {
    auto& s = *S;
    if (gems == 0) { CB.zero_round = s.round; CB.wait = false; }
    CB.prev_bank = s.mode == Mode::bank;
}

// Nearest visible living ally other than ourselves (and our Cassi/Issac partner).
__attribute__((noinline)) int coll_ally_dist(vec2 pos, vec2& where) {
    auto& s = *S;
    int best = 99;
    for (auto& a : vision::champions(team::ally)) {
        if (!a.is_visible() || !a.is_alive()) continue;
        const vec2 p = a.position();
        if (p == pos) continue;
        const auto t = a.champion_name();
        if ((s.type == champion_type::cassi || s.type == champion_type::issac)
            && (t == champion_type::cassi || t == champion_type::issac)) continue;
        const int d = cheb(pos, p);
        if (d < best) { best = d; where = p; }
    }
    return best;
}

__attribute__((noinline)) void coll_reset_bank() {
    auto& s = *S;
    s.bank_turns = 0; s.bank_skip = -1; s.bank_best = 1 << 30; s.bank_age = 0; s.bank_cool_until = -1;
}

__attribute__((noinline)) bool coll_post(std::uint32_t gems, std::uint32_t banked) {
    auto& s = *S;
    auto& me = self();
    const vec2 pos = me.position();
#if CXT2_COLLISION_BANK18
    if(s.mode==Mode::farm && s.type==champion_type::ryth && gems>=18
        && s.df_altar.get(pos)<=2 && s.round>=s.bank_cool_until
        && !cxt2_engaged(pos) && cxt2_collision())s.mode=Mode::bank;
#endif

    {
        if (CB_C > 0 && s.mode == Mode::farm && gems < 60 && !CB.wait) {
            // Escort a visible must-bank carrier.
            for (auto& a : vision::champions(team::ally)) {
                if (!a.is_visible() || !a.is_alive()) continue;
                const vec2 p = a.position();
                if (p == pos) continue;
                const int g = static_cast<int>(a.gem_count());
                if (g < CB_C && !(g > 0 && banked + g >= constants::winning_gems)) continue;
                const int d = cheb(pos, p);
                if (d > CB_CR) continue;
                if (s.target >= 0 && s.groups[s.target].st == 1 && s.groups[s.target].tiles().is_set(pos)) break;
                publish(-1, 0); s.target = -1;
                if (d > 2) { s.goals.clear(); s.goals.draw_chebyshev(p, 1.0f); step_to_goals(true); }
                ++s.idle;
                return true;
            }
        }
    }
    if (s.mode == Mode::recall || gems == 0) return false;
    const bool big = CB_C > 0 && (static_cast<int>(gems) >= CB_C || banked + gems >= constants::winning_gems);
    // (b) A failed bank run with a real bag: regroup before the next attempt.
    if (CB_B > 0 && !CB.wait && static_cast<int>(gems) >= CB_B) {
        const bool gave_up = CB.prev_bank && s.mode == Mode::farm && s.bank_cool_until == s.round + P_BANK_COOL;
        const bool beaten = CB.prev_bank && s.df_altar.get(pos) <= 6
            && (s.bank_unsafe_round >= s.round - 1 || (s.round - s.hurt_round <= 1 && s.round - s.enemy_round <= 1));
        if (gave_up || beaten) { CB.wait = true; CB.wait_since = s.round; }
    }
    if (CB.wait) {
        vec2 ally{};
        const int ad = coll_ally_dist(pos, ally);
        if (ad <= CB_BR || s.round - CB.wait_since > CB_BMAX) {
            CB.wait = false;
            coll_reset_bank();
            s.mode = Mode::bank;
            return false;
        }
        publish(-1, 0); s.target = -1; s.mode = Mode::farm;
        if (CB_BMODE == 1 && ad < 99) {
            s.goals.clear(); s.goals.draw_chebyshev(ally, 1.0f); step_to_goals(true);
        } else if (s.df_altar.get(pos) < CB_HOLD) {
            refresh_occ(); s.occ.set(pos, false); step_field(s.df_spawn, false);
        }
        ++s.idle;
        return true;
    }
    if (big) {
        if (s.mode != Mode::bank) { coll_reset_bank(); s.mode = Mode::bank; }
        s.bank_age = 0;
        // Safest altar: fewest visible enemies within 6.
        if (s.altars.size() >= 2) {
            int cnt[2] = { 0, 0 };
            for (auto& e : vision::champions(team::enemy)) {
                if (!e.is_visible() || !e.is_alive()) continue;
                for (int i = 0; i < 2; ++i) if (cheb(e.position(), s.altars[i]) <= 6) ++cnt[i];
            }
            if (cnt[0] != cnt[1]) s.bank_skip = cnt[0] > cnt[1] ? 0 : 1;
        }
        return false;
    }
    if (CB_D > 0 && s.mode == Mode::farm && s.round - CB.zero_round > CB_D
        && !(s.target >= 0 && s.groups[s.target].st == 1)) {
        coll_reset_bank();
        s.mode = Mode::bank;
    }
    return false;
}

__attribute__((noinline)) bool coll_choice(int choice, std::uint32_t gems) {
    auto& s = *S;
    if (CB_A <= 0 || choice < 0 || static_cast<int>(gems) < CB_A || choice == s.target) return false;
    const auto& g = s.groups[choice];
    if (CO.altar_dist[choice] < CB_AD && g.spawn_dist < CB_SD) return false;
    s.mode = Mode::bank; publish(-1, 0); s.target = -1; ++s.t_bank; do_bank();
    return true;
}

// Finish a camp we can kill before it kills us: no enemy in sight lately, remaining camp health at our damage rate.
__attribute__((noinline)) bool fb_commit(float hp, float maxhp) {
    auto& s = *S;
    auto& me = self();
#if FB_PAD
    for (volatile int i = 0; i < FB_PAD; i = i + 1) {}
#endif
    if (s.mode != Mode::farm || s.target < 0 || s.round - s.enemy_round < FB_CQ) return false;
    const auto& g = s.groups[s.target];
    const vec2 pos = me.position();
    if (g.st == 1) return g.tiles().is_set(pos) && g.alive_dps() == 0;
    if (g.st != 0) return false;
    bool near = false;
    for (auto& c : g.camps) if (c.hp > 0 && cheb(pos, c.pos) <= s.range + 1) near = true;
    if (!near) return false;
    if (FB_COMMIT >= 2) { float top = 0; for (auto& c : g.camps) top = std::max(top, c.str); if (top < 60.0f) return false; }
    const float mine = cxh_attack(me) * 1.1f;
    const float rounds = g.hp() / mine + 1.0f;
    float rate = g.alive_dps() * s.jungle;
    // FB_COMMIT 3: the health we actually lost per round in this fight (Varg's shield halves an elk), with 25% slack.
    if (FB_COMMIT == 3 && s.fb_eng_tgt == s.target && s.round - s.fb_eng_r0 >= 6)
        rate = std::min(rate, 1.25f * (s.fb_eng_hp0 - hp) / static_cast<float>(s.round - s.fb_eng_r0));
    const bool ok = rate * rounds < hp - FB_CMARGIN * maxhp;
#if P_LOG
    if (ok && s.round % 4 == 0) std::cout << "COMMIT " << s.round << " " << static_cast<int>(s.type) << " hp " << hp << " camp " << g.hp() << "\n";
#endif
    return ok;
}

#if Q9_LNSHOP
// q9 (17 Oct): lanes (90x30) shopping recalls are 60-tile home round trips from the midfield camps; platform lanes
// went 1-8 since 18:50 with ~3x more gold-triggered recalls than opponents. Q9_LNSHOP replaces the shop limit there.
// Only reached once the old limit is met (never on a first turn); other maps decide identically (fuel cost +~10 then).
__attribute__((noinline)) bool q9_lanes_hold(std::uint32_t gold) {
    return S->W == 90 && S->H == 30 && gold < static_cast<std::uint32_t>(Q9_LNSHOP);
}
#define Q9_GOLD_DUE(g, lim) ([&] { const auto q9g = (g); return q9g >= (lim) && !q9_lanes_hold(q9g); }())
#else
#define Q9_GOLD_DUE(g, lim) ((g) >= (lim))
#endif
// Bank from spawn: recall (heals) and walk the short spawn-to-altar path when that beats walking.
__attribute__((noinline)) bool fb_recall_bank(vec2 pos) {
    auto& s = *S;
    auto& me = self();
    if (FB_RBMAP == 1 && !s.rows_like) return false;
    if (me.is_submitting_gems() || s.round - s.enemy_round < P_RECALL_QUIET) return false;
    if (map::spawn_region(team::ally).is_set(pos)) return false;
    const int walk = s.df_altar.get(pos);
    return walk != UNREACH && walk > constants::recall_time + s.spawn_altar + FB_RB;
}

#if RC_ON
// While recalling: true when a visible enemy can damage us before the recall completes and the risk is worth cancelling.
__attribute__((noinline)) bool rc_threat(vec2 pos, float hp, float maxhp) {
    if (hp >= RC_HP * maxhp) return false;
    int left = 0;
    try { left = self().remaining_channel_time(); } catch (const cambc::error&) { return false; }
    if (left > RC_LEFT) return false;
    const int h = std::max(1, std::min(left, 1 + RC_TURNS));
    float dmg = 0; bool any = false;
    for (auto& e : vision::champions(team::enemy)) {
        if (!e.is_visible() || !e.is_alive()) continue;
        const int d = cheb(pos, e.position());
        const bool melee = e.attack_range() <= 1;
        const int reach = melee ? h + 1 + RC_DASH : (h - 1) + std::max(RC_SPR, static_cast<int>(e.attack_range()));
        if (d > reach) continue;
        any = true;
        dmg += h * (cxh_attack(e) * (melee ? 2.0f : 1.0f)) + champ_burst(e);
    }
    if (!any) return false;
    return RC_DMG <= 0.0f || dmg >= RC_DMG * hp;
}
// After cancelling: step (dash first) away from visible enemies, preferring the way home.
__attribute__((noinline)) void rc_flee() {
    auto& s = *S;
    auto& me = self();
    const vec2 pos = me.position();
    const auto enemies = vision::champions(team::enemy);
    refresh_occ();
    auto near_d = [&](vec2 q) { int nd = 99; for (auto& e : enemies) if (e.is_visible() && e.is_alive()) nd = std::min(nd, cheb(q, e.position())); return nd; };
    if (!s.cast && me.can_cast()) {
        for (auto& ab : me.abilities()) {
            if (!ab.is_dash() || !ab.is_ready()) continue;
            vec2 dbest = pos;
            int dbd = near_d(pos), dhome = s.df_spawn.get(pos);
            for (int dx = -2; dx <= 2; ++dx) for (int dy = -2; dy <= 2; ++dy) {
                vec2 q{ pos.x + dx, pos.y + dy };
                if (!in_bounds(q) || s.occ.is_set(q)) continue;
                const int home = s.df_spawn.get(q);
                if (home == UNREACH) continue;
                const int nd = near_d(q);
                if (nd > dbd || (nd == dbd && home < dhome)) { dbest = q; dbd = nd; dhome = home; }
            }
            if (dbest != pos) { try { if (ab.use(dbest)) { s.cast = true; ++s.casts; } } catch (const cambc::error&) {} }
            break;
        }
    }
    const vec2 here = me.position();
    vec2 best = here;
    int best_d = near_d(here), best_home = s.df_spawn.get(here);
    for (auto d : DIRS) {
        vec2 p{ here.x + d.x, here.y + d.y };
        if (!in_bounds(p) || s.occ.is_set(p)) continue;
        const int home = s.df_spawn.get(p);
        if (home == UNREACH) continue;
        const int nd = near_d(p);
        if (nd > best_d || (nd == best_d && home < best_home)) { best = p; best_d = nd; best_home = home; }
    }
    if (best != here && me.can_move()) { try { if (me.move_absolute(best)) { s.moved = true; ++s.moves; } } catch (const cambc::error&) {} }
    publish(-1, 0);
}
#endif

// ph (18 Oct, pilgrimage 50x65 only, PH_ON): coordinated push on the bottom-centre 144-gem elk herd.
// Deterministic squad by champion type (PH_SQ bitmask) and round window [PH_R0, PH_R1): squad members target the
// herd while it is alive, hold on a staging ring (Chebyshev 4..PH_STG of every herd camp, outside reach 3) until
// PH_N allied bodies are within PH_RAD of the herd centre, then attack together. A squad that waits PH_WAIT rounds
// without gathering drops the herd for PH_COOL rounds. Everyone else (and the squad outside the window) farms as before.
// PH_V92 > 0: Varg (outside the squad) multiplies the score of the own-side 92-gem inner-lane group by PH_V92.
#ifndef PH_ON
#define PH_ON 0
#endif
#if PH_ON
#ifndef PH_SQ
#define PH_SQ 1541    // Ryth(0) | Vendetta(2) | Ren(9) | Varg(10)
#endif
#ifndef PH_N
#define PH_N 3
#endif
#ifndef PH_R0
#define PH_R0 0
#endif
#ifndef PH_R1
#define PH_R1 2000
#endif
#ifndef PH_RAD
#define PH_RAD 6
#endif
#ifndef PH_STG
#define PH_STG 5
#endif
#ifndef PH_WAIT
#define PH_WAIT 40
#endif
#ifndef PH_COOL
#define PH_COOL 40
#endif
#ifndef PH_HP
#define PH_HP 0.8f    // join a fresh herd only at or above this health fraction
#endif
#ifndef PH_G
#define PH_G 60       // carriers with this many gems bank instead of joining a fresh herd
#endif
#ifndef PH_RH
#define PH_RH 0       // 1: also rally on a hurt herd while its health is above PH_FIN of full
#endif
#ifndef PH_FIN
#define PH_FIN 0.3f
#endif
#ifndef PH_PRE
#define PH_PRE 0      // also walk to the staging ring this many rounds before a respawn
#endif
#ifndef PH_LOG
#define PH_LOG 0
#endif
#ifndef PH_CAPLOG
#define PH_CAPLOG 1   // one stdout line per herd capture (for evaluation counts)
#endif
static int g_ph = -1;           // -1 unknown, -2 not this map, else herd group index
static int ph_wait0 = -1, ph_cool = 0;
__attribute__((noinline)) int ph_herd() {
    auto& s = *S;
    if (g_ph != -1) return g_ph;
    g_ph = -2;
    if (!(s.W == 50 && s.H == 65)) return g_ph;
    for (int gi = 0; gi < static_cast<int>(s.groups.size()); ++gi)
        if (s.groups[gi].gems >= 144) { g_ph = gi; break; }
#if PH_LOG
    if (s.type == champion_type::varg) for (auto& g : s.groups) std::cout << "PHG " << g.gems << " n" << g.camps.size() << " @" << g.camps[0].pos.x << "," << g.camps[0].pos.y << " sd " << g.spawn_dist << " mdps " << g.mdps << " gold " << g.gold << "\n";
    if (g_ph >= 0) { auto& g = s.groups[g_ph]; std::cout << "PHI g" << g_ph << " camps " << g.camps.size() << " mdps " << g.mdps << " respawn " << g.respawn << " hp " << g.hp() << " @" << g.camps[0].pos.x << "," << g.camps[0].pos.y << "\n"; }
#endif
    return g_ph;
}
__attribute__((noinline)) bool ph_squad() { return ((PH_SQ >> static_cast<int>(S->type)) & 1) != 0; }
__attribute__((noinline)) vec2 ph_centre(const Group& g) {
    int x = 0, y = 0;
    for (auto& c : g.camps) { x += c.pos.x; y += c.pos.y; }
    const int n = std::max<int>(1, static_cast<int>(g.camps.size()));
    return vec2{ x / n, y / n };
}
__attribute__((noinline)) float ph_full(const Group& g) { float f = 0; for (auto& c : g.camps) f += c.maxhp; return f; }
__attribute__((noinline)) bool ph_fresh(const Group& g) {
    for (auto& c : g.camps) if (c.hp < c.maxhp) return false;
    return true;
}
// Override the farm target: the herd for squad members inside the window.
__attribute__((noinline)) int ph_pick(float hp, float maxhp, int gems) {
    auto& s = *S;
    const int h = ph_herd();
    if (h < 0 || !ph_squad() || s.round < PH_R0 || s.round >= PH_R1) return -1;
    auto& g = s.groups[h];
    if (g.st == 1) return -1;
    const bool hurt = g.st == 0 && !ph_fresh(g);
    if (hurt && (!PH_RH || g.hp() < PH_FIN * ph_full(g))) return (hp >= P_RECALL_HP * maxhp + 0.1f * maxhp) ? h : -1;
    if (s.round < ph_cool || gems >= PH_G || hp < PH_HP * maxhp) return -1;
    if (g.st == 2 && (PH_PRE <= 0 || g.respawn_round - s.round > PH_PRE)) return -1;
    return h;
}
// Staging: true if this turn was spent holding/walking to the ring.
__attribute__((noinline)) bool ph_stage(Group& g) {
    auto& s = *S;
    const int h = ph_herd();
    if (h < 0 || s.target != h || !ph_squad()) return false;
    if (g.st == 0 && !ph_fresh(g) && (!PH_RH || g.hp() < PH_FIN * ph_full(g))) { ph_wait0 = -1; return false; }
    const vec2 pos = self().position();
    const vec2 c = ph_centre(g);
    const int bodies = ix_bodies_near(c, PH_RAD);
    int dmin = 99;
    for (auto& cp : g.camps) dmin = std::min(dmin, cheb(pos, cp.pos));
    if (g.st == 0 && bodies >= PH_N && dmin <= PH_STG + 1) {
#if PH_LOG
        std::cout << "PHGO " << s.round << " " << static_cast<int>(s.type) << " n " << bodies << "\n";
#endif
        ph_wait0 = -1; return false;
    }
    if (dmin <= PH_STG + 1) {
        if (ph_wait0 < 0) ph_wait0 = s.round;
        if (g.st == 0 && s.round - ph_wait0 > PH_WAIT) {
            ph_cool = s.round + PH_COOL; ph_wait0 = -1; s.target = -1; publish(-1, 0);
#if PH_LOG
            std::cout << "PHX " << s.round << " " << static_cast<int>(s.type) << " n " << bodies << "\n";
#endif
            return true;
        }
    } else ph_wait0 = -1;
    s.goals.clear();
    for (auto& cp : g.camps) s.goals.draw_chebyshev(cp.pos, static_cast<float>(PH_STG));
    bitboard inner; inner.clear();
    for (auto& cp : g.camps) inner.draw_chebyshev(cp.pos, 3.0f);
    s.goals &= ~inner; s.goals &= ~s.terrain; s.goals &= ~g.tiles();
    step_to_goals(true);
#if PH_LOG
    if (s.round % 5 == 0) std::cout << "PHW " << s.round << " " << static_cast<int>(s.type) << " " << pos.x << "," << pos.y << " n " << bodies << " dmin " << dmin << " st " << g.st << "\n";
#endif
    ++s.t_wait;
    return true;
}
#endif


// Local EV experiment: one escape decision before optional combat and channels.
// Only Eight Varg with a substantial bag. Unknown enemies are never fabricated;
// a seen position is retained for at most three rounds with expanding reach.
struct EvThreat { vec2 pos{}; int seen=-99; float reach=0, hit=0; };
std::array<EvThreat,13> EV;
bool ev_run(bool requested) {
    static int tavi_draft=-1;
    if(tavi_draft<0){const auto d=draft::current_draft(team::enemy);tavi_draft=(std::find(d.begin(),d.end(),champion_draft::tavi)!=d.end() || std::find(d.begin(),d.end(),champion_draft::heart_of_ruin)!=d.end());}
    if(!tavi_draft)return false;
    auto& s=*S;auto& me=self();
    const vec2 pos=me.position();
    if(!me.is_alive() || me.gem_count()<72 || map::spawn_region(team::ally).is_set(pos)) {
        for(auto& e:EV)e.seen=-99;
        return false;
    }
    for(const auto& e:vision::champions(team::enemy)) {
        const int i=static_cast<int>(e.champion_name());
        if(i<0 || i>=static_cast<int>(EV.size()))continue;
        if(!e.is_alive()){EV[i].seen=-99;continue;}
        if(!e.is_visible())continue;
        float reach=static_cast<float>(e.attack_range()),hit=cxh_attack(e);
        for(const auto& ab:e.abilities()) {
            if(!ab.is_damage() && !ab.is_aoe_damage() && !ab.is_cc() && !(cxh_present && ab.is_dash() && e.champion_name()==champion_type::heart_of_ruin))continue;
            const auto& info=ab.info();if(!info)continue;
            std::visit([&](const auto& p){
                if constexpr(requires{p.range;}) {
                    float r=p.range;
                    if constexpr(requires{p.radius;})r+=p.radius;
                    reach=std::max(reach,r);
                }
            },*info);
            hit=std::max(hit,ab.damage_amount());
        }
        EV[i]={e.position(),s.round,reach,hit};
    }
    if(me.is_submitting_gems() || me.is_teleporting())return false;
    if(!requested && !me.is_recalling() && s.mode!=Mode::recall)return false;
    const auto items=me.item_count();
    const bool home=items[item_type::home_emblem_of_strength]>0 || items[item_type::home_emblem_of_focus]>0 || items[item_type::home_emblem_of_health]>0;
    int ticks=home ? (constants::recall_time+1)/2 : constants::recall_time;
    if(me.is_recalling())ticks=std::max(1,me.remaining_channel_time());
    bool unsafe=false;
    for(auto type:{champion_type::tavi,champion_type::heart_of_ruin}) {
        const auto& cc=EV[static_cast<int>(type)];
        if(s.round-cc.seen<=3 && cheb(pos,cc.pos)<=cc.reach+(s.round-cc.seen)+ticks-1)unsafe=true;
    }
    if(!unsafe)return false;
    if(me.is_recalling())me.cancel_recall();
    s.mode=Mode::recall;s.enemy_round=s.round;s.target=-1;end_mission();publish(-1,0);++s.t_recall;
    // Protect the retreat with the existing movement-compatible Varg shield.
    if(me.can_cast())for(auto& ab:me.abilities())if(ab.is_shield() && ab.is_ready()) {
        try{if(ab.use()){s.cast=true;++s.casts;}}catch(const cambc::error&){}
        break;
    }
    refresh_occ();s.occ.set(pos,false);
    auto score=[&](vec2 p) {
        float danger=0,clearance=99;
        for(const auto& e:EV) {
            const int age=s.round-e.seen;if(age>3)continue;
            const float d=cheb(p,e.pos)-e.reach-age;
            clearance=std::min(clearance,d);
            if(d<=1)danger+=e.hit*(2-d);
        }
        for(const auto& g:s.groups)for(const auto& c:g.camps)
            if(c.hp>0 && c.str>0 && c.h.is_visible() && c.h.is_awake() && cheb(p,c.pos)<=c.range)
                danger+=c.str*s.jungle;
        return std::array<float,3>{danger,-clearance,static_cast<float>(s.df_spawn.get(p))};
    };
    vec2 best=pos;auto best_score=score(pos);
    for(auto d:DIRS) {
        const vec2 p{pos.x+d.x,pos.y+d.y};
        if(!in_bounds(p) || s.occ.is_set(p) || s.df_spawn.get(p)==UNREACH)continue;
        const auto value=score(p);
        if(value<best_score){best=p;best_score=value;}
    }
    if(best!=pos && me.can_move()) {
        try{if(me.move_absolute(best)){s.moved=true;++s.moves;}}catch(const cambc::error&){}
    }
    return true;
}

#if ED_MASK || ED_MASK2 || ED_HOLD
__attribute__((noinline)) void ed_pre() {
    auto& s = *S;
    auto& me = self();
    const int gems = static_cast<int>(me.gem_count());
    if (gems == 0) { s.ed_on = 0; s.ed_dec = -1; s.ed_near = 0; return; }
    if (s.mode != Mode::farm && s.mode != Mode::bank) return;
#if ED_MASK || ED_MASK2
    if (!s.ed_on) {
        const int eb = static_cast<int>(global_submitted_gems(team::enemy));
        if ((ED_MASK && NXB(ED_MASK) && eb >= ED_EB && gems >= ED_BAG) || (ED_MASK2 && NXB(ED_MASK2) && eb >= ED_EB2 && gems >= ED_BAG2)) {
            s.ed_on = 1;
#if ED_LOG
            std::cout << "EDON " << s.round << " " << static_cast<int>(s.type) << " g " << gems << " eb " << eb << " m " << static_cast<int>(s.mode) << "\n";
#endif
        }
    }
    if (s.ed_on) { s.mode = Mode::bank; s.bank_age = 0; }
#endif
#if ED_HOLD
    if (NXB(ED_HOLD) && s.mode == Mode::bank && s.bank_skip < 0 && s.altars.size() >= 2) {
        const vec2 pos = me.position();
        int near = 0;
        for (std::size_t i = 1; i < s.altars.size(); ++i) if (cheb(pos, s.altars[i]) < cheb(pos, s.altars[near])) near = static_cast<int>(i);
        const vec2 a = s.altars[near];
        const int d = cheb(pos, a);
        if (s.ed_dec < 0 && d <= ED_HV) {
            int n = 0; bool lock = false;
            for (auto& e : vision::champions(team::enemy)) if (e.is_visible() && e.is_alive() && cheb(e.position(), a) <= ED_HR) ++n;
            for (const auto& L : vision::altar_locks(team::enemy)) if (cheb(L.position(), a) <= 2) lock = true;
            s.ed_dec = (n >= ED_HN || lock) ? 1 : 0;
            if (s.ed_dec) s.bank_skip = near;
#if ED_LOG
            std::cout << "EDHOLD " << s.round << " " << static_cast<int>(s.type) << " g " << gems << " altar " << near << " n " << n << " lock " << lock << " skip " << s.ed_dec << "\n";
#endif
        }
        if (s.bank_skip < 0 && d <= ED_SR && (!ED_SE || s.round - s.enemy_round <= 1) && ++s.ed_near > ED_STALL) {
            s.bank_skip = near;
#if ED_LOG
            std::cout << "EDSTALL " << s.round << " " << static_cast<int>(s.type) << " g " << gems << " altar " << near << "\n";
#endif
        }
    }
#endif
}
#endif

static int cxk_until=-100;
#if Z362_ON
#include "z362_deposit.h"
#endif
#line 6449 "z114_policy.cpp"
void play_turn() {
    mx_init();
    auto& me = self();
    if (!S) init();
    auto& s = *S;
    const int now = static_cast<int>(round_number());
    if (s.last_round >= 0 && now > s.last_round + 1) s.late += now - s.last_round - 1;
    s.last_round = now;
    s.round = now;
    s.moved = s.cast = false;
    ++s.turns;
    if (s.turns == 3) { ccx_map = ccx_::detect(); g_gwc = GWC_ON && ccx_map == 2; }
#if Z327_ON
    // Exact layout, once on turn 3; no added SDK calls on turns 1-2.
    if(s.turns==3 && s.W==80 && s.H==80) z327_expanse=z327_::detect()==1;
#endif
#if DR_PAD
    for (volatile int i = 0; i < DR_PAD; i = i + 1) {}
#endif
    read_shm();
    ce_tick();

    if (!me.is_alive()) {
#if P_LOG
        if (!s.was_dead) std::cout << "DEAD " << s.round << " " << static_cast<int>(s.type) << " lasthp " << s.last_hp << " lastgems " << s.last_gems << " mode " << static_cast<int>(s.mode) << " tgt " << s.target << "\n";
#endif
        s.was_dead = true;
        if (IX_ON && IX_MODE == 3 && g_ix) ix_hp_pub(0);
        end_mission();
        shop(); publish(-1, 0); s.target = -1; s.mode = Mode::farm; return;
    }
    s.was_dead = false;
    if (IX_ON && IX_MODE == 3 && g_ix) ix_hp_pub(std::max(1, static_cast<int>(100.0f * me.health() / me.max_health())));
    vec2 pos = me.position();
#if Z362_ON
    if (z362_deposit_here(me, s.altars)) { ++s.t_bank; return; }
#endif
#line 6481 "z114_policy.cpp"
    {
        const int now_hp = static_cast<int>(me.health());
        if (s.seen_hp >= 0 && now_hp < s.seen_hp) s.hurt_round = s.round;
        s.seen_hp = now_hp;
    }
#if IX_LOG
    if (s.round % 5 == 0) std::cout << "IXT " << s.round << " " << static_cast<int>(s.type) << " " << pos.x << "," << pos.y << " hp " << static_cast<int>(me.health()) << " t " << s.target << " m " << static_cast<int>(s.mode) << " g " << me.gem_count() << " ghp " << (s.target >= 0 ? static_cast<int>(s.groups[s.target].hp()) : -1) << "\n";
#endif
    if(s.W==41 && s.H==30 && s.type==champion_type::varg && ev_run(false))return;
    const bool at_spawn = map::spawn_region(team::ally).is_set(pos);
    if (at_spawn || s.type == champion_type::tavi) shop();
#if CXK_MAPS
    if ((CXK_MAPS & 0x200) && nxm()==9 && (CXK_SW & 64) && me.gem_count()>=CXK_GEMS && !at_spawn) {
        bool danger=false;
        for(const auto& e:vision::champions(team::enemy))if(e.is_visible() && e.is_alive()) {
            const auto t=e.champion_name();
            const bool caster=t==champion_type::stellar || t==champion_type::cassi || t==champion_type::ren;
            if(cheb(pos,e.position()) <= (caster?5:3))danger=true;
        }
        if(me.is_recalling() && me.remaining_channel_time()>1 && danger && me.health()<.65f*me.max_health()) {
            try{me.cancel_recall();}catch(const cambc::error&){}
            if(!me.is_recalling()){cxk_until=s.round+4;s.mode=Mode::recall;}
        }
        if(!me.is_recalling() && s.round<cxk_until) {
            end_mission();publish(-1,0);s.target=-1;++s.t_recall;do_recall();return;
        }
    }
#endif
    if (me.is_recalling()) {
#if RC_ON
#if RA_ON
        if (((((RC_MAPS | RA_MAPS) >> nxm()) & 1) || BL_R) && rc_threat(
#else
        if ((((RC_MAPS >> nxm()) & 1) || BL_R) && rc_threat(
#endif
            pos, me.health(), me.max_health())) {
            bool ok = false;
            try { me.cancel_recall(); ok = !me.is_recalling(); } catch (const cambc::error&) {}
#if RC_LOG
            std::cout << "RCX " << s.round << " " << static_cast<int>(s.type) << " hp " << static_cast<int>(me.health()) << " at " << pos.x << "," << pos.y << " ok " << ok << "\n";
#endif
            if (!ok) { ++s.t_recall; return; }
            s.enemy_round = s.round;
#if RA_ON
            if ((RA_MAPS >> nxm()) & 1) s.ra_round = s.round;
            if (RC_FT || ((RA_ON & 2) && ((RA_MAPS >> nxm()) & 1)) || BL_R) { ++s.t_recall; rc_flee(); return; }
#else
            if (RC_FT || BL_R) { ++s.t_recall; rc_flee(); return; }
#endif
        } else
#endif
        { if (g_gwc && !gwc_ready) gwc_init(); ++s.t_recall; return; }   // cc_crystal2: build the route field on a cheap turn
    }
    if (me.is_submitting_gems()) {
#if P_SAFEBANK
        // Abandon a tribute we would not live to finish.
        const float dps = enemy_dps_near(pos, 5);
        if (dps > 0) {
            int left = 0;
            try { left = me.remaining_channel_time(); } catch (const cambc::error&) {}
            if (left > 1 && s.round - s.hurt_round <= 2 && me.health() / dps < left + 1.0f) {
                try { me.cancel_submit_gems(); } catch (const cambc::error&) {}
                s.bank_unsafe_round = s.round;
            }
        }
        if (me.is_submitting_gems()) { ++s.t_bank; return; }
#else
        ++s.t_bank; return;
#endif
    }
    if (me.is_teleporting()) { ++s.t_bank; return; }
    if (ccx_teleport()) return;
    if (gwc_act()) return;
#if RA_ON & 2
    if (!at_spawn && ((RA_MAPS >> nxm()) & 1) && s.round - s.ra_round < RA_K) { ++s.t_recall; rc_flee(); return; }
#endif
    const float hp = me.health(), maxhp = me.max_health();
    if (at_spawn) {
        if (s.mode == Mode::recall) {
            s.mode = Mode::farm; s.target = -1;
            const bool monolith_map = s.W == 50 && s.H == 50 && !s.rows_like;
            bool physical_interlock = false;
            if (s.il) {
                const auto foe = draft::current_draft(team::enemy);
                const auto has = [&](champion_draft c) { return std::find(foe.begin(), foe.end(), c) != foe.end(); };
                physical_interlock = !has(champion_draft::stellar) && !has(champion_draft::tavi)
                    && !has(champion_draft::prism) && !has(champion_draft::willow)
                    && !has(champion_draft::cassi_and_issac);
            }
            if (monolith_map || physical_interlock) {
                // Recall changed the route: do not treat the fresh altar trip as stalled.
                s.bank_turns = 0; s.bank_skip = -1; s.bank_best = 1 << 30; s.bank_age = 0;
            }
        }
        if (hp < 0.9f * maxhp) { publish(-1, 0); ++s.t_heal; return; }
    }
#if FB_LOG
    if (static_cast<int>(me.gem_count()) > s.last_gems) std::cout << "CAP " << s.round << " " << static_cast<int>(s.type) << " +" << (static_cast<int>(me.gem_count()) - s.last_gems) << " @" << pos.x << "," << pos.y << " hp " << hp << "\n";
#endif
#if PH_ON && PH_CAPLOG
    if (g_pg && static_cast<int>(me.gem_count()) >= s.last_gems + 144 && g_ph >= 0 && cheb(pos, ph_centre(s.groups[g_ph])) <= 4) std::cout << "PHCAP " << s.round << " " << static_cast<int>(s.type) << "\n";
#endif
    s.last_hp = static_cast<int>(hp);
    s.last_gems = static_cast<int>(me.gem_count());
    place_lock();
#if P_LOCK
    // From spawn, one champion at a time carries a lock to an altar we have not locked yet.
    if (at_spawn && s.mission < 0 && s.shm[7] == 0 && static_cast<int>(s.altars.size()) <= P_LOCKMAX && !(RX_ROWS && s.rows_like)
        && me.consumables()[consumable_type::altar_lock].count() > 0) {
        int pick = -1, far = -1;
        for (std::size_t i = 0; i < s.altars.size() && i < 8; ++i) {
            if (s.shm[6] & (1u << i)) continue;
            int d = UNREACH;
            for (auto dd : DIRS) {
                vec2 q{ s.altars[i].x + dd.x, s.altars[i].y + dd.y };
                if (in_bounds(q)) d = std::min<int>(d, s.df_spawn.get(q));
            }
            if (d != UNREACH && d > far) { far = d; pick = static_cast<int>(i); }
        }
        if (pick >= 0) { s.mission = pick; shared_memory[7] = static_cast<std::byte>(pick + 1); s.shm[7] = static_cast<std::uint8_t>(pick + 1); }
    }
#endif
    if (!me.can_move() && !me.can_attack()) return;

    if (s.jungle == 1.0f && me.item_count()[item_type::jungle_emblem_of_strength] > 0) s.jungle = 0.7f;
    observe();
#if Z180_ON
    // Z180: heal the lowest-health injured visible ally in the official mask.
    if (s.type == champion_type::willow && !s.cast && me.can_cast()) {
        for (const auto& z180_ab : me.abilities()) {
            if (!z180_ab.is_heal() || !z180_ab.is_ready() || !z180_ab.can_target_allies()) continue;
            bitboard z180_reach; z180_ab.mask_into(z180_reach);
            std::optional<champion> z180_best;
            float z180_hp = 1e30f;
            for (const auto& z180_ally : vision::champions(team::ally)) {
                if (!z180_ally.is_visible() || !z180_ally.is_alive() || z180_ally.position() == pos) continue;
                if (!z180_reach.is_set(z180_ally.position()) || z180_ally.health() >= z180_ally.max_health()) continue;
                if (z180_ally.health() < z180_hp) { z180_best = z180_ally; z180_hp = z180_ally.health(); }
            }
            if (z180_best && z180_ab.use(*z180_best)) { s.cast = true; ++s.casts; }
            break;
        }
    }
#endif

    if (s.ekmap && s.W == 41 && s.type == champion_type::willow && me.can_cast()) {
        for (const auto& ab : me.abilities()) {
            if (!ab.is_heal() || !ab.is_ready() || !ab.can_target_allies()) continue;
            bitboard reach;
            ab.mask_into(reach);
            std::optional<champion> best;
            float best_value = 0;
            for (const auto& ally : vision::champions(team::ally)) {
                if (!ally.is_visible() || !ally.is_alive() || !reach.is_set(ally.position())) continue;
                const float amount = ab.heal_amount(ally);
                if (amount < 80) continue;
                const float value = amount * (ally.champion_name() == champion_type::cassi ? 1.6f : 1.0f);
                if (value > best_value) { best = ally; best_value = value; }
            }
            if (best) {
                try { if (ab.use(*best)) { s.cast = true; ++s.casts; } }
                catch (const cambc::error&) {}
            }
            break;
        }
    }
#if ED_MASK || ED_MASK2 || ED_HOLD
    if (NXB(ED_MASK | ED_MASK2 | ED_HOLD)) ed_pre();
#endif
    ST4L("pre");
#if ST4_MASK
    if (st4_pre(hp, maxhp)) { ST4L("hook"); return; }
#endif

#if CXK_MAPS
    if ((CXK_MAPS & 0x200) && nxm()==9 && (CXK_SW & 2) && me.gem_count()>=CXK_GEMS && !at_spawn) {
        bool threat=false;for(const auto& e:vision::champions(team::enemy))if(e.is_visible() && e.is_alive() && cheb(me.position(),e.position())<=CXK_NEAR)threat=true;
        if(threat || s.mode==Mode::recall || s.mode==Mode::bank) {
            s.mode=Mode::bank;s.target=-1;end_mission();publish(-1,0);
            if(threat && hp<.75f*maxhp) {rc_flee();return;}
            do_bank();return;
        }
    }
#endif
#if CXK_MAPS
    if ((CXK_MAPS & 0x200) && nxm()==9 && (CXK_SW & 16) && me.gem_count()>=CXK_GEMS && s.mode==Mode::recall && !at_spawn) {
        // Keep the existing home-bound intent. No new bank threshold or capture recall.
        end_mission();publish(-1,0);s.target=-1;++s.t_recall;do_recall();return;
    }
#endif
#if CXK_MAPS
    if ((CXK_MAPS & 0x200) && nxm()==9 && (CXK_SW & 32) && me.gem_count()>=CXK_GEMS && hp<P_RECALL_HP*maxhp && !at_spawn) {
        // Make the already-applicable low-health recall decision before optional contact.
        s.mode=Mode::recall;end_mission();publish(-1,0);s.target=-1;++s.t_recall;do_recall();return;
    }
#endif
    if (sr_map() ? pvp_t<true>(hp, maxhp) : pvp_t<false>(hp, maxhp)) { ++s.t_pvp; return; }  // sr: rushbait_v2 instantiation only

    const std::uint32_t gems = me.gem_count();
    const std::uint32_t banked = global_submitted_gems(team::ally);
    auto engaged = [&] {
        if (s.target < 0) return false;
        auto& g = s.groups[s.target];
        if (g.st == 1) return true;
        if (g.st != 0) return false;
        for (auto& c : g.camps) if (c.hp > 0 && cheb(pos, c.pos) <= s.range + 1) return true;
        return false;
    };

    if (CB.on) coll_pre(gems);
#if FB_COMMIT
    if (s.target >= 0 && s.groups[s.target].st == 0 && engaged()) {
        if (s.fb_eng_tgt != s.target) { s.fb_eng_tgt = s.target; s.fb_eng_r0 = s.round; s.fb_eng_hp0 = hp; }
    } else if (!(s.target >= 0 && s.target == s.fb_eng_tgt)) s.fb_eng_tgt = -1;
    {
        float rth = (FX_RHP > 0.0f && g_nm == 3) ? FX_RHP : P_RECALL_HP;
        if (FD_RG > 0 && !g_mx_cp && (FD_RGWHO == 4095 || ((FD_RGWHO >> static_cast<int>(s.type)) & 1)) && static_cast<int>(me.gem_count()) >= FD_RG && NXB(FD_RGMAPS)) rth = std::max(rth, (FD_RHP2MAPS && NXB(FD_RHP2MAPS)) ? FD_RHP2 : FD_RHP);
        if (FD_RGI > 0.0f && s.type == champion_type::issac && static_cast<int>(me.gem_count()) >= FD_RG && NXB(FD_RGIMAPS)) rth = std::max(rth, FD_RGI);
#if TF_ON
        // tf: low after a PvP exchange: go home and heal (pvp_t has already stepped us clear of every enemy within 6).
        if (tf_on() && tf_dis() && s.round - s.tf_pvp <= TF_HOLD) rth = std::max(rth, TF_LOWHP);
#endif
        if (hp < rth * maxhp && !at_spawn && !fb_commit(hp, maxhp)) s.mode = Mode::recall;
    }
#else
    if (hp < P_RECALL_HP * maxhp && !at_spawn) s.mode = Mode::recall;
#endif
#if FB_LOG
    if (s.mode == Mode::recall && hp < P_RECALL_HP * maxhp && !s.fb_logged) {
        s.fb_logged = true;
        std::cout << "LOWREC " << s.round << " " << static_cast<int>(s.type) << " hp " << hp << " tgt " << s.target
                  << " ghp " << (s.target >= 0 ? s.groups[s.target].hp() : -1.0f) << " st " << (s.target >= 0 ? s.groups[s.target].st : -1)
                  << " gems " << me.gem_count() << " enemy_ago " << (s.round - s.enemy_round) << "\n";
    }
    if (s.mode != Mode::recall) s.fb_logged = false;
#endif
    if (s.mode == Mode::farm || s.mode == Mode::bank) {
        const bool finishing = gems > 0 && banked + gems >= constants::winning_gems;
        if (gems == 0) { if (s.mode == Mode::bank) s.mode = Mode::farm; s.bank_turns = 0; s.bank_skip = -1; s.bank_best = 1 << 30; s.bank_age = 0; }
#if ED_MASK || ED_MASK2
        else if (s.mode == Mode::bank && !finishing && !s.ed_on && ++s.bank_age > P_BANK_GIVEUP) {
#else
        else if (s.mode == Mode::bank && !finishing && ++s.bank_age > P_BANK_GIVEUP) {
#endif
            // This bank run is going nowhere: farm for a while and try again later.
            s.mode = Mode::farm; s.bank_turns = 0; s.bank_skip = -1; s.bank_best = 1 << 30; s.bank_age = 0;
            s.bank_cool_until = s.round + P_BANK_COOL;
#if P_LOG
            std::cout << "GIVEUP " << s.round << " " << static_cast<int>(s.type) << " g " << gems << "\n";
#endif
        }
        else if (finishing || s.mode == Mode::bank) s.mode = Mode::bank;
        else {
            const bool mite_early_bank = s.mite_elkring_finish && s.round >= 350
                && banked >= 500 && gems >= 36;
            if (s.round >= s.bank_cool_until && (!engaged() || mite_early_bank)
#if Z170_ON
                && (mite_early_bank || gems >= z170_bank_threshold(((s.W == 80 && s.H == 80) ? 40 : (s.W == 30 && s.H == 30 && !s.mite_elkring_roster) ? 36 : (s.rows_like ? P_ROWS_BANK : P_BANK)))
#else
                && (mite_early_bank || gems >= ((s.W == 80 && s.H == 80) ? 40 : (s.W == 30 && s.H == 30 && !s.mite_elkring_roster) ? 36 : (s.rows_like ? P_ROWS_BANK : P_BANK))
#endif
                    || (CT_BANK > 0 && g_ct && gems >= CT_BANK && !(RX_ROWS && s.rows_like))
                    || (g_mm_bank > 0 && gems >= g_mm_bank)
#if N4_GW_SIDES || N4_BB_SIDES || LT_BB
                    || [&] { static const int n4id = s.W == 50 ? n4_::detect(s.W, s.H, s.groups) : 0; const int t = n4id ? n4_::bank(n4id, s.round) : 0; return t > 0 && static_cast<int>(gems) >= t && s.df_altar.get(pos) <= n4_::near(n4id); }()   // cc_next4 (n4_gate.h)
#endif
                    || (EI_BANK > 0 && s.ekmap && s.W == 41 && static_cast<int>(gems) >= EI_BANK)
                    || (g_conley_packs && s.round >= 450 && banked >= 600 && gems >= 44)
#if DG_BANK > 0
                    || (g_dgb && gems >= DG_BANK)
#endif
                    || (CX.on && cx_bank_now(gems, pos))
                    || (gems >= CXV_NEAR_BANK && s.df_altar.get(pos) <= 2)
                    || (LS_BANKN > 0 && CB.on && static_cast<int>(gems) >= LS_BANKN && s.df_altar.get(pos) <= LS_BANKD)
                    || (gems >= 20 && banked + P_ENDGAME >= constants::winning_gems))) s.mode = Mode::bank;
        }
    }
#if CXY_BOSS_DEFER
    // One decision: defer bank trips while the mandatory altar footprint is alive.
    if(s.mode==Mode::bank && s.W==50 && s.H==41 && cxy_exact_boss()
        && s.groups[16].st==0 && s.groups[16].hp()>0) s.mode=Mode::farm;
#endif
    int shop_limit = (FX_SHOP > 0 && g_nm == 3 && s.round < FX_SHOPU) ? FX_SHOP : P_SHOP;
    // Post-bank Expanse Varg: keep farming only if a live 100-gem group is
    // within one short field trip. Otherwise take the normal shopping recall.
    if (s.W == 80 && s.H == 80 && s.type == champion_type::varg
        && banked >= 400 && s.df_spawn.get(pos) > 20) {
        const auto foe = draft::current_draft(team::enemy);
        const bool sneaky_expanse = foe.size() == 3
            && foe[0] == champion_draft::cassi_and_issac
            && foe[1] == champion_draft::stellar
            && foe[2] == champion_draft::varg;
        bool near_rich_camp = false;
        for (const auto& g : s.groups) {
            if (g.st != 0 || g.gems < 100 || g.hp() <= 0) continue;
            for (const auto& c : g.camps)
                if (c.hp > 0 && cheb(pos, c.pos) <= 20) near_rich_camp = true;
        }
        if (sneaky_expanse && near_rich_camp) shop_limit = std::max(shop_limit, 400);
    }
    if (s.mode == Mode::farm && !engaged() && !at_spawn && s.type != champion_type::tavi
        && (Q9_GOLD_DUE(me.gold_count(), shop_limit) || hp < P_REST_HP * maxhp))
        s.mode = Mode::recall;
    if (s.mode == Mode::farm && !engaged() && !at_spawn && s.type == champion_type::tavi && hp < P_REST_HP * maxhp)
        s.mode = Mode::recall;

    if (CB.on && coll_post(gems, banked)) return;
    if (CX.on && cx_post(gems)) return;
    if (s.mission >= 0 && s.mode != Mode::recall) {
        if ((s.shm[6] & (1u << s.mission)) || me.consumables()[consumable_type::altar_lock].count() == 0) end_mission();
        else {
            publish(-1, 0);
            s.target = -1;
            s.goals.clear();
            s.goals.draw_chebyshev(s.altars[s.mission], 2.0f);
            s.goals &= ~s.terrain;
            step_to_goals(true);
            place_lock();
            return;
        }
    }
#if FD_BTOG
    if (fd_on() && s.mode == Mode::farm && static_cast<int>(gems) >= FD_BTOG && fd_partner_banking()
        && !(s.target >= 0 && s.groups[s.target].st == 1)) {
        vec2 pp{}; bool vis = false;
        if (fd_partner(pp, vis) && vis && cheb(pos, pp) <= FD_BTOGR) s.mode = Mode::bank;
    }
#endif
    if (s.mode == Mode::recall) { publish(-1, 0); s.target = -1; ++s.t_recall; do_recall(); return; }
#if FB_RB
    if (s.mode == Mode::bank && fb_recall_bank(pos)) { publish(-1, 0); s.target = -1; ++s.t_recall; do_recall(); return; }
#endif
    if (s.mode == Mode::bank) { if (CX.on || (FD_BTOG && fd_on() && (s.type == champion_type::cassi || s.type == champion_type::issac))) publish(62, 3); else publish(-1, 0); s.target = -1; ++s.t_bank; do_bank(); return; }

#if FD_TETH
    if (fd_on() && (s.type == champion_type::issac || (FD_TETHC && s.type == champion_type::cassi))
        && !(s.target >= 0 && s.groups[s.target].st == 1 && s.groups[s.target].tiles().is_set(pos))) {
        vec2 pp{}; bool vis = false;
        const bool fdp = fd_partner(pp, vis);
#ifdef FD_DBG
        if (s.round % 20 == 0) std::cout << "TETH " << vis << " " << s.round << " " << static_cast<int>(s.type) << " p " << fdp << " d " << (fdp ? cheb(pos, pp) : -1) << "\n";
#endif
        if (fdp && cheb(pos, pp) > FD_TETH) {
            s.goals.clear();
            s.goals.draw_chebyshev(pp, 2.0f);
            s.goals &= ~s.terrain;
            if (step_to_goals(true)) { ++s.t_walk; return; }
        }
    }
#endif
    // Answer a nearby ally's enemy sighting when healthy and not mid-capture.
    vec2 alarm{};
    if (P_ALARM > 0 && hp > 0.6f * maxhp && alarm_near(pos, alarm) && cheb(pos, alarm) > 3
        && !(s.target >= 0 && s.groups[s.target].st == 1 && s.groups[s.target].tiles().is_set(pos))) {
        s.goals.clear();
        s.goals.draw_chebyshev(alarm, 2.0f);
        if (step_to_goals(true)) { ++s.t_pvp; return; }
    }


#if Z180_ON
    // Z180: follow the densest visible allied group while free to farm.
    // Existing recall, banking, missions and PvP decisions retain priority.
    if (s.type == champion_type::willow && s.mode == Mode::farm && me.can_move()) {
        const auto z180_allies = vision::champions(team::ally);
        std::optional<vec2> z180_anchor; int z180_count = -1, z180_distance = 1 << 30;
        for (const auto& z180_a : z180_allies) {
            if (!z180_a.is_visible() || !z180_a.is_alive() || z180_a.position() == pos) continue;
            const vec2 z180_p = z180_a.position(); int z180_n = 0;
            for (const auto& z180_b : z180_allies)
                if (z180_b.is_visible() && z180_b.is_alive() && z180_b.position() != pos && cheb(z180_p, z180_b.position()) <= 4) ++z180_n;
            const int z180_d = cheb(pos, z180_p);
            if (z180_n > z180_count || (z180_n == z180_count && z180_d < z180_distance)) {
                z180_anchor = z180_p; z180_count = z180_n; z180_distance = z180_d;
            }
        }
        if (z180_anchor && z180_distance > 3) {
            s.goals.clear(); s.goals.draw_chebyshev(*z180_anchor, 3.0f); s.goals &= ~s.terrain;
            if (step_to_goals(false)) { ++s.t_walk; return; }
        }
    }
#endif
    // Farm.
#if EK3_ON
    if (g_ek3 == 1) { ek3_init(); g_ek3 = 2; }
    if (g_ek3 && g_ek3open && (g_ek3near < 0 || s.groups[g_ek3near].st != 0 || s.round >= EK3_OPEN)) g_ek3open = false;
#endif
    const int choice = s.lanes_ownhalf ? choose_target<false, true>(hp, maxhp)
#if EK3_ON
        : g_ek3 ? (s.old_elkring_mages ? choose_target<true, false, false, false, false, false, false, false, false, false, true>(hp, maxhp)
                                       : choose_target<false, false, false, false, false, false, false, false, false, false, true>(hp, maxhp))
#endif
#if FR_GROUP_ON
        : s.old_elkring_mages ? (g_nm == 3 ? choose_target<false, false, false, false, false, false, false, false, false, true>(hp, maxhp) : choose_target<true, false>(hp, maxhp))
#else
        : s.old_elkring_mages ? choose_target<true, false>(hp, maxhp)
#endif
#if SH_ON
        : (CB.on && s.coll) ? (sh_on() ? choose_target<false, false, true, false, false, true, false, false, false, false, false, false, false, true>(hp, maxhp) : choose_target<false, false, true, false, false, true>(hp, maxhp))
#else
        : (CB.on && s.coll) ? choose_target<false, false, true, false, false, true>(hp, maxhp)
#endif
        : CB.on ? choose_target<false, false, false, false, false, true>(hp, maxhp)
#if SH_ON
        : s.coll ? (sh_on() ? choose_target<false, false, true, false, false, false, false, false, false, false, false, false, false, true>(hp, maxhp) : choose_target<false, false, true>(hp, maxhp))
#else
        : s.coll ? choose_target<false, false, true>(hp, maxhp)
#endif
        : s.il ? choose_target<false, false, false, true>(hp, maxhp)
#if SH_ON
        : s.pk ? g_shpk ? (s.fg_route ? (sh_on() ? choose_target<false, false, false, false, true, false, false, false, false, false, false, false, false, true>(hp, maxhp) : choose_target<false, false, false, false, true>(hp, maxhp))
                            : (sh_on() ? choose_target<false, false, false, false, false, false, false, false, false, false, false, false, false, true>(hp, maxhp) : choose_target<false, false>(hp, maxhp)))
        : (pk_tick(), s.fg_route ? (sh_on() ? choose_target<false, false, false, false, true, false, false, true, false, false, false, false, false, true>(hp, maxhp) : choose_target<false, false, false, false, true, false, false, true>(hp, maxhp))
                                        : (sh_on() ? choose_target<false, false, false, false, false, false, false, true, false, false, false, false, false, true>(hp, maxhp) : choose_target<false, false, false, false, false, false, false, true>(hp, maxhp)))
#else
        : s.pk ? (pk_tick(), s.fg_route ? choose_target<false, false, false, false, true, false, false, true>(hp, maxhp) : choose_target<false, false, false, false, false, false, false, true>(hp, maxhp))
#endif
        : s.fg_route ? choose_target<false, false, false, false, true>(hp, maxhp)
#if FR_GROUP_ON
        : (NM_BIG && nm_map_id(s.W, s.H, s.altars, s.groups.size()) > 0) ? (g_nm == 3 ? choose_target<false, false, false, false, false, false, false, false, false, true>(hp, maxhp) : choose_target<false, false, false, false, false, false, true>(hp, maxhp))
#else
        : (NM_BIG && nm_map_id(s.W, s.H, s.altars, s.groups.size()) > 0) ? choose_target<false, false, false, false, false, false, true>(hp, maxhp)
#endif
        : s.ekmap ? choose_target<false, false, false, false, false, false, false, false, true>(hp, maxhp)
#if PH_ON
                             : g_pg ? choose_target<false, false, false, false, false, false, false, false, false, false, false, true>(hp, maxhp)
#endif
#if AM_ON && PH_ON
                             : am_map() ? choose_target<false, false, false, false, false, false, false, false, false, false, false, false, true>(hp, maxhp)
#endif
#if SH_ON
                             : sh_on() ? choose_target<false, false, false, false, false, false, false, false, false, false, false, false, false, true>(hp, maxhp)
#endif
                             : choose_target<false, false>(hp, maxhp);
#if EK3_ON && EK3_LOG
    if (g_ek3 && s.round <= 300) { const vec2 tp = choice >= 0 ? s.groups[choice].camps[0].pos : vec2{ -1, -1 }; std::cout << "K3 " << s.round << " " << static_cast<int>(s.type) << " " << pos.x << "," << pos.y << " t " << tp.x << "," << tp.y << " st " << (choice >= 0 ? s.groups[choice].st : -1) << " hp " << static_cast<int>(hp) << "\n"; }
#endif
    if (CB.on && coll_choice(choice, gems)) return;
    if (choice != s.target) s.target = choice;
#if SF_ON && SF_RAID
    { const int r = sf_raid_pick(hp, maxhp, static_cast<int>(gems)); if (r >= 0) s.target = r; }
#endif
    if (IX_ON && IX_MODE == 3 && g_ix) {
        const int op = ix_op_tick(hp, maxhp);
        if (op >= 0 && g_ixsq && static_cast<int>(gems) < IX_OPG && s.groups[op].st != 2 && hp >= P_REST_HP * maxhp) s.target = op;
    }
#if PH_ON
    if (g_pg) { const int ph = ph_pick(hp, maxhp, static_cast<int>(gems)); if (ph >= 0) s.target = ph; }
#if PH_LOG
    if (s.target >= 0 && s.target == g_ph && s.round % 5 == 0) std::cout << "PHT " << s.round << " " << static_cast<int>(s.type) << " hp " << static_cast<int>(hp) << "/" << static_cast<int>(maxhp) << " herd " << static_cast<int>(s.groups[g_ph].hp()) << " st " << s.groups[g_ph].st << "\n";
#endif
#endif
    if (s.target < 0) {
        publish(-1, 0);
        if (gems > 0) { s.mode = Mode::bank; ++s.t_bank; do_bank(); return; }
        ++s.idle;
        return;
    }
    auto& g = s.groups[s.target];
    if (g.st == 2) {
        // Spare time before the respawn: bank or shop instead of standing around.
        int d = UNREACH;
        for (auto& c : g.camps) d = std::min<int>(d, s.df_me.get(c.pos));
        const int slack = g.respawn_round - s.round - d;
        const int altar = s.df_altar.get(pos);
        if (P_IDLEBANK > 0 && static_cast<int>(gems) >= P_IDLEBANK && altar != UNREACH
            && slack > altar + static_cast<int>(gems) / 10 + 2) {
            s.mode = Mode::bank; publish(-1, 0); s.target = -1; ++s.t_bank; do_bank(); return;
        }
        if (P_IDLESHOP > 0 && s.type != champion_type::tavi && !at_spawn
            && (static_cast<int>(me.gold_count()) >= P_IDLESHOP || hp < 0.7f * maxhp)
            && slack > constants::recall_time + g.spawn_dist - d + 2) {
            s.mode = Mode::recall; publish(-1, 0); s.target = -1; ++s.t_recall; do_recall(); return;
        }
    }

    if (g.st == 1) {
        ++s.t_cap;
        if (g.tiles().is_set(pos)) { publish(s.target, 2); ++s.channel; return; }
        publish(s.target, 1);
        s.goals = g.tiles();
        step_to_goals(true);
        if (g.tiles().is_set(me.position())) publish(s.target, 2);
        return;
    }
    publish(s.target, 1);
    if (g.st == 2) {
        // Wait beside the spawn tiles, never on them.
        s.goals.clear();
#if BM_MASK
        const bool bmw = g_nm == 2 && bm_on() && bm_group(g);
        for (auto& c : g.camps) s.goals.draw_chebyshev(c.pos, bmw ? 4.0f : static_cast<float>(s.range));
        if (bmw) { bitboard inner; inner.clear(); for (auto& c : g.camps) inner.draw_chebyshev(c.pos, 3.0f); s.goals &= ~inner; }
#else
        for (auto& c : g.camps) s.goals.draw_chebyshev(c.pos, static_cast<float>(s.range));
#endif
        s.goals &= ~g.tiles();
        if (!step_to_goals(true) && g.tiles().is_set(pos)) {
            refresh_occ();
            for (auto d : DIRS) {
                vec2 p{ pos.x + d.x, pos.y + d.y };
                if (!in_bounds(p) || s.occ.is_set(p) || g.tiles().is_set(p)) continue;
                try { if (me.move_absolute(p)) break; } catch (const cambc::error&) {}
            }
        }
        ++s.idle; ++s.t_wait;
        return;
    }

#if DBG
    if (g.mdps >= 200 && s.round % 5 == 0) { std::cout << "MONO R" << s.round << " " << to_string(s.type) << " hp " << hp << " camps"; for (auto& c : g.camps) std::cout << " " << c.hp; std::cout << "\n"; }
#endif
    if (IX_ON && g_ix && IX_RALLY > 0 && s.target < 64 && ((g_ixelk >> s.target) & 1u) && g.st == 0) {
        // Untouched elk pair: wait just outside its reach until the squad is here.
        bool fresh = true;
        if (IX_MODE >= 2) { float full = 0; for (auto& c : g.camps) full += c.maxhp; fresh = g.hp() > IX_FIN * full; }
        if (IX_MODE == 3 && s.round < ix_cool) fresh = false;
        else for (auto& c : g.camps) if (c.hp < c.maxhp) fresh = false;
        int dmin = 99;
        for (auto& c : g.camps) dmin = std::min(dmin, cheb(pos, c.pos));
        int bodies = 1;
        if (dmin <= 5) publish(s.target, 3);
        for (int i = 0; i < 6; ++i) if (i != s.idx && s.shm[i] == static_cast<std::uint8_t>((3u << 6) | static_cast<unsigned>(s.target + 1))) ++bodies;
        if (fresh && (dmin > 3 || IX_MODE >= 2) && bodies < IX_RALLY) {
            if (ix_wait_tgt != s.target || dmin > 5) { ix_wait_tgt = s.target; ix_wait0 = s.round; }
            if (dmin <= 5 && s.round - ix_wait0 > IX_WAITMAX) { ix_cool = s.round + IX_COOL; s.target = -1; ix_wait_tgt = -1; publish(-1, 0); return; }
            if (dmin > 5 || dmin <= 3) {
                s.goals.clear();
                for (auto& c : g.camps) s.goals.draw_chebyshev(c.pos, 5.0f);
                bitboard inner; inner.clear();
                for (auto& c : g.camps) inner.draw_chebyshev(c.pos, 3.0f);
                s.goals &= ~inner; s.goals &= ~s.terrain;
                step_to_goals(true);
            }
#if IX_LOG
            if (s.round % 5 == 0) std::cout << "IXW " << s.round << " " << static_cast<int>(s.type) << " g" << s.target << " n " << bodies << " c " << g.camps[0].pos.x << "," << g.camps[0].pos.y << " dmin " << dmin << " fresh " << fresh << "\n";
#endif
            ++s.t_wait; return;
        }
    }
#if BM_MASK
    if (g_nm == 2 && bm_on() && bm_group(g) && bm_poke(g)) return;  // g_nm == 2: stampede (cheap guard on fuel-tight maps)
#endif
#if PH_ON
    if (g_pg && ph_stage(g)) return;
#endif
    if (fight(g)) { ++s.t_fight; return; }
    ++s.t_walk;
    s.goals.clear();
    const bool varg = s.type == champion_type::varg;
    int most = 0;
    if (varg)
        for (auto& c : g.camps) {
            if (c.hp <= 0) continue;
            int n = 0;
            n = alive_neighbours(g, c);
            most = std::max(most, n);
        }
    for (auto& c : g.camps) {
        if (c.hp <= 0) continue;
        if (varg) {
            int n = 0;
            n = alive_neighbours(g, c);
            if (n < most) continue;
        }
        s.goals.draw_chebyshev(c.pos, static_cast<float>(s.range));
    }
    const bool moved = step_to_goals(true);
    if (moved && !s.ranged) {
        observe();
        fight(g);
    } else if (!moved) {
        // In range but the target is hidden (line of sight): close in.
        s.goals.clear();
        for (auto& c : g.camps) if (c.hp > 0) s.goals.draw_chebyshev(c.pos, 1.0f);
        if (!step_to_goals(false)) ++s.idle;
    }
}

void play_draft() {
#if Z59_MAP1 || Z59_MAP2 || Z59_MAP3
    if(z59::play_draft())return;
#endif
    auto chosen = draft::current_draft(team::ally);
    for(auto& pick:chosen)if(pick==champion_draft::heart_of_ruin)pick=static_cast<champion_draft>(9);
    // Cassi and Issac die together, which halves a three-pick team in one kill:
    // on three-pick maps take three sturdy melee champions instead.
    // Fifth pick Ren (as the top teams do) instead of Tavi.
    constexpr std::array big{ 9, 2, 3, 0, 8, 10, 6, 7, 1, 4, 5 };
    // Tiny maps keep the v52 fifth pick (Tavi): Cassi/Issac pairing there was tuned with it.
    // Elkring: Ren fifth (SHINK and the other top elkring teams), not Tavi, which farmed almost nothing.
    constexpr std::array tiny{ 9, 2, 3, 0, 8, 10, 6, 7, 1, 4, 5 };
    constexpr std::array tiny_mite{ 9, 2, 3, 0, 10, 8, 6, 7, 1, 4, 5 };
    constexpr std::array small{ P_SMALL_DRAFT };
    // Contested three-pick maps: Cassi & Issac, Ryth, Ren.
    constexpr std::array hot_small{ 3, 0, 8, 9, 2, 10, 6, 7, 1, 4, 5 };
    const bool three = static_cast<int>(draft::team_size()) <= P_SMALL_TEAM && map::width() * map::height() <= P_SMALL_AREA;
    static int hot = -1;
    if (three && hot < 0) {
        const auto terrain = map::walls() | map::holes() | map::altars();
        distance_field own, theirs;
        bitboard visited;
        own.compute(terrain, map::spawn_region(team::ally), visited);
        theirs.compute(terrain, map::spawn_region(team::enemy), visited);
        int best = std::numeric_limits<int>::max(), enemy = 0;
        for (auto a : map::altars().positions())
            for (int dx = -1; dx <= 1; ++dx) for (int dy = -1; dy <= 1; ++dy) {
                const vec2 q{ a.x + dx, a.y + dy };
                if (q.x < 0 || q.y < 0 || q.x >= map::width() || q.y >= map::height() || terrain.is_set(q)) continue;
                const int o = own.get(q);
                if (o < best) { best = o; enemy = theirs.get(q); }
            }
        hot = (best != std::numeric_limits<int>::max() && enemy <= 1.35f * best) ? 1 : 0;
    }
    // Four-pick small maps (interlock): Varg, Vendetta, Ryth, Ren, as Quantom drafts there.
    constexpr std::array four_small{ 9, 2, 0, 8, 3, 10, 6, 7, 1, 4, 5 };
    constexpr std::array four_ren_response{ 9, 0, 2, 8, 3, 10, 6, 7, 1, 4, 5 };
    constexpr std::array four_stellar_counter{ 9, 2, 3, 8, 0, 10, 6, 7, 1, 4, 5 };
    const bool four = draft::team_size() == 4 && map::width() * map::height() <= 1600;
    const auto enemy_picks = draft::current_draft(team::enemy);
    const auto enemy_has = [&](champion_draft pick) {
        return std::find(enemy_picks.begin(), enemy_picks.end(), pick) != enemy_picks.end();
    };
    const bool stellar_counter = four && map::width() == 40 && map::height() == 40
        && enemy_has(champion_draft::vendetta) && enemy_has(champion_draft::cassi_and_issac)
        && (chosen.size() < 3 || enemy_has(champion_draft::stellar))
        || (IL_DRAFTCI && four && map::width() == 40 && map::height() == 40 && enemy_has(champion_draft::cassi_and_issac));
    const bool mite_elkring_draft = map::width() == 30 && map::height() == 30
        && chosen.size() >= 4 && enemy_has(champion_draft::varg)
        && enemy_has(champion_draft::stellar) && enemy_has(champion_draft::cassi_and_issac)
        && enemy_has(champion_draft::tavi);
    const bool e_st = enemy_has(champion_draft::stellar), e_tv = enemy_has(champion_draft::tavi);
    const auto gate = [&](int g) { return g == 0 || (g == 1 && e_st) || (g == 2 && (e_st || e_tv)); };
    (void)gate;
#ifdef DR_HOT
    constexpr std::array dr_hot{ DR_HOT };
#endif
#ifdef DR_FOUR
    constexpr std::array dr_four{ DR_FOUR };
#endif
#ifdef DR_BIG
    constexpr std::array dr_big{ DR_BIG };
#endif
#ifdef DR_TINY
    constexpr std::array dr_tiny{ DR_TINY };
#endif
    const std::array<int, 11>* op = nullptr;
    if (three && hot == 1) {
        op = &hot_small;
#ifdef DR_HOT
        if (gate(DR_HOTG)) op = &dr_hot;
#endif
    } else if (three) op = &small;
    else if (four) {
        op = stellar_counter ? &four_stellar_counter : &four_small;
        if (!stellar_counter && map::width() == 40 && map::height() == 40
            && !chosen.empty() && !enemy_picks.empty() && enemy_picks[0] == champion_draft::ren) {
            bool west_spawn = false;
            for (const vec2 q : map::spawn_region(team::ally).positions())
                if (q.x < map::width() / 2) { west_spawn = true; break; }
            if (west_spawn) op = &four_ren_response;
        }
#ifdef DR_FOUR
        if (gate(DR_FOURG)) op = &dr_four;
#endif
#ifdef IX_DR
        // ix (16 Oct): platform interlock (elk pairs since ~12:00Z): Zarzorki's Varg, Stellar, C&I, Ren (8/9 there);
        // locally the best four-pick draft on the platform map file.
        static constexpr std::array<int, 11> ix_dr{ IX_DR };
        if (map::width() == 40 && map::height() == 40 && draft::team_size() == 4) op = &ix_dr;
#endif
    } else if (map::width() * map::height() <= P_PAIR_AREA) {
        op = mite_elkring_draft ? &tiny_mite : &tiny;
#ifdef DR_TINY
        if (gate(DR_TINYG)) op = &dr_tiny;
#endif
    } else {
        op = &big;
#ifdef DR_BIG
        const int W = map::width(), H = map::height();
        const int sel = (W == 71 && H == 71 ? 1 : 0) | (W == 90 && H == 30 ? 2 : 0) | (W == 50 && H == 50 ? 4 : 0) | (W == 80 && H == 80 ? 8 : 0);
        if (gate(DR_BIGG) && (DR_BIGSEL & sel)) op = &dr_big;
#endif
#if DC_VG
        // dc_vg: on corners keep v196's Vendetta second pick when the enemy opened with Varg
        // (Memory?: Varg, C&I, Stellar, Ren, Ryth beat Stellar 1014:154, lost to Vendetta 568:1002).
        // Once one of the two is drafted, keep that order so later picks stay consistent.
        if (W == 71 && H == 71) {
            const auto own_has = [&](champion_draft c) { return std::find(chosen.begin(), chosen.end(), c) != chosen.end(); };
            if (own_has(champion_draft::vendetta)) op = &big;
            else if (!own_has(champion_draft::stellar) && enemy_has(champion_draft::varg)) op = &big;
        }
#endif
    }
#if defined(MM_DR_IL) || defined(MM_DR_EK) || defined(MM_DR_PK) || defined(MM_DR_LN)
    {
        // Memory?-style enemy: Varg + Cassi&Issac (+ Stellar once three picks are in), no Tavi/Prism/Willow.
        const bool mm = enemy_has(champion_draft::varg) && enemy_has(champion_draft::cassi_and_issac)
            && (enemy_picks.size() < 3 || enemy_has(champion_draft::stellar))
            && !enemy_has(champion_draft::tavi) && !enemy_has(champion_draft::prism) && !enemy_has(champion_draft::willow);
        const int W = map::width(), H = map::height();
#ifdef MM_DR_IL
        static constexpr std::array<int, 11> mm_il{ MM_DR_IL };
        if (mm && W == 40 && H == 40) op = &mm_il;
#endif
#ifdef MM_DR_EK
        static constexpr std::array<int, 11> mm_ek{ MM_DR_EK };
        if (mm && W == 30 && H == 30) op = &mm_ek;
#endif
#ifdef MM_DR_PK
        static constexpr std::array<int, 11> mm_pk{ MM_DR_PK };
        if (mm && W == 31 && H == 31) op = &mm_pk;
#endif
#ifdef MM_DR_LN
        static constexpr std::array<int, 11> mm_ln{ MM_DR_LN };
        if (mm && W == 90 && H == 30) op = &mm_ln;
#endif
    }
#endif
#ifdef CD_ORDER
    {
        // cd_* (15 Oct): counter-draft vs Stellar drafts, gated on map size and an enemy Stellar visible at this pick (CD_MAPONLY: map only, for orders that differ at pick 1, where no enemy pick is visible yet).
        static constexpr std::array<int, 11> cd_order{ CD_ORDER };
        // Only while our picks so far follow cd_order, so a draft never mixes the two orders.
        bool prefix = chosen.size() <= cd_order.size();
        for (std::size_t i = 0; prefix && i < chosen.size(); ++i) prefix = static_cast<int>(chosen[i]) == cd_order[i];
        if (map::width() == CD_W && map::height() == CD_H && prefix && (CD_MAPONLY || enemy_has(champion_draft::stellar))) op = &cd_order;
    }
#endif
#ifdef PK_DRAFT
    {
        // pk_* forced packs draft (31x31 only).
        static constexpr std::array<int, 11> pk_dr{ PK_DRAFT };
        if (map::width() == 31 && map::height() == 31) op = &pk_dr;
    }
#endif
#if DD_ON
    {
        // dd_draft (16 Oct): data-driven policy table from 3,804 top-40 ranked map-games (09:00-17:20 UTC, new map pool).
        // Each row: map identity (size, team size, contest altars), required enemy picks (bitmask over champion_draft,
        // 0 = map-only), our order. First matching row wins; a row applies only while our picks so far are a prefix
        // of its order, so one draft never mixes two orders. Enemy-conditioned rows: none passed the bar
        // (n>=30 per cell, >=4 teams, |z|>=2.5 against the map-wide effect), so all rows are map-only.
        struct dd_row { int W, H, n, contest; unsigned need; std::array<int, 11> order; bool on; };
        static constexpr dd_row dd_rows[] = {
            // contest: with Stellar +21 (z 3.1), with Vendetta -19 (z -2.7); Va St CI Ry Ta = Ry CI St Va Ta (6 teams).
            { 50, 50, 5, 1, 0u, { 9, 4, 3, 0, 10, 8, 2, 6, 7, 1, 5 }, DD_CT != 0 },
            // stampede: Stellar +23 (z 3.0), Tavi +18 (z 3.8), Vendetta -29 (z -4.2), Ren -17 (z -2.8).
            { 60, 50, 5, 0, 0u, { 9, 4, 3, 0, 10, 8, 2, 6, 7, 1, 5 }, DD_ST != 0 },
            // eight: Ry CI St Re Va +9.7 (n=120, 13 teams); ours (Vd for St) 2/23.
            { 41, 30, 5, 0, 0u, { 9, 4, 3, 0, 8, 2, 10, 6, 7, 1, 5 }, DD_EI != 0 },
            // swirl (4 picks): Vendetta -27 (z -3.6); Ry CI St Va +16.8 (n=61, 6 teams).
            { 45, 30, 4, 0, 0u, { 9, 4, 3, 0, 8, 2, 10, 6, 7, 1, 5 }, DD_SW == 1 },
            // swirl variant 2: only Vendetta -> Stellar (Va St Ry Re).
            { 45, 30, 4, 0, 0u, { 9, 4, 0, 8, 3, 2, 10, 6, 7, 1, 5 }, DD_SW == 2 },
            // rushbait-v2 (4 picks): Va St CI Ta (w3 agent; local vs s2_top 18/32 vs 11/32).
            { 30, 80, 4, 0, 0u, { 9, 4, 3, 10, 8, 0, 2, 6, 7, 1, 5 }, true },
            // elkring: Tavi +13 (z 2.8, 24 teams; +23 vs Varg openers): Ren -> Tavi. OFF by default: local 21/24 -> 6/24 (Tavi farms poorly in our play).
            { 30, 30, 5, 0, 0u, { 9, 2, 3, 0, 10, 8, 6, 7, 1, 4, 5 }, DD_EK != 0 },
            // fragment (fx2): C&I, Stellar, Ren (field +6.5 over C&I, Ryth, Ren; weak, z 1.5).
            { 30, 30, 3, 0, 0u, { 3, 4, 8, 0, 9, 2, 10, 6, 7, 1, 5 }, FX_DR == 1 },
        };
        const int W = map::width(), H = map::height(), n = static_cast<int>(draft::team_size());
        bool contest = false;
        if (W == 50 && H == 50) {
            int hit = 0, cnt = 0;
            for (auto a : map::altars().positions()) { ++cnt; if ((a.x == 24 && a.y == 25) || (a.x == 25 && a.y == 24)) ++hit; }
            contest = cnt == 2 && hit == 2;
        }
        unsigned emask = 0;
        for (auto c : enemy_picks) emask |= 1u << static_cast<int>(c);
        for (const auto& r : dd_rows) {
            if (!r.on || r.W != W || r.H != H || r.n != n || (r.contest != 0) != contest || (r.need & emask) != r.need) continue;
            bool prefix = true;
            for (std::size_t i = 0; prefix && i < chosen.size(); ++i) prefix = static_cast<int>(chosen[i]) == r.order[i];
            if (!prefix) continue;
            op = &r.order;
            break;
        }
    }
#endif
#ifdef SF_DRAFT
    static constexpr std::array<int, 11> sf_dr{ SF_DRAFT };
    op = &sf_dr;
#endif
#if defined(SF_DR_ELK) || defined(SF_DR_COL) || defined(SF_DR_IL) || defined(SF_DR_EI) || defined(SF_DR_PIL) || defined(SF_DR_STA) || defined(SF_DR_EXP) || defined(SF_DR_ROW)
    {   // sf: per-map drafts (what the field drafts against us there).
        struct sfm { int W, H, n; std::array<int, 11> o; };
        static constexpr sfm sf_maps[] = {
#ifdef SF_DR_ELK
            { 30, 30, 5, { SF_DR_ELK } },
#endif
#ifdef SF_DR_COL
            { 41, 41, 3, { SF_DR_COL } },
#endif
#ifdef SF_DR_IL
            { 40, 40, 4, { SF_DR_IL } },
#endif
#ifdef SF_DR_EI
            { 41, 30, 5, { SF_DR_EI } },
#endif
#ifdef SF_DR_PIL
            { 50, 65, 5, { SF_DR_PIL } },
#endif
#ifdef SF_DR_STA
            { 60, 50, 5, { SF_DR_STA } },
#endif
#ifdef SF_DR_EXP
            { 80, 80, 3, { SF_DR_EXP } },
#endif
#ifdef SF_DR_ROW
            { 50, 50, 4, { SF_DR_ROW } },
#endif
        };
        for (const auto& m : sf_maps)
            if (map::width() == m.W && map::height() == m.H && static_cast<int>(draft::team_size()) == m.n) op = &m.o;
    }
#endif
#ifdef SF_DRAFTF
    // Fight maps where top teams beat us with Ryth fifth (elkring 30x30 five picks, eight 41x30).
    static constexpr std::array<int, 11> sf_drf{ SF_DRAFTF };
    if (draft::team_size() == 5 && ((map::width() == 30 && map::height() == 30 && (SF_FMAP & 1)) || (map::width() == 41 && map::height() == 30 && (SF_FMAP & 2))))
        op = &sf_drf;
#endif
    {   // cp: per-map caster drafts (CP_DR<map index>).
        static constexpr std::array<int, 11> cp_none{ -1 };
        static constexpr std::array<std::array<int, 11>, 8> cp_dr{
#ifdef CP_DR0
            std::array<int, 11>{ CP_DR0 },
#else
            cp_none,
#endif
#ifdef CP_DR1
            std::array<int, 11>{ CP_DR1 },
#else
            cp_none,
#endif
#ifdef CP_DR2
            std::array<int, 11>{ CP_DR2 },
#else
            cp_none,
#endif
#ifdef CP_DR3
            std::array<int, 11>{ CP_DR3 },
#else
            cp_none,
#endif
#ifdef CP_DR4
            std::array<int, 11>{ CP_DR4 },
#else
            cp_none,
#endif
#ifdef CP_DR5
            std::array<int, 11>{ CP_DR5 },
#else
            cp_none,
#endif
#ifdef CP_DR6
            std::array<int, 11>{ CP_DR6 },
#else
            cp_none,
#endif
#ifdef CP_DR7
            std::array<int, 11>{ CP_DR7 },
#else
            cp_none,
#endif
        };
        const int mi = cp_map();
        if (mi < 8 && cp_dr[mi][0] >= 0) {
            if (mi == 3) {
                static constexpr std::array<int, 11> west_eight{ 3, 4, 8, 9, 0, 10, 2, 6, 7, 1, 5 };
                const bool crimson_open = enemy_picks.size() >= 3
                    && enemy_picks[0] == champion_draft::stellar
                    && enemy_picks[1] == champion_draft::tavi
                    && enemy_picks[2] == champion_draft::cassi_and_issac;
                op = crimson_open ? &cp_dr[mi] : &west_eight;
            } else op = &cp_dr[mi];
        }
    }
#if S2_ON
    {   // s2_top (17 Oct, test opponent only): per-map draft by full map index nxm() (S2_DR<k>, k = nxm()).
        static constexpr std::array<int, 11> s2_none{ -1 };
#define S2_ROW(k) s2_none
        static constexpr std::array<std::array<int, 11>, 18> s2_dr{
#ifdef S2_DR0
            std::array<int, 11>{ S2_DR0 },
#else
            s2_none,
#endif
#ifdef S2_DR1
            std::array<int, 11>{ S2_DR1 },
#else
            s2_none,
#endif
#ifdef S2_DR2
            std::array<int, 11>{ S2_DR2 },
#else
            s2_none,
#endif
#ifdef S2_DR3
            std::array<int, 11>{ S2_DR3 },
#else
            s2_none,
#endif
#ifdef S2_DR4
            std::array<int, 11>{ S2_DR4 },
#else
            s2_none,
#endif
#ifdef S2_DR5
            std::array<int, 11>{ S2_DR5 },
#else
            s2_none,
#endif
#ifdef S2_DR6
            std::array<int, 11>{ S2_DR6 },
#else
            s2_none,
#endif
#ifdef S2_DR7
            std::array<int, 11>{ S2_DR7 },
#else
            s2_none,
#endif
#ifdef S2_DR8
            std::array<int, 11>{ S2_DR8 },
#else
            s2_none,
#endif
#ifdef S2_DR9
            std::array<int, 11>{ S2_DR9 },
#else
            s2_none,
#endif
#ifdef S2_DR10
            std::array<int, 11>{ S2_DR10 },
#else
            s2_none,
#endif
#ifdef S2_DR11
            std::array<int, 11>{ S2_DR11 },
#else
            s2_none,
#endif
#ifdef S2_DR12
            std::array<int, 11>{ S2_DR12 },
#else
            s2_none,
#endif
#ifdef S2_DR13
            std::array<int, 11>{ S2_DR13 },
#else
            s2_none,
#endif
#ifdef S2_DR14
            std::array<int, 11>{ S2_DR14 },
#else
            s2_none,
#endif
#ifdef S2_DR15
            std::array<int, 11>{ S2_DR15 },
#else
            s2_none,
#endif
#ifdef S2_DR16
            std::array<int, 11>{ S2_DR16 },
#else
            s2_none,
#endif
            s2_none,
        };
#undef S2_ROW
        const int mi = nxm();
        if (mi < 18 && s2_dr[mi][0] >= 0) op = &s2_dr[mi];
    }
#endif
    const std::array<int, 11>& order = *op;
#if DR_LOG
    std::cout << "DRAFT " << map::width() << "x" << map::height() << " n" << draft::team_size() << " hot" << hot << " own";
    for (auto c : chosen) std::cout << " " << static_cast<int>(c);
    std::cout << " enemy";
    for (auto c : enemy_picks) std::cout << " " << static_cast<int>(c);
    std::cout << std::endl;
#endif
    std::vector<std::uint8_t> plan;
    for (auto c : chosen) plan.push_back(static_cast<std::uint8_t>(static_cast<int>(c)==9?11:static_cast<int>(c)));
    for (int id : order) {
        const int own_id=id==9?11:id;
        const auto pick = static_cast<champion_draft>(own_id);
        if (std::find(chosen.begin(), chosen.end(), static_cast<champion_draft>(id)) != chosen.end()) continue;
        plan.push_back(static_cast<std::uint8_t>(own_id));
        plan.resize(16,255); plan[15]=114; draft_memory = plan;
        draft::pick(pick);
        return;
    }
}

#if AM_ON
#if SH_ON
// Map ids (W, H, altars, groups) for the 15 candidate maps; see scratchpad/sh_agent/notes.txt.
static int sh_map_id() {
    auto& s = *S;
    const int k = s.W * 1000000 + s.H * 10000 + static_cast<int>(s.altars.size()) * 1000 + static_cast<int>(s.groups.size());
    static const int keys[] = { SH_KEYS };
    for (int i = 0; i < static_cast<int>(sizeof(keys) / sizeof(keys[0])); ++i) if (keys[i] == k) return i;
    return -1;
}
static int sh_who(int id) { static const int w[] = { SH_WHOS }; return id >= 0 ? w[id] : 0; }
__attribute__((noinline)) void sh_map() {
    auto& s = *S;
    g_shg = -1;
    const int id = sh_map_id();
#if SH_LOG
    std::cout << "SHM t " << static_cast<int>(s.type) << " key " << s.W << " " << s.H << " " << s.altars.size() << " " << s.groups.size() << " id " << id << "\n";
#else
    if (id < 0 || !((SH_MAPS >> id) & 1) || !((sh_who(id) >> static_cast<int>(s.type)) & 1)) return;
#endif
    distance_field de;
    de.compute(map::walls() | map::holes() | map::altars(), map::spawn_region(team::enemy), s.visited);
    int bd = UNREACH, best = -1;
    for (std::size_t gi = 0; gi < s.groups.size(); ++gi) {
        auto& g = s.groups[gi];
        int own = UNREACH, foe = UNREACH;
        for (auto& c : g.camps) { own = std::min<int>(own, s.df_spawn.get(c.pos)); foe = std::min<int>(foe, de.get(c.pos)); }
#if SH_LOG
        std::cout << "SHG t " << static_cast<int>(s.type) << " g" << gi << " gems " << g.gems << " gold " << g.gold << " n" << g.camps.size() << " @" << g.camps[0].pos.x << "," << g.camps[0].pos.y << " own " << own << " foe " << foe << " mdps " << g.mdps << "\n";
#endif
        static const int gems[] = { SH_GEMS };
        if (id >= 0 && own <= foe && own < UNREACH && g.mdps > 0 && g.gems == gems[id] && own < bd) { bd = own; best = static_cast<int>(gi); }
    }
#if SH_LOG
    std::cout << "SHSEL t " << static_cast<int>(s.type) << " best " << best << "\n";
    if (id < 0 || !((SH_MAPS >> id) & 1) || !((sh_who(id) >> static_cast<int>(s.type)) & 1)) return;
#endif
    g_shg = best;
}
#endif
__attribute__((noinline)) bool am_map() {
    if (g_ams >= 0) return g_ams != 0;
    auto& s = *S;
    g_ams = 0;
    if (!(s.W == 100 && s.H == 100 && ((AM_WHO >> static_cast<int>(s.type)) & 1))) return false;
    distance_field de;
    de.compute(map::walls() | map::holes() | map::altars(), map::spawn_region(team::enemy), s.visited);
    int bd = UNREACH, best = -1;
    for (std::size_t gi = 0; gi < s.groups.size(); ++gi) {
        auto& g = s.groups[gi];
        int own = UNREACH, foe = UNREACH;
        for (auto& c : g.camps) { own = std::min<int>(own, s.df_spawn.get(c.pos)); foe = std::min<int>(foe, de.get(c.pos)); }
#if AM_LOG
        std::cout << "AMG t " << static_cast<int>(s.type) << " g" << gi << " gems " << g.gems << " gold " << g.gold << " n" << g.camps.size() << " @" << g.camps[0].pos.x << "," << g.camps[0].pos.y << " own " << own << " foe " << foe << " mdps " << g.mdps << "\n";
#endif
        if (own < foe && g.gems >= AM_MING && g.gems <= AM_MAXG && own < bd) { bd = own; best = static_cast<int>(gi); }
    }
    g_amg = best;
#if AM_LOG
    std::cout << "AMSEL t " << static_cast<int>(s.type) << " best " << best << "\n";
#endif
    g_ams = best >= 0 ? 1 : 0;
    return g_ams != 0;
}
#endif
#if RW_PRIO
// rw (18 Oct, rows 50x50 only): a carrier whose bag would reach the winning total acts first within our team
// from the next round (set_priority), so a tribute completing in the same round as the enemy's lands on our earlier turn.
void rw_prio() {
    if (!S) return;
    auto& s = *S;
    if (!(s.rows_like && s.W == 50 && s.H == 50)) return;
    auto& me = self();
    std::int32_t want = 0;
    if (me.is_alive() && me.gem_count() > 0
        && global_submitted_gems(team::ally) + me.gem_count() + RW_PRIO_SLACK >= constants::winning_gems) want = 1;
    try { if (me.priority() != want) me.set_priority(want); } catch (const cambc::error&) {}
}
#endif
} // namespace

int optimized_main() {
    while (true) {
        try {
            if (is_draft()) play_draft();
            else {
                play_turn();
                ::wardx::tick();
#if RW_PRIO
                rw_prio();
#endif
#if DBG
                auto& s = *S;
                if (s.round % 100 == 0 || s.round == 1)
                    std::cout << "R" << s.round << " " << to_string(s.type) << " idx" << s.idx << " turns " << s.turns << " late " << s.late
                              << " atk " << s.attacks << " cast " << s.casts << " mv " << s.moves << " ch " << s.channel << " idle " << s.idle
                              << " gems " << self().gem_count() << " gold " << self().gold_count() << " str " << self().strength()
                              << " fuel " << fuel_remaining() << " tgt " << s.target
                              << " | T recall " << s.t_recall << " bank " << s.t_bank << " pvp " << s.t_pvp << " heal " << s.t_heal << " wait " << s.t_wait << " cap " << s.t_cap << " fight " << s.t_fight << " walk " << s.t_walk << "\n";
#endif
            }
        } catch (const std::exception& error) {
            std::cout << "bot error: " << error.what() << std::endl;
        } catch (...) {
            std::cout << "bot error: unknown exception" << std::endl;
        }
        end_turn();
    }
}

}
int z114_main(){return z114_policy::optimized_main();}

#endif
