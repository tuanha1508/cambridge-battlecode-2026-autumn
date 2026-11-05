#pragma once
// cc_vesper (29 Oct 2026): Vesper reward funnel. Ranked top-10 teams give Vesper 31% of their team's camp-reward
// captures by r300 (4.8 per game, first at r39, 328 earned gold at r300); ours gets 16% (2.6, first at r124, 106 gold).
// Capture rewards are split among the capturing team's bodies on the group tiles and the last hit gives no claim, so
// the lever is occupancy at capture time. With VF_ON, on the exact maps in VF_MAPS and only while Vesper is drafted:
//  - a non-Vesper body skips a ready reward (st == 1) that Vesper has published as her target, or that Vesper, farming
//    and healthy, stands within VF_N tiles of, unless an enemy is visible within VF_EN tiles of the group;
//  - Vesper scores ready rewards x VF_VB.
// Camp targets (st 0/2), fight, bank and recall rules are unchanged. VF_R: rule active before this round only.
#ifndef VF_ON
#define VF_ON 1
#endif
#ifndef VF_MAPS
#define VF_MAPS 1       // bit 1: 02fields (z431 id 7), 2: 00eight (z451 id 3), 4: 01starvation (z436 id 4; off: 2/6 vs 4/6 locally), 8: 02treasure_box (z431 id 8)
#endif
#ifndef VF_N
#define VF_N 5
#endif
#ifndef VF_EN
#define VF_EN 7
#endif
#ifndef VF_VB
#define VF_VB 2.0f
#endif
#ifndef VF_R
#define VF_R 2001
#endif
#ifndef VF_HP
#define VF_HP 0.5f
#endif
