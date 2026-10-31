# Credits and provenance

Record every new third-party asset or adapted snippet here before it is included in a submission. Preserve the original copyright and license files beside bundled source.

## Libraries

| Component | Author / project | Upstream | Version | License |
|---|---|---|---|---|
| GLFW | GLFW contributors | [GLFW repository](https://github.com/glfw/glfw) | 3.4 | zlib/libpng; see `external/glfw/LICENSE.md` |
| GLAD generator and generated loader | David Herberth and contributors; Khronos registry contributors | [GLAD repository](https://github.com/Dav1dde/glad/tree/v0.1.36) | 0.1.36; OpenGL 3.3 core, no extensions | Generator MIT; Khronos notices retained; see `external/glad/LICENSE` and `external/glad/include/KHR/khrplatform.h` |
| GLM | G-Truc Creation and GLM contributors | [GLM repository](https://github.com/g-truc/glm/tree/1.0.1) | 1.0.1 | MIT or Modified MIT; see `external/glm/copying.txt` |
| stb_image | Sean Barrett and contributors | [stb source snapshot](https://github.com/nothings/stb/tree/003501f6d7a8668ce8b6d875e34dd97ad0d5d5b5) | Commit `003501f6d7a8668ce8b6d875e34dd97ad0d5d5b5` | MIT or public domain; see `external/stb/LICENSE` and `stb_image.h` |

The project builds GLFW and generated GLAD source locally; no precompiled library binaries are distributed. GLM is used for matrices/vectors, and stb_image decodes the starter heightmap. Download URLs and SHA-256 checksums are recorded in [`external/versions.json`](external/versions.json); the GLAD regeneration command is in [`external/README.md`](external/README.md).

## Assets and project code

| Item | Origin | Notes |
|---|---|---|
| Starter grayscale heightmap | Generated for this project by `scripts/generate_heightmap.py` | Deterministic mathematical terrain and lake basin; source recipe is included. No external photograph or heightmap was used. |
| Initial terrain, lake, aurora and prop shaders | Written for this project with Codex assistance | Review and tuning by the team are required; disclose assistance in the report. |
| Lake basin, water surface and `shaders/lake.*` | Written for this project with Claude assistance | Basin carved by `scripts/generate_heightmap.py`; wave normals, Schlick fresnel and Blinn-Phong are standard formulations reproduced from the course material, not copied code. Review and tuning by the team are required; disclose assistance in the report. |
| Initial scene geometry and application modules | Written for this project with Codex assistance | Basic mathematical geometry; no imported model assets in the first milestone. |

No skybox, grass/rock/snow texture pack, tent texture or third-party model has been included in this milestone. Add author, source link, license and any modifications here when selecting those assets. If course or LearnOpenGL code is adapted later, cite the exact source here and next to the adapted code.

The repository does not yet declare a license for the team's original application code. Third-party license notices apply to their respective files; adding them does not license the team's work automatically.
