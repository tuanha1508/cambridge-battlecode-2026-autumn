# Reproduction notes

The bot requires the official Cambridge Autumn SDK. It is not the MIT game or
Cambridge Spring game. The final local preparation record used cambc 4.0.0
with engine 2.0.0. SDK code and toolchains are not vendored here.

First verify the archived source without executing the bot:

```sh
python3 tools/verify_archive.py
```

For a future build, install the pinned official kit into a separate environment
and initialize a fresh SDK workspace. These are reproduction instructions,
not commands executed during this closeout:

```sh
python3 -m venv .venv
.venv/bin/python -m pip install 'cambc==4.0.0'
.venv/bin/cambc init official-workspace --yes
cp -R bots/v792 official-workspace/bots/still-testing-v792
.venv/bin/cambc build official-workspace/bots/still-testing-v792 --release
```

The source-only platform ZIP does not contain a CMake file, SDK or project
configuration. Keep all 209 source members; the SDK owns the build setup.
The init and build syntax was checked against the retained 4.0.0 CLI help,
but this exported layout has not been built or played. A future reproduction
must establish its own toolchain, artifact and game evidence.

Do not substitute the candidates for v792 when reproducing the qualifier.
v10 was release-compiled in the original workspace and statically audited,
but was never played. v11 has unresolved source blockers and was never
cleanly assembled, compiled or played. No new performance claim is made.
