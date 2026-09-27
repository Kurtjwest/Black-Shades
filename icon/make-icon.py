#!/usr/bin/env python3
"""
Draw the game's icon: the 1024px master next to this script, the Mac's
.icns in macos/, and the Switch's 256px JPEG in switch/.

Original artwork for this port: a dark rounded tile with a pair of stylized
shades and a faint red glow behind them.  Re-run with:

    python3 icon/make-icon.py

Needs Pillow; the generated .icns is checked in, so this is only needed if you
want to change the icon.
"""
import os
import struct

from PIL import Image, ImageDraw, ImageFilter

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
S = 1024  # master size


def rounded_rect(draw, box, radius, fill):
    draw.rounded_rectangle(box, radius=radius, fill=fill)


def render():
    img = Image.new("RGBA", (S, S), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    # background tile with a soft vertical gradient
    tile = Image.new("RGBA", (S, S), (0, 0, 0, 0))
    td = ImageDraw.Draw(tile)
    rounded_rect(td, (0, 0, S - 1, S - 1), int(S * 0.22), (22, 22, 26, 255))
    grad = Image.new("L", (1, S))
    for y in range(S):
        grad.putpixel((0, y), int(255 * (y / S) ** 1.5 * 0.55))
    grad = grad.resize((S, S))
    dark = Image.new("RGBA", (S, S), (0, 0, 0, 255))
    tile = Image.composite(dark, tile, grad).convert("RGBA")
    # keep the rounded mask
    mask = Image.new("L", (S, S), 0)
    ImageDraw.Draw(mask).rounded_rectangle((0, 0, S - 1, S - 1), radius=int(S * 0.22), fill=255)
    tile.putalpha(mask)
    img.alpha_composite(tile)

    # red glow behind the glasses
    glow = Image.new("RGBA", (S, S), (0, 0, 0, 0))
    gd = ImageDraw.Draw(glow)
    gd.ellipse((S * 0.14, S * 0.30, S * 0.86, S * 0.74), fill=(190, 24, 32, 150))
    glow = glow.filter(ImageFilter.GaussianBlur(S * 0.09))
    glow.putalpha(glow.getchannel("A").point(lambda a: int(a * 0.85)))
    img.alpha_composite(Image.composite(glow, Image.new("RGBA", (S, S), (0, 0, 0, 0)), mask))

    # the shades: two lenses and a bridge
    lens_y0, lens_y1 = S * 0.40, S * 0.60
    left = (S * 0.13, lens_y0, S * 0.465, lens_y1)
    right = (S * 0.535, lens_y0, S * 0.87, lens_y1)
    r = int(S * 0.055)

    shadow = Image.new("RGBA", (S, S), (0, 0, 0, 0))
    sd = ImageDraw.Draw(shadow)
    off = S * 0.012
    for box in (left, right):
        sd.rounded_rectangle((box[0] + off, box[1] + off, box[2] + off, box[3] + off),
                             radius=r, fill=(0, 0, 0, 170))
    sd.rectangle((S * 0.44 + off, S * 0.455 + off, S * 0.56 + off, S * 0.50 + off),
                 fill=(0, 0, 0, 170))
    shadow = shadow.filter(ImageFilter.GaussianBlur(S * 0.012))
    img.alpha_composite(shadow)

    for box in (left, right):
        d.rounded_rectangle(box, radius=r, fill=(10, 10, 12, 255))
        d.rounded_rectangle(box, radius=r, outline=(60, 60, 66, 255), width=int(S * 0.008))

    # bridge + temple hints
    d.rectangle((S * 0.44, S * 0.455, S * 0.56, S * 0.495), fill=(10, 10, 12, 255))
    d.rectangle((S * 0.085, S * 0.425, S * 0.14, S * 0.465), fill=(10, 10, 12, 255))
    d.rectangle((S * 0.86, S * 0.425, S * 0.915, S * 0.465), fill=(10, 10, 12, 255))

    # Translucent bits go on their own layer: ImageDraw writes RGBA straight
    # into the target instead of blending, which would punch holes in the tile.
    gloss = Image.new("RGBA", (S, S), (0, 0, 0, 0))
    gd2 = ImageDraw.Draw(gloss)
    for box in (left, right):
        lens = Image.new("L", (S, S), 0)
        ImageDraw.Draw(lens).rounded_rectangle(box, radius=r, fill=255)
        streak = Image.new("RGBA", (S, S), (0, 0, 0, 0))
        ImageDraw.Draw(streak).polygon(
            [(box[0] + S * 0.03, box[3]), (box[0] + S * 0.11, box[1]),
             (box[0] + S * 0.17, box[1]), (box[0] + S * 0.09, box[3])],
            fill=(255, 255, 255, 40))
        streak.putalpha(Image.composite(streak.getchannel("A"),
                                        Image.new("L", (S, S), 0), lens))
        gloss.alpha_composite(streak)

    # a single red glint in the right lens
    gd2.ellipse((S * 0.745, S * 0.437, S * 0.80, S * 0.467), fill=(226, 58, 58, 235))
    img.alpha_composite(gloss)

    return img


def write_icns(img, path):
    """Minimal .icns writer: PNG-backed entries, which macOS has read since 10.7."""
    entries = [
        (b"icp4", 16), (b"icp5", 32), (b"icp6", 64),
        (b"ic07", 128), (b"ic08", 256), (b"ic09", 512), (b"ic10", 1024),
        (b"ic11", 32), (b"ic12", 64), (b"ic13", 256), (b"ic14", 512),
    ]
    import io
    chunks = []
    for ostype, size in entries:
        buf = io.BytesIO()
        img.resize((size, size), Image.LANCZOS).save(buf, format="PNG")
        data = buf.getvalue()
        chunks.append(ostype + struct.pack(">I", len(data) + 8) + data)
    body = b"".join(chunks)
    with open(path, "wb") as f:
        f.write(b"icns" + struct.pack(">I", len(body) + 8) + body)


def write_switch_icon(img, path):
    """The .nro icon hbmenu shows: 256x256, JPEG, no transparency.

    hbmenu draws it as a plain square, so the rounded corners are filled with
    the colour just inside the artwork's own edge rather than left black."""
    icon = img.convert("RGBA").resize((256, 256), Image.LANCZOS)
    edge = icon.getpixel((128, 3))[:3]
    flat = Image.new("RGB", icon.size, edge)
    flat.paste(icon, mask=icon.split()[3])
    flat.save(path, format="JPEG", quality=92, optimize=True, progressive=False)


if __name__ == "__main__":
    icon = render()
    icon.save(os.path.join(HERE, "BlackShades-1024.png"))
    write_icns(icon, os.path.join(ROOT, "macos", "BlackShades.icns"))
    write_switch_icon(icon, os.path.join(ROOT, "switch", "icon.jpg"))
    print("wrote icon/BlackShades-1024.png, macos/BlackShades.icns and switch/icon.jpg")
