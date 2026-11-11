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
out vec3 vWorldPosition;
flat out float vCurtain;
void main() {
    float t = uTime * uSpeed;
    float id = aNormal.x;
    float phase = id * 2.37;
    float u = aUV.x;
    float rise = aUV.y * aUV.y;
    vec3 position = aPosition;
    // World-space deformation keeps the reflection exactly in step.
    position.z += sin(u * 13.0 + phase + t * 0.095) * 3.0
                + sin(u * 29.0 - t * 0.13 + phase) * (0.8 + rise * 1.5);
    float lean = id < 0.5 ? 8.0 : 4.0;
    position.x += rise * (sin(u * 10.0 + phase - t * 0.08) * lean
                          + sin(u * 23.0 + t * 0.11) * 1.0);
    position.y += sin(u * 9.0 + phase - t * 0.07) * 2.0
                + rise * sin(u * 21.0 + phase + t * 0.10) * 2.7;
    vUV = aUV;
    vCurtain = id;
    vec4 worldPosition = uModel * vec4(position, 1.0);
    vWorldPosition = worldPosition.xyz;
    gl_Position = uProjection * uView * worldPosition;
}
