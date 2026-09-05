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
| Enemy speeds, HP = current wave number rule | **Real**, extracted |
| Grid/lane mechanic (fixed path, directional tower firing, wall-blocked range) | **Real mechanic**, reimplemented from the decompiled logic |
| Map tile layouts (Forest/Village/Ravine) | **Original layouts** - real per-map tile data wasn't extracted this pass, see reference doc |
| Sprite art (towers/enemies/tiles) | **Original placeholders** - the 422 images extracted from the SWF decoded fine but mostly aren't matched to a specific role yet |
| Sound effects (place/select/fire) | **Real**, extracted from the SWF and converted to WAV |
| Starting currency/lives, sell refund %, wave pacing | **Reasonable defaults**, not extracted values |
| Newgrounds medals/leaderboards, ads, cutscenes | **Intentionally dropped** - no PSP equivalent |

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
./build-desktop/mtd2_desktop        # optional arg: map index (0=Forest, 1=Village, 2=Ravine)
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

Produces `build-psp/EBOOT.PBP`. Copy it (plus the `assets/` folder,
next to it or under `ms0:/PSP/GAME/MTD2/`) to a real PSP or PPSSPP.
**This EBOOT is confirmed to build and link cleanly against the real
pspdev toolchain, but hasn't been run on real hardware or in an
emulator** - there's no PSP available in the environment this was
built in, so first boot may still turn up runtime issues (an early
target: check for anything obviously wrong in the GU init sequence or
texture upload before debugging gameplay logic).

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
- Matching specific extracted sprites to specific towers/enemies
  (needs walking each sprite's `DefineSprite` placement chain - the
  raw images are already sitting in `assets/images/`)
- The other ~10 enemy types and ~13 maps documented but not yet wired
  up (`docs/original-game-reference.md` has full stats for all of them)
- The permanent lifetime-kills tower tier system (1-5, cosmetic +
  likely a stat bonus) isn't implemented
- No PSP hardware/emulator testing yet (see above)
