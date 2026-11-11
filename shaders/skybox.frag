#version 330 core
in vec3 vDirection;
uniform samplerCube uSky;
uniform vec3 uFogColor;
out vec4 fragColor;

void main() {
    vec3 direction = normalize(vDirection);
    // The cubemap stores display-space colours, like the fog colour.
    vec3 sky = texture(uSky, direction).rgb;
    // The ground fades into uFogColor at its far edge. Haze that reaches the
    // same colour at the horizon hides where the terrain mesh ends, and
    // follows whatever fog colour the scene uses without regenerating faces.
    float haze = 1.0 - smoothstep(-0.02, 0.22, direction.y);
    fragColor = vec4(mix(sky, uFogColor, haze), 1.0);
}
