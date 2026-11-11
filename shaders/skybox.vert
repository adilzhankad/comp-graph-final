#version 330 core
layout(location = 0) in vec3 aPosition;
uniform mat4 uView;
uniform mat4 uProjection;
out vec3 vDirection;

void main() {
    vDirection = aPosition;
    // The distant sky rotates with the view but never follows camera movement.
    vec4 clip = uProjection * mat4(mat3(uView)) * vec4(aPosition, 1.0);
    gl_Position = clip.xyww;
}
