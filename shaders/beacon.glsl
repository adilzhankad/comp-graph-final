// The rotating lighthouse lamp as a spot light. Included by terrain.frag,
// lake.frag and object.frag, so the beam falls off identically on the ground,
// on the water and on the tower itself.
//
// uBeaconColor is already zero while the lamp is switched off, so nothing here
// needs a branch, and uBeaconCone packs the three numbers src/Beacon.h stores:
//   .x  cosine of the outer half-angle - the edge of the cone
//   .y  cosine of the inner half-angle - its fully lit core
//   .z  range in world units, over the last quarter of which the beam fades
uniform vec3 uBeaconPosition;
uniform vec3 uBeaconDirection;
uniform vec3 uBeaconColor;
uniform vec3 uBeaconCone;

// Light reaching `point` from the lamp, before the surface's own cosine term.
// `toLamp` is returned for that term and for the specular half vector.
vec3 beaconLight(vec3 point, out vec3 toLamp) {
    vec3 offset = uBeaconPosition - point;
    float distance = max(length(offset), 0.001);
    toLamp = offset / distance;
    // How far inside the cone the point lies; smoothstep softens its rim.
    float cone = smoothstep(uBeaconCone.x, uBeaconCone.y, dot(-toLamp, uBeaconDirection));
    // The constant term keeps the lamp from blowing out whatever stands beside
    // it, and the quadratic one makes the far shore dimmer than the near bank.
    float attenuation = 1.0 / (1.0 + 0.004 * distance + 0.00012 * distance * distance);
    // Light must not reach past the cone that is actually drawn.
    float reach = 1.0 - smoothstep(uBeaconCone.z * 0.75, uBeaconCone.z, distance);
    return uBeaconColor * (cone * attenuation * reach);
}
