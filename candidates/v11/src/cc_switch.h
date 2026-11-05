#pragma once
#include "mg_switch.h"   // cc_merge_all
// cc_cc (29 Oct): exact 00contest (z436 id 2) and exact 02crossing (z434 id 6); docs/claude-cc-2026-10-29.md.
#ifndef CC_CT_HOLD
#define CC_CT_HOLD 0    // 00contest: loaded and outnumbered near the centre altars -> wait outside (cc_rs hold)
#endif
#ifndef CC_CR_BIG
#define CC_CR_BIG 1     // 02crossing: co-op the own-side 57/43-gem groups when CC_CR_NEAR healthy bodies are together
#endif
#ifndef CC_CR_NEAR
#define CC_CR_NEAR 3
#endif
#ifndef CC_CR_HOLD
#define CC_CR_HOLD 0    // 02crossing: the cc_rs altar hold
#endif
#ifndef CC_CRA_MODE
// cc_crossa (30 Oct; docs/claude-crossa-2026-10-30.md): side A only (cached_my_team == 0). Side B is untouched.
// 0 = unchanged, 1 = co-op off on side A, 2 = side A co-op only from round CC_CRA_R, 3 = side A co-op on the 57 only,
// 4 = side A co-op only while our own central 50-gem group is down,
// 5 = co-op unchanged; side A values our own central 50-gem group xCC_CRA_M before CC_CRA_R (non-co-op targets),
// 6 = side A co-op off for the rest of the game once side B's central 50-gem group is seen down before CC_CRA_RUSH
#define CC_CRA_MODE 6
#endif
#ifndef CC_CRA_R
#define CC_CRA_R 300
#endif
#ifndef CC_CRA_RUSH
#define CC_CRA_RUSH 80
#endif
#ifndef CC_CRA_M
#define CC_CRA_M 1.5f
#endif
#define CC_ANY (CC_CT_HOLD || CC_CR_BIG || CC_CR_HOLD)
