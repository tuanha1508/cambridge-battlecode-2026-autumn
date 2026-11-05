#pragma once
#ifndef Z335_GOLD
#define Z335_GOLD 0
#endif
#ifndef Z335_ROAD
#define Z335_ROAD 1
#endif
// Per exact map: 0 OFF, 1 draft, 2 objective order, 3 both.
#define Z335_DRAFT ((Z335_GOLD & 1) || (Z335_ROAD & 1))
#define Z335_ORDER ((Z335_GOLD & 2) || (Z335_ROAD & 2))
