#version 330 core
in vec2 vUV;
uniform float uTime;
uniform float uSpeed;
uniform float uIntensity;
out vec4 fragColor;
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}
float noise(vec2 p) {
    vec2 cell = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix(mix(hash(cell), hash(cell + vec2(1, 0)), f.x),
               mix(hash(cell + vec2(0, 1)), hash(cell + vec2(1, 1)), f.x), f.y);
}
void main() {
    float t = uTime * uSpeed;
    float drift = noise(vec2(vUV.x * 6.0, t * 0.08));
    float streaks = noise(vec2(vUV.x * 100.0 + drift * 3.0, t * 0.35));
    streaks = 0.25 + 0.75 * streaks * streaks;
    streaks *= 0.75 + 0.25 * sin(vUV.x * 380.0 + t * 0.7);
    float verticalFade = smoothstep(0.0, 0.035, vUV.y) * pow(1.0 - vUV.y, 1.6);
    float sideFade = smoothstep(0.0, 0.10, vUV.x) * smoothstep(0.0, 0.10, 1.0-vUV.x);
    vec3 color = mix(vec3(0.12, 0.95, 0.36), vec3(0.12, 0.62, 0.85),
                     smoothstep(0.05, 0.58, vUV.y));
    color = mix(color, vec3(0.62, 0.20, 0.78), smoothstep(0.45, 1.0, vUV.y));
    float alpha = clamp(uIntensity * streaks * verticalFade * sideFade * 0.85, 0.0, 1.0);
    fragColor = vec4(color, alpha);
}
