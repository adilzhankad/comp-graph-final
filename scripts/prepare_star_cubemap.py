"""Reproject NASA's bundled 4k celestial panorama to six OpenGL cubemap faces.

Development-only dependency: Pillow. Normal builds use the checked-in PNGs.
This is a coordinate conversion, not a procedurally generated star field.
Source and usage information: assets/skybox/README.md.
"""

import argparse
import hashlib
import math
from pathlib import Path

from PIL import Image


SOURCE_SHA256 = "0b33503895abebeb92fd89d3332e3ca62515b8cc458d243f208e4ca5d8c1452d"
FACES = ("right", "left", "top", "bottom", "front", "back")


def direction(face, s, t):
    # Inverse OpenGL cubemap face selection. Image row 0 is uploaded as t=0;
    # do not vertically flip these images in stb_image.
    return ((1, -t, -s), (-1, -t, s), (s, 1, t),
            (s, -1, -t), (s, -t, 1), (-s, -t, -1))[face]


def convert(source, output, size):
    if hashlib.sha256(source.read_bytes()).hexdigest() != SOURCE_SHA256:
        raise ValueError("Source differs from the documented NASA starmap_4k.jpg")
    with Image.open(source) as original:
        image = original.convert("RGB")
    width, height = image.size
    if width != height * 2:
        raise ValueError("Expected a full-sphere 2:1 equirectangular panorama")
    pixels = image.load()
    output.mkdir(parents=True, exist_ok=True)
    for face, name in enumerate(FACES):
        data = bytearray(size * size * 3)
        for row in range(size):
            t = 2 * (row + 0.5) / size - 1
            for col in range(size):
                s = 2 * (col + 0.5) / size - 1
                x, y, z = direction(face, s, t)
                length = math.sqrt(x * x + y * y + z * z)
                # The art orientation places the celestial equator on the
                # horizon. It is not a simulation of a location/date on Earth.
                u = (0.5 + math.atan2(z, x) / (2 * math.pi)) * width - 0.5
                v = (0.5 - math.asin(y / length) / math.pi) * height - 0.5
                ix, iy = math.floor(u), math.floor(v)
                fx, fy = u - ix, v - iy
                p00 = pixels[ix % width, max(0, min(height - 1, iy))]
                p10 = pixels[(ix + 1) % width, max(0, min(height - 1, iy))]
                p01 = pixels[ix % width, max(0, min(height - 1, iy + 1))]
                p11 = pixels[(ix + 1) % width, max(0, min(height - 1, iy + 1))]
                offset = (row * size + col) * 3
                for channel in range(3):
                    upper = p00[channel] * (1 - fx) + p10[channel] * fx
                    lower = p01[channel] * (1 - fx) + p11[channel] * fx
                    data[offset + channel] = round(upper * (1 - fy) + lower * fy)
        target = output / (name + ".png")
        Image.frombytes("RGB", (size, size), bytes(data)).save(target)
        print(target, flush=True)


if __name__ == "__main__":
    root = Path(__file__).resolve().parent.parent
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, default=root / "assets/skybox/source/starmap_4k.jpg")
    parser.add_argument("--output", type=Path, default=root / "assets/skybox/stars")
    parser.add_argument("--size", type=int, default=1024)
    args = parser.parse_args()
    if args.size < 1:
        parser.error("--size must be positive")
    convert(args.source, args.output, args.size)
