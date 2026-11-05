#pragma once
// ce (claude-econ, 28 Oct; docs/claude-econ-2026-10-28.md). Map ids (bit id-1):
// 1 00anvil, 2 00quarry, 3 02fields, 5 adgato-gemcombat2, 6 02treasure-box, 7 01fracture  (optimized.cpp)
// 8 02harvest (legacy.cpp), 9 00great-wall (z114_policy.cpp), 10 00gem-fields (z115_policy.cpp)
#ifndef CE_COOPMAPS
#define CE_COOPMAPS 0x204   // co-op big groups: Fields + Gem Fields (Harvest removed: lost 2 wins on platform G45)
#endif
#ifndef CE_GOLDMAPS
#define CE_GOLDMAPS 0x0     // Anvil gold cap off (G45: +2/-1, lost a win)
#endif
