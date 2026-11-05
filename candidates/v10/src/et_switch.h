#pragma once
// cc_et (29 Oct 2026): Elk Ring / Treasure Box exact-layout access fixes, docs/claude-et-2026-10-29.md.
// One switch per map; 0 = map plays exactly as cc_pin15.
// ET_ELK_BANK: on exact 00elkring (z160_ id 5, 30x30 prefilter, turn 3) a farming champion enters bank mode at
//   >= N gems instead of 36/32. Every camp pays 22, so the base banks after two camps (44-gem bags). Ranked/G98:
//   opponents bank 22-gem bags first at r45-70 (we r52-180) and lose 20 loaded gems/game vs our 108 in losses.
// ET_TBX: on exact 02treasure-box (z431_ id 8, 35x35 prefilter, turn 2) reuse the Fracture crystal bank route
//   (frc_*) with the table below: a carrier in bank mode on an L tile hops (range 3) over the ring wall.
//   Group 30/31 (61 gems) walk 47 -> ~20; spawn 29 -> ~19. We buy 0 crystals there; opponents 3.6-5/game.
//   Table: research/claude-et/gen_launch_tb.py 14 15 (landing <= 14 steps from a submit tile, saves >= 15).
#ifndef ET_ELK_BANK
#define ET_ELK_BANK 22
#endif
// ET_ELK_NEAR > 0: the lower trigger applies only within this many steps of an altar (near groups 6/9/7/8, foot 5);
//   0 = anywhere. 22-anywhere lost head-to-head vs cc_pin15 on both sides (research/claude-et/runs/v1_elk22any).
#ifndef ET_ELK_NEAR
#define ET_ELK_NEAR 8
#endif
#ifndef ET_TBX
#define ET_TBX 0   // off: G101 Treasure Box -1
#endif
#define ET_TBX_POST 17   // hop channel (3) + walk after landing (<= 14)
static const char* const ET_TBX_LAUNCH[35] = {
    "...................................",
    "...................................",
    "...................................",
    "...................................",
    "...................................",
    "...................................",
    "...................................",
    "............LLLLLLLLLLL............",
    "............LLLLLLLLLLL............",
    "...................................",
    "...................................",
    "...................................",
    ".....L.......................L.....",
    ".....L.......................L.....",
    ".....L.......................L.....",
    ".....L.......................L.....",
    ".....L.......................L.....",
    ".....L.......................L.....",
    ".....L.......................L.....",
    ".....L.......................L.....",
    ".....L.......................L.....",
    ".....L.......................L.....",
    ".....L.......................L.....",
    "...................................",
    "...................................",
    "...................................",
    "............LLLLLLLLLLL............",
    "............LLLLLLLLLLL............",
    "...................................",
    "...................................",
    "...................................",
    "...................................",
    "...................................",
    "...................................",
    "...................................",
};
