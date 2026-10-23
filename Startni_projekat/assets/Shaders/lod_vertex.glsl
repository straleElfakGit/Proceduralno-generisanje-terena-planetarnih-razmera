//Lod_vertex

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

const int MAX_LAYERS = 4;
const int FILTER_SIMPLE = 0;
const int FILTER_RIGID = 1;

struct TerrainLayer {
    NoiseSettings n;
    int filterType;
    int enabled;
    int useFirstLayerAsMask;
    float weightMultiplier;
};

uniform int layerCount;
uniform TerrainLayer layers[MAX_LAYERS];

const float SEA_GRAD_MIN = -0.003;
const float SEA_GRAD_MAX = 0.012;

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

uniform NoiseSettings noise;

const vec3 LOCAL_UP[6] = vec3[6](
    vec3(1.0, 0.0, 0.0),
    vec3(-1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, -1.0, 0.0),
    vec3(0.0, 0.0, 1.0),
    vec3(0.0, 0.0, -1.0)
    );

vec4 Noised(vec3 x, uint seed);

vec3 SpherifyCube(vec3 p)
{
    vec3 p2 = p * p;

    float x = p.x * sqrt(1.0 - (p2.y + p2.z) * 0.5 + (p2.y * p2.z) / 3.0);
    float y = p.y * sqrt(1.0 - (p2.x + p2.z) * 0.5 + (p2.x * p2.z) / 3.0);
    float z = p.z * sqrt(1.0 - (p2.x + p2.y) * 0.5 + (p2.x * p2.y) / 3.0);

    return vec3(x, y, z);
}

vec4 EvaluateSimple(NoiseSettings s, vec3 p)
{
    float sum = 0.0;
    vec3 grad = vec3(0.0);
    float frequency = s.baseRoughness;
    float amplitude = 1.0;

    for (int i = 0; i < s.numberOfOctaves; i++)
    {
        vec4 v = Noised(p * frequency + s.center, s.seed);
        sum += v.x * amplitude;
        grad += v.yzw * (amplitude * frequency);
        frequency *= s.roughness;
        amplitude *= s.persistance;
    }

    return vec4((sum - s.minValue) * s.strength, grad * s.strength);
}

vec4 EvaluateRigid(NoiseSettings s, float weightMultiplier, vec3 p)
{
    float sum = 0.0;
    vec3 grad = vec3(0.0);
    float frequency = s.baseRoughness;
    float amplitude = 1.0;

    float weight = 1.0;
    vec3 weightGrad = vec3(0.0);

    for (int i = 0; i < s.numberOfOctaves; i++)
    {
        vec4 nv = Noised(p * frequency + s.center, s.seed);
        float n = nv.x * 2.0 - 1.0;
        vec3 dn = nv.yzw * (2.0 * frequency);

        float a = 1.0 - abs(n);
        float v = a * a;
        vec3 dv = -2.0 * a * sign(n) * dn;

        float vw = v * weight;
        vec3 dvw = dv * weight + v * weightGrad;

        float w = vw * weightMultiplier;
        weight = clamp(w, 0.0, 1.0);
        weightGrad = (w > 0.0 && w < 1.0) ? dvw * weightMultiplier : vec3(0.0);

        sum += vw * amplitude;
        grad += dvw * amplitude;
        frequency *= s.roughness;
        amplitude *= s.persistance;
    }

    return vec4((sum - s.minValue) * s.strength, grad * s.strength);
}

vec4 EvaluateLayer(int i, vec3 p)
{
    if (layers[i].filterType == FILTER_RIGID)
        return EvaluateRigid(layers[i].n, layers[i].weightMultiplier, p);

    return EvaluateSimple(layers[i].n, p);
}


void ElevatedPosition(vec3 pointOnSphere, out vec3 outPos, out vec3 outNormal, out float outElevation)
{
    float total = 0.0;
    vec3 grad = vec3(0.0);

    float maskValue = 1.0;
    vec3 maskGrad = vec3(0.0);

    for (int i = 0; i < layerCount; i++)
    {
        if (i > 0 && layers[i].enabled == 0)
            continue;

        vec4 r = EvaluateLayer(i, pointOnSphere);

        if (i == 0)
        {
            if (r.x > 0.0)
            {
                maskValue = r.x;
                maskGrad = r.yzw;
            }
            else
            {
                maskValue = 0.0;
                maskGrad = vec3(0.0);
            }

            if (layers[0].enabled != 0)
            {
                total += r.x;
                grad += r.yzw;
            }
        }
        else if (layers[i].useFirstLayerAsMask != 0)
        {
            total += r.x * maskValue;
            grad += r.yzw * maskValue + r.x * maskGrad;
        }
        else
        {
            total += r.x;
            grad += r.yzw;
        }
    }

    outElevation = total;

    float finalElevation = 1.0 + max(0.0, total);
    outPos = pointOnSphere * finalElevation;

    float seaMask = smoothstep(SEA_GRAD_MIN, SEA_GRAD_MAX, total);
    vec3 g = grad * seaMask;

    vec3 tangentGrad = g - pointOnSphere * dot(g, pointOnSphere);

    float gradLen = length(tangentGrad);
    if (gradLen > 0.0001) {
        float maxAllowedSlope = 5.0;
        float newLen = min(gradLen, maxAllowedSlope);
        tangentGrad = (tangentGrad / gradLen) * newLen;
    }

    vec3 nrm = pointOnSphere - tangentGrad / finalElevation;
    outNormal = normalize(nrm);
}

void PositionAt(int face, float u, float v, out vec3 outPos, out vec3 outNormal, out float outElevation)
{
    vec3 localUp = LOCAL_UP[face];
    vec3 axisU = vec3(localUp.y, localUp.z, localUp.x);
    vec3 axisV = cross(axisU, localUp);

    vec3 cubePos = localUp + (u * axisU) + (v * axisV);

    vec3 oP;
    vec3 oN;
    float el;
    vec3 pointOnSphere = SpherifyCube(cubePos);
    ElevatedPosition(pointOnSphere, oP, oN, el);

    outPos = oP;
    outNormal = oN;
    outElevation = el;
}

void ComputeLodVertex(out vec3 aPos, out vec3 aNormal, out float aEl)
{
    ivec4 node = nodes[gl_InstanceID];
    int face = node.x;
    int lod = node.y;
    int nx = node.z;
    int ny = node.w;

    int gridX = gl_VertexID % (resolution + 1);
    int gridY = gl_VertexID / (resolution + 1);

    int level = maxLevel - lod;
    float nodeStep = 2.0 / float(1 << level);
    float u0 = -1.0 + float(nx) * nodeStep;
    float v0 = -1.0 + float(ny) * nodeStep;

    float cellU = nodeStep / float(resolution);
    float cellV = cellU;

    float u = u0 + float(gridX) * cellU;
    float v = v0 + float(gridY) * cellV;

    vec3 posFull;
    vec3 nor;
    float elev;
    PositionAt(face, u, v, posFull, nor, elev);

    bool oddX = (gridX & 1) == 1;
    bool oddY = (gridY & 1) == 1;

    aPos = posFull;
    aNormal = nor;
    aEl = elev;

    if (oddX || oddY)
    {
        vec4 worldPosFull = model * vec4(posFull, 1.0);
        float dist = distance(viewPos, worldPosFull.xyz);

        float mStart = morphStart[lod];
        float mEnd = morphEnd[lod];
        float morphFactor = clamp((dist - mStart) / max(mEnd - mStart, 0.0001), 0.0, 1.0);

        if (morphFactor > 0.0)
        {
            vec3 posCoarse = posFull;
            vec3 normCoarse = nor;
            float posE = elev;

            if (oddX && !oddY)
            {
                vec3 pL, pR, nL, nR;
                float e1, e2;
                PositionAt(face, u - cellU, v, pL, nL, e1);
                PositionAt(face, u + cellU, v, pR, nR, e2);
                posCoarse = 0.5 * (pL + pR);
                normCoarse = 0.5 * (nL + nR);
                posE = 0.5 * (e1 + e2);
            }
            else if (!oddX && oddY)
            {
                vec3 pD, pU, nD, nU;
                float e1, e2;
                PositionAt(face, u, v - cellV, pD, nD, e1);
                PositionAt(face, u, v + cellV, pU, nU, e2);
                posCoarse = 0.5 * (pD + pU);
                normCoarse = 0.5 * (nD + nU);
                posE = 0.5 * (e1 + e2);
            }
            else
            {
                vec3 p00, p10, p01, p11, n00, n10, n01, n11;
                float e1, e2, e3, e4;
                PositionAt(face, u - cellU, v - cellV, p00, n00, e1);
                PositionAt(face, u + cellU, v - cellV, p10, n10, e2);
                PositionAt(face, u - cellU, v + cellV, p01, n01, e3);
                PositionAt(face, u + cellU, v + cellV, p11, n11, e4);
                posCoarse = 0.25 * (p00 + p10 + p01 + p11);
                normCoarse = 0.25 * (n00 + n10 + n01 + n11);
                posE = 0.25 * (e1 + e2 + e3 + e4);
            }

            aPos = mix(posFull, posCoarse, morphFactor);
            aNormal = mix(nor, normCoarse, morphFactor);
            aEl = mix(elev, posE, morphFactor);
        }
    }

    aNormal = normalize(aNormal);
}

#include "noise_evaluation.glsl"