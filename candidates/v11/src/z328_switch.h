#pragma once
#ifndef Z328_FRACTURE
#define Z328_FRACTURE 0
#endif
#ifndef Z328_GW
#define Z328_GW 1
#endif
// 0 OFF, 1 draft, 2 objective order, 3 both. Maps are independent.
#define Z328_DRAFT ((Z328_FRACTURE & 1) || (Z328_GW & 1))
#define Z328_ORDER ((Z328_FRACTURE & 2) || (Z328_GW & 2))
