"""Generate connected, angular mountain ranges (Python standard library only).

The committed 513-square PGM covers 560 world units and encodes heights from
-4 to 28. The renderer supplies the coarse triangles and flat face normals.
Python is needed only when deliberately regenerating this source asset.
"""
from math import sqrt
from pathlib import Path

WORLD_SIZE = 560.0
IMAGE_SIZE = 513

# A crest point is (x, z, height, left flank width, right flank width).
# Connected points describe one mountain mass: saddle heights stay well above
# the valley, while unequal widths give its two sides different slopes.
# The first chain supplies the skyline seen from the starting camera.
MAIN_RIDGE = (
    (-118, -19, 3.5, 22, 29), (-96, -35, 12.4, 30, 37),
    (-77, -42, 21.7, 34, 45), (-62, -49, 18.3, 36, 47),
    (-48, -44, 24.0, 32, 42), (-31, -57, 27.0, 37, 49),
    (-21, -62, 20.8, 35, 46), (-9, -67, 23.1, 32, 43),
    (3, -66, 14.5, 32, 42), (16, -58, 26.8, 36, 48),
    (28, -60, 20.0, 34, 43), (42, -56, 23.5, 37, 46),
    (57, -49, 16.2, 34, 42), (72, -43, 21.9, 34, 45),
    (91, -30, 10.2, 29, 38), (116, -12, 3.0, 22, 28),
)

# Lateral ridges bend away from the valley instead of forming a circular wall.
WEST_RIDGE = (
    (-77, -42, 21.7, 25, 32), (-67, -23, 17.1, 25, 35),
    (-71, -6, 22.0, 28, 37), (-65, 11, 15.4, 28, 35),
    (-78, 32, 19.1, 32, 38), (-92, 47, 11.2, 29, 35),
    (-107, 72, 4.0, 25, 29),
)
EAST_RIDGE = (
    (72, -43, 21.9, 30, 25), (80, -21, 17.3, 34, 26),
    (70, -3, 22.7, 36, 29), (81, 17, 14.5, 33, 27),
    (96, 34, 18.5, 36, 30), (105, 59, 10.2, 31, 27),
    (119, 79, 3.5, 25, 22),
)

# Long, offset ranges make several depth layers rather than a repeated ring
# of cones. Their lower passes reveal the chain behind them as the camera moves.
BACK_RIDGE = (
    (-246, -127, 3.5, 29, 32), (-220, -145, 12.3, 36, 42),
    (-194, -153, 21.2, 40, 47), (-174, -140, 17.0, 38, 45),
    (-151, -153, 26.0, 42, 51), (-129, -160, 19.3, 40, 48),
    (-105, -148, 24.7, 42, 50), (-83, -138, 16.3, 38, 46),
    (-60, -146, 25.6, 42, 51), (-37, -157, 18.5, 39, 47),
    (-12, -147, 23.2, 42, 49), (9, -159, 17.2, 39, 47),
    (31, -151, 26.1, 43, 51), (54, -139, 20.0, 41, 48),
    (78, -145, 24.0, 40, 48), (100, -128, 15.2, 37, 45),
    (123, -109, 21.8, 39, 47), (145, -94, 12.4, 33, 42),
    (163, -72, 3.7, 26, 32),
)
EASTERN_RANGE = (
    (209, -218, 3.0, 27, 30), (194, -191, 15.4, 38, 43),
    (209, -164, 24.6, 44, 49), (195, -137, 17.8, 42, 48),
    (207, -113, 25.7, 43, 52), (193, -87, 19.2, 42, 49),
    (205, -61, 23.6, 44, 50), (186, -35, 15.8, 39, 47),
    (194, -11, 25.5, 42, 51), (178, 18, 19.0, 42, 48),
    (185, 42, 23.2, 43, 49), (166, 68, 16.7, 39, 45),
    (150, 91, 20.4, 40, 47), (130, 111, 9.1, 33, 39),
    (111, 133, 3.2, 25, 31),
)
WESTERN_RANGE = (
    (-224, -110, 4.0, 27, 30), (-209, -82, 18.3, 39, 45),
    (-218, -57, 24.7, 42, 49), (-202, -31, 17.7, 40, 47),
    (-211, -6, 25.9, 43, 50), (-193, 21, 18.0, 39, 46),
    (-201, 48, 23.1, 42, 49), (-182, 70, 16.7, 39, 45),
    (-165, 91, 24.2, 41, 48), (-144, 112, 13.0, 35, 41),
    (-119, 122, 3.5, 26, 31),
)
SOUTHERN_RANGE = (
    (-218, 171, 3.0, 26, 31), (-189, 183, 15.5, 38, 43),
    (-164, 174, 22.3, 43, 48), (-141, 191, 16.8, 40, 45),
    (-116, 182, 25.7, 44, 51), (-91, 198, 19.2, 42, 48),
    (-64, 189, 23.7, 42, 49), (-38, 205, 14.6, 38, 45),
    (-12, 194, 22.9, 41, 48), (16, 205, 17.3, 40, 47),
    (42, 192, 25.3, 43, 50), (68, 201, 18.4, 41, 47),
    (95, 183, 23.5, 42, 49), (119, 176, 15.9, 38, 44),
    (147, 157, 20.1, 40, 46), (175, 149, 7.0, 30, 36),
    (199, 132, 3.0, 24, 29),
)

# Buttresses leave the crests and descend into the shared foothills. These
# create lateral folds and gullies, instead of independent mountain bases.
SPURS = (
    ((-48, -44, 24.0, 16, 20), (-43, -25, 16.7, 15, 20),
     (-49, -6, 8.7, 14, 18), (-41, 13, 1.1, 11, 14)),
    ((-21, -62, 20.8, 14, 18), (-20, -42, 13.5, 14, 17),
     (-29, -24, 5.5, 12, 15), (-25, -9, -.1, 9, 12)),
    ((16, -58, 26.8, 17, 21), (23, -37, 17.0, 16, 20),
     (20, -19, 7.6, 13, 17), (29, -2, .5, 10, 13)),
    ((42, -56, 23.5, 17, 20), (48, -32, 14.7, 16, 20),
     (42, -12, 5.7, 13, 16), (49, 8, .8, 10, 13)),
    ((-71, -6, 22.0, 16, 20), (-53, 9, 11.5, 15, 19),
     (-43, 26, 3.2, 12, 15)),
    ((70, -3, 22.7, 18, 21), (54, 15, 12.2, 16, 19),
     (49, 33, 3.0, 12, 15)),
    ((-151, -153, 26.0, 20, 24), (-140, -129, 17.4, 20, 24),
     (-148, -104, 8.5, 17, 22), (-133, -84, 3.0, 14, 18)),
    ((-60, -146, 25.6, 20, 25), (-71, -123, 17.1, 20, 24),
     (-64, -101, 7.2, 17, 21), (-73, -82, 3.1, 14, 18)),
    ((31, -151, 26.1, 20, 26), (22, -128, 17.2, 19, 24),
     (32, -108, 7.3, 16, 21), (23, -88, 3.2, 13, 18)),
    ((194, -11, 25.5, 20, 25), (168, -2, 17.6, 20, 24),
     (149, -12, 8.2, 17, 21), (127, -3, 3.2, 13, 18)),
    ((-211, -6, 25.9, 22, 26), (-188, 6, 18.1, 20, 24),
     (-169, -3, 8.1, 17, 22), (-147, 7, 3.3, 14, 18)),
    ((42, 192, 25.3, 21, 26), (32, 168, 17.6, 20, 24),
     (41, 147, 8.0, 17, 21), (31, 125, 3.1, 13, 18)),
)
RIDGES = (MAIN_RIDGE, WEST_RIDGE, EAST_RIDGE, BACK_RIDGE,
          EASTERN_RANGE, WESTERN_RANGE, SOUTHERN_RANGE) + SPURS


def prepare_segments() -> tuple:
    """Cache bounds and projection factors; reject distant segments cheaply."""
    segments = []
    for ridge in RIDGES:
        for a, b in zip(ridge, ridge[1:]):
            x, z, height, left, right = a
            bx, bz, bh, bl, br = b
            dx, dz = bx - x, bz - z
            radius = max(left, right, bl, br)
            segments.append((min(x, bx) - radius, max(x, bx) + radius,
                             min(z, bz) - radius, max(z, bz) + radius,
                             x, z, dx, dz, 1.0 / (dx * dx + dz * dz),
                             height, bh - height, left, bl - left,
                             right, br - right))
    return tuple(segments)


SEGMENTS = prepare_segments()


def triangle_wave(value: float) -> float:
    return abs(value % 2.0 - 1.0)


def height_at(x: float, z: float) -> float:
    # Keep the low central valley open for the later lake stage.
    height = (-2.7 + min(abs(x), 70) * .045 + min(abs(z), 70) * .030
              + .38 * triangle_wave(x / 13.0 + z / 19.0)
              + .24 * triangle_wave(z / 11.0 - x / 23.0))
    for segment in SEGMENTS:
        min_x, max_x, min_z, max_z = segment[:4]
        if x < min_x or x > max_x or z < min_z or z > max_z:
            continue
        _, _, _, _, ax, az, dx, dz, inv_length2, h, dh, wl, dwl, wr, dwr = segment
        rx, rz = x - ax, z - az
        t = max(0.0, min(1.0, (rx * dx + rz * dz) * inv_length2))
        # Unequal flank widths avoid symmetrical piles. A gently concave
        # cross-section gives steep upper slopes and broad shared foothills.
        width = wl + dwl * t if dx * rz - dz * rx >= 0 else wr + dwr * t
        distance2 = (rx - t * dx) ** 2 + (rz - t * dz) ** 2
        if distance2 >= width * width:
            continue
        distance = sqrt(distance2) / width
        profile = (1.0 - distance) * (1.0 - .25 * distance)
        candidate = -2.5 + (h + dh * t + 2.5) * profile
        height = max(height, candidate)
    return height


def main() -> None:
    pixels = bytearray()
    for z in range(IMAGE_SIZE):
        world_z = (z / (IMAGE_SIZE - 1) - .5) * WORLD_SIZE
        for x in range(IMAGE_SIZE):
            world_x = (x / (IMAGE_SIZE - 1) - .5) * WORLD_SIZE
            height = height_at(world_x, world_z)
            if not -4.0 <= height < 28.0:
                raise ValueError(f'Height outside image encoding range at {world_x}, {world_z}')
            pixels.append(round((height + 4.0) / 32.0 * 255))
    destination = Path(__file__).resolve().parents[1] / 'assets/heightmaps/mountain_lake.pgm'
    destination.parent.mkdir(parents=True, exist_ok=True)
    header = f'P5\n{IMAGE_SIZE} {IMAGE_SIZE}\n255\n'.encode('ascii')
    destination.write_bytes(header + pixels)
    print(f'Generated {destination} ({IMAGE_SIZE} x {IMAGE_SIZE}, {len(SEGMENTS)} ridge segments)')
    print(f'Decoded range: {min(pixels) / 255 * 32 - 4:.3f} to {max(pixels) / 255 * 32 - 4:.3f}')
    print(f'Central valley: {height_at(0, 0):.3f}; starting-camera ground: {height_at(0, 36):.3f}')


if __name__ == '__main__':
    main()
