#define FX_FUNI 1
#define FX_SHOP 120
#define FX_SHOPU 200
#define FF_ROT 0.25f
#define FF_ROT_I 0.5f
#define IL_KEEPPAIR 1
#define CB_A 96
#define FG_GOLD 150
#define FG_BUILD 1
#define FM_RQ 2
#define FM_RQD 5
#define FB_RB 2
#define FB_COMMIT 3
#define FB_CHASE 3
#define FB_CHASEMAP 1
#define CT_BANK 36
#define DR_BIG 9, 4, 3, 0, 8, 2, 10, 6, 7, 1, 5
#define DR_BIGSEL 3
#define DC_VG 1
#define DG_DIS 40
#define DG_BANK 44
#define MM_LN_OWNHALF 1
#define MM_BANK_LN 24
#define MM_FLEE_EK 0.7f
#define MM_BANK_EK 24
#define OS_TEST_BANK 24, 0, 0, 24, 0, 0, 30, 0, 0, 0
#define OS_TEST_FLEE 0, 0, 0, 0.7f, 0, 0, 0.7f, 0, 0, 0
#define OS_GEM_BANK 24, 24, 0, 24, 0, 0, 0, 0, 0, 0
#define OS_GEM_FLEE 0, 0.7f, 0, 0.7f, 0, 0, 0, 0, 0, 0
#define CD_ORDER 6, 0, 8, 9, 2, 3, 10, 7, 1, 4, 5
#define CD_W 41
#define CD_H 41
#define CD_MAPONLY 1
#define RX_ROWS 1
#define CX_TERR 1
#define CX_ESC 0
#define CX_HOLD 0
#define PK_ON 1
#define PK_FAR 1
#define PK_OPENR 60
#define PK_OPENW 3.0f
#define EI_ON 1
#define EI_W72 2.0f
#define EI_W72L 2.0f
#define EK_ON 0
#define EK_WMID 1.3f
#define EK_NEEDCI 1
#define FR_GROUP_ON 1
#define FR_GROUP 0.8f
#ifndef DD_ON
#define DD_ON 1
#endif
#ifndef DD_CT
#define DD_CT 1
#endif
#ifndef DD_ST
#define DD_ST 1
#endif
#ifndef DD_EI
#define DD_EI 0
#endif
#ifndef DD_SW
#define DD_SW 2
#endif
#ifndef DD_EK
#define DD_EK 0
#endif
#define IX_DR 9, 4, 3, 8, 2, 0, 10, 6, 7, 1, 5
#define MC_CHUB 1
#define SF_ON 1
#define SF_SHOP 3
#define SF_SHOPM (511 & ~16 & ~128)
#define SF_WHOM 0
#define SF_WHO 5
#define FD_WHOMAPS 32965   // mx: fd_fix 215 minus collision (cp map)
#define FD_RG 30
#define FD_RHP 0.45f
#define FD_RGMAPS 101    // mx: fd_fix 127 minus collision, eight (cp maps)
#define SF_MCX 1
#define FD_MCXMAPS 1   // mx: fd_fix 25 minus eight (cp map)
// ---- cp (17 Oct): sf_hunter caster layers, gated per map in main.cpp (cp_map: 0 elkring, 1 collision, 2 interlock,
// 3 eight, 4 fragment, 5 lanes, 6 packs, 7 monolith). The sf_pvp knobs below only act inside sf_pvp (CP_WHOM maps).
// mx: cp's sf_pvp knobs only on cp-owned maps; fd/us maps keep the main.cpp defaults (1.3f, 5, 0, 0, 1).
#define SF_HUNT (mx_cpmap() ? 1.0f : 1.3f)
#define SF_OWNR (mx_cpmap() ? 6 : 5)
#define SF_MHIT (mx_cpmap() ? 1 : 0)
#define SF_BANKV (mx_cpmap() ? 1 : 0)
#define SF_CALL (mx_cpmap() ? 0 : 1)
#define P_RECALL_HP cp_recall()
// cp_fix per-map choices (17 Oct, vs sf_hunter s1-16 + sf_fin s41-56, and h2h vs us_shop3 seeds):
// collision, eight, fragment: sf caster layers (Stellar/Tavi sf_pvp, chase key, recall 0.45) + caster drafts; fragment also health shop.
// elkring, lanes: caster draft only. interlock, packs, monolith and all other maps: us_shop3 play.
#define CP_WHOM 26
#define CP_MCXM 26
#define CP_RECM 58
// mx: CP_DR0 (elkring caster draft) off -> fd_fix elkring
#define CP_DR1 8, 4, 0, 9, 3, 10, 2, 6, 7, 1, 5
#define CP_DR3 3, 4, 10, 7, 5, 8, 9, 0, 2, 6, 1
#define CP_DR4 8, 4, 0, 3, 9, 10, 2, 6, 7, 1, 5
#define CP_SHOPM 16
// p9 (17 Oct): w3/s2 per-map knobs enabled where they beat rb346 vs s2_top on both seed sets (+ hold-out w3_h_s81..96).
// contest (nxm 11): draft row VSRRT (9,4,8,0,10,3); rest .65 dropped (shifts fuel-tight corners); elkring (nxm 0): RSCRV draft + Fish shop5; fragment (nxm 16): Fish shop5.
#undef S2_ON
#define S2_ON 1
#define S2_DR11 9, 4, 8, 0, 10, 3, 2, 6, 7, 1, 5
#define S2_DR0 8, 4, 3, 0, 9, 10, 2, 6, 7, 1, 5
#undef S2_SHOP5
#define S2_SHOP5 65537
// ct (17 Oct, contest only): keep the gold weight at 1.0 all game (P_GOLDW decays to 0 at round 400, after which the
// Monolith was our only target), skip the lone 35-gold camps and weight the group allies already chose (+100% each)
// so the team farms the four central 105-gold camps together. Other maps compile to identical behaviour.
#define CT_ON 1
#define CT_GW 1.0f
#define CT_OPEN 2000
#define CT_GROUP 1.0f
#define CT_GR 2000
// gd (17 Oct): P_GOLDW decay floor per map vs s2_top s1-16/s41-56 + sf_hunter s1-16 (scratchpad/gd_agent/agg.py):
// expanse (nxm 6, 80x80) + monolith (nxm 10): constant gold weight (floor 1.0); eight (nxm 3): floor 0.2. Other maps: ct_fix decay.
#define GD_MASK 64
#define GD_FLOOR 1.0f
#define GD_MASK2 0
#define GD_FLOOR2 0.2f
// cp2 (17 Oct, corners vs Prism drafts only: Ry St Pr Re Ta = hello/Conley/tenshi): the v188 Prism plan (100-round safe
// opening + hot-hub corner-altar banking) predates the 16 Oct map change (centre crystals -> elk/wolf hub). Platform v319+
// went 0/5 on side a vs this draft; local proxy cp2_prx 1/16 -> 11/16 with both parts off. Both macros are read only in
// cp_skip(), which runs only when corners_hello_safe100 (71x71 and enemy drafted Prism): every other game is unchanged.
#define CP_SAFE 0
#define CP_HOTN 99
#define RC_ON 1
#define RC_FT 0
#undef RC_MAPS
#define RC_MAPS 0x12004
// q9 (17 Oct): lanes shopping recall at 450 gold instead of P_SHOP 150 (fewer 60-tile home round trips on the 90x30 map)
#define Q9_LNSHOP 450
// cz2: collision lane-sweep plan (2) vs C&I+Ren+Stellar (no Varg/Vendetta), Prism+Ren+Stellar and mirror Stellar+Ren+Ryth drafts
#define CZ2_CIRS 2
#define CZ2_PRRS 2
#define CZ2_SRRY 2
// e9 (17 Oct): eight only, vs an enemy draft with Tavi: every champion (Stellar, Cassi too) may farm the own-side 72-gem elk pair
// (P_MONO skip lifted), so the team kills it together by ~r35 and is grouped when Tavi teams invade at r40-50.
#define E9_ON 1
#define E9_TAVI 1
#define EK3_ON 1
#define EK3_N 2
#define EK3_R 150
#define PH_ON 1
#define PH_SQ 0
#define PH_V92 3.0f
// am (18 Oct): ambush only (100x100): Ryth+Vendetta may farm our own-half 72-gem elk pair, P_MONO check divides its dps among max(claims, AM_N-1)=2 helpers
#define AM_ON 1
#define AM_WHO 5
#define AM_N 3
#define AM_W 1.0f
// rw: rows finisher arrival tribute + priority
#define RW_ARR 1
#define RW_PRIO 1
#undef UC2_ON
#define UC2_ON 1
// sh (18 Oct): shared own-half camp helpers on swirl only (all types, 1 assumed helper on the 88-gem group)
#define SH_ON 1
#define SH_MAPS 64
#define SH_N 2
#define AT_MAPS 1552
#define AT_MIN 45
#define AT_MMASK 16
#define AT_THR 6
#define AT_TMASK 1536
// ra (18 Oct): eight-only recall abort memory, K=4 (also enables rc recheck on eight).
#define RA_ON 2
#define RA_K 4
#define RA_MAPS 0x8
// tf2_fin (18 Oct): team-fight controller parts per map (nxm bits: contest 11, corners 14, swirl 12, eight 3, collision 1).
// Split test vs tv_ar393 (s2_top s1-16/41-56, draft proxies, sf_hunter, px_q hunters): only corners passed
// s2 >= +4/64, every proxy family >= 0 and no reduced win margin. Corners runs (a) ranged step-then-attack,
// (b) disengage at TF_FIGHT 0.7 with a 3-round hold and (c) no channel with an enemy near (= tf2_v4 there).
// Every other map is the unchanged v394 code path.
#define TF_ON 1
#define TF_MAPS 0x4000
#define TF_DISMAPS 0x4000
#define TF_RECMAPS 0x4000
#undef TF_FIGHT
#define TF_FIGHT 0.7f
#undef TF_HOLD
#define TF_HOLD 3
// go (19 Oct): gold weight floor, collision only on the combined base (eight was -3/64 vs wd398 with go on; scratchpad/cb_agent/notes.md)
#undef GO_MASK
#define GO_MASK 2
#undef GO_FLOOR
#define GO_FLOOR 0.4f
// bl (18 Oct): burst leave / safe recall variant
#undef BL_ON
#define BL_ON 2
#undef BL_API
#define BL_API 0
#undef BL_RMAPS
#define BL_RMAPS 0x2
// dd (18 Oct): field-draft rows by nxm() for maps that passed the local gates (scratchpad/dd_agent/agg.py)
#undef S2_DR4
#define S2_DR4 9, 4, 3, 10, 8, 2, 6, 7, 1, 5, 0
// bm (18 Oct): stampede (nxm 5) range-4 poke protocol, see scratchpad/bm_agent/big-monster-playbook.md
#define BM_MASK 0x20
// tr (19 Oct): gold audit switches vs cb_fin (scratchpad/tr_agent/notes.md). Kept: Varg/Tavi/Vendetta buy one Treasure emblem as their
// SECOND emblem (after Jungle) on monolith only (nxm 10): s2_top +7/64, px +0 (+57 margin), sf -2 (+12 margin).
// Eight (s2 +9) failed the real-draft proxies (-3/48, sf -1); pilgrimage was +2/64. Treasure-first (TR_TPOS 1), Cassi reward
// sharing (TR_CMASK) and the contest 35-gold camp opening (CT_OPEN 300) failed their local gates; all stay off (code compiles to cb_fin).
#define TR_TMASK 1024
#define TR_TPOS 2
// wf (19 Oct): cb2 without the per-fight-turn E log (FU_ELOGF above the 100k turn budget = never print it)
#define FU_ELOGF 1000000
// jc (19 Oct): corners vs Willow drafts: own-quadrant stack opening (scratchpad/jc_agent)
#define JC_ON 1
#define JC_N 4
#define JC_R 80

// il5 (19 Oct): collision lane-sweep plan: Stellar does not solo enemy-half camps (scratchpad/il5_agent/notes.md). IL5_PAIR/IL5_LSST failed their gates and stay off.
#define IL5_STOWN 1
#define ED_HOLD 1024
#define ED_SE 1
#undef RR_MASK
#define RR_MASK 0x800
#undef RR_COND
#define RR_COND 16
#undef RR_B
#define RR_B 1
#undef RR_KITE
#define RR_KITE 1
#define CTX_DR 2

// st5_fin (19 Oct): stampede herd occupation hook (main.cpp st4_pre): hold/enter while the reward is available (SW 1) and while the
// last attacker is nearly dead (SW 4), at most 2 holders uncontested; Cassi never holds (SW 16); hook off while outnumbered
// around the herd by 2+ (SW 32, OM 1). HouseOfCoders v416 loss fix. Stampede only (nxm bit 5).
#define ST4_MASK 32
#define ST4_SW 53
#define ST4_N 2
#define ST4_OM 1

#define CXK_MAPS 512
#define CXK_SW 9
#define CXK_PROD 5
#define CXK_GEMS 44
#define CXK_NEAR 6

// Fuel-only switches: reuse the from-position field; omit remaining diagnostic prints.
#define FU_DFME 1
#define P_LOG 0
