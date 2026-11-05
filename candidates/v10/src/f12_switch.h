#pragma once
// cc_next12 (28 Oct): one compile switch per map; both are exact-layout gated (z451 id 1 / z436 id 3) and inactive elsewhere.
#ifndef F12_FARM
#define F12_FARM 3      // 01the-farm: bit 1 = camp-free bank detour around the centre-altar wall; bit 2 = south-west group weight
#endif
#ifndef F12_SWR
#define F12_SWR 150     // Farm south-west weight from this round
#endif
#ifndef F12_SWW
#define F12_SWW 2.0f
#endif
#ifndef F12_LW
#define F12_LW 0        // 00lost-world off (G67: -1, banked down)
#endif
#ifndef F12_LWWHO
#define F12_LWWHO 0x1000  // champion_type mask (bit 12 = Heart of Ruin)
#endif
#ifndef F12_LWR
#define F12_LWR 2000
#endif
#ifndef F12_LWW
#define F12_LWW 6.0f
#endif
#ifndef F12_LWOFF
#define F12_LWOFF 0.1f
#endif
