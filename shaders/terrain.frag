#version 330 core
flat in vec3 vFaceNormal;
flat in vec3 vFaceAnchor;
in vec3 vPosition;
uniform vec3 uCameraPosition;
uniform vec3 uLightDirection;
uniform float uTerrainHalfSize;
out vec4 fragColor;

void main() {
    vec3 n = normalize(vFaceNormal);
    float height = vFaceAnchor.y;
    float slope = 1.0 - n.y;
    vec3 meadow = vec3(0.12, 0.21, 0.16);
    vec3 scree = vec3(0.26, 0.31, 0.29);
    vec3 rock = vec3(0.37, 0.43, 0.45);
    vec3 snow = vec3(0.79, 0.86, 0.85);
    vec3 base = mix(meadow, scree, smoothstep(0.5, 8.0, height));
    float rockWeight = max(smoothstep(7.0, 17.0, height), smoothstep(0.18, 0.52, slope));
    base = mix(base, rock, rockWeight);
    float snowWeight = smoothstep(18.5, 24.5, height) * (1.0 - smoothstep(.40, .85, slope));
    base = mix(base, snow, snowWeight);

    // A small deterministic variation adds colour blocks without a noisy texture.
    float variation = fract(sin(dot(vFaceAnchor.xz, vec2(12.9898, 78.233))) * 43758.5453);
    base *= 0.94 + 0.12 * variation;
    float diffuse = max(dot(n, normalize(uLightDirection)), 0.0);
    float skyFill = 0.5 + 0.5 * n.y;
    vec3 ambient = mix(vec3(0.13, 0.16, 0.18), vec3(0.30, 0.36, 0.40), skyFill);
    vec3 color = base * (ambient + vec3(0.95, 0.86, 0.70) * diffuse * 0.85);
    color = pow(max(color, vec3(0.0)), vec3(1.0 / 2.2));
    // Lighting stays flat per triangle; fog blends smoothly across the surface.
    float distanceFog = smoothstep(95.0, 230.0, length(uCameraPosition - vPosition));
    // Distance fog alone would expose the edge when the camera approaches it.
    // This fixed outer band reaches the exact background colour before the mesh ends.
    float edge = max(abs(vPosition.x), abs(vPosition.z));
    float edgeFog = smoothstep(uTerrainHalfSize - 75.0, uTerrainHalfSize - 20.0, edge);
    float fog = 1.0 - (1.0 - distanceFog) * (1.0 - edgeFog);
    color = mix(color, vec3(0.18, 0.25, 0.29), fog);
    fragColor = vec4(color, 1.0);
}
