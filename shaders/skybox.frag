#version 330 core
in vec3 vDirection;
uniform samplerCube uSkybox;
uniform vec3 uFogColor;
out vec4 fragColor;

void main() {
    vec3 direction = normalize(vDirection);
    // Preserve bright stars while quieting the dense faint background behind
    // the aurora; the lake's cubemap fallback uses the identical treatment.
    vec3 stars = pow(texture(uSkybox, direction).rgb, vec3(1.15));
    // Same display-space atmosphere and star brightness as lake.frag::sky.
    vec3 night = vec3(0.008, 0.014, 0.032) + stars * vec3(0.50, 0.57, 0.70);
    // Match the terrain/water fog at the horizon. The upper sky keeps the
    // texture's stars; below the horizon the finite terrain fades into haze.
    float visibility = smoothstep(0.0, 0.22, direction.y);
    fragColor = vec4(mix(uFogColor, night, visibility), 1.0);
}
