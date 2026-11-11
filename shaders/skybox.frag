#version 330 core
in vec3 vDirection;
uniform samplerCube uSkybox;
uniform vec3 uFogColor;
out vec4 fragColor;

void main() {
    vec3 direction = normalize(vDirection);
    vec3 stars = texture(uSkybox, direction).rgb;
    // Same display-space atmosphere and star brightness as lake.frag::sky.
    vec3 night = vec3(0.008, 0.014, 0.032) + stars * vec3(0.75, 0.80, 0.90);
    // Match the terrain/water fog at the horizon. The upper sky keeps the
    // texture's stars; below the horizon the finite terrain fades into haze.
    float visibility = smoothstep(0.0, 0.22, direction.y);
    fragColor = vec4(mix(uFogColor, night, visibility), 1.0);
}
