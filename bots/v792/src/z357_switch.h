#pragma once
#ifndef Z357_FRAGMENT
#define Z357_FRAGMENT 2
#endif
#ifndef Z357_STAMPEDE
#define Z357_STAMPEDE 0
#endif
// Each exact layout independently: 0 OFF, 1 winning draft, 2 objective order, 3 both.
#define Z357_DRAFT ((Z357_FRAGMENT | Z357_STAMPEDE) & 1)
#define Z357_ORDER ((Z357_FRAGMENT | Z357_STAMPEDE) & 2)
