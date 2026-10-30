#version 330 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform float uTime;
uniform float uSpeed;
out vec2 vUV;
void main() {
    vec3 position = aPosition;
    position.z += sin(aUV.x * 19.0 + uTime * uSpeed * 0.35) * (0.5 + aUV.y);
    position.y += sin(aUV.x * 11.0 - uTime * uSpeed * 0.22) * 0.6;
    vUV = aUV;
    gl_Position = uProjection * uView * uModel * vec4(position, 1.0);
}
