#version 330 core
in vec3 vPosition;
in vec3 vNormal;
in float vAlong;
uniform vec3 uCameraPosition;
uniform vec3 uBeamColor;
uniform vec3 uFogColor;
uniform float uTerrainHalfSize;
uniform float uWaterLevel;
out vec4 fragColor;

void main() {
    // The shaft is lit air and has to stop at the surface. The water writes no
    // depth, so without this the cone would be drawn straight through the lake;
    // what the beam does to the water is the lake shader's business.
    if (vPosition.y < uWaterLevel) discard;
    vec3 v = normalize(uCameraPosition - vPosition);
    // The mesh is a hollow cone standing in for a cone of lit air. Where the
    // line of sight grazes the shell it stays inside the beam for longer, so
    // that is where the shaft has to look thickest. Both sides of the shell are
    // drawn - culling is off - and add up to the depth of the volume.
    float grazing = pow(1.0 - abs(dot(normalize(vNormal), v)), 1.5);
    // The same light spread over a wider cone, so the far end is the faintest.
    float shaft = (0.22 + 0.78 * grazing) * pow(1.0 - vAlong, 1.8);
    // A tight halo on the lamp itself, where the beam has not spread yet.
    float halo = exp(-16.0 * vAlong);
    // Lit air is still air: the fog that hides the far terrain hides it too.
    float distanceFog = smoothstep(95.0, 230.0, length(uCameraPosition - vPosition));
    float edge = max(abs(vPosition.x), abs(vPosition.z));
    float edgeFog = smoothstep(uTerrainHalfSize - 75.0, uTerrainHalfSize - 20.0, edge);
    float fog = 1.0 - (1.0 - distanceFog) * (1.0 - edgeFog);
    // Additive pass: the alpha is how much of uBeamColor this fragment adds.
    fragColor = vec4(uBeamColor, (0.15 * shaft + 0.26 * halo) * (1.0 - fog));
}
