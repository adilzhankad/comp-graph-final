"""Generate the six night-sky cubemap faces in assets/skybox/.

Everything is computed from a 3D direction, never from a face's own pixel
grid: the background, the Milky Way band and the stars are functions on the
sphere, so neighbouring faces agree along their shared edge and the cube has
no seams. Stars are splatted into every face their footprint touches.

Faces follow OpenGL's cubemap convention for images loaded *without* a
vertical flip (row 0 of the file is t = 0):

    +X: sc = -z, tc = -y      -X: sc = +z, tc = -y
    +Y: sc = +x, tc = +z      -Y: sc = +x, tc = -z
    +Z: sc = +x, tc = -y      -Z: sc = -x, tc = -y

Requires NumPy. Run from the project root:  python scripts/generate_skybox.py
Python is a development convenience; ordinary builds use the committed PNGs.
"""
import struct
import zlib
from pathlib import Path

import numpy as np

SIZE = 1024
SEED = 1107
STAR_COUNT = 9000
OUT = Path(__file__).resolve().parent.parent / "assets" / "skybox"

# Display-space colours (0..1). Kept dark and blue so the aurora, added on
# top later, has room to glow. The horizon colour only matters above the
# haze band in skybox.frag; below it the shader blends to the fog colour.
ZENITH = np.array([0.010, 0.016, 0.036])
HORIZON = np.array([0.040, 0.060, 0.095])
BAND_TINT = np.array([0.060, 0.065, 0.085])
# Milky Way: a great circle tilted against the horizon.
BAND_POLE = np.array([0.42, 0.55, -0.72])

# name, major axis, axis that s grows along, axis that t grows along.
FACES = (
    ("px", np.array([1, 0, 0]), np.array([0, 0, -1]), np.array([0, -1, 0])),
    ("nx", np.array([-1, 0, 0]), np.array([0, 0, 1]), np.array([0, -1, 0])),
    ("py", np.array([0, 1, 0]), np.array([1, 0, 0]), np.array([0, 0, 1])),
    ("ny", np.array([0, -1, 0]), np.array([1, 0, 0]), np.array([0, 0, -1])),
    ("pz", np.array([0, 0, 1]), np.array([1, 0, 0]), np.array([0, -1, 0])),
    ("nz", np.array([0, 0, -1]), np.array([-1, 0, 0]), np.array([0, -1, 0])),
)


def face_directions(major, s_axis, t_axis):
    """Unit direction through the centre of every pixel of one face."""
    coords = (np.arange(SIZE) + 0.5) / SIZE * 2.0 - 1.0
    tc, sc = np.meshgrid(coords, coords, indexing="ij")  # rows are t
    d = major[None, None, :] + sc[..., None] * s_axis + tc[..., None] * t_axis
    return d / np.linalg.norm(d, axis=-1, keepdims=True)


def hash3(cell):
    """Deterministic pseudo-random value in [0, 1) per integer lattice cell."""
    h = (cell[..., 0] * 73856093) ^ (cell[..., 1] * 19349663) ^ (cell[..., 2] * 83492791)
    h = (h ^ (h >> 13)) * 1274126177
    return ((h ^ (h >> 16)) & 0xFFFF) / 65536.0


def value_noise(p):
    """Trilinear value noise with smoothstep weights, evaluated on 3D points."""
    base = np.floor(p).astype(np.int64)
    f = p - base
    w = f * f * (3.0 - 2.0 * f)
    total = 0.0
    for dx in (0, 1):
        for dy in (0, 1):
            for dz in (0, 1):
                corner = hash3(base + np.array([dx, dy, dz]))
                weight = (np.where(dx, w[..., 0], 1 - w[..., 0]) *
                          np.where(dy, w[..., 1], 1 - w[..., 1]) *
                          np.where(dz, w[..., 2], 1 - w[..., 2]))
                total = total + corner * weight
    return total


def fbm(p, octaves=5):
    total, amplitude = 0.0, 0.5
    for _ in range(octaves):
        total = total + amplitude * value_noise(p)
        p = p * 2.03
        amplitude *= 0.5
    return total


def background(d):
    # Smooth on both sides of the horizon: a kink at y = 0 shows as a line.
    x = np.clip((d[..., 1] + 0.15) / 0.85, 0.0, 1.0)
    up = x * x * (3.0 - 2.0 * x)
    sky = HORIZON + (ZENITH - HORIZON) * up[..., None]
    pole = BAND_POLE / np.linalg.norm(BAND_POLE)
    across = d @ pole                               # 0 on the band's centre line
    band = np.exp(-(across / 0.17) ** 2)
    clouds = fbm(d * 3.0 + 11.0)
    dust = fbm(d * 9.0 - 4.0)                       # dark lanes break the band up
    glow = band * np.clip(clouds * 1.6 - 0.45, 0.0, 1.0) * (0.55 + 0.45 * dust)
    return sky + BAND_TINT * glow[..., None], band


def make_stars(rng):
    d = rng.normal(size=(STAR_COUNT, 3))
    d /= np.linalg.norm(d, axis=1, keepdims=True)
    # Many faint stars, few bright ones.
    brightness = 0.10 + 0.90 * rng.random(STAR_COUNT) ** 6.0
    # Slight colour variety: blue-white to warm white.
    warmth = rng.random(STAR_COUNT)
    colour = np.stack([0.80 + 0.20 * warmth, 0.86 + 0.06 * warmth, 1.0 - 0.18 * warmth], 1)
    # Gaussian footprint in pixels; bright stars are a little larger.
    radius = 0.55 + 0.9 * brightness
    return d, brightness, colour, radius


def splat_stars(image, major, s_axis, t_axis, stars):
    d, brightness, colour, radius = stars
    ma = d @ major
    in_front = ma > 0.5                             # far enough to project safely
    s = (d[in_front] @ s_axis) / ma[in_front]
    t = (d[in_front] @ t_axis) / ma[in_front]
    px = (s + 1.0) * 0.5 * SIZE - 0.5
    py = (t + 1.0) * 0.5 * SIZE - 0.5
    for x, y, b, c, r in zip(px, py, brightness[in_front], colour[in_front], radius[in_front]):
        reach = int(np.ceil(r * 3))
        x0, x1 = max(int(x) - reach, 0), min(int(x) + reach + 2, SIZE)
        y0, y1 = max(int(y) - reach, 0), min(int(y) + reach + 2, SIZE)
        if x0 >= x1 or y0 >= y1:
            continue                                # footprint lies on another face
        gx = np.arange(x0, x1) - x
        gy = np.arange(y0, y1) - y
        weight = np.exp(-(gy[:, None] ** 2 + gx[None, :] ** 2) / (2 * r * r))
        image[y0:y1, x0:x1] += b * weight[..., None] * c


def write_png(path, rgb):
    data = np.clip(np.round(rgb * 255.0), 0, 255).astype(np.uint8)
    raw = b"".join(b"\x00" + row.tobytes() for row in data)

    def chunk(tag, body):
        return (struct.pack(">I", len(body)) + tag + body +
                struct.pack(">I", zlib.crc32(tag + body) & 0xFFFFFFFF))

    header = struct.pack(">IIBBBBB", SIZE, SIZE, 8, 2, 0, 0, 0)
    path.write_bytes(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", header) +
                     chunk(b"IDAT", zlib.compress(raw, 9)) + chunk(b"IEND", b""))


def main():
    rng = np.random.default_rng(SEED)
    stars = make_stars(rng)
    # A second, denser layer of very faint stars only inside the Milky Way.
    extra = make_stars(rng)
    pole = BAND_POLE / np.linalg.norm(BAND_POLE)
    keep = np.abs(extra[0] @ pole) < 0.12
    faint = (extra[0][keep], extra[1][keep] * 0.35, extra[2][keep], extra[3][keep] * 0.7)
    OUT.mkdir(parents=True, exist_ok=True)
    for name, major, s_axis, t_axis in FACES:
        d = face_directions(major, s_axis, t_axis)
        image, _ = background(d)
        splat_stars(image, major, s_axis, t_axis, stars)
        splat_stars(image, major, s_axis, t_axis, faint)
        write_png(OUT / f"{name}.png", image)
        print(f"wrote {OUT / (name + '.png')}")


if __name__ == "__main__":
    main()
