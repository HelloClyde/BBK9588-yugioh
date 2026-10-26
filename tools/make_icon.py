"""Original geometric icon; no commercial image or external font is read.

Adapted from the pinned upstream tools/make_open_assets.py, Apache-2.0.
"""
from pathlib import Path
from PIL import Image, ImageDraw

def make_icon(target: Path):
    target.parent.mkdir(parents=True, exist_ok=True)
    icon = Image.new('RGBA', (96, 96), (0, 0, 0, 0))
    draw = ImageDraw.Draw(icon)
    draw.rounded_rectangle((6, 6, 89, 89), radius=19, fill='#17354e', outline='#abc7d3', width=3)
    draw.polygon([(23, 24), (60, 17), (72, 69), (35, 76)], fill='#3c6576', outline='#eef5f5', width=2)
    draw.rounded_rectangle((39, 29, 77, 80), radius=3, fill='#c7a85c', outline='#f8e8ba', width=2)
    draw.polygon([(58, 39), (70, 53), (58, 67), (46, 53)], fill='#26465b')
    icon.save(target, optimize=False)
