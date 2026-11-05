// cc_pin20 (from cc_wards2): deterministic free-ward placement step, shared by every policy file.
// Called once per champion turn after play_turn() and before end_turn().
//
// Determinism by construction: no decision reads fuel_remaining() except one hard
// safety bail (WARD_FUEL_BAIL, default 25000; fired 93 times in 26 local games), so
// unrelated code changes elsewhere cannot shift when or where wards fire.
// Scoring uses only cheap bitboard maths (a Chebyshev disc of the ward vision
// radius, no line-of-sight queries) over a fixed set of 9 candidate tiles, so the
// step has a small, bounded cost.
//
// It never moves, attacks, casts or shops; wards are free, so it cannot spend gold.
// WARD_ON=0 compiles the step out (behaviour identical to the base bot).
#pragma once
#include <cambc.h>
#include <cstdint>
#include <iostream>
#include "z431_layout.h"
#include "mg_switch.h"   // cc_merge_all

#ifndef WARD_ON
#define WARD_ON 1
#endif
// First round a ward may be placed.
#ifndef WARD_START
#define WARD_START 6
#endif
// Hard safety bail: skip if fuel left after play_turn is below this. The step costs ~13k for
// place_ward itself + ~5k scoring, so a lower bail would turn these turns into fuelExhausted.
// It only fires on turns where the base bot already used >75k of its 100k.
#ifndef WARD_FUEL_BAIL
#define WARD_FUEL_BAIL 25000
#endif
// Score weight of camp (visible) / altar tiles inside the ward disc relative to plain new tiles.
#ifndef WARD_W_INTEREST
#define WARD_W_INTEREST 6
#endif
// 1 = never place inside the enemy spawn region.
#ifndef WARD_NO_ENEMY_SPAWN
#define WARD_NO_ENEMY_SPAWN 1
#endif
// Minimum weighted score needed to place.
#ifndef WARD_MIN_SCORE
#define WARD_MIN_SCORE 4
#endif
// 1 = no wards on 41x41 maps (00collision; it lost 2 in platform gauntlet g109).
#ifndef WARD_SKIP_COLLISION
#define WARD_SKIP_COLLISION 1
#endif
// Only place while carrying at least this many gems (0 = always, top-team style).
#ifndef WARD_GEMS
#define WARD_GEMS 0
#endif
// 1 = print a WARD line per placement with the step's fuel cost (local only; adds no decisions).
#ifndef WARD_LOG
#define WARD_LOG 0
#endif
// cc_pin20: 1 = wards only on exact 00stampede (z69 draft marker) and 02treasure-box (z431 id 8).
// Platform: both maps gained with no lost win under cc_wards (g109) and cc_wards2 (g113).
#ifndef WARD_MAP_GATE
#define WARD_MAP_GATE 1
#endif
// Test-only: busy-loop iterations burned every turn before the step (determinism proof).
// cc_tba (30 Oct; docs/claude-tba-2026-10-30.md): 1 = on exact 02treasure-box (side A, the only side with wards
// there) the WARD_FUEL_BAIL check is skipped, so ward timing never reads fuel_remaining() (no fuel drift).
// cc_merge_all: TBA_DET defaults to 0 in mg_switch.h (candidate value 1).
#ifndef TBA_DET
#define TBA_DET 1
#endif
// cc_tba: with TBA_DET, the first Treasure Box ward round (the old bail fired mostly on r8-r20 heavy turns).
#ifndef TBA_R
#define TBA_R 75
#endif
#ifndef WARD_NOOP_GATE
#define WARD_NOOP_GATE 0
#endif

namespace wardx {

struct State {
    bool off = false;
#if TBA_DET
    bool tbox = false;          // cc_tba: exact Treasure Box (side A)
#endif
    cambc::bitboard interest;   // altar tiles (visible camps added per placement)
    cambc::bitboard banned;     // walls | holes | altars | (enemy spawn)
    int W = 0, H = 0, R = 0;
    float V = 0;
    int fails = 0, placed = 0;
    int last_fail_round = -100;
};
static State* g = nullptr;

static void init_once() {
    g = new State();
    g->W = cambc::map::width();
    g->H = cambc::map::height();
    g->R = static_cast<int>(cambc::constants::ward_placement_radius);
    if (g->R < 0) g->R = 0;
    g->V = cambc::constants::ward_vision_radius;
#if WARD_MAP_GATE
    {
        // Stampede: z69::detect() after play_turn exhausted fuel on round 6 (5 events), so read the
        // draft-memory marker the z69 policy writes when its exact detector fires (plan[8] = 69).
        const bool stampede = g->W == 60 && g->H == 50 && cambc::draft_memory.size() > 8 &&
                              std::to_integer<int>(cambc::draft_memory[8]) == 69;
        // cc_pin22: Treasure Box wards on SIDE A only (side-B gauntlet G129: wards off 0/6 -> 2/6 on side B).
        const bool tbox = g->W == 35 && g->H == 35 &&
                          !cambc::map::spawn_region(cambc::team::ally).is_set(cambc::vec2{ 20, 31 }) &&
                          z431_::detect() == 8;
#if TBA_DET
        g->tbox = tbox;   // cc_tba
#endif
        if (!stampede && !tbox) { g->off = true; return; }
    }
#endif
#if WARD_SKIP_COLLISION
    if (g->W == 41 && g->H == 41) g->off = true;
#endif
    g->interest = cambc::map::altars();
    g->banned = cambc::map::walls() | cambc::map::holes() | cambc::map::altars();
#if WARD_NO_ENEMY_SPAWN
    g->banned |= cambc::map::spawn_region(cambc::team::enemy);
#endif
}

static void tick() {
#if WARD_ON
    try {
        const int round = static_cast<int>(cambc::round_number());
#if WARD_NOOP_GATE
        // Test-only: burn fuel every turn (as an unrelated code change would) without deciding anything.
        for (volatile int z = 0; z < WARD_NOOP_GATE; z = z + 1) {}
#endif
        if (round < WARD_START) return;
#if TBA_DET
        if (!(g && g->tbox) && cambc::fuel_remaining() < static_cast<std::uint64_t>(WARD_FUEL_BAIL)) {   // safety only (cc_tba)
#else
        if (cambc::fuel_remaining() < static_cast<std::uint64_t>(WARD_FUEL_BAIL)) {   // safety only
#endif
#if WARD_LOG
            std::cout << "WARDBAIL " << round << "\n";
#endif
            return;
        }
        auto& me = cambc::self();
        if (!me.is_alive()) return;
#if WARD_LOG
        const auto fuel0 = cambc::fuel_remaining();
#endif
        if (!g) init_once();
        if (g->off) return;
#if TBA_DET
        if (g->tbox && round < TBA_R) return;   // cc_tba: deterministic start instead of the fuel bail
#endif
        if (g->fails >= 6 && round - g->last_fail_round < 50) return;
        if (me.ward_cooldown() > 0) return;
        if (WARD_GEMS > 0 && me.gem_count() < static_cast<std::uint32_t>(WARD_GEMS)) return;
        if (me.restrictions().contains(cambc::buff_restrictions::ward)) return;
        {
            // Placing during a tribute/recall channel throws "cancel ... before acting".
            const auto st = me.state();
            if (st.contains(cambc::champion_state::recalling) || st.contains(cambc::champion_state::submitting_gems) ||
                st.contains(cambc::champion_state::recall_started) || st.contains(cambc::champion_state::submit_gems_started) ||
                st.contains(cambc::champion_state::teleporting))
                return;
        }

        const cambc::vec2 pos = me.position();
        const int R = g->R;
        const cambc::bitboard seen = cambc::vision::visible_tiles();
        const cambc::bitboard wards = cambc::vision::ward_tiles(cambc::team::ally) | cambc::vision::ward_tiles(cambc::team::enemy);
        const cambc::bitboard interest = g->interest | cambc::vision::camp_tiles();
        const cambc::bitboard unseen = ~seen;
        cambc::vec2 best = pos;
        int best_score = -1;
        cambc::bitboard disc{cambc::uninitialized};
        // Fixed candidate order: own tile, then the 4 diagonal tiles at the placement radius
        // (diagonals extend the Chebyshev disc furthest; 9 candidates cost ~7-10k fuel, 5 cost ~4k).
        static constexpr int DX[5] = {0, -1, 1, -1, 1}, DY[5] = {0, -1, -1, 1, 1};
        for (int i = 0; i < 5; ++i) {
            const int dx = DX[i], dy = DY[i];
            const cambc::vec2 p(pos.x + dx * R, pos.y + dy * R);
            if (p.x < 0 || p.y < 0 || p.x >= g->W || p.y >= g->H) continue;
            if (g->banned.is_set(p) || wards.is_set(p) || !seen.is_set(p)) continue;
            disc.clear();
            disc.draw_chebyshev(p, g->V);
            const int sc = static_cast<int>((disc & unseen).count()) + WARD_W_INTEREST * static_cast<int>((disc & interest).count());
            if (sc > best_score) { best_score = sc; best = p; }
        }
        if (best_score < WARD_MIN_SCORE) return;
        const bool ok = me.place_ward(best);
        if (ok) {
            ++g->placed;
            g->fails = 0;
#if WARD_LOG
            std::cout << "WARD " << round << " t" << static_cast<int>(me.champion_name()) << " at " << best.x << "," << best.y
                      << " from " << pos.x << "," << pos.y << " sc " << best_score << " n " << g->placed
                      << " fuel " << (fuel0 - cambc::fuel_remaining()) << "\n";
#endif
        } else {
            ++g->fails; g->last_fail_round = round;
        }
    } catch (const std::exception& e) {
        if (g) { ++g->fails; g->last_fail_round = static_cast<int>(cambc::round_number()); }
#if WARD_LOG
        std::cout << "WARDERR " << e.what() << "\n";
#endif
    } catch (...) {
        if (g) ++g->fails;
    }
#endif
}

}  // namespace wardx
