<div align="center">

<h1>Still Testing</h1>

<p><strong>Cambridge Battlecode Autumn 2026</strong></p>
<p><code>C++</code> &nbsp; <code>Solo project</code> &nbsp; <code>SDK 4.0.0</code></p>

<p>
  <a href="#algorithm">Algorithm</a> &nbsp; / &nbsp;
  <a href="#results">Results</a> &nbsp; / &nbsp;
  <a href="#source-and-reproduction">Source &amp; reproduction</a>
</p>

</div>

---

I built **Still Testing** as my solo entry in Cambridge Battlecode Autumn 2026.
The bot coordinates multiple champions to clear camps, collect resources and
deliver gems while managing combat, upgrades and a per-turn execution budget.

## Algorithm

I used a rule-based controller built around **utility scoring**, **grid distance
fields** and a **finite-state machine**. Generic policies handle the core game
loop, with map-specific drafting and strategy overrides for recognized layouts.

### Map-aware drafting

I identify known layouts from map dimensions and terrain patterns, then select
champions from predefined draft orders. The general draft policy also considers
team size, access to altars and the opponent's revealed picks.

Source: [draft dispatch](bots/v792/src/main.cpp) and
[general draft policy](bots/v792/src/optimized.cpp#L8862).

### Utility-based target selection

I rank reachable camp groups by estimated reward per turn. The score combines
gems, a changing gold weight, expected reward share, travel time, respawn timing,
clear time and the cost of recovering from damage. For an uncleared camp, the
base heuristic is:

```text
value    = gems + gold_weight * gold
priority = value * reward_share
           / (arrival + clear_time + risk_penalty + capture_time * reward_share + 1)
```

Map and champion weights adjust this score. A bonus for the current target
reduces unnecessary switching, while allied claims influence estimated damage
and reward sharing.

Source: [target selection](bots/v792/src/optimized.cpp#L5962).

### Distance-field navigation

I use the SDK's distance fields over bitboard terrain to estimate travel costs
and choose steps toward a goal. Movement checks visible occupants, uses dashes
when available and can fall back to terrain-only routing when units block a
path. Specialized routes also handle teleport access on selected maps.

Source: [movement and occupancy](bots/v792/src/optimized.cpp#L3132).

### Shared-memory coordination

Champions publish compact target IDs and activity states through shared memory.
I use those claims to avoid duplicate capture assignments, estimate allied
contributions to camp fights and support role-specific grouping.

Source: [target claims](bots/v792/src/optimized.cpp#L2578).

### State-based combat and economy

The main controller switches between **farming**, **banking** and **recall**.
Health, carried gems, altar access and nearby threats determine when to leave
camp work or secure resources. Local combat rules choose attacks, abilities and
retreats; shopping follows champion-specific build priorities.

Source: [turn controller](bots/v792/src/optimized.cpp#L7990) and
[purchase policy](bots/v792/src/optimized.cpp#L2848).

## Results

**International Qualifiers, 4 November 2026**

| Entrants | Seed | Pairing record | Game record |
| :---: | :---: | :---: | :---: |
| **504** | **83** | **4-2** | **87-39** |

I finished in the **tied 65-96 group**, outside the eight international finals
places. This range is derived from the completed double-elimination bracket,
rather than a published individual rank.

[Official bracket](https://2026a.battlecode.co.uk/game/tournaments/3e98ebb8-a7eb-4751-bb6a-4aac2d565981)
&nbsp; / &nbsp; [Match results](results/qualifier-summary.json)

## Source and reproduction

The qualifier entry was **v792**. Its [209 source files](bots/v792/src) match
the downloaded platform archive byte for byte. I also retained two later
research drafts: [v10](candidates/v10/README.md), compiled but unplayed, and
[v11](candidates/v11/README.md), held before compilation.

Verify the archived source and recorded results without running a bot:

```sh
python3 tools/verify_archive.py
```

[Build guide](docs/REPRODUCING.md) &nbsp; / &nbsp;
[Source provenance](docs/PROVENANCE.md) &nbsp; / &nbsp;
[Research notes](research/README.md)
