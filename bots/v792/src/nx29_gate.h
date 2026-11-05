#pragma once
// cc_pin32: GC1 only (row id 14 here; ids 11-13 are cc_sideb7). GD2/CR/EI off and their rows not ported; nx27 NX_BB not ported.
// cc_next29 (29 Oct; docs/claude-next29-2026-10-29.md). Gentle exact-layout levers, one switch per map, all from r300.
// Evidence: ranked games since 27 Oct (research/claude-next29/: up to 5 wins + 8 losses per map).
//   NX_GC1 adgato-gemcombat1 (optimized.cpp, nx25 row 12): the contested 21-gem side groups 2/3 swing it (side-A wins
//          clear g3 8-10 / g2 6-7; losses 0-4 while the opponent takes 8-11). From NX_GC1_R groups 2/3 value x NX_GC1_M.
//   NX_GD2 02guardians (optimized.cpp, row 13): wins clear the 200-gem monoliths 20/21 1-3 times each, losses 0 (the
//          opponent takes them). From NX_GD2_R groups 20/21 value x NX_GD2_M (claude-next23's 4-body gang-up lost a win).
//   NX_CR  02crossing (optimized.cpp, row 14): the 50-gem/90-gold groups 0/1 decide it (wins: we out-clear 9-12 vs 2-6;
//          losses: 2-8 vs 6-16). From NX_CR_R groups 0/1 value x NX_CR_M.
//   NX_EI  00eight (optimized.cpp, row 15): in losses the opponent farms the 8-gem/120-gold centre groups 2/3 (9-13
//          clears each) and we die 15-36 times. From NX_EI_R the own-side centre group (2 or 3, spawn_dist <= 22) values
//          x NX_EI_M (v1 valued both 2/3 and lost the local side-B pin win).
//   NX_BB  01boss-battle (interim_optimized.cpp, nx27 row 3): see nx27_gate.h (+1 body on the 400-gem boss group 16).
#ifndef NX_GC1
#define NX_GC1 1
#endif
#ifndef NX_GC1_R
#define NX_GC1_R 300
#endif
#ifndef NX_GC1_M
#define NX_GC1_M 1.3f
#endif
#ifndef NX_GD2
#define NX_GD2 0   // cc_pin32: off (only GC1 ported)
#endif
#ifndef NX_GD2_R
#define NX_GD2_R 300
#endif
#ifndef NX_GD2_M
#define NX_GD2_M 1.3f
#endif
#ifndef NX_CR
#define NX_CR 0   // cc_pin32: off (only GC1 ported)
#endif
#ifndef NX_CR_R
#define NX_CR_R 300
#endif
#ifndef NX_CR_M
#define NX_CR_M 1.3f
#endif
#ifndef NX_EI
#define NX_EI 0   // cc_pin32: off (only GC1 ported)
#endif
#ifndef NX_EI_R
#define NX_EI_R 300
#endif
#ifndef NX_EI_M
#define NX_EI_M 1.3f
#endif
namespace nx29_ {
inline float group_mult25(int id, int gi, int sd, int r) {
    if (NX_GC1 && id == 14 && (gi == 2 || gi == 3) && r >= NX_GC1_R) return NX_GC1_M;
    return 1.0f;
}
}  // namespace nx29_
