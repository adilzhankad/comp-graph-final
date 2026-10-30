# Implementation roadmap

The README preserves the final project specification. Current work starts with the terrain and its low-poly art direction; other objects will return in later milestones. Registration, team allocation, submission dates and academic deliverables still need team input.

## Current milestone: terrain

The active scene renders only the ground and mountains. A 513×513 grayscale heightmap spans 560×560 world units, sixteen times the area of the initial landscape. A coarse, irregular 128×128-cell grid produces 32,768 triangles with roughly 4.375-unit cells, preserving the low-poly scale. Connected ridge lines, elevated saddles, asymmetric shared flanks and descending spurs form long mountain ranges around the central valley. Duplicated triangle vertices carry geometric face normals, and flat shader interpolation keeps the muted green/grey/pale colour bands and lighting distinct on each facet.

Smooth fog fades distant terrain from 95 to 230 units from the camera. An additional world-space fog band reaches the background colour from 205 to 260 units along either horizontal axis, before the mesh boundary at ±280. Fog blends continuously across faces while the material and directional lighting remain flat.

Lake, aurora and prop prototype files remain in the repository for later use, but the current application does not compile or render them. The reusable camera, mesh, shader and hierarchy infrastructure remains. GLFW and GLAD compile from bundled source; GLM and stb_image are bundled too. The default build needs no dependency download or precompiled libraries.

Regenerate the heightmap with `python scripts/generate_heightmap.py`; Python is a development convenience, not a normal build dependency. `Heightmap::normalAt` retains finite-difference normals for CPU slope queries. Rendered terrain normals come from triangle edge cross products. Textures and a cubemap remain future work.

## Requirement status

| Requirement | Present in the active terrain milestone | Work still required |
|---|---|---|
| R1: modern pipeline | OpenGL 3.3 core context, indexed meshes, separate GLSL shaders | Preserve these constraints when adding features; inspect render passes on target machines. |
| R2: scene objects | Terrain only | Introduce the lake, aurora, campfire and remaining props incrementally; eventually demonstrate at least five distinct object types. |
| R3: transforms and hierarchy | Camera view/projection and model transforms; reusable parent–child code is retained | Integrate campfire children and boat → lantern when those objects return; derive lantern light position from its world transform. |
| R4: lighting | Flat ambient/directional diffuse shading with adjustable light direction | Add final material shading and moon/campfire/lantern lighting while preserving the chosen art direction. |
| R5: textures/equivalent | Grayscale heightmap input and per-face height/slope colours | Add texture ownership/loading, UV validation, mipmaps, terrain/log/tent textures and six cubemap faces; reintroduce procedural aurora. |
| R6: interaction | Fly camera, directional-light keys, wireframe and mouse capture | Add scene controls alongside their respective objects, then on-screen help and cinematic glide. |
| R7: real-time performance | Window-title FPS counter; static geometry buffers | Measure average/minimum FPS at 1280×720 on named machines; profile slow passes before adding complexity. |
| R8: reproducible build | CMake project, bundled sources with version/hash manifest, test commands, resource copying and platform CI configuration | Record clean-build results on Windows and macOS; verify runtime from a different working directory and failure diagnostics. |

## Next work, in order

### 1. Refine and review the terrain

- Build Release and Debug, run CPU tests and the graphics smoke test.
- Check mouse capture, resize/minimize behaviour, camera speed, `[`/`]` light rotation and `P` wireframe.
- Review the mountain silhouettes, peak sharpness, facet size and palette before adding other scene objects. Check the mesh from multiple camera angles for cracks or reversed faces, and inspect the fog transition when looking toward or moving near the outer terrain.
- Save the approved terrain screenshot and a frame-rate measurement. The FPS counter alone is not a benchmark result.
- Review the Windows/macOS/Linux CI results after pushing the changes. The configured workflow has not run merely because it is present in the source tree.
- Assign each module to a team member; each person should own commits and be able to explain their code.

### 2. Add the lake and environment after the terrain is approved

- Reintroduce the lake prototype as a separate step. Tune its silhouette, transparency and highlights against the faceted terrain before adding reflections or other effects.
- Add a `Texture` wrapper for 2D images and cubemaps, with mipmaps and explicit texture-unit bindings. Decode each image once at startup. Keep texture detail compatible with the low-poly palette and face shading.
- Integrate grass/rock/snow textures as required by the final specification, retaining height/slope material selection. Add prop textures when their geometry is introduced.
- Add a `Skybox` module with six consistently oriented faces, translation-free view matrix and `GL_LEQUAL`; restore depth state afterward.
- Reintroduce the campfire and add pine, tent and boat geometry with deliberate scale and shoreline placement. Begin with ordinary draws so R2 does not depend on instancing.
- Parent the lantern mesh and its point-light position to the boat node. Animate the boat parent and verify that both follow.
- Extend the current directional light into the final moon/campfire/lantern setup, extracting shared light data into `Lights` when useful. Restore the campfire toggle with that feature.

### 3. Reintroduce the aurora and scene interaction

- Reintroduce the aurora prototype and tune ribbon placement, vertical fades, noise frequencies and colour transitions against the night sky. Restore arrow-key controls with the shader feature.
- Add the Catmull-Rom camera glide and its aurora-intensity ramp. Manual camera input should have a clearly defined effect on glide mode.
- Add time pause and on-screen help. Keep the README, terminal help and on-screen controls consistent; inactive future bindings should stay marked as planned.
- Decide whether mouse-drag moon control and ImGui are useful after the core controls are complete.

### 4. Implement and verify stretch goals

Choose at least two completed, demonstrable goals rather than starting all three at once.

| Goal | Suggested implementation | Acceptance check |
|---|---|---|
| Lake waves | Grid displacement or animated normals, with coherent normal calculation | Presets visibly change wave strength; highlights move smoothly; time pause freezes waves. |
| Planar reflection | Resize-aware framebuffer, mirrored camera, above-water clip plane, projective sampling | A moving object and aurora are mirrored correctly; `R` toggles the pass; resize and grazing views remain correct. |
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
