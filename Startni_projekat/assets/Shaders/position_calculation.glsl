
void CalculatePosition(out vec3 outPos, out vec3 outNormal)
{
    int vertexInQuad = gl_VertexID % 6;
    int quadId = gl_VertexID / 6;

    int quadsPerFace = resolution * resolution;
    int faceId = quadId / quadsPerFace;
    int quadInFace = quadId % quadsPerFace;

    vec2 offset = QUAD_OFFSET[vertexInQuad];

    int gridX = quadInFace % resolution;
    int gridY = quadInFace / resolution;

    float u = ((float(gridX) + offset.x) / float(resolution) - 0.5f) * 2.0f;
    float v = ((float(gridY) + offset.y) / float(resolution) - 0.5f) * 2.0f;

    vec3 localUp = LOCAL_UP[faceId];
    vec3 axisU = vec3(localUp.y, localUp.z, localUp.x);
    vec3 axisV = cross(axisU, localUp);

    vec3 aPos;
    vec3 aNormal;

    ElevatedPosition(localUp, axisU, axisV, u, v, aPos, aNormal);

    outPos = aPos;
    outNormal = aNormal;
}

void ElevatedPosition(vec3 localUp, vec3 axisU, vec3 axisV, float u, float v, out vec3 outPos, out vec3 outNormal)
{
    vec3 cubePos = localUp + (u * axisU) + (v * axisV);
    vec3 pointOnSphere = SpherifyCube(cubePos);

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

    //float mask = step(noise.minValue, noiseValue);
    float mask = smoothstep(noise.minValue - 0.01, noise.minValue + 0.05, noiseValue);
    noiseValue = max(0.0, noiseValue - noise.minValue);
    noiseGrad *= mask;

    float elevation = 1.0f + (noiseValue * noise.strength);
    outPos = pointOnSphere * elevation;

    vec3 tangentGrad = noiseGrad - pointOnSphere * dot(noiseGrad, pointOnSphere);

    float gradLen = length(tangentGrad);
    if (gradLen > 0.0001) {
        float maxAllowedSlope = 5.0;
        float newLen = min(gradLen, maxAllowedSlope);
        tangentGrad = (tangentGrad / gradLen) * newLen;
    }

    vec3 normal = pointOnSphere - (noise.strength / elevation) * tangentGrad;
    outNormal = normalize(normal);
}

vec3 SpherifyCube(vec3 p)
{
    vec3 p2 = p * p;

    float x = p.x * sqrt(1.0 - (p2.y + p2.z) * 0.5 + (p2.y * p2.z) / 3.0);
    float y = p.y * sqrt(1.0 - (p2.x + p2.z) * 0.5 + (p2.x * p2.z) / 3.0);
    float z = p.z * sqrt(1.0 - (p2.x + p2.y) * 0.5 + (p2.x * p2.y) / 3.0);

    return vec3(x, y, z);
}

#include "noise_evaluation.glsl"