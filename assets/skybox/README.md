# Starry sky source and usage

Source: [Deep Star Maps (2012), NASA SVS](https://svs.gsfc.nasa.gov/3895/).

Credit: **NASA/Goddard Space Flight Center Scientific Visualization Studio**.
Visualizations by Ernie Wright and Tom Bridgman. The source star map plots the
Bright Star, Tycho-2 and UCAC3 catalogues. No constellation overlays are used.

The original 4096 x 2048 celestial-coordinate image is bundled unchanged at
`source/starmap_4k.jpg`, downloaded on 2026-11-07 from:

https://svs.gsfc.nasa.gov/vis/a000000/a003800/a003895/starmap_4k.jpg

SHA-256: `0b33503895abebeb92fd89d3332e3ca62515b8cc458d243f208e4ca5d8c1452d`.

Usage follows [NASA Images and Media Usage Guidelines](https://www.nasa.gov/nasa-brand-center/images-and-media/).
NASA media is generally not subject to US copyright; the guidelines permit
educational use including computer graphical simulations with acknowledgment.
This asset is not labelled CC0. NASA does not endorse this project.

## Conversion

`scripts/prepare_star_cubemap.py` converts the panorama to six 1024 x 1024 RGB
PNGs using bilinear spherical reprojection, with horizontal wrapping at the
panorama seam. It needs Python and Pillow only when regenerating assets:

```sh
python scripts/prepare_star_cubemap.py
```

Face order is `right/left/top/bottom/front/back`, corresponding to OpenGL
`+X/-X/+Y/-Y/+Z/-Z`. The output is uploaded without a vertical image flip.
The chosen orientation is artistic, not a date/location-accurate sky model.
No stars, ground, moon or aurora were painted into the source. The shader adds
a blue night atmosphere and attenuates the map near the horizon to match fog;
the lake uses the same treatment for reflections. Aurora remains independent
3D ribbon geometry and is not part of this texture.
