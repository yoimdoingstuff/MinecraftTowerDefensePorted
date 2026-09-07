"""Generates simple original placeholder sprites for enemies/towers
that don't yet have a real matched asset from the extracted SWF.

Status as of this pass: all 9 towers and 5 enemies (creeper, ghast,
magma, slime, herobrine) now use REAL extracted images - resolved by
walking each named sprite's DefineSprite/DefineShape reference chain
to find its actual bitmap fill (see tools/ notes in
docs/original-game-reference.md for how). This script deliberately
does NOT regenerate tower_*.png or those 5 enemy files - only the
enemies below that are still placeholders.

Why the rest are still placeholders: zombie/skeleton/spider/
cave_spider/silverfish/zombie_pig/blaze/spider_jockey/enderman all
resolve to a single LIMB/fragment of a multi-part articulated
character (Minecraft mobs are built from separately-animated body
parts), not one flat sprite - using the raw fragment would look
broken rather than better than a placeholder. Correctly compositing
them needs each part's placement matrix from the parent clip, which
wasn't done this pass.
"""
import os
from PIL import Image, ImageDraw

OUT = os.path.join(os.path.dirname(__file__), '..', 'assets', 'images')
os.makedirs(OUT, exist_ok=True)

REAL_ASSET_FILES = {
    'tile_buildable.png', 'tile_path.png',  # not real, but hand-designed and fine as-is
    'tower_egg.png', 'tower_snow.png', 'tower_arrow.png', 'tower_fireball.png',
    'tower_slime.png', 'tower_enderpearl.png', 'tower_golden.png', 'tower_tnt.png',
    'tower_poison.png', 'enemy_creeper.png', 'enemy_ghast.png', 'enemy_magma.png',
    'enemy_slime.png', 'enemy_herobrine.png',
}

def save(name, img):
    if name in REAL_ASSET_FILES:
        print('skipping (real asset already in place):', name)
        return
    img.save(os.path.join(OUT, name))
    print('wrote', name)

def tile(name, base, edge):
    img = Image.new('RGBA', (40, 40), base)
    d = ImageDraw.Draw(img)
    for i in range(0, 40, 8):
        d.line([(i, 0), (i, 40)], fill=edge, width=1)
        d.line([(0, i), (40, i)], fill=edge, width=1)
    d.rectangle([0, 0, 39, 39], outline=edge, width=1)
    save(name, img)

tile('tile_buildable.png', (86, 138, 74, 255), (70, 115, 60, 255))
tile('tile_path.png', (196, 172, 116, 255), (168, 145, 92, 255))

def tower(name, color, shape):
    img = Image.new('RGBA', (40, 40), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    d.rectangle([2, 2, 37, 37], fill=(60, 60, 60, 255), outline=(20, 20, 20, 255), width=2)
    if shape == 'circle':
        d.ellipse([8, 8, 31, 31], fill=color, outline=(0, 0, 0, 255), width=2)
    elif shape == 'diamond':
        d.polygon([(20, 6), (34, 20), (20, 34), (6, 20)], fill=color, outline=(0, 0, 0, 255))
    elif shape == 'square':
        d.rectangle([9, 9, 30, 30], fill=color, outline=(0, 0, 0, 255), width=2)
    elif shape == 'triangle':
        d.polygon([(20, 6), (34, 32), (6, 32)], fill=color, outline=(0, 0, 0, 255))
    elif shape == 'star':
        import math
        pts = []
        for i in range(10):
            ang = -math.pi / 2 + i * math.pi / 5
            r = 15 if i % 2 == 0 else 7
            pts.append((20 + r * math.cos(ang), 20 + r * math.sin(ang)))
        d.polygon(pts, fill=color, outline=(0, 0, 0, 255))
    save(name, img)

## Towers now all use real extracted assets (see REAL_ASSET_FILES
## above) - placeholder generation calls removed, tower() helper kept
## in case a specific tower ever needs a placeholder again.

def enemy(name, color, shape):
    img = Image.new('RGBA', (32, 32), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    if shape == 'circle':
        d.ellipse([4, 4, 27, 27], fill=color, outline=(0, 0, 0, 255), width=2)
    elif shape == 'square':
        d.rectangle([4, 4, 27, 27], fill=color, outline=(0, 0, 0, 255), width=2)
    elif shape == 'octagon':
        import math
        pts = [(16 + 13 * math.cos(a), 16 + 13 * math.sin(a))
               for a in [i * math.pi / 4 for i in range(8)]]
        d.polygon(pts, fill=color, outline=(0, 0, 0, 255))
    d.ellipse([9, 12, 13, 16], fill=(0, 0, 0, 255))
    d.ellipse([19, 12, 23, 16], fill=(0, 0, 0, 255))
    save(name, img)

enemy('enemy_zombie.png',      (70, 140, 70, 255),   'square')
enemy('enemy_skeleton.png',    (225, 225, 210, 255), 'square')
enemy('enemy_spider.png',      (60, 40, 40, 255),    'octagon')
enemy('enemy_cave_spider.png', (40, 90, 90, 255),    'octagon')
enemy('enemy_silverfish.png',  (150, 150, 160, 255), 'circle')
enemy('enemy_zombie_pig.png',   (190, 110, 110, 255), 'square')
enemy('enemy_blaze.png',        (250, 200, 60, 255),  'circle')
enemy('enemy_spider_jockey.png',(90, 60, 40, 255),    'octagon')
enemy('enemy_enderman.png',     (30, 20, 40, 255),    'square')

# cursor highlight (40x40, transparent center, bright border)
img = Image.new('RGBA', (40, 40), (0, 0, 0, 0))
d = ImageDraw.Draw(img)
d.rectangle([1, 1, 38, 38], outline=(255, 255, 60, 255), width=3)
save('cursor.png', img)

# small projectile dot
img = Image.new('RGBA', (8, 8), (0, 0, 0, 0))
d = ImageDraw.Draw(img)
d.ellipse([0, 0, 7, 7], fill=(255, 240, 120, 255))
save('projectile.png', img)

print('done')
