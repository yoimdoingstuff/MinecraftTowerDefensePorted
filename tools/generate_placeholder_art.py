#!/usr/bin/env python3
"""Generates simple original placeholder sprites for the starter build.

Why placeholders instead of the 422 images already extracted from the
SWF: those decoded correctly, but most only have a generic char_<id>
name - the *sprites* (game objects) have real export names like
"327_arrow_dispenser", but figuring out which raw bitmap is that
sprite's actual frame means walking its DefineSprite's PlaceObject
chain, which hasn't been done yet (see docs/original-game-reference.md).
Guessing would risk shipping the wrong art under the right name. These
placeholders are clean and functional so the engine is fully playable
now; swap them for real matched assets later without touching any
engine code - only these file contents change.
"""
import os
from PIL import Image, ImageDraw

OUT = os.path.join(os.path.dirname(__file__), '..', 'assets', 'images')
os.makedirs(OUT, exist_ok=True)

def save(name, img):
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

tower('tower_egg.png',        (240, 230, 200, 255), 'circle')
tower('tower_snow.png',       (220, 245, 255, 255), 'diamond')
tower('tower_arrow.png',      (160, 120, 70, 255),  'triangle')
tower('tower_fireball.png',   (230, 90, 30, 255),   'circle')
tower('tower_slime.png',      (80, 200, 90, 255),   'square')
tower('tower_enderpearl.png', (40, 200, 190, 255),  'diamond')
tower('tower_golden.png',     (235, 195, 40, 255),  'star')
tower('tower_tnt.png',        (200, 40, 40, 255),   'square')
tower('tower_poison.png',     (140, 60, 190, 255),  'diamond')

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
enemy('enemy_creeper.png',     (60, 170, 70, 255),   'square')
enemy('enemy_cave_spider.png', (40, 90, 90, 255),    'octagon')
enemy('enemy_silverfish.png',  (150, 150, 160, 255), 'circle')

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
