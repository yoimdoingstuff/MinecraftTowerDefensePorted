# Minecraft Tower Defense 2 — reverse-engineered reference

Source: `minecrafttowerdefence2.swf` (andkon.com), Flash Player 10, AS2, 11.9MB
compressed / 14.6MB decompressed, 3150 SWF tags. Extracted via a custom tag
parser + JPEXS ffdec 26.2.1 decompilation of all 1678 AS2 script blocks.
This doc is hand-written notes from reading that decompiled output — it is
NOT a copy of the original code, and is meant to ground an original C
reimplementation, not to reproduce Adobe/original-author source.

## Core loop
- 40x40px tile grid per map. Tile `type` field: `1` = buildable plot,
  `>1` = fixed walkable lane (enemies path along a pre-authored corridor
  baked into each map's art — not dynamic A*; confirmed by both the
  `get_directions()`/`restrict_range()` logic in tower classes and the
  pre-carved grass-corridor shape visible in the extracted map art).
- Towers ("dispensers") are placed on buildable plots adjacent to the lane
  and fire in whichever of the 4 cardinal directions touches a lane tile.
  A wall/plot tile in the way shortens the firing range (`restrict_range`).
- Enemy HP = current wave number for most enemies (`health = player.wave`).
  A few are flat (allies / boss). Enemy count per wave is a per-map formula
  (see Maps below).
- Currency ("resources") is granted per kill; e.g. zombie kill = 10.
- Tower cost escalates per-purchase (buying another of the same tower type
  raises that type's price for the next one; sell value = price-before-this-
  purchase minus a fixed delta, i.e. you don't get a full refund).
- Persistent (SharedObject-saved, cross-session) lifetime kill counters per
  tower type unlock permanent tiers 1-5 (thresholds vary, e.g. arrow: 100 /
  250 / 500 / 1000 kills) — confirmed to swap in an upgraded visual at the
  top threshold; likely a stat bonus too at higher tiers (not fully traced).
- Newgrounds API (`com.newgrounds.API`) handles medals/leaderboards. No PSP
  equivalent — drop this system entirely rather than trying to port it.

## Towers ("dispensers") — base stats before any tier bonus
| Type        | Cost  | Power | Range | Firerate | Notes |
|-------------|-------|-------|-------|----------|-------|
| Egg         | 30    | 1     | 1     | 0.5      | cheapest starter |
| Snow        | 100   | 1     | 0.5   | 0.5      | short range, likely slow/freeze effect |
| Arrow       | 150   | 1     | 2.5   | 0.5      | long range basic |
| Fireball    | 200   | 1     | 1     | 0.5      | |
| Slime       | 250   | 1     | 2     | 0.5      | |
| Enderpearl  | 300   | 0.5   | 1     | 0.5      | low power, likely a utility effect (teleport/displace?) |
| Golden(ga)  | 2500  | 5     | 5     | 2.0      | top-tier, big stats across the board |
| TNT-d       | 2500  | —     | 2     | 0.5      | power not in onLoad — likely AoE/explosion-based, not flat power |
| Poison      | 2500  | 8     | 3     | 1.0      | highest raw power of the non-golden set |

Non-attack placeables (traps/utility, separate price table):
Cactus (20, or 10 after 100 cactus kills unlocked), Water trench (25),
Lava trench (200), TNT block (200), Iron bars (200, or 150 after 25 uses
unlocked), Trapdoor (450), Spike pit (2500), Portal (1500), Bone (100).

## Enemies — base speed (tiles/sec-ish, engine-relative units)
Zombie 0.6 · Skeleton 0.6 · Creeper 0.5 · Spider 1.0 · Cave spider 1.1 ·
Silverfish 0.8 · Enderman 4.0 (very fast) · Slime 0.3 · Magma cube 0.3 ·
Ghast 0.4 · Zombie pig 0.7 · Spider jockey 1.0 · Blaze 0.6 ·
Wolf 0.8 (health flat 2 — ally, not a wave enemy) ·
Snow golem 0.8 (health flat 4 — ally) ·
Herobrine 0.6, health flat **750** — fixed boss, does not scale with wave.
All others: `health = current wave number`.
Kill reward observed: 10 currency for a basic zombie (other types not yet
individually confirmed — treat 10 as the baseline and adjust to taste).

## Maps confirmed (symbol names from the decompile)
Adventure/story (numbered `level` 1-13 internally, each with its own
enemies-per-wave formula): forest, deserted beach, village, ravine,
ice peak, abandoned mine (+ exit variant), slime room, spider cavern,
mountain ascent, nether portal, nether stronghold, underground ravine,
"the end". Separate "classic" mode maps: classic island, classic dungeon,
classic nether. Survival variants use `total_waves = 1000` (endless,
HUD shows "Wave N" with no "of X"); story levels show "Wave N of <total>".
Wave 50 is a tracked challenge/achievement threshold in survival modes.

## Systems intentionally out of scope for the port
- Newgrounds medals/leaderboards/API — no equivalent, drop it.
- Flash ad integration (`FlashAd` symbol) — drop it.
- Cutscene/credits timeline sprites — nice-to-have, not core gameplay.

## Assets extracted (see /assets)
- 422 images (PNG), decoded from DefineBitsLossless/2 (paletted, RGB555,
  ARGB-premultiplied) and DefineBits/JPEG2/JPEG3 (incl. reconstructing the
  shared JPEGTables header for old-style DefineBits, and merging the
  separate zlib alpha channel for JPEG3). All 422 decoded without error.
- 80 sound effects/music as MP3 (SWF DefineSound format 2, header stripped
  to leave a directly playable MP3), 5 more in raw PCM. Filenames use the
  SWF's own linkage/export names where one exists (e.g. `silverfishdeath`,
  `explode1`, `boss_music`, `creeper_fuse`).
- Most images only have a generic `char_<id>.png` name — the *sprites*
  (game objects) have the meaningful export names, but their child bitmaps
  usually don't. Cross-referencing which raw bitmap belongs to which named
  sprite would take shape/placement-tag parsing we haven't done yet — worth
  doing later if a specific tower/enemy's exact art is needed.

## Update: real sprite matching (towers done, most enemies not)

The cross-referencing mentioned above turned out to be tractable: walk a
named symbol's `DefineSprite` nested tag stream for `PlaceObject2/3`
`CharacterId` references (recursing through child sprites), then for any
`DefineShape*` reached that way, scan its fill-style bytes for a
`0x40-0x43` (bitmap fill) type byte followed by a 2-byte little-endian ID
that matches a real extracted bitmap. This is a byte-scan heuristic, not a
full SWF shape-record parser (it doesn't walk the bit-packed fill-style
array structurally) — it worked cleanly in practice because false
positives are self-rejecting (the candidate ID has to actually exist in
the bitmap set), but a shape with unusual fill data could in principle
still be missed or mismatched.

**Result: all 9 towers resolved to a single clean, correct 40x40 image**
(e.g. arrow_dispenser#327 → 326 → shapes {323,325} → bitmaps {322,324},
and 324 is visibly a dispenser block with an arrow icon). These are now
in `assets/images/tower_*.png`, replacing the placeholders.

**Most enemies did NOT resolve cleanly.** zombie/skeleton/spider/
cave_spider/silverfish/zombie_pig/blaze/spider_jockey/enderman each
resolve to a single small fragment (e.g. zombie_character#474 → three
28x22 pieces) — these are Minecraft-style mobs built from separately-
animated body parts (head/body/limbs), and the resolver finds one limb,
not a composited whole character. Using the raw fragment would look
broken, not better than a placeholder, so those 9 still use generated
placeholder art. **Five enemies did resolve to one complete, correct
image**: creeper, ghast, magma cube, slime, and Herobrine (his is a
40x40 icon, not an in-game animation frame, but it's a real, clean, boss-
appropriate image) — these now use the real asset too.

Compositing the articulated enemies properly would mean: find the
parent clip's per-child `PlaceObject` MATRIX (position/rotation) for
each limb, render each limb bitmap through its own matrix, and pick (or
render) a representative animation frame — meaningfully more work than
the towers, not attempted this pass.

## Update: engine now covers more of the real roster

Since the first pass (6 enemies, 3 maps, no tiers), the engine added:
- All 14 hostile enemy types from the table above (was 6) — Wolf and
  Snow Golem still excluded (they're player-summonable allies, a
  different mechanic, not implemented).
- 6 more maps (was 3): Ice Peak, Abandoned Mine, Spider Cavern, Deserted
  Beach, Mountain Ascent, Nether Stronghold — same honesty caveat as
  the first 3 (original layouts using the confirmed mechanic, not
  extracted tile-for-tile).
- Herobrine as a one-time boss wave (final wave of Nether Stronghold
  only) — which map/wave gets him is a design choice made for the port,
  not an extracted fact about the original.
- The tier 1-5 system: implemented using arrow's confirmed thresholds
  (100/250/500/1000 kills) applied to every tower type (assumption, see
  Core Loop above), with a stat bonus of +15% power / +10% range per
  tier above 1 — that bonus formula is an original design choice, not
  extracted, since the original's exact tier bonus was never traced.
  Kills are tracked in memory only (reset each run) — persisting them
  needs a save file, not added yet.
- Per-map mood tinting (a translucent color wash over the ground, not a
  per-tile multiply — multiplying an already-green/tan tile texture by
  a tint color can only darken it, never shift its hue, and it was
  flattening the buildable-vs-path color distinction) so the 9 maps
  read as visually distinct despite sharing the same two tile textures.
