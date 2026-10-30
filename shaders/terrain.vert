#version 330 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
flat out vec3 vFaceNormal;
flat out vec3 vFaceAnchor;
out vec3 vPosition;
void main() {
    vec4 world = uModel * vec4(aPosition, 1.0);
    vFaceNormal = mat3(transpose(inverse(uModel))) * aNormal;
    // Flat interpolation keeps material constant within a face.
    vFaceAnchor = world.xyz;
    vPosition = world.xyz;
    gl_Position = uProjection * uView * world;
}
