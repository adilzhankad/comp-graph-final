#version 330 core
in float vAlong;
in float vAcross;
flat in float vLength;
uniform vec3 uColor;
uniform float uHalfWidth;
uniform float uBrightness;
out vec4 fragColor;

void main() {
    float progress = clamp(vAlong / max(vLength, 0.00001), 0.0, 1.0);
    float trail = pow(progress, 1.35) * smoothstep(0.0, 0.13, progress)
                * (1.0 - smoothstep(vLength, vLength + uHalfWidth, vAlong));
    float taper = mix(0.26, 1.0, progress);
    float naturalWidth = uHalfWidth * 0.18 * taper;
    // Preserve the energy of thin cores as they become subpixel in the lake.
    float coreWidth = max(naturalWidth, fwidth(vAcross) * 0.55);
    float core = exp(-pow(vAcross / max(coreWidth, 0.00001), 2.0))
               * naturalWidth / max(coreWidth, 0.00001);
    float glow = exp(-pow(vAcross / (uHalfWidth * taper), 2.0));

    vec2 headOffset = vec2(vAlong - vLength, vAcross);
    float headRadius = uHalfWidth * 0.62;
    float head = exp(-dot(headOffset, headOffset) / (headRadius * headRadius));
    float haloRadius = uHalfWidth * 1.4;
    float halo = exp(-dot(headOffset, headOffset) / (haloRadius * haloRadius));
    // Cool or warm tails lead into a white-hot compact head. Only emission is
    // written for GL_ONE, GL_ONE blending; there is no opaque billboard edge.
    vec3 emission = mix(uColor, vec3(0.90, 0.96, 1.0), 0.45) * core * trail * 2.9
                  + uColor * glow * trail * 0.30
                  + vec3(1.0, 0.97, 0.90) * head * 2.4
                  + uColor * halo * 0.22;
    float border = 1.0 - smoothstep(uHalfWidth * 2.0, uHalfWidth * 2.5, abs(vAcross));
    fragColor = vec4(emission * border * max(uBrightness, 0.0), 0.0);
}
