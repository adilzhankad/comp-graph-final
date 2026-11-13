# Credits and provenance

Record every new third-party asset or adapted snippet here before it is included in a submission. Preserve the original copyright and license files beside bundled source.

## Libraries

| Component | Author / project | Upstream | Version | License |
|---|---|---|---|---|
| GLFW | GLFW contributors | [GLFW repository](https://github.com/glfw/glfw) | 3.4 | zlib/libpng; see `external/glfw/LICENSE.md` |
| GLAD generator and generated loader | David Herberth and contributors; Khronos registry contributors | [GLAD repository](https://github.com/Dav1dde/glad/tree/v0.1.36) | 0.1.36; OpenGL 3.3 core, no extensions | Generator MIT; Khronos notices retained; see `external/glad/LICENSE` and `external/glad/include/KHR/khrplatform.h` |
| GLM | G-Truc Creation and GLM contributors | [GLM repository](https://github.com/g-truc/glm/tree/1.0.1) | 1.0.1 | MIT or Modified MIT; see `external/glm/copying.txt` |
| stb_image | Sean Barrett and contributors | [stb source snapshot](https://github.com/nothings/stb/tree/003501f6d7a8668ce8b6d875e34dd97ad0d5d5b5) | Commit `003501f6d7a8668ce8b6d875e34dd97ad0d5d5b5` | MIT or public domain; see `external/stb/LICENSE` and `stb_image.h` |

The project builds GLFW and generated GLAD source locally; no precompiled library binaries are distributed. GLM is used for matrices/vectors, and stb_image decodes the starter heightmap and cubemap faces. Download URLs and SHA-256 checksums are recorded in [`external/versions.json`](external/versions.json); the GLAD regeneration command is in [`external/README.md`](external/README.md).

## Assets and project code

| Item | Origin | Notes |
|---|---|---|
| Starter grayscale heightmap | Generated for this project by `scripts/generate_heightmap.py` | Deterministic mathematical terrain and lake basin; source recipe is included. No external photograph or heightmap was used. |
| Starry sky panorama and derived cubemap faces | NASA/Goddard Space Flight Center Scientific Visualization Studio; visualizations by Ernie Wright and Tom Bridgman. [Deep Star Maps (2012)](https://svs.gsfc.nasa.gov/3895/) | The original 4096×2048 `starmap_4k.jpg` is bundled unchanged. `scripts/prepare_star_cubemap.py` reprojects it into six 1024×1024 PNG faces; sky and water shaders add blue atmosphere and horizon haze. Use follows [NASA Images and Media Usage Guidelines](https://www.nasa.gov/nasa-brand-center/images-and-media/), which permit educational computer graphical simulations with source acknowledgment; this is not a CC0 asset. See [asset notes](assets/skybox/README.md) for the direct source URL, SHA-256 and conversion details. No aurora is included in the texture. |
| `Skybox`, cubemap reprojection script, sky shaders and night environment integration | Written for this project with Codex assistance | Standard cubemap sampling, perspective directions and reflection formulas; implementation was not copied from a tutorial. Review the six-face orientation, horizon haze and lake reflection, and disclose assistance in the report. |
| `ReflectionTarget`, mirrored scene pass and projective lake reflection | Written for this project with Codex assistance; conceptual references: Khronos OpenGL Wiki, [General OpenGL: Transformations](https://wikis.khronos.org/opengl/General_OpenGL%3A_Transformations) and [Vertex Shader Defined Outputs](https://wikis.khronos.org/opengl/Vertex_Shader/Defined_Outputs) | Original framebuffer ownership, mirrored-view integration, above-water clipping and ripple-driven projective sampling. The references describe standard mirror transforms and clip-distance semantics; no tutorial implementation was copied. Review the projection, shoreline alignment and GL state restoration, and disclose assistance in the report. |
| Folded aurora geometry and `shaders/aurora.*` | Written for this project with Codex assistance | Original procedural curtain meshes, animated folds, vertical rays, colour variation and fades, integrated into the scene and planar reflection. No aurora photograph, imported model, texture asset or tutorial implementation was used. Team review and AI-use disclosure remain required. |
| Initial terrain, lake, aurora and prop shaders | Written for this project with Codex assistance | Review and tuning by the team are required; disclose assistance in the report. |
| Initial lake basin, water surface and `shaders/lake.*` | Written for this project with Claude assistance | Basin carved by `scripts/generate_heightmap.py`; wave normals, Schlick fresnel and Blinn-Phong are standard formulations reproduced from the course material, not copied code. Review and tuning by the team are required; disclose assistance in the report. |
| Revised lake ripple model in `shaders/lake.frag` | Written for this project with Codex assistance; conceptual reference: Mark Finch, [GPU Gems, Chapter 1: Effective Water Simulation from Physical Models](https://developer.nvidia.com/gpugems/gpugems/part-i-natural-effects/chapter-1-effective-water-simulation-physical-models) | Original 16-wave implementation with analytic wind/packet and phase-bend gradients, mild crest shaping and `fwidth` filtering. The reference supplies wave-combination, dispersion and analytic-normal concepts; no tutorial implementation was copied. This is a visual approximation on a flat mesh, not a full fluid or shoreline simulation. |
| Initial scene geometry and application modules | Written for this project with Codex assistance | Basic mathematical geometry; no imported model assets in the first milestone. |
| Night-sky cubemap `assets/skybox/{px,nx,py,ny,pz,nz}.png` | Generated for this project by `scripts/generate_skybox.py` (written with Claude assistance) | Procedural: background gradient, value-noise Milky Way band and random stars, all computed from a 3D direction so faces meet without seams. No photograph, third-party image or AI image generator was used. 1024×1024 per face. |
| `src/Texture.*`, `src/Skybox.*`, `shaders/skybox.*` | Written for this project with Claude assistance | Standard OpenGL cubemap/skybox technique (translation-free view, `.xyww` depth, `GL_LEQUAL`) as taught in the course; no code copied. Review by the team required; disclose assistance in the report. |

No grass/rock/snow texture pack, tent texture or third-party model has been included in this milestone. Add author, source link, license and any modifications here when selecting those assets. If course or LearnOpenGL code is adapted later, cite the exact source here and next to the adapted code.

The repository does not yet declare a license for the team's original application code. Third-party license notices apply to their respective files; adding them does not license the team's work automatically.
