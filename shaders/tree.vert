#version 330 core

// Per-vertex geometry (same layout as every other mesh in the project).
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

// Per-instance model matrix, split across four consecutive vec4 attribute
// slots (a mat4 always occupies four locations in OpenGL).
// glVertexAttribDivisor(3..6, 1) is set in InstancedMesh so the driver
// advances this four-column block once per instance instead of per vertex.
layout(location = 3) in vec4 aInstanceCol0;
layout(location = 4) in vec4 aInstanceCol1;
layout(location = 5) in vec4 aInstanceCol2;
layout(location = 6) in vec4 aInstanceCol3;

uniform mat4 uView;
uniform mat4 uProjection;

out vec3 vPosition;
out vec3 vNormal;

void main() {
    // Reconstruct the per-instance model matrix from its four column vectors.
    mat4 model = mat4(aInstanceCol0, aInstanceCol1, aInstanceCol2, aInstanceCol3);

    vec4 worldPos = model * vec4(aPosition, 1.0);
    vPosition = worldPos.xyz;

    // Correct normal transform for non-uniform scale (transpose of inverse of
    // the upper-left 3×3).
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    vNormal = normalMatrix * aNormal;

    gl_Position = uProjection * uView * worldPos;
}
