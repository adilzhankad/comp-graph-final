# Terrain, lake and starry-sky validation

Updated 2026-11-07. The current scene renders the low-poly terrain, lake and starry skybox with matching blue night haze. Aurora remains separate deferred 3D ribbon geometry; it is not part of the sky texture or active scene. Prop modules are also deferred.

## Finer low-poly terrain — 2026-11-07

Terrain sampling increased from 128x128 to 256x256 cells: cell width is now
2.1875 instead of 4.375 world units, with 131,072 triangles instead of 32,768.
Split face normals, flat material interpolation, proportional vertex jitter
and mixed diagonals retain the low-poly style. The source heightmap is unchanged.
The lake retains its independent 128x128 sampling grid and 780 triangles.

Release and Debug previews built with MSVC 19.44 and passed the four-frame
OpenGL smoke test on Intel Iris Xe; both CPU suites passed (2/2 each).
The capture harness passed pause, upper-sky and
render-state checks. Default, near-water and downward captures were reviewed:
the mountain silhouette and facets are finer, and no shoreline coverage gaps
were observed. `preview.png` records the new terrain at scene time 3 seconds.

At 1280x720, 15 warmup plus 120 synchronized frames measured mean CPU+GPU
serial frame latency of 6.140 ms, median 5.950 ms and p95 7.999 ms; a preceding
128-cell run measured mean 4.723 ms. These hidden-window `glFinish` measurements
are indicative cost evidence on this machine, not an interactive FPS benchmark.
Historical counts and timings below refer to the earlier geometry.

## Water palette and reflection tuning — 2026-11-07

Softened broad waves to 80% and fine waves toward 45% of their preceding
amplitudes, reduced the crest harmonic, and moved the body/highlight palette
toward muted blue-grey. Sky reflection now uses an artistic `0.20 + 0.80*F`
blend weight; opacity varies from 0.82 to 0.98. The 16-wave motion is retained.

The hidden-context harness passed time variation, exact pause, unchanged upper
sky and render-state checks with no OpenGL errors. Default, low near-water and
downward captures were reviewed. Release-context validation and the Debug
application's smoke test passed. `preview.png` records this tuning at time 3.
The reflection contains the sky only; planar terrain reflection remains planned.

## Irregular wind ripples — 2026-11-07

The lake fragment shader now combines 16 directional gravity-wave components,
advected wind-strength patches and wave groups, gently bent crests and a mild
second harmonic. Analytic gradients drive the normals; screen derivatives fade
subpixel waves. The 780-triangle surface and shoreline clipping are unchanged.

| Check | Result |
|---|---|
| Shader compatibility | Release and Debug applications compiled/linked the GLSL 330 shader and passed hidden-window smoke checks on Intel Iris Xe, OpenGL 3.3 / driver 32.0.101.6913 |
| Temporal behaviour | Captured scene times 0, 1, 3 and 8 seconds; lower-frame water changed while the upper sky remained byte-identical |
| Pause | Rendering time 3 twice produced byte-identical frames |
| Render state | No OpenGL errors; depth/blend/polygon state restored; filled frame after wireframe matched the original |
| Views | Default, low near-water and downward captures checked; existing shoreline occlusion retained |
| Preview | `preview.png` updated to the new ripples at scene time 3 seconds |

A temporary harness measured the entire scene in a hidden 1280x720 window with
15 warmup frames and 120 measured frames, synchronizing each frame with
`glFinish`. Final candidate CPU+GPU serial latency was mean 4.344 ms, median
4.312 ms and p95 5.123 ms. A preceding baseline run measured mean 2.876 ms;
separate runs are affected by GPU clocks and system load, so this is indicative
cost evidence, not an interactive FPS benchmark or an isolated shader timing.
No mesh displacement, fluid dynamics or shoreline-wave simulation is claimed.

## Starry sky and night environment — 2026-11-07

The following checks used a clean isolated source copy of the new skybox implementation on Windows x64.

| Check | Result |
|---|---|
| Release and Debug builds | Both clean builds passed with MSVC 19.44 |
| CPU tests | Camera/hierarchy and heightmap suites passed in both configurations (2/2 each) |
| Graphics smoke test | Both configurations rendered four hidden-window frames successfully; shaders compiled/linked and no OpenGL errors were reported |
| GPU | Intel Iris Xe Graphics, OpenGL 3.3.0 / driver 32.0.101.6913 |
| Additional views | Temporary harness rendered default, upward, opposite, translated, wireframe and filled-after-wireframe views |
| Visual review | Default, upward, opposite and translated captures reviewed; no obvious cubemap seams observed |
| Render state | Depth, blending and polygon state restored; filled frame after wireframe was byte-identical to the initial filled frame |
| Sky translation | Upper-sky pixels changed by at most 1/255 per RGB channel under camera translation, consistent with floating-point view-matrix precision |
| Resource failure | Missing-face error reported correctly; partial-resource cleanup check passed |

`preview.png` is the new 1280×720 night-scene capture. The lake samples the same star cubemap as the sky, with matching night tint and horizon haze. This is distant-sky reflection, not the planned planar reflection of terrain, props or animated aurora.

No FPS benchmark, interactive keyboard/mouse validation or cross-platform runtime result is claimed for this pass. The temporary harness checks render behaviour directly; it does not verify input dispatch. Team interaction review, performance measurements, fresh-clone validation and GitHub CI results remain outstanding.

## Terrain refinement — 2026-10-31

Seven connected ridge chains and twelve descending spurs replaced the isolated peak envelopes. The starting skyline gained elevated saddles and shared flanks. The regenerated heightmap passed the existing heightmap test executable, and a visible-window OpenGL capture was checked visually and passed the application's error checks. C++ and shader source did not change in that pass; the build results below record the preceding code baseline.

| Check | Result |
|---|---|
| Windows x64 Release and Debug | Both builds passed with MSVC 19.44 |
| CPU tests | Camera/hierarchy and heightmap suites passed in both configurations (2/2 each) |
| Graphics check | Visible-window capture completed; terrain shaders compiled/linked and the OpenGL error check passed |
| GPU | Intel Iris Xe Graphics, OpenGL 3.3.0 / driver32.0.101.6913 |
| World | 560x560 units; 513x513 image samples; 128x128 cells / 32,768 triangles |
| Low-poly geometry | Split triangle vertices with flat geometric normals; jittered shared positions and mixed diagonals |
| Fog | Smooth distance fade from95 to230 units; boundary fade from205 to260 on each axis before the mesh ends at280 |

The capture at this stage showed fully opaque fog matching the background before the outer boundary, including when approaching it. Face lighting remained flat while fog interpolated smoothly. The preview image has since been replaced by the night-scene capture above.

No performance benchmark or cross-platform runtime result is claimed. Interactive exploration and GitHub CI remain team review tasks. Python was used only to regenerate the heightmap and convert the preview; ordinary builds still need no Python or downloads.

## Lake pass

Run on a second machine after the terrain revision above, so the two sections describe different hardware.

| Check | Result |
|---|---|
| Windows x64 Release and Debug | Both builds passed with MSVC 19.51, CMake 4.4.3, no warnings |
| CPU tests | Camera/hierarchy and heightmap suites passed in both configurations (2/2 each) |
| Graphics check | Hidden-window smoke test passed in both configurations: shaders compiled and linked, no OpenGL errors, frame not uniform |
| GPU | NVIDIA GeForce RTX 3050, OpenGL 3.3.0 / driver 610.88, Windows 11 |
| Heightmap | 513x513 samples, decoded range -8.212 to 26.659 inside the [-10, 28] encoding; 1.59% of samples below the water level |
| Lake surface | 780 triangles kept out of the 32,768-triangle grid; basin centre 7.2 units under the water level |
| Shoreline | Visual check from the bank and from above: the water is clipped to the terrain with no mesh-edge stair-stepping and no gap at the shore |
| Highlight | Blinn-Phong streak confirmed by rotating the light with `[` / `]` until it faces the camera |

A lake capture was produced with `--smoke-test --capture` for this pass; the preview image has since been replaced by the night-scene capture above. The heightmap test covers the flooded area, the bed depth, the banks and the southern bank profile, so a regenerated heightmap that loses the basin fails the suite rather than silently draining the lake.

No performance benchmark is claimed for this pass either: the smoke test renders four frames and does not measure frame time. The FPS table in the README still needs a real measurement at 1280x720 on each team machine. Linux and macOS runtime, GitHub CI results and interactive exploration remain team review tasks.
