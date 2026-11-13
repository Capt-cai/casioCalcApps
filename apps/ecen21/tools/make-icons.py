#!/usr/bin/env python3
"""Generate the app's two 92x64 menu icons using repository conventions."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

out = Path(__file__).resolve().parents[1] / "assets-cg"
out.mkdir(exist_ok=True)
for selected in (False, True):
    image = Image.new("RGB", (92, 64), "white")
    draw = ImageDraw.Draw(image)
    if selected:
        for y in range(64):
            draw.line((0, y, 91, y), fill=(110-y//2, 220-y//2, 248))
    draw.rounded_rectangle((5, 5, 86, 43), radius=5, fill=(16, 99, 140))
    draw.line((15, 17, 34, 17), fill="white", width=2)
    draw.line((15, 31, 34, 31), fill="white", width=2)
    draw.pieslice((33, 10, 65, 38), -90, 90, fill="white")
    draw.rectangle((32, 10, 48, 38), fill="white")
    draw.line((65, 24, 77, 24), fill="white", width=2)
    draw.text((46, 53), "ECEN21", font=ImageFont.load_default(), fill="black", anchor="mm")
    image.save(out / ("icon-sel.png" if selected else "icon-uns.png"))
