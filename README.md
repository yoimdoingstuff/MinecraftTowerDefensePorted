# Minecraft Tower Defense 2 - PSP port

A from-scratch, cross-platform reimplementation of the Flash game
*Minecraft Tower Defense 2* (andkon.com), targeting PSP homebrew and
desktop (Linux/macOS/Windows via SDL2) from one shared C codebase.

## Status - what's real, what's a placeholder

This is a **working core engine with a curated starter slice**, not a
1:1 port of the whole game - see `docs/original-game-reference.md` for
the full reverse-engineering notes and exactly what was and wasn't
extracted. Quick summary of what's real vs. stand-in right now:

| Piece | Status |
|---|---|
| Tower stats (cost/power/range/firerate) for all 9 dispensers | **Real**, pulled from the decompiled AS2 |
| Tower art (all 9) | **Real** - resolved from the SWF's own sprite/shape data, see reference doc |
| Enemy speeds, HP = current wave number rule, all 14 hostile types | **Real**, extracted |
| Enemy art: creeper, ghast, magma cube, slime, Herobrine | **Real** |
| Enemy art: the other 9 (zombie, skeleton, spider, cave_spider, silverfish, zombie_pig, blaze, spider_jockey, enderman) | **Placeholder** - these resolve to one limb of a multi-part animated character, not a full sprite; using the raw fragment would look broken, see reference doc |
| Grid/lane mechanic (fixed path, directional tower firing, wall-blocked range) | **Real mechanic**, reimplemented from the decompiled logic |
| Tier 1-5 system (lifetime kills per tower type) | **Real mechanic** (arrow's thresholds), **designed stat bonus** (original's exact bonus wasn't traced) - in-memory only, doesn't persist across runs yet |
| Map tile layouts (all 9) | **Original layouts** - real per-map tile data wasn't extracted this pass, see reference doc |
| Per-map visual tinting | **Original** - a mood wash, not real per-map background art |
| Herobrine boss wave (Nether Stronghold, final wave) | **Original design choice** - a boss exists in the source, but this pairing/placement is ours |
| Sound effects (place/select/fire) | **Real**, extracted from the SWF and converted to WAV |
| Starting currency/lives, sell refund %, wave pacing | **Reasonable defaults**, not extracted values |
| Newgrounds medals/leaderboards, ads, cutscenes | **Intentionally dropped** - no PSP equivalent |
| Wolf / Snow Golem allies | **Not implemented** - these are player-summonable allies in the original, a different mechanic than the hostile wave roster |

None of this is locked in - swapping in real map layouts or matched
sprite art later only touches data files (`src/core/map_data.c`,
`assets/images/`), not engine code.

## Controls

| Action | PSP | Desktop |
|---|---|---|
| Move cursor | D-pad / analog stick | Arrow keys / WASD |
| Place tower | X | Enter / Z |
| Sell tower under cursor | O | Backspace / X |
| Next tower type | Triangle | E / `]` |
| Previous tower type | Square | Q / `[` |
| Pause | Start | Space |
| Toggle 2x speed | L | Tab |
| Start next wave | R | N |
| Quit | HOME | Esc |

## Building

### Desktop (Linux/macOS/Windows)

Needs CMake and SDL2 development libraries.

```bash
# Linux
sudo apt install libsdl2-dev cmake build-essential
# macOS
brew install sdl2 cmake

cmake -B build-desktop -DCMAKE_BUILD_TYPE=Release
cmake --build build-desktop
./build-desktop/mtd2_desktop        # optional arg: map index, 0-8 (see docs/original-game-reference.md for the list)
```

On Windows, grab an `SDL2-devel-*-VC.zip` from
[libsdl-org/SDL releases](https://github.com/libsdl-org/SDL/releases),
point `SDL2_DIR` at its `cmake/` folder, then run the same two `cmake`
commands.

### PSP

Needs the [pspdev](https://github.com/pspdev/pspdev) toolchain.

```bash
export PSPDEV=/path/to/pspdev
export PATH=$PSPDEV/bin:$PATH
cmake -B build-psp -DCMAKE_TOOLCHAIN_FILE=$PSPDEV/psp/share/pspdev.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build-psp
```

Produces `build-psp/EBOOT.PBP`. **The `assets/` folder must sit right
next to EBOOT.PBP** - e.g. `ms0:/PSP/GAME/MTD2/EBOOT.PBP` needs
`ms0:/PSP/GAME/MTD2/assets/` (not `assets/` anywhere else under
`PSP/GAME/`, specifically alongside that EBOOT). The game finds its own
assets by asking the OS for the full path it was launched from and
looking next to that (via `argv[0]`), not by assuming the working
directory is already correct.

**Status: tested on real PSP hardware once, and assets failed to
load.** That's what the argv[0] fix above addresses - the previous
version assumed the working directory was already the EBOOT's folder,
which isn't guaranteed for every launch method. This fix hasn't been
re-confirmed on hardware yet (that needs another test pass), so treat
it as the most likely fix based on the code, not a verified one. If
assets still don't show up after this, the folder placement above is
the next thing to double check, followed by actually reporting back
what's on screen (crash / black screen / tiles but no sprites / etc) so
the next fix can target the real cause instead of guessing again.

### GitHub Actions

`.github/workflows/build.yml` builds all four targets (PSP + Linux +
macOS + Windows) on every push and uploads them as workflow artifacts
- no local toolchain needed to get a build. The PSP job downloads the
same pspdev release used above; bump the version tag in the workflow
file to pick up newer toolchain releases.

## Project layout

```
src/core/       portable game logic - no platform-specific code at all
src/platform/   one backend per target (desktop = SDL2, psp = pspgu/sceCtrl/pspaudiolib)
third_party/    vendored lodepng (PNG decode, plain C, no dependencies)
assets/         extracted images/sounds + the generated placeholder sprites
tools/          asset generation/conversion scripts
docs/           reverse-engineering notes from the original SWF
```

The split is deliberate: `src/core` never includes a platform header
or calls a platform API directly, only what's declared in
`src/core/platform.h`. Adding a third platform (Vita homebrew, a web
build, whatever) means writing one new file that implements that
header - none of the game logic changes.

## Known gaps / natural next steps

- Real per-map tile layouts (the actual Forest/Village/etc. corridors,
  not the original stand-ins here)
- Compositing the 9 articulated-character enemies (zombie, skeleton,
  spider, cave_spider, silverfish, zombie_pig, blaze, spider_jockey,
  enderman) from their real limb pieces instead of using placeholder
  art - needs each part's placement matrix from the parent clip, see
  reference doc
- The 3 "classic" maps and a few other named locations documented but
  not yet wired up (`docs/original-game-reference.md`)
- Wolf / Snow Golem player-summonable allies - a mechanic, not just a
  data entry
- Persisting the tier system's kill counts across runs (needs a save
  file - not the same problem on PSP vs. desktop, hasn't been designed)
- Fix confirmation for the PSP asset-loading bug (see PSP build section
  above) - reasoned through and fixed, not yet re-verified on hardware
