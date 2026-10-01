#version 430 core

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

const vec3 LOCAL_UP[6] = vec3[6] (
	vec3( 1.0,  0.0,  0.0),
    vec3(-1.0,  0.0,  0.0),
    vec3( 0.0,  1.0,  0.0),
    vec3( 0.0, -1.0,  0.0),
	vec3( 0.0,  0.0,  1.0),
    vec3( 0.0,  0.0, -1.0)
);

vec3 SpherifyCube(vec3 p);
vec3 PositionAt(int face, float u, float v);

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

    vec3 posFull = PositionAt(face, u, v);

    bool oddX = (gridX & 1) == 1;
    bool oddY = (gridY & 1) == 1;

    // Morphing
    vec3 posCoarse = posFull;
    if (oddX && !oddY)
    {
        vec3 pL = PositionAt(face, u - cellU, v);
        vec3 pR = PositionAt(face, u + cellU, v);
        posCoarse = 0.5 * (pL + pR);
    }
    else if (!oddX && oddY)
    {
        vec3 pD = PositionAt(face, u, v - cellV);
        vec3 pU = PositionAt(face, u, v + cellV);
        posCoarse = 0.5 * (pD + pU);
    }
    else if (oddX && oddY)
    {
        vec3 p00 = PositionAt(face, u - cellU, v - cellV);
        vec3 p10 = PositionAt(face, u + cellU, v - cellV);
        vec3 p01 = PositionAt(face, u - cellU, v + cellV);
        vec3 p11 = PositionAt(face, u + cellU, v + cellV);
        posCoarse = 0.25 * (p00 + p10 + p01 + p11);
    }

    vec4 worldPosFull = model * vec4(posFull, 1.0);
    float dist = distance(viewPos, worldPosFull.xyz);

    float mStart = morphStart[lod];
    float mEnd = morphEnd[lod];
    float morphFactor = clamp((dist - mStart) / max(mEnd - mStart, 0.0001), 0.0, 1.0);

    vec3 aPos = mix(posFull, posCoarse, morphFactor);
    vec3 aNormal = normalize(aPos);

	vec4 worldPos = model * vec4(aPos, 1.0f);
	gl_Position = camMat * worldPos;

    normal = normalMatrix * aNormal; 
	fragPos = vec3(worldPos);
}

vec3 PositionAt(int face, float u, float v)
{
    vec3 localUp = LOCAL_UP[face];
    vec3 axisU = vec3(localUp.y, localUp.z, localUp.x);
    vec3 axisV = cross(axisU, localUp);

    vec3 cubePos = localUp + (u * axisU) + (v * axisV);
    return SpherifyCube(cubePos);
}

vec3 SpherifyCube(vec3 p)
{
    vec3 p2 = p * p;

    float x = p.x * sqrt(1.0 - (p2.y + p2.z) * 0.5 + (p2.y * p2.z) / 3.0);
    float y = p.y * sqrt(1.0 - (p2.x + p2.z) * 0.5 + (p2.x * p2.z) / 3.0);
    float z = p.z * sqrt(1.0 - (p2.x + p2.y) * 0.5 + (p2.x * p2.y) / 3.0);

    return vec3(x, y, z);
}