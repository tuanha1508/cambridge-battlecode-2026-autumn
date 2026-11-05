#pragma once
#ifndef Z340_PILGRIMAGE
#define Z340_PILGRIMAGE 0
#endif
#ifndef Z340_SWIRL
#define Z340_SWIRL 3
#endif
#ifndef SWB_MODE
#define SWB_MODE 0
#endif
// SWB_MODE (Swirl side B only): 0 off, 1 leave own 15 group after first clear, 2 prefer own 23 group from the start, 3 = 1 but Marcella keeps r90, 4 = 3 plus Vesper, 5 = 3 plus HoR, 6 = 3 with movers waiting until r60.
// Each exact map: 0 OFF, 1 draft, 2 objective order, 3 combined.
#define Z340_DRAFT ((Z340_PILGRIMAGE | Z340_SWIRL) & 1)
#define Z340_ORDER ((Z340_PILGRIMAGE | Z340_SWIRL) & 2)
