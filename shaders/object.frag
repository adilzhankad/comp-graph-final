#version 330 core
#include "beacon.glsl"
in vec3 vPosition;
in vec3 vNormal;
uniform vec3 uCameraPosition;
uniform vec3 uLightDirection;
uniform vec3 uFogColor;
uniform float uTerrainHalfSize;
// Material: albedo, plus the light a surface gives off by itself. The lamp in
// the lantern room is lit from the inside, not by the scene.
uniform vec3 uColor;
uniform float uEmission;
out vec4 fragColor;

void main() {
    vec3 n = normalize(vNormal);
    vec3 v = normalize(uCameraPosition - vPosition);
    vec3 l = normalize(uLightDirection);
    // The terrain's sky fill and moonlight, so a prop belongs to the same night.
    float skyFill = 0.5 + 0.5 * n.y;
    vec3 ambient = mix(vec3(0.035, 0.05, 0.085), vec3(0.09, 0.12, 0.18), skyFill);
    vec3 moon = vec3(0.48, 0.62, 0.90);
    vec3 color = uColor * ambient + uColor * uEmission;
    color += uColor * moon * max(dot(n, l), 0.0) * 0.34;
    // Props are painted and smoother than bare rock, so unlike the terrain they
    // get a Blinn-Phong lobe: the half vector h = normalize(l + v).
    color += moon * pow(max(dot(n, normalize(l + v)), 0.0), 48.0) * 0.06;
    // Second light: the beam lights the gallery and the tower under it.
    vec3 toLamp;
    vec3 beacon = beaconLight(vPosition, toLamp);
    color += uColor * beacon * max(dot(n, toLamp), 0.0);
    color += beacon * pow(max(dot(n, normalize(toLamp + v)), 0.0), 48.0) * 0.10;
    color = pow(max(color, vec3(0.0)), vec3(1.0 / 2.2));
    // The terrain's fade, so a distant prop goes with the ground it stands on.
    float distanceFog = smoothstep(95.0, 230.0, length(uCameraPosition - vPosition));
    float edge = max(abs(vPosition.x), abs(vPosition.z));
    float edgeFog = smoothstep(uTerrainHalfSize - 75.0, uTerrainHalfSize - 20.0, edge);
    fragColor = vec4(mix(color, uFogColor,
                         1.0 - (1.0 - distanceFog) * (1.0 - edgeFog)), 1.0);
}
