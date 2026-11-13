#version 330 core
in vec2 vUV;
in vec3 vWorldPosition;
flat in float vCurtain;
uniform float uTime;
uniform float uSpeed;
uniform float uIntensity;
uniform int uPalette;
uniform vec3 uCameraPosition;
out vec4 fragColor;

float hash(vec2 p) {
    vec3 q = fract(vec3(p.xyx) * 0.1031);
    q += dot(q, q.yzx + 33.33);
    return fract((q.x + q.y) * q.z);
}

float noise(vec2 p) {
    vec2 cell = floor(p);
    vec2 f = fract(p);
    f = f * f * f * (f * (f * 6.0 - 15.0) + 10.0);
    return mix(mix(hash(cell), hash(cell + vec2(1.0, 0.0)), f.x),
               mix(hash(cell + vec2(0.0, 1.0)), hash(cell + vec2(1.0)), f.x), f.y);
}

float gaussian(float x, float width) {
    return exp(-x * x / (width * width));
}

// Average away subpixel rays, particularly in the smaller reflection texture.
float rayNoise(float coordinate, float frequency, float time) {
    float detail = noise(vec2(coordinate * frequency, time));
    float resolution = 1.0 - smoothstep(0.45, 1.5, fwidth(coordinate) * frequency);
    return mix(0.5, detail, resolution);
}

void main() {
    float u = vUV.x;
    float v = vUV.y;
    float id = vCurtain;
    float t = uTime * uSpeed;
    float phase = id * 2.37;

    // Coherent columns, gently sheared toward the top, form luminous curtains.
    // Different scales drift separately rather than translating one texture.
    float drift = noise(vec2(u * 6.0 + phase, t * 0.025));
    float flow = u + (drift - 0.5) * 0.023
                  + v * v * 0.006 * sin(u * 19.0 + phase + t * 0.10);
    float broad = noise(vec2(flow * 13.0 + phase, t * 0.045 + id * 5.0));
    float strands = rayNoise(flow, 175.0, t * 0.12 + id * 9.0);
    float fine = rayNoise(flow + 0.0015 * sin(v * 8.0 + t * 0.18),
                          420.0, t * 0.075 + id * 13.0);
    float rays = 0.28 + 0.45 * pow(strands, 1.4) + 0.14 * pow(fine, 2.0);
    float clusters = 0.58 + 0.42 * smoothstep(0.16, 0.83, broad);

    float lower = 0.12 + 0.009 * sin(u * 36.0 + phase + t * 0.13)
                       + (noise(vec2(u * 21.0 + phase, t * 0.04)) - 0.5) * 0.016;
    float altitude = v - lower;
    float rayLength = 0.68 + 0.26 * noise(vec2(flow * 24.0 + phase, t * 0.06));
    float foot = smoothstep(-0.008, 0.035, altitude);
    float tips = 1.0 - smoothstep(rayLength * 0.45, rayLength, max(altitude, 0.0));
    float body = foot * exp(-max(altitude, 0.0) * 1.75) * tips * rays;
    float veil = foot * exp(-max(altitude, 0.0) * 2.8)
               * (1.0 - smoothstep(0.40, 0.92, v)) * 0.12;
    float rim = gaussian(altitude, 0.022) * (0.72 + 0.28 * strands);
    float halo = gaussian(altitude - 0.025, 0.095) * (0.45 + broad * 0.55);
    float edges = smoothstep(0.0, 0.13, u) * smoothstep(0.0, 0.16, 1.0 - u)
                * smoothstep(0.0, 0.035, v) * (1.0 - smoothstep(0.83, 1.0, v));

    // Each palette keeps a bright lower arc and softer high-altitude rays.
    // Local colour washes travel along the same geometry in both passes.
    float tide = 0.5 + 0.5 * sin(u * 7.5 + phase - t * 0.10
                                 + 1.1 * sin(u * 3.0 + t * 0.035));
    vec3 base = vec3(0.13, 0.89, 0.38);
    vec3 accent = vec3(0.07, 0.70, 0.79);
    vec3 high = vec3(0.40, 0.20, 0.78);
    vec3 highAccent = vec3(0.78, 0.22, 0.51);
    if (uPalette == 1) {
        base = vec3(0.08, 0.30, 1.00);
        accent = vec3(0.12, 0.75, 1.00);
        high = vec3(0.33, 0.20, 0.95);
        highAccent = vec3(0.60, 0.43, 1.00);
    } else if (uPalette == 2) {
        base = vec3(1.00, 0.25, 0.045);
        accent = vec3(1.00, 0.58, 0.12);
        high = vec3(0.95, 0.18, 0.24);
        highAccent = vec3(0.90, 0.35, 0.60);
    }
    vec3 lowerColor = mix(base, accent, smoothstep(0.22, 0.91, tide) * 0.88);
    float violetPatch = smoothstep(0.32, 0.88,
        0.5 + 0.5 * sin(u * 10.0 - phase * 0.7 + t * 0.065));
    vec3 upperWash = mix(high, highAccent,
                     0.5 + 0.5 * sin(u * 5.0 + phase + t * 0.055));
    vec3 upperColor = mix(accent, upperWash, 0.30 + violetPatch * 0.70);
    vec3 bodyColor = mix(lowerColor, upperColor,
                        smoothstep(0.02, 0.40, altitude) * (0.55 + 0.45 * violetPatch));
    float breath = 0.84 + 0.16 * sin(t * 0.12 + u * 9.0 + phase);
    float strength = id < 0.5 ? 1.0 : (id < 1.5 ? 0.48 : 0.61);
    // Broad pulses run along the arc without moving or flickering the rays.
    float surge = pow(0.5 + 0.5 * sin(u * 12.0 - t * 0.22 + phase), 5.0);
    vec3 emission = lowerColor * (rim * (0.50 + 0.12 * surge) + halo * 0.18)
                  + bodyColor * (body * (1.45 + 0.20 * surge) + veil);
    // A thin sheet folds to zero projected width at grazing angles. Fade that
    // silhouette smoothly so overlapping folds do not begin with a hard bright
    // slat. The mirrored eye makes this work identically in the lake pass.
    vec3 geometricNormal = cross(dFdx(vWorldPosition), dFdy(vWorldPosition));
    geometricNormal /= max(length(geometricNormal), 0.000001);
    vec3 towardEye = uCameraPosition - vWorldPosition;
    towardEye /= max(length(towardEye), 0.000001);
    float facing = abs(dot(geometricNormal, towardEye));
    float foldFade = smoothstep(0.015, 0.20, facing);
    emission *= edges * clusters * breath * strength * foldFade * max(uIntensity, 0.0);
    // Premultiplied emission for GL_ONE, GL_ONE; no opaque surface.
    fragColor = vec4(emission, 0.0);
}
