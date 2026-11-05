#pragma once
// cc_quarry2b (29 Oct; docs/claude-quarry2-2026-10-29.md). 00quarry only: every lever below acts only when g_ce == 2,
// which ce_tick sets on turn 3 from the exact gate (35x29 and z431_::detect() == 6).
// Ranked Quarry since 27 Oct (W3 L33): winners clear g8 (46 gems) ~9 and g9 (32) ~8.5 times a game, we 3 and 2;
// deaths 2 vs 9.5, kills 9 vs 1.5. Our bodies contest the central gem groups one at a time and get picked off.
// Chosen: Q2B_BANK 14 (v3). Local: beats cc_pin28 as A 1032:196, loses as B 734:1000 (pin mirror B 280); wins
// every proxy/old-line game the pin wins, incl. B vs cc_sfq 1002:288 (pin 1021:965). Co-op (v2) lost B vs cc_sfq.
#ifndef Q2B_COOP
#define Q2B_COOP 0      // 1 = CE co-op on Quarry: a farming body with >=2 healthy allies within 8 counts them as helpers on big groups
#endif
#ifndef Q2B_MING
#define Q2B_MING 30     // Quarry co-op minimum group gems (CE_MING is 40 = g8 only; 30 adds g9's 32)
#endif
#ifndef Q2B_BANK
#define Q2B_BANK 14     // >0: Quarry bank threshold (Z170 gives 32); 14 = bank after each g10/g11 clear
#endif
