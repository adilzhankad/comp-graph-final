# Mountain Lake & Aurora

> A calm mountain lake under a night sky with a dancing aurora.

Final project for **Computer Graphics Fundamentals** (Astana IT University).
Topic #11 from the *Final Project Topics* list, difficulty ★★ — *course material + one new simple technique*.
The new technique is an **animated aurora shader**.

Real-time interactive 3D scene written in **C++** with **OpenGL 3.3 core profile**: mountains built from a heightmap, a semi-transparent lake with strong specular highlights, curved aurora ribbons animated in a fragment shader, a starry skybox and a flickering campfire on the shore.

**Wow moment (end of the demo video):** the aurora brightens while the camera glides low over the lake.

---

## Contents

- [Status](#status)
- [Features](#features)
- [How the requirements R1–R8 are met](#how-the-requirements-r1r8-are-met)
- [Controls](#controls)
- [Build and run](#build-and-run)
- [Project structure](#project-structure)
- [Technical notes](#technical-notes)
- [Team](#team)
- [Deliverables](#deliverables)
- [Credits](#credits)
- [AI Use note](#ai-use-note)

---

## Status

🚧 In development. Checkboxes below are updated as features land.

| Milestone | What we show | Status |
|---|---|---|
| Team & topic registration | Topic name + every member in MS Teams | ☐ |
| Week 7 progress check | Window, camera, terrain, first objects | ☐ |
| Week 8 progress check | Lighting, textures, lake, aurora, interaction | ☐ |
| Week 9 progress check | Almost finished project | ☐ |
| Final submission (3 days before defense) | Tag `v1.0`, report PDF, slides, demo video | ☐ |

---

## Features

### Core (required)

- [ ] **Mountain terrain from a heightmap** — grayscale image → grid mesh, heights from pixel brightness, normals from finite differences, height/slope-based texturing (grass → rock → snow).
- [ ] **Lake** — semi-transparent plane (alpha blending, drawn after opaque geometry) with strong Blinn-Phong specular highlights from the moon and the campfire.
- [ ] **Aurora** — several curved quad ribbons with an animated colour shader (scrolling noise + sine waves, green → teal → purple gradient, additive blending).
- [ ] **Starry skybox** — night cubemap.
- [ ] **Campfire point light** — warm, flickering, with distance attenuation.

### Stretch goals (each working one = 4 pts in A5, two = full 8 pts)

- [ ] **Real reflection in the lake** — render the scene mirrored below the water plane into a framebuffer texture, sample it in the lake shader with projective UVs.
- [ ] **Small waves on the lake** — vertex displacement by summed sines and/or scrolling normal map, so highlights and reflection ripple.
- [ ] **Instanced pine trees** — hundreds of trees with `glDrawElementsInstanced`, placed on the terrain by height and slope.

### Extra polish (only if time allows)

- [ ] Aurora light reflected in the lake as a coloured tint.
- [ ] Distance fog / mountain haze.
- [ ] Campfire spark particles.
- [ ] Dear ImGui panel with all scene parameters.

---

## How the requirements R1–R8 are met

| # | Requirement | How this project meets it |
|---|---|---|
| R1 | Modern pipeline | OpenGL 3.3 core profile only; every mesh uses VAO/VBO/EBO; our own `.vert`/`.frag` shaders for each object type; no fixed-function calls. |
| R2 | Scene setup (5+ objects) | Terrain, lake, aurora ribbons, skybox, campfire (logs + stones), pine trees, tent, rowing boat with lantern. |
| R3 | Transformation pipeline + hierarchy | Own model/view/projection matrices (GLM). Scene graph with parent–child nodes: **boat → lantern** (the lantern and its light drift with the boat), **campfire → logs/stones**. |
| R4 | Lighting / shading | Per-fragment **Blinn-Phong** (ambient + diffuse + specular). Lights: moon (directional, can be moved across the sky), campfire (point, flickers, toggle), boat lantern (point, moves with the boat). |
| R5 | Texturing or equivalent | Terrain, logs and tent textures with correct UVs and mipmapping (`GL_LINEAR_MIPMAP_LINEAR`); night **cubemap skybox**; **procedural** aurora colour in the fragment shader. |
| R6 | Interaction | FPS/fly camera with keyboard + mouse; plus scene interactions: aurora intensity/speed, campfire on/off, moon position, time speed, wireframe toggle (see [Controls](#controls)). |
| R7 | Real-time performance | Target 60+ FPS at 1280×720; FPS counter shown in the window title (and in the ImGui overlay if enabled). |
| R8 | Build | Same setup as the course assignments; exact steps in [Build and run](#build-and-run). |

---

## Controls

> Final key bindings are fixed before `v1.0`; the on-screen help (`H`) always shows the current list.

| Key / input | Action |
|---|---|
| `W` `A` `S` `D` | Move camera |
| `Space` / `Left Ctrl` | Move up / down |
| Mouse | Look around |
| Mouse wheel | Zoom (field of view) |
| `Left Shift` | Move faster |
| `G` | Start/stop the cinematic glide over the lake |
| `↑` / `↓` | Aurora intensity up / down |
| `←` / `→` | Aurora animation speed down / up |
| `F` | Campfire on / off |
| `M` + mouse drag / `[` `]` | Move the moon across the sky |
| `R` | Lake reflection on / off (stretch) |
| `1` / `2` / `3` | Calm / gentle / windy lake waves (stretch) |
| `T` | Pause / resume time |
| `P` | Wireframe on / off |
| `H` | Show / hide on-screen help |
| `Tab` | Release / capture the mouse (for the ImGui panel) |
| `Esc` | Quit |

---

## Build and run

Language: C++17. Libraries: **GLFW** (window/input), **GLAD** (loader), **GLM** (math), **stb_image** (images), optionally **Dear ImGui** (UI). All third-party code lives in `external/` and is built together with the project — **nothing has to be installed on the TA machine except a C++ compiler and CMake**.

### Requirements

- CMake ≥ 3.16
- A C++17 compiler: MSVC (Visual Studio 2019+), Clang (Xcode CLT on macOS) or GCC 9+
- GPU and driver with OpenGL 3.3 support

### Steps

```bash
git clone https://github.com/adilzhankad/comp-graph-final.git
cd comp-graph-final
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Run from the repository root so that `shaders/` and `assets/` are found:

```bash
# macOS / Linux
./build/MountainLakeAurora

# Windows
build\Release\MountainLakeAurora.exe
```

**Visual Studio / CLion:** open the folder as a CMake project, choose the `MountainLakeAurora` target, set the working directory to the repository root and run.

> Before submission: check that a **fresh clone** builds with exactly these steps on Windows and macOS.

---

## Project structure

```
comp-graph-final/
├── CMakeLists.txt
├── README.md
├── CREDITS.md            # every third-party asset / snippet with author, link, license
├── src/
│   ├── main.cpp          # window, main loop, input dispatch
│   ├── Shader.h/.cpp     # load, compile, link, set uniforms
│   ├── Mesh.h/.cpp       # VAO/VBO/EBO wrapper, draw / draw instanced
│   ├── Texture.h/.cpp    # 2D textures with mipmaps, cubemaps (stb_image)
│   ├── Camera.h/.cpp     # FPS camera, frame-rate independent movement, glide path
│   ├── SceneNode.h/.cpp  # scene graph: local transform, children, world matrix
│   ├── Terrain.h/.cpp    # heightmap → mesh, normals, height lookup
│   ├── Lake.h/.cpp       # water plane, waves, reflection framebuffer
│   ├── Aurora.h/.cpp     # curved ribbon geometry + animation parameters
│   ├── Lights.h/.cpp     # moon, campfire, lantern; uniform upload
│   └── Primitives.h/.cpp # cube, cylinder, cone, sphere, plane generators
├── shaders/
│   ├── terrain.vert / terrain.frag
│   ├── lake.vert    / lake.frag
│   ├── aurora.vert  / aurora.frag
│   ├── skybox.vert  / skybox.frag
│   ├── object.vert  / object.frag     # Blinn-Phong for props
│   └── tree.vert    / tree.frag       # instanced pine trees
├── assets/
│   ├── heightmaps/
│   ├── textures/
│   └── skybox/
└── external/             # glfw, glad, glm, stb_image, imgui
```

Rule of thumb for D2: one class per concept, no file over ~500 lines, all shaders in separate `.vert`/`.frag` files.

---

## Technical notes

### Render order per frame

1. *(stretch)* Reflection pass: camera mirrored below the water plane, clip plane `y > waterLevel`, render terrain + trees + campfire + skybox + aurora into a framebuffer.
2. Opaque pass: terrain, trees, campfire, tent, boat.
3. Skybox (depth test `GL_LEQUAL`, depth written as 1.0).
4. Lake — alpha blending, depth write off.
5. Aurora — additive blending (`GL_SRC_ALPHA, GL_ONE`), depth write off, so it glows over the stars.
6. UI (FPS, help, ImGui).

### Terrain from a heightmap

Grid of `W × H` vertices; vertex `(i, j)` gets height `h = heightScale · I(i, j)` where `I` is the pixel brightness in `[0, 1]`. Normal by central differences:

```
n = normalize( ( h(i-1,j) - h(i+1,j),  2·cellSize,  h(i,j-1) - h(i,j+1) ) )
```

Fragment colour blends grass / rock / snow textures by height and by slope (`1 - n.y`).

### Lighting — Blinn-Phong

```
L_out = k_a·I_a + Σ_lights att · ( k_d·max(n·l, 0) + k_s·max(n·h, 0)^shininess ) · I_light
h     = normalize(l + v)
att   = 1 / (1 + k_l·d + k_q·d²)            # point lights only
```

The lake uses a high shininess and specular strength so the moon and campfire leave long bright streaks on the water. Campfire flicker: intensity multiplied by a sum of a few sines with incommensurate frequencies plus small noise.

### Aurora shader

- **Geometry:** each ribbon is a strip of quads along a curve (Catmull-Rom or sine path) high above the lake; `v` runs from the bottom edge to the top edge, `u` along the ribbon.
- **Vertex shader:** sways the ribbon sideways with `sin(u·k + time·speed)`.
- **Fragment shader:**
  - vertical curtain streaks: scrolling 1D/2D value noise in `u` and time;
  - vertical fade: bright, sharp lower edge, soft top — `smoothstep` on `v`;
  - colour gradient by `v`: green at the bottom → teal → purple/pink at the top;
  - output alpha = intensity uniform × streaks × fade, drawn with additive blending.
- **Uniforms:** `uTime`, `uIntensity`, `uSpeed` — changed by keys, and ramped up automatically during the glide for the wow moment.

### Lake

- Large quad (or a fine grid for waves) at `waterLevel`, alpha ≈ 0.6–0.8.
- Fresnel term `F = F0 + (1 − F0)·(1 − cosθ)^5` mixes reflection colour and deep-water colour: more reflective at grazing angles.
- *(stretch)* Reflection: screen-space projective UVs `uv = clip.xy / clip.w · 0.5 + 0.5`, flipped in Y, distorted slightly by the wave normal.

### Instanced pine trees *(stretch)*

One tree mesh (cone + cylinder), per-instance `mat4` model matrix in a VBO with `glVertexAttribDivisor`, positions sampled once at start-up from the terrain (only on slopes below a threshold and above the waterline). Uploaded once — no per-frame uploads of static data (B4).

### Camera

FPS camera with yaw/pitch from the mouse, movement scaled by `deltaTime` so speed does not depend on FPS (C1). The cinematic glide follows a Catmull-Rom spline of control points low over the lake.

---

## Team

| Member | Group | Responsibilities |
|---|---|---|
| _Surname Name_ | _SE-XXXX_ | _e.g. window, camera, scene graph, build system_ |
| _Surname Name_ | _SE-XXXX_ | _e.g. terrain from heightmap, texturing, instanced trees_ |
| _Surname Name_ | _SE-XXXX_ | _e.g. lake: transparency, specular, waves, reflection_ |
| _Surname Name_ | _SE-XXXX_ | _e.g. aurora shader, skybox_ |
| _Surname Name_ | _SE-XXXX_ | _e.g. lights, campfire, UI/ImGui, FPS counter_ |

Every member commits their own work regularly throughout the project and can explain any part of the code at the defense.

---

## Deliverables

- [ ] **Repository** — this README, `CREDITS.md`, shaders in separate files, no build artifacts, commits from every member, tag `v1.0` on the submission date.
- [ ] **Report** (PDF, Final Project Report Template) — Introduction, Literature Review (15+ Q1–Q2 sources), Project Description, Technical Implementation, Graphics and Rendering, Testing and Evaluation with FPS table, Conclusion and Future Work, References, AI Use Note.
- [ ] **Slides** — 8–12 slides, 7–10 minutes, every member presents a part.
- [ ] **Demo video** — 3–6 minutes, 1080p, real-time capture with the FPS counter visible, voice-over or captions naming each R1–R7 feature and each stretch goal, ending on the wow moment.

### FPS table (filled in during testing)

| Machine | CPU | GPU | OS | Resolution | Avg FPS | Min FPS |
|---|---|---|---|---|---|---|
| | | | | 1280×720 | | |

---

## Credits

All third-party assets (heightmaps, textures, skybox, models), shader snippets and tutorials are listed in [`CREDITS.md`](CREDITS.md) with author, link and license.
Code adapted from LearnOpenGL or the course labs is also cited in a comment next to the code.

---

## AI Use note

Required by the course rules; kept up to date during the project and copied into the report.

| Tool | What it was used for | What we changed / checked |
|---|---|---|
| Claude (Anthropic) | Drafted the initial structure of this README from the course requirements and topic description. | _Team reviewed and edited the text; fill in further uses here._ |
