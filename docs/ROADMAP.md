# Implementation roadmap

The README preserves the final project specification. The active scene now includes the terrain with its low-poly art direction, the lake, a starry skybox and animated aurora; other objects will return in later milestones. Registration, team allocation, submission dates and academic deliverables still need team input.

## Current milestone: aurora over the mountain lake

The active scene renders the ground, the mountains, the starry sky, aurora and the lake. A 513×513 grayscale heightmap spans 560×560 world units, sixteen times the area of the initial landscape. An irregular 256×256-cell grid produces 131,072 triangles with 2.1875-unit cells, half the previous cell width and four times the triangle count. Connected ridge lines, elevated saddles, asymmetric shared flanks and descending spurs form long mountain ranges around the central valley. Deterministic vertex jitter and varied diagonals break up the grid. Duplicated triangle vertices carry geometric face normals, and flat shader interpolation preserves the low-poly style and distinct muted colour bands on each facet.

`Terrain` applies the runtime sample mapping `h' = h + 0.25·max(h − 5, 0)·smoothstep(5, 12, h)`, raising the highest samples from about 26.66 to 32.07 units (roughly 20%). Samples at or below 5 and the lake basin remain unchanged. Rendering and CPU terrain queries use the same adjusted field; the raw image, encoding and generic heightmap loader are unchanged. Local rollback checkpoints are recorded in the README, and [validation notes](VALIDATION.md) track the checks.

Smooth fog fades distant terrain from 95 to 230 units from the camera. An additional world-space fog band reaches the blue night haze colour from 205 to 260 units along either horizontal axis, before the mesh boundary at ±280. The skybox horizon and the lake use that same haze colour. Fog blends continuously across faces while the material and directional lighting remain flat.

The application compiles and renders the aurora; prop prototype files remain deferred. The reusable camera, mesh, shader and hierarchy infrastructure remains. GLFW and GLAD compile from bundled source; GLM and stb_image are bundled too. The default build needs no dependency download or precompiled libraries.

Regenerate the heightmap with `python scripts/generate_heightmap.py`; Python is a development convenience, not a normal build dependency. `Heightmap::normalAt` retains finite-difference normals for CPU slope queries. Rendered terrain normals come from triangle edge cross products. Terrain and prop textures remain future work.

### Starry sky

The `Skybox` module loads six 1024×1024 RGB cubemap faces reprojected from NASA SVS's Deep Star Maps panorama. It owns its GPU texture and cube mesh, validates face dimensions, generates mipmaps and uses seamless cubemap sampling. A translation-free view keeps the sky at infinity; the sky pass uses `GL_LEQUAL` with depth writes disabled between terrain and water. It remains filled when wireframe is enabled.

The sky shader adds a dark blue atmosphere and blends the star texture into the shared fog colour near the horizon. The water samples the same map and applies the same sky treatment. Source, NASA usage guidelines and reproducible Python/Pillow conversion instructions are in [the asset notes](../assets/skybox/README.md). Aurora is separate animated 3D geometry. [Validation notes](VALIDATION.md) record the scope and results of build, graphics and camera-view checks; `preview.png` records an application capture.

### Aurora

Three soft 3D curtains sit above the mountains with animated folds, fine vertical rays, brighter lower edges and fading tips. Procedural green, teal and violet colour shifts locally over time. The additive pass retains mountain occlusion and is included in the lake reflection. It adds visible emission, without a second scene light or HDR/bloom pipeline.

`U` controls visibility, `Up`/`Down` set intensity from 0 to 2, and `Left`/`Right` set speed from 0.1 to 2.5. An accumulated aurora clock prevents phase jumps when speed changes; `T` freezes that clock and the waves. The window title shows brightness, speed and pause state.

### Lake

The generator carves a lobed basin into the valley floor, which is why the encoded height range is now [-10, 28] rather than [-4, 28]; `mla::world::waterLevel` in `src/World.h` is the single level the generator, the renderer and the tests agree on. The water uses its own 128×128-cell grid, independent of the denser terrain mesh. It retains cells within one coarse cell of a submerged sample — 780 triangles — and overlaps the shore so the depth test clips it to the terrain intersection. It is drawn after the terrain with alpha blending and depth writes off.

The mesh is flat; the fragment shader builds its normal from 16 directional gravity-wave components at several scales. Moving wind patches and wave-packet envelopes vary their strength, a weak bounded phase bend curves their crests, and a mild second harmonic shapes them. Analytic envelope and phase gradients keep the normals consistent; `fwidth` filtering fades subpixel waves and harmonics to reduce reflection shimmer. This is an approximate visual model without vertex displacement or per-frame uploads, not a full fluid, FFT or shoreline simulation. The conceptual reference is Mark Finch's [GPU Gems, Chapter 1](https://developer.nvidia.com/gpugems/gpugems/part-i-natural-effects/chapter-1-effective-water-simulation-physical-models); the implementation and tuning are original to this project.

Shading is Schlick fresnel between a dark body colour and the reflected scene, with one Blinn-Phong lobe for the cool directional highlight and a shared horizon haze colour. The single directional light now suggests moonlight; the full moon controls, campfire and lantern remain planned. See [validation notes](VALIDATION.md) for measured checks and their limits.

The planar reflection pass captures mountains, shore, sky and animated aurora from a camera mirrored across `y = -1`. `gl_ClipDistance[0]` removes underwater terrain, and clipping is disabled before drawing the sky. `ReflectionTarget` owns an RGBA8 colour texture plus a 24-bit depth renderbuffer, resizing with the viewport while preserving aspect within a 1280×720 cap. The water samples this capture with projective UVs and small ripple-driven distortion; no additional Y flip is needed. `R` toggles this pass, falling back to the original sky-only cubemap reflection. Underwater views also use that fallback. The main and reflected aurora share geometry, shaders and animation time. Future props will need integration into the same pass.

## Requirement status

| Requirement | Present in the active milestone | Work still required |
|---|---|---|
| R1: modern pipeline | OpenGL 3.3 core context, indexed meshes, separate GLSL shaders | Preserve these constraints when adding features; inspect render passes on target machines. |
| R2: scene objects | Terrain, lake, skybox and aurora | Introduce the campfire and remaining props incrementally; eventually demonstrate at least five distinct object types. |
| R3: transforms and hierarchy | Camera view/projection and model transforms; reusable parent–child code is retained | Integrate campfire children and boat → lantern when those objects return; derive lantern light position from its world transform. |
| R4: lighting | Flat ambient/directional diffuse terrain shading with adjustable light direction; per-fragment fresnel and Blinn-Phong specular on the water | Add a second light source: the requirement needs two, one of them dynamic or toggleable. Then the final material shading and the moon/campfire/lantern setup, while preserving the chosen art direction. |
| R5: textures/equivalent | Grayscale heightmap input, per-face height/slope colours, procedural water normals and aurora, and a mipmapped six-face cubemap used by the sky and lake | Add a general 2D texture wrapper, UV validation and terrain/log/tent textures with mipmaps. |
| R6: interaction | Fly camera, directional-light keys, aurora visibility/brightness/speed, animation pause, reflection toggle, wireframe and mouse capture | Add scene controls alongside their respective objects, then on-screen help and cinematic glide. |
| R7: real-time performance | Window-title FPS counter; static geometry buffers | Measure average/minimum FPS at 1280×720 on named machines; profile slow passes before adding complexity. |
| R8: reproducible build | CMake project, bundled sources with version/hash manifest, test commands, resource copying and platform CI configuration | Record clean-build results on Windows and macOS; verify runtime from a different working directory and failure diagnostics. |

## Next work, in order

### 1. Validate and review the night environment

- Repeat Release/Debug builds, CPU tests and the graphics smoke test after future renderer changes; the 2026-11-07 local checks passed.
- Check mouse capture, resize/minimize behaviour, camera speed, `[`/`]` light rotation, `R` reflections, `P` wireframe, `U` aurora, arrow-key brightness/speed and `T` animation pause.
- Review the mountain silhouettes, peak sharpness, facet size and palette before adding other scene objects. Check the mesh from multiple camera angles for cracks or reversed faces, and inspect the fog transition when looking toward or moving near the outer terrain.
- Review the lake: shoreline shape from the air and from the bank, wave scale, how much of the bed shows through, the highlight as `[` / `]` sweep it, and the view from under the surface.
- Extend the automated camera-view and missing-face checks with interactive review of sky seams, reflection and fog; mismatched-face diagnostics remain to be exercised.
- Measure frame rate on team machines. A new night-scene screenshot is recorded, but neither the graphics smoke test nor the FPS counter is a benchmark.
- Review the Windows/macOS/Linux CI results after pushing the changes. The configured workflow has not run merely because it is present in the source tree.
- Assign each module to a team member; each person should own commits and be able to explain their code.

### 2. Extend the environment after the night scene is approved

- Add a general `Texture` wrapper for 2D images with mipmaps and explicit texture-unit bindings; cubemap ownership already lives in `Skybox`. Decode each image once at startup. Keep texture detail compatible with the low-poly palette and face shading.
- Integrate grass/rock/snow textures as required by the final specification, retaining height/slope material selection. Add prop textures when their geometry is introduced.
- Reintroduce the campfire and add pine, tent and boat geometry with deliberate scale and shoreline placement. Begin with ordinary draws so R2 does not depend on instancing.
- Parent the lantern mesh and its point-light position to the boat node. Animate the boat parent and verify that both follow.
- Extend the current directional light into the final moon/campfire/lantern setup, extracting shared light data into `Lights` when useful. Restore the campfire toggle with that feature. R4 is not met until a second light exists, so this is the highest-value item in this group; the lake shader already has the specular lobe it will need.

### 3. Extend cinematic presentation and scene interaction

- Review the implemented aurora from multiple camera positions and over longer animation intervals: folded silhouettes, soft fades, fine rays, local colour changes and reflected movement. Check arrow-key settings and pause on team machines.
- Add the Catmull-Rom camera glide and its aurora-intensity ramp. Manual camera input should have a clearly defined effect on glide mode.
- Preserve the shared pause behaviour as new animated objects are introduced, and add on-screen help. Keep the README, terminal help and on-screen controls consistent; inactive future bindings should stay marked as planned.
- Decide whether mouse-drag moon control and ImGui are useful after the core controls are complete.

### 4. Implement and verify stretch goals

Planar reflection is implemented for the current landscape and animated aurora. Complete and demonstrate at least two goals, extending the reflection pass as new objects are introduced.

| Goal | Suggested implementation | Acceptance check |
|---|---|---|
| Lake waves | Grid displacement or animated normals, with coherent normal calculation | Presets visibly change wave strength; highlights move smoothly; time pause freezes waves. |
| Planar reflection | Implemented for terrain, shore, sky and aurora: resize-aware framebuffer, mirrored camera, above-water clip plane, projective sampling and `R` toggle | Review resize, grazing views and shoreline alignment on team machines. Review reflected aurora movement and include future moving props in the capture. |
| Instanced pines | Shared mesh, static per-instance matrices and divisors, terrain height/slope sampling | One instanced draw per tree mesh/material; trees stay above water and out of steep slopes; compare FPS before/after. |

### 5. Submission and evidence

- Verify a fresh checkout on Windows and macOS using only the documented setup. Confirm build artifacts are ignored.
- Fill the README FPS table with hardware, OS, resolution, average and minimum FPS.
- Record source/asset author, URL and license in `CREDITS.md` as assets are introduced.
- Write the required report, including the literature review, R1–R8 evidence, stretch-goal evidence, testing and AI-use disclosure.
- Produce 8–12 slides and a 3–6 minute demo ending with the cinematic glide. Keep all team members involved.
- Review the final repository, create the submission tag and submit the required deliverables by the course deadline.

## Development boundaries

Keep GPU resource ownership in the wrapper that creates the resource. Destroy scene objects before the GLFW context. Use frame delta time for input, scene time for animation, and restore blending/depth state between passes. Keep source files focused and shaders outside C++ strings. Tests should exercise meaningful behaviour, especially hierarchy transforms, terrain sampling and resource failure handling.
