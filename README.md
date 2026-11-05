# Still Testing: Cambridge Battlecode Autumn 2026

C++ bot and research archive for the Still Testing solo team.

## Result

The International Qualifiers finished on 4 November 2026. We did not reach the
top eight and did not qualify for the international finals.

- Seed 83 among 504 entrants.
- Four pairing wins and two pairing losses in double elimination.
- 87 game wins and 39 losses across six 21-game pairings.
- Tied 65-96 by the completed bracket's elimination cohort. This is a derived
  range, not an official published ordinal placement.

| Pairing | Opponent | Our score | Result |
| --- | --- | --- | --- |
| Upper 1 | Gilded Pathfinders | 21-0 | Win |
| Upper 2 | kata | 16-5 | Win |
| Upper 3 | SilverSamurai | 10-11 | Loss |
| Lower 4 | NotSoLonely | 19-2 | Win |
| Lower 5 | Feral Monoliths | 13-8 | Win |
| Lower 6 | estrogen :3 | 8-13 | Eliminated |

The [official bracket](https://2026a.battlecode.co.uk/game/tournaments/3e98ebb8-a7eb-4751-bb6a-4aac2d565981),
[saved public response](results/official-tournament.json) and
[machine-readable result](results/qualifier-summary.json) support these results.
Ladder rank, tournament seed and final placing are different measurements.

## What we built

- A submitted C++ bot with drafting, map recognition, navigation, camp work,
  combat, resource collection, delivery and upgrades. The final played version
  was v792. Its last promoted change reduced a late Big World monolith's value.
- Source and artifact checks that separate the exact submitted bytes from
  local variants. All 209 files in the retrieved v792 platform archive match
  the retained local source byte for byte.
- A dated research census of 817 teams, with engine and time filters, and
  analyses separating economy, contested work, cargo delivery and upgrades.
  This archive includes a [research summary](research/README.md), rather than
  private chat logs or bulk opponent histories.
- A compiled but unplayed v10 research candidate and a held, uncompiled v11
  draft. Their source work did not establish a competitive improvement and
  neither replaced the saved qualifier bot.

## Contents

| Path | Status |
| --- | --- |
| [bots/v792/src](bots/v792/src) | Exact final platform source, 209 files |
| [archives/v792-platform-source.zip](archives/v792-platform-source.zip) | Original downloaded platform archive |
| [candidates/v10](candidates/v10/README.md) | Compiled research candidate, never played or promoted |
| [candidates/v11](candidates/v11/README.md) | Held source draft, never compiled or played |
| [docs/REPRODUCING.md](docs/REPRODUCING.md) | Dependencies, build instructions and limits |
| [docs/PROVENANCE.md](docs/PROVENANCE.md) | Hashes and evidence boundaries |
| [provenance/source-manifests.json](provenance/source-manifests.json) | Per-file source hashes |
| [tools/verify_archive.py](tools/verify_archive.py) | Offline source and result verification |

Run the non-executing archive checks with Python 3:

```sh
python3 tools/verify_archive.py
```

This closeout ran no new bot builds, games, trials or platform changes. Private
Discord messages, credentials, browser state, downloaded SDKs and bulk runtime
data are excluded. No license grant for the archived code is implied.
