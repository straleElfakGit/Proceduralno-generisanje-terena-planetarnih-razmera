#version 430 core

in vec2 vUV;
out vec4 FragColor;

uniform vec3 sunColor;
uniform float glowScale;
uniform float glowStrength;
uniform float coreIntensity;

void main()
{
    float d = length(vUV);
    float r = d * glowScale;

    float mu = sqrt(max(1.0 - r * r, 0.0));
    float limb = 1.0 - 0.55 * (1.0 - mu);
    float disc = 1.0 - smoothstep(0.94, 1.0, r);

    float innerGlow = exp(-max(r - 1.0, 0.0) * 2.5);
    float outerGlow = 1.0 / (1.0 + r * r * 0.5);
    float glow = glowStrength * (0.55 * innerGlow + 0.45 * outerGlow);

    vec3 glowColor = mix(sunColor, sunColor * vec3(1.0, 0.6, 0.35), clamp((r - 1.0) / 6.0, 0.0, 1.0));

    float edge = 1.0 - smoothstep(0.5, 1.0, d);

    vec3 col = sunColor * disc * limb * coreIntensity + glowColor * glow;
    FragColor = vec4(col * edge, 1.0);   // aditivno mesanje: alfa se ne koristi
}
