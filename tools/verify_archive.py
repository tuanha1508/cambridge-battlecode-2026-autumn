"""Offline byte and result checks. Does not build or execute any bot."""
import hashlib
import json
from pathlib import Path
import zipfile

root = Path(__file__).resolve().parents[1]
sha = lambda data: hashlib.sha256(data).hexdigest()
manifests = json.loads((root / "provenance/source-manifests.json").read_text())
for version, manifest in manifests.items():
    base = root / ("bots/v792/src" if version == "v792" else f"candidates/{version}/src")
    actual = sorted(p.relative_to(base).as_posix() for p in base.rglob("*") if p.is_file())
    assert actual == [f["path"] for f in manifest["files"]], version
    combined = hashlib.sha256()
    for row in manifest["files"]:
        path = base / row["path"]
        assert not path.is_symlink()
        data = path.read_bytes()
        assert len(data) == row["bytes"] and sha(data) == row["sha256"], str(path)
        combined.update(data)
    assert len(actual) == manifest["fileCount"]
    assert combined.hexdigest() == manifest["sortedSourceSHA256"], version
archive = root / "archives/v792-platform-source.zip"
assert sha(archive.read_bytes()) == "7151b68575bed1c9692831a3c41304d77ec4d51716c5a8111090978d46fad993"
with zipfile.ZipFile(archive) as z:
    expected = ["src/" + f["path"] for f in manifests["v792"]["files"]]
    assert sorted(z.namelist()) == expected
    for name in expected:
        assert z.read(name) == (root / "bots/v792" / name).read_bytes(), name
summary = json.loads((root / "results/qualifier-summary.json").read_text())
raw = (root / "results/official-tournament.json").read_bytes()
assert sha(raw) == summary["sourceSHA256"]
official = json.loads(raw)
assert official["tournament"]["status"] == "complete" and official["tournament"]["resultsPublished"]
own = summary["teamId"]
entries = [x for x in official["snapshots"] if x["teamId"] == own]
assert len(entries) == 1 and entries[0]["seed"] == summary["seed"] == 83
assert len(official["snapshots"]) == summary["entrants"] == 504
rows = [b for b in official["bracket"] if any(isinstance(b.get(s), dict) and b[s]["teamId"] == own for s in ["a", "b"])]
rows.sort(key=lambda b: b["round"])
assert len(rows) == len(summary["pairings"]) == 6
wins = our_games = their_games = 0
for b, p in zip(rows, summary["pairings"]):
    side = "a" if b["a"]["teamId"] == own else "b"
    other = "b" if side == "a" else "a"
    assert b["status"] == "finished" and b["revealed"]
    assert b["code"] == p["code"] and b["groupId"] == p["groupId"]
    assert b[other]["teamName"] == p["opponent"]
    assert b[side + "Wins"] == p["ourWins"] and b[other + "Wins"] == p["opponentWins"]
    assert (b["winner"]["teamId"] == own) == p["won"]
    wins += p["won"]
    our_games += p["ourWins"]
    their_games += p["opponentWins"]
assert wins == summary["pairingWins"] == 4 and 6-wins == summary["pairingLosses"] == 2
assert our_games == summary["gameWins"] == 87 and their_games == summary["gameLosses"] == 39
assert not summary["pairings"][-1]["won"] and rows[-1]["loserTo"] is None
print("PASS: 671 source files, original v792 ZIP and six official qualifier pairings")
