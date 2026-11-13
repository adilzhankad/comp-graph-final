#version 330 core
layout(location = 0) in vec3 aPosition;
uniform mat4 uView;
uniform mat4 uProjection;
uniform vec3 uCameraPosition;
uniform vec3 uHead;
uniform vec3 uTail;
uniform float uHalfWidth;
out float vAlong;
out float vAcross;
flat out float vLength;

void main() {
    vec3 displacement = uHead - uTail;
    float lengthAlong = max(length(displacement), 0.00001);
    vec3 axis = dot(displacement, displacement) > 0.0000000001
              ? displacement / lengthAlong : vec3(1.0, 0.0, 0.0);
    vec3 towardEye = uCameraPosition - (uHead + uTail) * 0.5;
    vec3 side = cross(axis, towardEye);
    if (dot(side, side) < 0.000001) side = cross(axis, vec3(0.0, 1.0, 0.0));
    if (dot(side, side) < 0.000001) side = cross(axis, vec3(1.0, 0.0, 0.0));
    side = normalize(side);

    // Extra space on either end lets the head and tail fade to transparent.
    // The same world-space head/tail is billboarded for the mirrored eye too.
    float padding = uHalfWidth * 2.5;
    vAlong = mix(-padding, lengthAlong + padding, aPosition.x);
    vAcross = aPosition.y * padding;
    vLength = lengthAlong;
    vec3 world = uTail + axis * vAlong + side * vAcross;
    gl_Position = uProjection * uView * vec4(world, 1.0);
}
