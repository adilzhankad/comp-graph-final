# Terrain, fog, lake and lighthouse validation

Updated 2026-11-11. The current scene renders the low-poly terrain, the lake and the lighthouse. The aurora module is retained as a future prototype.

Latest terrain revision: seven connected ridge chains and twelve descending spurs replace the isolated peak envelopes. The visible starting skyline now has elevated saddles and shared flanks. The regenerated heightmap passed the existing heightmap test executable, and a visible-window OpenGL capture was checked visually and passed the application's error checks. `preview.png` records this revision. C++ and shader source did not change in this pass; the build results below remain the preceding code baseline.

| Check | Result |
|---|---|
| Windows x64 Release and Debug | Both builds passed with MSVC 19.44 |
| CPU tests | Camera/hierarchy and heightmap suites passed in both configurations (2/2 each) |
| Graphics check | Visible-window capture completed; terrain shaders compiled/linked and the OpenGL error check passed |
| GPU | Intel Iris Xe Graphics, OpenGL 3.3.0 / driver32.0.101.6913 |
| World | 560x560 units; 513x513 image samples; 128x128 cells / 32,768 triangles |
| Low-poly geometry | Split triangle vertices with flat geometric normals; jittered shared positions and mixed diagonals |
| Fog | Smooth distance fade from95 to230 units; boundary fade from205 to260 on each axis before the mesh ends at280 |

`preview.png` is the current application capture. Fog is fully opaque and exactly matches the background before reaching the outer boundary, including when approaching it. Face lighting remains flat while fog interpolates smoothly.

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

`preview.png` is the current application capture, taken by `--smoke-test --capture`. The heightmap test now covers the flooded area, the bed depth, the banks and the southern bank profile, so a regenerated heightmap that loses the basin fails the suite rather than silently draining the lake.

No performance benchmark is claimed for this pass either: the smoke test renders four frames and does not measure frame time. The FPS table in the README still needs a real measurement at 1280x720 on each team machine. Linux and macOS runtime, GitHub CI results and interactive exploration remain team review tasks.

## Lighthouse and beacon pass

Run on the same machine as the lake pass, on 2026-11-11.

| Check | Result |
|---|---|
| Windows x64 Release and Debug | Both builds passed with MSVC 19.51, CMake 4.4.3, no warnings under `/W4 /permissive-` |
| CPU tests | Camera/hierarchy/beacon and heightmap suites passed in both configurations (2/2 each) |
| Graphics check | Hidden-window smoke test passed in both configurations: the four shader programs compiled and linked, no OpenGL errors, frame not uniform |
| GPU | NVIDIA GeForce RTX 3050, OpenGL 3.3.0 / driver 610.88, Windows 11 |
| Lighthouse | 18 sections; foot dropped onto the terrain at `(29, -5)`, lamp at `(29.0, 17.6, -5.0)` as reported at start-up |
| Shader includes | `#include "beacon.glsl"` resolved in terrain, lake and object fragment shaders; a missing or malformed include fails the shader load with the including file named |
| Second light | Verified by rendering a frame with the beam mesh suppressed: the beacon alone leaves a visible lit patch on the west bank, so the terrain and water response is the light and not the additive shaft |
| Lit air clipped at the water | With the cone tilted down, fragments below `world::waterLevel` are discarded; without the clip the shaft was drawn straight through the lake, since the water writes no depth |
| Beam phase | Integrated from the animation clock in the main loop, so `T` freezes the sweep and a speed change does not jump it |

`preview.png` was regenerated with `--smoke-test --capture` and now shows the scene with the beacon lit; the beam crosses the valley and lands on the west ridge.

The beacon's colour is deliberately bright: the scene is still lit for daylight, and the patch has to show against that ambient. It has to be re-tuned when the night palette lands.

No performance benchmark is claimed for this pass: the smoke test renders four frames and does not measure frame time. Interactive checks remain a team review task - the `L` toggle, the `,` / `.` sweep speed, the shaft seen from behind the lamp and from under the water surface, and the view of the tower from the air.
