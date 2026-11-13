#version 330 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
out vec3 vPosition;
out vec3 vNormal;
// 0 at the lamp, 1 at the mouth of the cone; the primitive stores it in UV.x.
out float vAlong;
void main() {
    vec4 world = uModel * vec4(aPosition, 1.0);
    vPosition = world.xyz;
    vNormal = mat3(transpose(inverse(uModel))) * aNormal;
    vAlong = aUV.x;
    gl_Position = uProjection * uView * world;
}
