#pragma once
#ifndef Z346_FARM
#define Z346_FARM 3
#endif
#ifndef Z346_EIGHT
#define Z346_EIGHT 3
#endif
// Exact-layout modes: 0 OFF, 1 winning draft, 2 objective order, 3 combined.
#define Z346_DRAFT ((Z346_FARM | Z346_EIGHT) & 1)
#define Z346_ORDER ((Z346_FARM | Z346_EIGHT) & 2)
