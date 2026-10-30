#version 330 core
in vec3 vPosition;
in vec3 vNormal;
uniform vec3 uCameraPosition;
uniform vec3 uMoonDirection;
uniform vec3 uMoonColor;
uniform vec3 uFirePosition;
uniform vec3 uFireColor;
uniform vec3 uColor;
uniform float uEmission;
out vec4 fragColor;
vec3 illuminate(vec3 n, vec3 v, vec3 l, vec3 light) {
    float diffuse = max(dot(n, l), 0.0);
    float specular = diffuse > 0.0
        ? pow(max(dot(n, normalize(l + v)), 0.0), 32.0) : 0.0;
    return light * (uColor * diffuse + vec3(0.12 * specular));
}
void main() {
    vec3 n = normalize(vNormal);
    vec3 v = normalize(uCameraPosition - vPosition);
    vec3 color = uColor * vec3(0.16, 0.20, 0.30) + uColor * uEmission;
    color += illuminate(n, v, normalize(uMoonDirection), uMoonColor * 0.68);
    vec3 toFire = uFirePosition - vPosition;
    float d = length(toFire);
    color += illuminate(n, v, normalize(toFire),
                        uFireColor / (1.0 + 0.14 * d + 0.06 * d * d));
    fragColor = vec4(pow(max(color, vec3(0.0)), vec3(1.0 / 2.2)), 1.0);
}
