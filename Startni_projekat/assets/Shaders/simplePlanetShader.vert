#version 430 core

#include "lod_vertex.glsl"


out vec3 fragPos;
out vec3 normal;
out vec3 objectPos;
out float biomePercent;
out float elevation;

uniform mat4 camMat;
uniform mat3 normalMatrix;

const int MAX_BIOMES = 8; 
uniform int biomeCount;
uniform float biomeStart[MAX_BIOMES];
uniform float biomeBlendRange;
uniform float biomeNoiseOffset;
uniform float biomeNoiseStrength;
uniform NoiseSettings biomeNoise;

float BiomePrecentFromPoint(vec3 pointOnSphere);

void main()
{
    vec3 aPos;
    vec3 aNormal;
    float aEl;
    ComputeLodVertex(aPos, aNormal, aEl);

	vec4 worldPos = model * vec4(aPos, 1.0f);
	gl_Position = camMat * worldPos;

    objectPos = aPos;
    elevation = aEl;

    biomePercent = BiomePrecentFromPoint(normalize(aPos));
    normal = normalMatrix * aNormal; 
	fragPos = vec3(worldPos);
}

float BiomeNoiseElevation(vec3 pointOnSphere)
{
    float sum = 0.0;
    float frequency = biomeNoise.baseRoughness;
    float amplitude = 1.0;

    for (int i = 0; i < biomeNoise.numberOfOctaves; i++)
    {
        sum += Noised(pointOnSphere * frequency + biomeNoise.center, biomeNoise.seed).x * amplitude;
        frequency *= biomeNoise.roughness;
        amplitude *= biomeNoise.persistance;
    }

    return (sum - biomeNoise.minValue) * biomeNoise.strength;
}

float BiomePrecentFromPoint(vec3 pointOnSphere)
{
    float heightPercent = (pointOnSphere.y + 1.0) * 0.5;
	heightPercent += (BiomeNoiseElevation(pointOnSphere) - biomeNoiseOffset) * biomeNoiseStrength;

	float biomeIndex = 0.0;
    for (int i = 0; i < biomeCount; i++)
    {
        float dst = heightPercent - biomeStart[i];
        float weight = clamp((dst + biomeBlendRange) / (2.0 * biomeBlendRange), 0.0, 1.0);
        biomeIndex = biomeIndex * (1.0 - weight) + float(i) * weight;
    }

    return biomeIndex / float(max(1, biomeCount - 1));
}