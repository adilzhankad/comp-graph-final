# Implementation roadmap

The README preserves the final project specification. Current work starts with the terrain and its low-poly art direction, and now the lake; other objects will return in later milestones. Registration, team allocation, submission dates and academic deliverables still need team input.

## Current milestone: terrain and lake

The active scene renders the ground, the mountains and the lake. A 513×513 grayscale heightmap spans 560×560 world units, sixteen times the area of the initial landscape. A coarse, irregular 128×128-cell grid produces 32,768 triangles with roughly 4.375-unit cells, preserving the low-poly scale. Connected ridge lines, elevated saddles, asymmetric shared flanks and descending spurs form long mountain ranges around the central valley. Duplicated triangle vertices carry geometric face normals, and flat shader interpolation keeps the muted green/grey/pale colour bands and lighting distinct on each facet.

Smooth fog fades distant terrain from 95 to 230 units from the camera. An additional world-space fog band reaches the background colour from 205 to 260 units along either horizontal axis, before the mesh boundary at ±280. Fog blends continuously across faces while the material and directional lighting remain flat.

Aurora and prop prototype files remain in the repository for later use, but the current application does not compile or render them. The reusable camera, mesh, shader and hierarchy infrastructure remains. GLFW and GLAD compile from bundled source; GLM and stb_image are bundled too. The default build needs no dependency download or precompiled libraries.

Regenerate the heightmap with `python scripts/generate_heightmap.py`; Python is a development convenience, not a normal build dependency. `Heightmap::normalAt` retains finite-difference normals for CPU slope queries. Rendered terrain normals come from triangle edge cross products. Textures and a cubemap remain future work.

### Lake

The generator carves a lobed basin into the valley floor, which is why the encoded height range is now [-10, 28] rather than [-4, 28]; `mla::world::waterLevel` in `src/World.h` is the single level the generator, the renderer and the tests agree on. The water surface keeps only the terrain cells within one cell of a submerged sample — 780 triangles — and is left overlapping the shore so the depth test clips it to the exact terrain intersection. It is drawn after the terrain with alpha blending and depth writes off.

The mesh is flat; the fragment shader builds the normal from the analytic gradient of four crossing wave trains, so ripples cost no vertex work and no per-frame upload. Shading is Schlick fresnel between a dark body colour and a sky stand-in, with one Blinn-Phong lobe for the sun streak and the terrain's fog repeated so shore and water reach the background together. The sky stand-in should become a real skybox sample once the cubemap exists, and the single directional light should become the planned moon.

## Requirement status

| Requirement | Present in the active terrain milestone | Work still required |
|---|---|---|
| R1: modern pipeline | OpenGL 3.3 core context, indexed meshes, separate GLSL shaders | Preserve these constraints when adding features; inspect render passes on target machines. |
| R2: scene objects | Terrain and lake | Introduce the aurora, campfire and remaining props incrementally; eventually demonstrate at least five distinct object types. |
| R3: transforms and hierarchy | Camera view/projection and model transforms; reusable parent–child code is retained | Integrate campfire children and boat → lantern when those objects return; derive lantern light position from its world transform. |
| R4: lighting | Flat ambient/directional diffuse terrain shading with adjustable light direction; per-fragment fresnel and Blinn-Phong specular on the water | Add a second light source: the requirement needs two, one of them dynamic or toggleable. Then the final material shading and the moon/campfire/lantern setup, while preserving the chosen art direction. |
| R5: textures/equivalent | Grayscale heightmap input, per-face height/slope colours and a procedural wave normal in the water shader | Add texture ownership/loading, UV validation, mipmaps, terrain/log/tent textures and six cubemap faces; reintroduce procedural aurora. |
| R6: interaction | Fly camera, directional-light keys, animation pause, wireframe and mouse capture | Add scene controls alongside their respective objects, then on-screen help and cinematic glide. |
| R7: real-time performance | Window-title FPS counter; static geometry buffers | Measure average/minimum FPS at 1280×720 on named machines; profile slow passes before adding complexity. |
| R8: reproducible build | CMake project, bundled sources with version/hash manifest, test commands, resource copying and platform CI configuration | Record clean-build results on Windows and macOS; verify runtime from a different working directory and failure diagnostics. |

## Next work, in order

### 1. Refine and review the terrain

- Build Release and Debug, run CPU tests and the graphics smoke test.
- Check mouse capture, resize/minimize behaviour, camera speed, `[`/`]` light rotation and `P` wireframe.
- Review the mountain silhouettes, peak sharpness, facet size and palette before adding other scene objects. Check the mesh from multiple camera angles for cracks or reversed faces, and inspect the fog transition when looking toward or moving near the outer terrain.
- Review the lake: shoreline shape from the air and from the bank, wave scale, how much of the bed shows through, the highlight as `[` / `]` sweep it, and the view from under the surface.
- Save the approved terrain screenshot and a frame-rate measurement. The FPS counter alone is not a benchmark result.
- Review the Windows/macOS/Linux CI results after pushing the changes. The configured workflow has not run merely because it is present in the source tree.
- Assign each module to a team member; each person should own commits and be able to explain their code.

### 2. Add the environment after the terrain and lake are approved

- Add a `Texture` wrapper for 2D images and cubemaps, with mipmaps and explicit texture-unit bindings. Decode each image once at startup. Keep texture detail compatible with the low-poly palette and face shading.
- Integrate grass/rock/snow textures as required by the final specification, retaining height/slope material selection. Add prop textures when their geometry is introduced.
- Add a `Skybox` module with six consistently oriented faces, translation-free view matrix and `GL_LEQUAL`; restore depth state afterward.
- Reintroduce the campfire and add pine, tent and boat geometry with deliberate scale and shoreline placement. Begin with ordinary draws so R2 does not depend on instancing.
- Parent the lantern mesh and its point-light position to the boat node. Animate the boat parent and verify that both follow.
- Extend the current directional light into the final moon/campfire/lantern setup, extracting shared light data into `Lights` when useful. Restore the campfire toggle with that feature. R4 is not met until a second light exists, so this is the highest-value item in this group; the lake shader already has the specular lobe it will need.
- Replace the lake's analytic sky stand-in with a cubemap sample once the skybox lands, so the water reflects the real sky instead of a tint of the fog colour.

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
