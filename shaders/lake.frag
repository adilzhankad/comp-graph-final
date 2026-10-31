#version 330 core
in vec3 vPosition;
uniform vec3 uCameraPosition;
uniform vec3 uLightDirection;
uniform vec3 uFogColor;
uniform float uTerrainHalfSize;
uniform float uTime;
out vec4 fragColor;

// Crossing wave trains. Only their slope is needed, so the surface stays a
// flat mesh and every ripple is produced per fragment from the analytic
// gradient of h(p, t) = sum a_i * sin(dot(d_i, p) * k_i + t * w_i).
const vec4 WAVES[4] = vec4[4](
    // direction.xy, frequency, speed
    vec4( 0.98,  0.20, 0.42, 0.95),
    vec4(-0.62,  0.78, 0.67, 1.31),
    vec4( 0.31, -0.95, 1.13, 0.74),
    vec4(-0.87, -0.49, 1.90, 1.73));
const float AMPLITUDE[4] = float[4](0.075, 0.052, 0.028, 0.016);

vec3 rippleNormal(vec2 p, float t) {
    vec2 slope = vec2(0.0);
    for (int i = 0; i < 4; ++i) {
        vec2 direction = WAVES[i].xy;
        float frequency = WAVES[i].z;
        float phase = dot(direction, p) * frequency + t * WAVES[i].w;
        slope += AMPLITUDE[i] * frequency * direction * cos(phase);
    }
    return normalize(vec3(-slope.x, 1.0, -slope.y));
}

// Stand-in for the planned skybox: the overcast tone the terrain fades into,
// lifted at the horizon and deepened overhead. Already in display space, so
// grazing water and the background meet in the same colour.
vec3 sky(vec3 direction) {
    return uFogColor * (1.02 - 0.55 * sqrt(max(direction.y, 0.0)));
}

void main() {
    vec3 n = rippleNormal(vPosition.xz, uTime);
    vec3 v = normalize(uCameraPosition - vPosition);
    if (dot(n, v) < 0.0) n = -n;            // the surface is also seen from below
    vec3 l = normalize(uLightDirection);
    vec3 h = normalize(l + v);

    // Body colour of the water, shaded in linear space.
    vec3 body = vec3(0.008, 0.030, 0.042);
    body += vec3(0.020, 0.062, 0.070) * max(dot(n, l), 0.0);
    // One narrow Blinn-Phong lobe: the sun leaves a long glittering streak.
    body += vec3(0.95, 0.86, 0.70) * pow(max(dot(n, h), 0.0), 180.0) * 3.2;
    body = pow(max(body, vec3(0.0)), vec3(1.0 / 2.2));

    // Schlick's approximation: water turns mirror-like at grazing angles.
    float fresnel = 0.02 + 0.98 * pow(1.0 - max(dot(n, v), 0.0), 5.0);
    vec3 color = mix(body, sky(reflect(-v, n)), fresnel);

    // Shallow water shows the bed through it; a grazing view hides it.
    float alpha = mix(0.70, 0.97, fresnel);
    // Same fade as terrain.frag, so shore and water reach the background together.
    float distanceFog = smoothstep(95.0, 230.0, length(uCameraPosition - vPosition));
    float edge = max(abs(vPosition.x), abs(vPosition.z));
    float edgeFog = smoothstep(uTerrainHalfSize - 75.0, uTerrainHalfSize - 20.0, edge);
    float fog = 1.0 - (1.0 - distanceFog) * (1.0 - edgeFog);
    fragColor = vec4(mix(color, uFogColor, fog), mix(alpha, 1.0, fog));
}
