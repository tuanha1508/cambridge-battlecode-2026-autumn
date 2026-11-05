#pragma once
// cc_cargo (28 Oct 2026): gemcombat cargo fix, docs/claude-cargo-2026-10-28.md.
// CARGO_BANK > 0: on the exact adgato-gemcombat layouts in CARGO_LAYOUTS, a farming champion switches to bank mode
//   once it holds >= CARGO_BANK gems (one 21-gem camp reward) instead of the global 32 (Z170), so a single reward is
//   banked instead of being carried while farming the next group. Same !engaged()/cooldown conditions as the base.
// CARGO_TRACE: stdout trace of loaded turns and the map at turn 3 (diagnostics only; must be 0 in any upload).
#ifndef CARGO_BANK
#define CARGO_BANK 21
#endif
#ifndef CARGO_TRACE
#define CARGO_TRACE 0
#endif
// Exact layout digests (cargo_layout.h, same digest as research/claude-bank/cc_layout_dump.txt):
// adgato-gemcombat1 (31x31, 3 picks), adgato-gemcombat2 (51x31, 4 picks).
#ifndef CARGO_LAYOUTS
#define CARGO_LAYOUTS 7039503372822958125ULL   // GC2 only (G67: GC1 +1/-1 off)
#endif
#ifndef CARGO_DIMS
#define CARGO_DIMS {51, 31, 4}
#endif
