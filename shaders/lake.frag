#version 330 core
in vec3 vPosition;
uniform vec3 uCameraPosition;
uniform vec3 uMoonDirection;
uniform vec3 uMoonColor;
uniform vec3 uFirePosition;
uniform vec3 uFireColor;
uniform float uTime;
out vec4 fragColor;
vec3 lightWater(vec3 n, vec3 v, vec3 l, vec3 light) {
    float diffuse = max(dot(n, l), 0.0);
    float specular = diffuse > 0.0
        ? pow(max(dot(n, normalize(l + v)), 0.0), 160.0) : 0.0;
    return light * (vec3(0.015, 0.05, 0.07) * diffuse + 1.8 * specular);
}
void main() {
    // A flat plane with analytic normal ripples; no reflection framebuffer yet.
    float nx = 0.045 * cos(vPosition.x * 0.9 + uTime * 0.7)
             + 0.025 * cos(vPosition.z * 1.4 - uTime * 0.4);
    float nz = 0.04 * sin(vPosition.z * 1.1 - uTime * 0.6)
             + 0.02 * sin(vPosition.x * 0.8 + uTime * 0.5);
    vec3 n = normalize(vec3(nx, 1.0, nz));
    if (!gl_FrontFacing) n = -n;
    vec3 v = normalize(uCameraPosition - vPosition);
    float fresnel = 0.02 + 0.98 * pow(1.0 - max(dot(n, v), 0.0), 5.0);
    vec3 color = mix(vec3(0.012, 0.04, 0.055), vec3(0.06, 0.10, 0.16), fresnel);
    color += lightWater(n, v, normalize(uMoonDirection), uMoonColor);
    vec3 toFire = uFirePosition - vPosition;
    float d = length(toFire);
    color += lightWater(n, v, normalize(toFire),
                        uFireColor / (1.0 + 0.14 * d + 0.06 * d * d));
    fragColor = vec4(pow(max(color, vec3(0.0)), vec3(1.0 / 2.2)), 0.76);
}
