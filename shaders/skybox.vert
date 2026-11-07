#version 330 core
layout(location = 0) in vec3 aPosition;
uniform mat4 uView;          // rotation only, translation removed on the CPU
uniform mat4 uProjection;
out vec3 vDirection;
void main() {
    // A cube corner seen from the centre is also the direction to sample.
    vDirection = aPosition;
    // z = w makes depth exactly 1.0 after the perspective divide, so the sky
    // sits behind everything already drawn.
    gl_Position = (uProjection * uView * vec4(aPosition, 1.0)).xyww;
}
