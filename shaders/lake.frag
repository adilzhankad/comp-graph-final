#version 330 core
in vec3 vPosition;
in vec4 vReflectionClip;
uniform vec3 uCameraPosition;
uniform vec3 uLightDirection;
uniform vec3 uFogColor;
uniform float uTerrainHalfSize;
uniform float uTime;
uniform samplerCube uSkybox;
uniform sampler2D uReflection;
uniform int uHasReflection;
uniform mat4 uView;
uniform mat4 uProjection;
out vec4 fragColor;

// Directional gravity-wave spectrum: long undulations, wind ripples, then
// weaker cross-waves. .z is wave number (radians/world unit), .w is k*A.
// The mesh stays at water level; only the analytic surface slope is shaded.
// Physical background: GPU Gems 1, chapter 1 (Mark Finch):
// https://developer.nvidia.com/gpugems/gpugems/part-i-natural-effects/chapter-1-effective-water-simulation-physical-models
// This implementation and the packet/crest tuning are original to this project.
const vec4 WAVES[16] = vec4[16](
    vec4( 0.93,  0.37,  0.32, 0.047),
    vec4( 0.76,  0.65,  0.49, 0.050),
    vec4( 0.99,  0.10,  0.73, 0.057),
    vec4( 0.64,  0.77,  1.03, 0.070),
    vec4( 0.90, -0.44,  1.43, 0.072),
    vec4( 0.98,  0.20,  1.97, 0.070),
    vec4( 0.73,  0.68,  2.71, 0.064),
    vec4( 0.99, -0.12,  3.83, 0.057),
    vec4( 0.84,  0.54,  5.29, 0.050),
    vec4( 0.96, -0.28,  7.37, 0.043),
    vec4( 0.63,  0.78, 10.13, 0.034),
    vec4( 0.94,  0.34, 13.91, 0.024),
    vec4(-0.41,  0.91,  0.87, 0.043),
    vec4(-0.80,  0.60,  1.69, 0.043),
    vec4( 0.12, -0.99,  4.31, 0.033),
    vec4(-0.66, -0.75,  8.63, 0.018));

float windHash(vec2 cell) {
    return fract(sin(dot(cell, vec2(127.1, 311.7))) * 43758.5453);
}

// Smooth wind strength and its analytic x/z gradient. This only modulates
// broad patches: wave crests themselves propagate, rather than boiling noise.
vec3 windField(vec2 p) {
    vec2 cell = floor(p);
    vec2 f = fract(p);
    vec2 u = f * f * f * (f * (6.0 * f - 15.0) + 10.0);
    vec2 du = 30.0 * f * f * (f - 1.0) * (f - 1.0);
    float a = windHash(cell);
    float b = windHash(cell + vec2(1.0, 0.0));
    float c = windHash(cell + vec2(0.0, 1.0));
    float d = windHash(cell + vec2(1.0, 1.0));
    float crossTerm = a - b - c + d;
    float value = a + (b - a) * u.x + (c - a) * u.y + crossTerm * u.x * u.y;
    vec2 gradient = (vec2(b - a, c - a) + crossTerm * u.yx) * du;
    return vec3(value * 2.0 - 1.0, gradient * 2.0);
}

vec3 rippleNormal(vec2 p, float t) {
    vec2 wind = normalize(vec2(0.92, 0.39));
    vec3 gust = windField(p * 0.07 - wind * t * 0.045);
    float windStrength = 0.80 + 0.25 * gust.x;
    vec2 windGradient = 0.25 * 0.07 * gust.yz;
    vec2 slope = vec2(0.0);
    for (int i = 0; i < 16; ++i) {
        vec2 direction = normalize(WAVES[i].xy);
        vec2 across = vec2(-direction.y, direction.x);
        float k = WAVES[i].z;
        // Keep the layered motion, but soften fine relief to suit the broad
        // shapes of the low-poly landscape and leave the sky reflection legible.
        float detailWeight = mix(0.80, 0.45, smoothstep(1.8, 9.0, k));
        float amplitude = WAVES[i].w / k * detailWeight;
        float omega = sqrt(9.81 * k);
        float seed = float(i) * 2.39996323;
        // A bounded, slowly advected phase offset gently bends crests. Its
        // derivative is included below; it never rotates or reseeds the waves.
        float bend = 0.8 * k / (k + 0.5) * sin(seed + 0.7);
        float phase = dot(direction, p) * k - omega * t + seed + bend * gust.x;
        vec2 phaseGradient = k * direction + bend * 0.07 * gust.yz;

        // Broad wave groups move at the deep-water group velocity, half the
        // phase velocity. Different packet axes/phases break uniform stripes.
        vec2 packetK = direction * (0.20 + 0.013 * float(i)) + across * 0.11;
        float packetPhase = dot(packetK, p - direction * (0.5 * omega / k) * t) + seed * 1.71;
        float packet = 0.75 + 0.25 * sin(packetPhase);
        vec2 packetGradient = 0.25 * cos(packetPhase) * packetK;
        float envelope = packet * windStrength;
        vec2 envelopeGradient = packetGradient * windStrength + packet * windGradient;

        // Remove subpixel waves (and their shorter second harmonic) before
        // they turn the reflected stars into temporal sparkle at a distance.
        float footprint = fwidth(phase);
        float resolved = 1.0 - smoothstep(0.8, 2.4, footprint);
        float harmonicResolved = 1.0 - smoothstep(0.8, 2.4, footprint * 2.0);
        float s = sin(phase);
        float c = cos(phase);
        // A mild second harmonic gives narrower crests and broader troughs.
        float height = resolved * s - 0.08 * harmonicResolved * (1.0 - 2.0 * s * s);
        float derivative = resolved * c + 0.32 * harmonicResolved * s * c;
        slope += amplitude * (envelope * derivative * phaseGradient + height * envelopeGradient);
    }
    return normalize(vec3(-slope.x, 1.0, -slope.y));
}

// Match skybox.frag, including the horizon haze. Both use display-space RGB
// so the stars and their reflection receive the same brightness and colour.
vec3 sky(vec3 direction) {
    direction = normalize(direction);
    vec3 stars = texture(uSkybox, direction).rgb;
    vec3 night = vec3(0.008, 0.014, 0.032) + stars * vec3(0.75, 0.80, 0.90);
    return mix(uFogColor, night, smoothstep(0.0, 0.22, direction.y));
}

vec3 reflectedScene(vec3 n, vec3 v) {
    vec3 reflectedRay = reflect(-v, n);
    vec3 fallback = sky(reflectedRay);
    if (uHasReflection == 0) return fallback;

    vec2 uv = vReflectionClip.xy / max(vReflectionClip.w, 0.0001) * 0.5 + 0.5;
    float edge = min(min(uv.x, 1.0 - uv.x), min(uv.y, 1.0 - uv.y));
    // Distort by the change in reflected ray, not the absolute view normal:
    // a flat surface then joins the shoreline without an arbitrary UV shift.
    vec3 flatRay = reflect(-v, vec3(0.0, 1.0, 0.0));
    vec3 rayDelta = reflectedRay - flatRay;
    // The target camera includes the reflection about the horizontal plane.
    vec2 rayChange = (mat3(uView) * vec3(rayDelta.x, -rayDelta.y, rayDelta.z)).xy;
    vec2 distortion = rayChange * vec2(uProjection[0][0], uProjection[1][1]) * 0.025;
    distortion *= smoothstep(0.0, 0.04, edge);
    distortion *= mix(0.25, 1.0, smoothstep(0.0, 0.25, abs(v.y)));
    vec2 texel = 1.0 / vec2(textureSize(uReflection, 0));
    // Fade large offsets smoothly before they can sample beyond the target,
    // including at narrow camera FOVs; avoid stretching its edge texels.
    vec2 margin = max(min(uv, vec2(1.0) - uv) - texel * 0.5, vec2(0.0));
    distortion *= margin / (margin + abs(distortion) + vec2(0.000001));
    vec2 sampleUV = clamp(uv + distortion, texel * 0.5, vec2(1.0) - texel * 0.5);
    vec3 reflected = texture(uReflection, sampleUV).rgb;
    float valid = step(0.0001, vReflectionClip.w) * step(0.0, edge);
    return mix(fallback, reflected, valid);
}

void main() {
    vec3 n = rippleNormal(vPosition.xz, uTime);
    vec3 v = normalize(uCameraPosition - vPosition);
    if (dot(n, v) < 0.0) n = -n;            // the surface is also seen from below
    vec3 l = normalize(uLightDirection);
    vec3 h = normalize(l + v);

    // Body colour of the water, shaded in linear space.
    vec3 body = vec3(0.006, 0.013, 0.019);
    body += vec3(0.004, 0.009, 0.012) * max(dot(n, l), 0.0);
    // A softer, restrained moonlight lobe shares the landscape's muted palette.
    body += vec3(0.38, 0.48, 0.58) * pow(max(dot(n, h), 0.0), 112.0) * 0.7;
    body = pow(max(body, vec3(0.0)), vec3(1.0 / 2.2));

    // Schlick's approximation: water turns mirror-like at grazing angles.
    float fresnel = 0.02 + 0.98 * pow(1.0 - max(dot(n, v), 0.0), 5.0);
    // Artistic reflection floor: retain the angular Fresnel response while
    // making the reflected landscape visible across more of the lake.
    float reflectionWeight = mix(0.20, 1.0, fresnel);
    // Shallow water shows the bed through it; a grazing view hides it.
    float alpha = mix(0.82, 0.98, fresnel);
    // The reflected terrain already contains fog along the mirrored sightline.
    // Fog the water body separately instead of fading the mountains twice.
    float distanceFog = smoothstep(95.0, 230.0, length(uCameraPosition - vPosition));
    float edge = max(abs(vPosition.x), abs(vPosition.z));
    float edgeFog = smoothstep(uTerrainHalfSize - 75.0, uTerrainHalfSize - 20.0, edge);
    float fog = 1.0 - (1.0 - distanceFog) * (1.0 - edgeFog);
    vec3 color = mix(mix(body, uFogColor, distanceFog), reflectedScene(n, v), reflectionWeight);
    fragColor = vec4(mix(color, uFogColor, edgeFog), mix(alpha, 1.0, fog));
}
