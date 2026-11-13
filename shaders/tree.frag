#version 330 core
#include "beacon.glsl"

in vec3 vPosition;
in vec3 vNormal;

uniform vec3 uCameraPosition;
uniform vec3 uLightDirection;
uniform vec3 uFogColor;
uniform float uTerrainHalfSize;

// Pine colours — two materials selected by the caller via uIsTrunk.
// Trunk: warm dark brown; canopy: deep cold pine-green.
uniform int uIsTrunk;

out vec4 fragColor;

void main() {
    vec3 n = normalize(vNormal);
    vec3 v = normalize(uCameraPosition - vPosition);
    vec3 l = normalize(uLightDirection);

    // Material colour chosen by part type.
    vec3 trunkColor  = vec3(0.18, 0.13, 0.09);   // dark bark brown
    vec3 canopyColor = vec3(0.09, 0.17, 0.11);   // deep pine green

    vec3 albedo = (uIsTrunk != 0) ? trunkColor : canopyColor;

    // Same ambient + moonlight model as object.frag so trees belong to the
    // same night scene as the lighthouse and terrain.
    float skyFill = 0.5 + 0.5 * n.y;
    vec3 ambient  = mix(vec3(0.035, 0.05, 0.085), vec3(0.09, 0.12, 0.18), skyFill);
    vec3 moon     = vec3(0.48, 0.62, 0.90);

    vec3 color = albedo * ambient;
    color += albedo * moon * max(dot(n, l), 0.0) * 0.34;
    // Blinn-Phong lobe — trees are matte, so keep the exponent low.
    color += moon * pow(max(dot(n, normalize(l + v)), 0.0), 12.0) * 0.015;

    // Lighthouse beacon illumination (same as object.frag).
    vec3 toLamp;
    vec3 beacon = beaconLight(vPosition, toLamp);
    color += albedo * beacon * max(dot(n, toLamp), 0.0);

    color = pow(max(color, vec3(0.0)), vec3(1.0 / 2.2));

    // Fog — identical fade curve to the terrain and props.
    float distanceFog = smoothstep(95.0, 230.0, length(uCameraPosition - vPosition));
    float edge        = max(abs(vPosition.x), abs(vPosition.z));
    float edgeFog     = smoothstep(uTerrainHalfSize - 75.0, uTerrainHalfSize - 20.0, edge);
    fragColor = vec4(mix(color, uFogColor,
                         1.0 - (1.0 - distanceFog) * (1.0 - edgeFog)), 1.0);
}
