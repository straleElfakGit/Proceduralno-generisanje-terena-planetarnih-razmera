#version 430 core

struct NoiseSettings {
    float strength;
    int numberOfOctaves;
    float baseRoughness;
    float roughness;
    float persistance;
    float minValue;
    vec3 center;
    uint seed;
};

out vec3 fragPos;
out vec3 normal;

layout(std430, binding = 0) readonly buffer NodeBuffer
{
    ivec4 nodes[];
};

uniform int resolution;
uniform int maxLevel;

uniform float morphStart[16];
uniform float morphEnd[16];
uniform vec3 viewPos; 

uniform mat4 model;
uniform mat4 camMat;
uniform mat3 normalMatrix;

uniform NoiseSettings noise;

const vec3 LOCAL_UP[6] = vec3[6] (
	vec3( 1.0,  0.0,  0.0),
    vec3(-1.0,  0.0,  0.0),
    vec3( 0.0,  1.0,  0.0),
    vec3( 0.0, -1.0,  0.0),
	vec3( 0.0,  0.0,  1.0),
    vec3( 0.0,  0.0, -1.0)
);

vec3 SpherifyCube(vec3 p);
void PositionAt(int face, float u, float v, out vec3 outPos, out vec3 outNormal);
void ElevatedPosition(vec3 pointOnSphere, out vec3 outPos, out vec3 outNormal);
vec4 Noised(vec3 x);

void main()
{
    ivec4 node = nodes[gl_InstanceID];
    int face = node.x;
    int lod  = node.y;
    int nx   = node.z;
    int ny   = node.w;

    int gridX = gl_VertexID % (resolution + 1);
    int gridY = gl_VertexID / (resolution + 1);

    // Pocetak cvora quadTree-a
    int level = maxLevel - lod;
    float nodeStep = 2.0 / float(1 << level);
    float u0 = -1.0 + float(nx) * nodeStep;
    float v0 = -1.0 + float(ny) * nodeStep;

    float cellU = nodeStep / float(resolution);
    float cellV = cellU;

    // Pozicija temena u okviru cvora
    float u = u0 + float(gridX) * cellU;
    float v = v0 + float(gridY) * cellV;

    vec3 posFull;
    vec3 nor;
    PositionAt(face, u, v, posFull, nor);

    bool oddX = (gridX & 1) == 1;
    bool oddY = (gridY & 1) == 1;

    vec3 aPos = posFull;
    vec3 aNormal = nor;
    
    // Morphing
    if(oddX || oddY)
    {
        vec4 worldPosFull = model * vec4(posFull, 1.0);
        float dist = distance(viewPos, worldPosFull.xyz);

        float mStart = morphStart[lod];
        float mEnd = morphEnd[lod];
        float morphFactor = clamp((dist - mStart) / max(mEnd - mStart, 0.0001), 0.0, 1.0);

        if(morphFactor > 0.0) 
        {
            vec3 posCoarse;
            vec3 normCoarse;

            if (oddX && !oddY)
            {
                vec3 pL, pR, nL, nR;
                PositionAt(face, u - cellU, v, pL, nL);
                PositionAt(face, u + cellU, v, pR, nR);
                posCoarse = 0.5 * (pL + pR);
                normCoarse = 0.5 * (nL + nR);
            }
            else if (!oddX && oddY)
            {
                vec3 pD, pU, nD, nU;
                PositionAt(face, u, v - cellV, pD, nD);
                PositionAt(face, u, v + cellV, pU, nU);
                posCoarse = 0.5 * (pD + pU);
                normCoarse = 0.5 * (nD + nU);
            }
            else if (oddX && oddY)
            {
                vec3 p00, p10, p01, p11, n00, n10, n01, n11;
                PositionAt(face, u - cellU, v - cellV, p00, n00);
                PositionAt(face, u + cellU, v - cellV, p10, n10);
                PositionAt(face, u - cellU, v + cellV, p01, n01);
                PositionAt(face, u + cellU, v + cellV, p11, n11);
                posCoarse = 0.25 * (p00 + p10 + p01 + p11);
                normCoarse = 0.25 * (n00 + n10 + n01 + n11);
            }

            aPos = mix(posFull, posCoarse, morphFactor);
            aNormal = mix(nor, normCoarse, morphFactor);
        }
    }
    aNormal = normalize(aNormal);

	vec4 worldPos = model * vec4(aPos, 1.0f);
	gl_Position = camMat * worldPos;

    normal = normalMatrix * aNormal; 
	fragPos = vec3(worldPos);
}

void PositionAt(int face, float u, float v, out vec3 outPos, out vec3 outNormal)
{
    vec3 localUp = LOCAL_UP[face];
    vec3 axisU = vec3(localUp.y, localUp.z, localUp.x);
    vec3 axisV = cross(axisU, localUp);

    vec3 cubePos = localUp + (u * axisU) + (v * axisV);

    vec3 oP;
    vec3 oN;
    vec3 pointOnSphere = SpherifyCube(cubePos);
    ElevatedPosition(pointOnSphere, oP, oN);

    outPos = oP;
    outNormal = oN;
}

vec3 SpherifyCube(vec3 p)
{
    vec3 p2 = p * p;

    float x = p.x * sqrt(1.0 - (p2.y + p2.z) * 0.5 + (p2.y * p2.z) / 3.0);
    float y = p.y * sqrt(1.0 - (p2.x + p2.z) * 0.5 + (p2.x * p2.z) / 3.0);
    float z = p.z * sqrt(1.0 - (p2.x + p2.y) * 0.5 + (p2.x * p2.y) / 3.0);

    return vec3(x, y, z);
}

void ElevatedPosition(vec3 pointOnSphere, out vec3 outPos, out vec3 outNormal)
{
    vec3 noiseGrad = vec3(0.0);

    float noiseValue = 0.0f;
    float frequency = noise.baseRoughness;
    float amplitude = 1.0f;

    for (int i = 1; i <= noise.numberOfOctaves; i++)
    {
        vec4 val = Noised((pointOnSphere * frequency) + noise.center);
        noiseValue += val.x * amplitude;
        noiseGrad += val.yzw * amplitude * frequency;
        frequency *= noise.roughness;
        amplitude *= noise.persistance;
    }

    float elevation = (noiseValue - noise.minValue) * noise.strength;

    //float mask = step(noise.minValue, noiseValue);
    float mask = smoothstep(noise.minValue - 0.01, noise.minValue + 0.05, noiseValue);
    noiseValue = max(0.0, noiseValue - noise.minValue);
    noiseGrad *= mask;

    float finalElevation = 1.0f + (noiseValue * noise.strength);
    outPos = pointOnSphere * finalElevation;

    vec3 tangentGrad = noiseGrad - pointOnSphere * dot(noiseGrad, pointOnSphere);

    float gradLen = length(tangentGrad);
    if (gradLen > 0.0001) {
        float maxAllowedSlope = 5.0;
        float newLen = min(gradLen, maxAllowedSlope);
        tangentGrad = (tangentGrad / gradLen) * newLen;
    }

    vec3 normal = pointOnSphere - (noise.strength / finalElevation) * tangentGrad;
    outNormal = normalize(normal);
}

#include "noise_evaluation.glsl"