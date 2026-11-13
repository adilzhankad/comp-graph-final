# Mountain-lake scene validation

Updated 2026-11-14. The combined scene includes low-poly terrain, a starry sky, three animated aurora curtains, instanced pine trees, a sweeping lighthouse beacon and a plane with blinking lights, with planar lake reflections. The first section records validation of the combined scene. Later dated sections preserve earlier per-feature evidence.

## Combined scene, Bézier curtains and palettes — 2026-11-14

Merged `origin/main` at `a447bd0` into `feat/aurora`, retaining the forest, lighthouse and plane. The aurora now uses three joined cubic Bézier segments per curtain, sampled into 480 × 24 cells per sheet (69,120 triangles across three sheets). The primary horizontal span is 508 units, with nominal height 138; vertical variation and animated displacement modify the final mesh. Green, blue and orange palettes share the same geometry and reflected rendering.

| Check | Result |
|---|---|
| Windows x64 builds | Release and Debug passed with MSVC 19.44; no compiler warnings reported |
| CPU tests | 3/3 passed in both configurations: camera/hierarchy/beacon/Bézier, heightmap and forest placement |
| Bézier coverage | Endpoint and known curved interior samples, out-of-plane handle influence and tangent continuity across joined segments passed |
| Application graphics smoke | Release passed for all three palettes; Debug passed with orange. Four hidden-window frames per run, shader compilation/linking and OpenGL error checks passed |
| GPU | Intel Iris Xe Graphics, OpenGL 3.3.0, driver 32.0.101.6913 |
| Palette rendering | Temporary Release harness confirmed changes in both the upper sky and lower lake; blue emission was blue-dominant and orange emission red-dominant |
| Effect isolation | Disabled aurora and zero intensity matched exactly; palette changes had no effect while aurora was off. With planar reflections disabled, palette changes left the lower lake unchanged |
| Animation and state | Changing only aurora time changed sky and lake; fixed-time frames were deterministic. Filled render after wireframe matched the original; framebuffer, viewport, depth, blend, clipping, polygon and active texture state checks passed |
| Input handlers | The application's registered key callback passed `C` Green → Blue → Orange → Green, repeat/release filtering, and `U`, `R`, `T` toggles |
| Visual review | Green, blue and orange 1280 × 720 application captures reviewed with the forest, lighthouse and plane present; enlarged curtains and corresponding coloured lake reflections are visible |

`preview.png` is the green Release application capture from `--smoke-test --aurora-color green --capture`, converted losslessly from PPM to PNG. The temporary harness lives under the ignored build directory and invokes registered callbacks directly; physical keyboard/mouse delivery was not tested. No FPS benchmark, cross-platform run or GitHub CI result is claimed for this pass.

## Soft aurora curtains and flowing colour — 2026-11-11

Three asymmetric 3D curtains use 320 columns and 20 rows each (38,400 triangles
in total). Vertex deformation moves their folds; the fragment shader adds a
continuous veil, filtered vertical rays, soft lower glow and fading violet/rose
tips over green and teal. Colour waves travel independently across the curtains.
Grazing-angle attenuation softens their silhouettes from both viewing cameras.
The sky and the lake's cubemap fallback share a quieter star treatment. The
starting view is tilted upward by five degrees to frame the taller curtains.

| Check | Result |
|---|---|
| Windows x64 builds | Clean staged Release and Debug builds passed with MSVC 19.44 |
| Existing CPU tests | Camera/hierarchy and heightmap tests passed, 2/2 in each configuration |
| Application graphics smoke tests | Four hidden-window frames passed in Release and Debug; GLSL 330 compiled/linked, no OpenGL errors |
| GPU | Intel Iris Xe Graphics, OpenGL 3.3 / driver 32.0.101.6913 |
| Visible contribution | At time 0, toggling aurora changed 112,228 upper-sky pixels and 55,336 lower-frame lake pixels |
| Animation and colour | Captures at times 0, 8, 20 and 40 changed in both sky and water; normalized emission RGB fractions also changed |
| Reflected animation | Lake pixels changed when only the aurora clock advanced and the water clock stayed fixed |
| Pause and intensity | Repeated frozen-time frames were byte-identical; zero intensity matched aurora disabled |
| Render state | Framebuffer, viewport, depth, blend, clip, polygon and active-texture state checks passed; filled frames matched after wireframe and reflection toggles |
| Views | Default, upward, low, near-water, translated, opposite and above-curtain views captured; default, upward, low, translated and above-curtain views visually reviewed |
| High brightness | 200% intensity rendered correctly; some bright green/teal regions saturate in the existing LDR framebuffer |

The standalone hidden-context harness passed in both configurations. Release
serial CPU+GPU timing with `glFinish` (10 warmup frames, 40 measured frames) had
a median of 6.01 ms with aurora and 3.22 ms without. Means were 10.11 and 4.17 ms;
p95 was 43.98 and 7.18 ms, showing substantial scheduling/system-load outliers.
This short diagnostic is not an interactive FPS benchmark or an isolated GPU
shader cost measurement.

`preview.png` records the actual application render at scene time 20. `U`
toggles aurora, arrows adjust brightness/speed, and `T` pauses both clocks.
The aurora clock accumulates `deltaTime * speed` so speed adjustments do not
jump animation phase. The harness verifies rendering at fixed settings; live
keyboard/mouse dispatch, cross-platform runtime and team-machine FPS remain
interactive review tasks. Aurora is emissive geometry reflected in the lake;
it does not yet light terrain materials or use a bloom postprocess.

## Planar mountain and shore reflections — 2026-11-07

The lake now samples a separate terrain/sky pass rendered from a camera mirrored
across `y = -1`. Underwater terrain is clipped; the lake is excluded from its own
reflection. Ripple-driven projective UVs distort the capture, with smaller
offsets at grazing angles and texture edges. Reflected terrain already contains
distance fog, so that fog is not applied a second time by the water shader.
`R` toggles between this pass and the preceding sky-only reflection.

Release and Debug application builds and four-frame graphics smoke tests passed
on Intel Iris Xe, OpenGL 3.3 / driver 32.0.101.6913. The temporary OpenGL harness
checked the following without graphics errors:

- RGBA8/depth24 allocation, linear/clamped sampling, unchanged-size reuse,
  zero-size preservation, rejected over-limit resize preserving existing pixels,
  and caller framebuffer/texture/renderbuffer/pixel-unpack bindings.
- Reflections on/off from default, low, translated, 15-degree FOV and near-water
  cameras. The default comparison changed 290,919 water pixels; upper sky and
  terrain remained identical in the default and low views.
- Underwater fallback produced identical on/off frames. Repeated fixed-time
  captures were byte-identical; wireframe left the next filled frame unchanged.
- Resizing from 1280x720 to a 797x449 viewport at offset (7, 9), then 1600x900,
  with separate caller read/draw framebuffers. Framebuffer, viewport, clear colour,
  clip, depth, blend, polygon and active texture state checks passed.

Default, low and translated captures were reviewed for mirrored mountain shapes
and shoreline alignment. `preview.png` shows the default view at time 3 seconds.
The reflection target scales with the viewport within 1280x720. This pass adds
a terrain draw; no new interactive FPS benchmark is claimed. The mesh remains
flat and the waves affect shading and reflection coordinates. Future objects
and aurora must be added to the capture when they enter the active scene.

## Slightly taller mountains and rollback points — 2026-11-07

`Terrain` now builds a shared adjusted height field before mesh generation.
The source asset and its decoding remain unchanged; a smooth uplift above
height 5 raises the highest field sample from 26.6588 to 32.0735 (about 20%).
The 256x256-cell low-poly mesh and flat face normals are retained.

The temporary OpenGL harness verified 151,199 low samples remain exactly
unchanged, the submerged count stays 4,203, and 52,095 interpolated queries
near the shoreline match the original field exactly. Water still has 780
triangles. CPU terrain queries see the raised peaks. Default, low and downward
captures were reviewed; no shoreline gaps or loss of the low-poly style were
observed. Render state, paused frames and upper-sky invariance checks passed.
Release and Debug previews passed their graphics smoke tests on Intel Iris Xe.

Local checkpoints were saved before applying the uplift:
`backup/terrain-original` at `f201ac3` preserves the original 128-cell mountains;
`backup/terrain-before-height-increase` at `a26d505` preserves the denser mesh
at its preceding height. The latter is also the current branch's committed
checkpoint before this height change. `preview.png` shows the raised version.

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
At this earlier stage the reflection contained the sky only; the later planar
reflection pass is recorded above.

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

## Skybox and textures (task 02)

Branch `feat/skybox-textures`. `Texture`, `Skybox`, `shaders/skybox.*` and the generated cubemap were written here; nothing below is an application run.

| Check | Result |
|---|---|
| Strict compile | `src/Texture.cpp` and `src/Skybox.cpp` compile with GCC and Clang under `-Wall -Wextra -Wpedantic -Werror`. MSVC `/W4` not yet run. |
| Headless render | A throwaway harness (not committed) created an OpenGL 4.5 core context on Mesa llvmpipe through EGL, loaded the six faces through `Texture::loadCubemap`, and drew the skybox from five directions into a 640×360 framebuffer cleared to magenta. |
| Coverage and depth | No magenta pixel remained in any view: the `.xyww` depth of 1.0 passes with `GL_LEQUAL`. `glGetIntegerv(GL_DEPTH_FUNC)` returned `GL_LESS` after every draw. `glGetError` stayed `GL_NO_ERROR`. |
| Seams and orientation | Views across a cube edge, a cube corner and the zenith showed no seam and no mirrored or rotated face; the Milky Way band continues across face boundaries. |
| Ownership | A moved-from `Skybox` (and its `Texture`) was destroyed without errors. A missing file throws `Cannot load texture <path>: can't fopen`. |
| Asset size | Six 1024×1024 PNGs, about 680 KB in total. |

**Superseded in part.** `feat/mountain-lake-foundation` had written its own skybox at the same paths and wired it into `Scene`, so the merge into `main` kept that one; these faces and this `Texture` wrapper stay in the tree, and the wrapper is still the one the terrain textures are meant to use. The team owns the decision of which module to keep long term.

## Lighthouse and beacon pass

Run on 2026-11-11, first on the lighthouse branch and then again after merging it with the night scene.

| Check | Result |
|---|---|
| Windows x64 Release and Debug | Both builds passed with MSVC 19.51, CMake 4.4.3, no warnings under `/W4 /permissive-` |
| CPU tests | Camera/hierarchy/beacon and heightmap suites passed in both configurations (2/2 each) |
| Graphics check | Hidden-window smoke test passed in both configurations: all five shader programs compiled and linked, no OpenGL errors, frame not uniform |
| GPU | NVIDIA GeForce RTX 3050, OpenGL 3.3.0 / driver 610.88, Windows 11 |
| Lighthouse | 18 sections; foot dropped onto the terrain at `(29, -5)`, lamp at `(29.0, 17.6, -5.0)` as reported at start-up |
| Shader includes | `#include "beacon.glsl"` resolved in the terrain, lake and object fragment shaders; a missing or malformed include fails the shader load with the including file named |
| Second light | Verified by rendering a frame with the beam mesh suppressed: the beacon alone leaves a visible lit patch on the far bank, so the terrain and water response is the light and not the additive shaft |
| Lit air clipped at the water | With the cone tilted down, fragments below `world::waterLevel` are discarded; without the clip the shaft was drawn straight through the lake, since the water writes no depth |
| Beam phase | Integrated from the animation clock in the main loop, so `T` freezes the sweep and a speed change does not jump it |
| In the reflection | The tower is drawn inside `renderEnvironment`, so the mirrored pass picks it up: the capture shows the tower and the lit bank in the water. The shaft itself is left out of that pass. |

Re-tuned during the merge, against the night palette rather than the daylight one the beacon was first set against: the beacon colour down from `(3.40, 2.80, 1.80)` to `(1.30, 1.05, 0.68)`, the shaft alpha from `0.26/0.34` to `0.15/0.26`, and `object.frag` moved onto the terrain's moonlight and night ambient. `preview.png` is the current capture, taken with `--smoke-test --capture`.

No performance benchmark is claimed for this pass: the smoke test renders four frames and does not measure frame time. Interactive checks remain a team review task - the `L` toggle, the `,` / `.` sweep speed, the shaft seen from behind the lamp and from under the water surface, and the view of the tower from the air.

## Plane

Added 2026-11-13, on the same machine as the lighthouse pass.

| Check | Result |
|---|---|
| Windows x64 Release and Debug | Both builds passed, no warnings |
| CPU tests | Both suites passed in both configurations (2/2 each) |
| Graphics check | Hidden-window smoke test passed in both configurations |
| Geometry | Seven parts: fuselage, wing, tailplane, fin and three lamps. Checked up close by temporarily parking the plane in front of the start camera, from the side and head on. |
| Path | A 72-unit circle at altitude 41, one lap per 52 seconds, banked 0.2 rad into the turn, heading along the tangent. |
| Lights | Flash 0.12s every 1.3s; the emission is scaled in one place so the plane keeps its material colour while they are off. |
| Clock | Driven by `SceneSettings::time`, so `T` stops the plane and the blinking with the waves. |

Fixed while tuning: the first wing was 15.5 units across on a 9-unit fuselage, which from the valley floor read as a thin bar rather than a plane. The wing is now 12 units and thicker, the fuselage fatter, the lamps larger, and the circle was brought in from 85 to 72 units so the plane is not sitting in the far fog.

The plane is drawn inside `renderEnvironment`, so the mirrored pass picks it up and it appears in the lake like the tower does. Interactive review is still a team task: how it reads while crossing the frame, whether the lap is too fast or too slow, and whether it should have a light the water can catch.
