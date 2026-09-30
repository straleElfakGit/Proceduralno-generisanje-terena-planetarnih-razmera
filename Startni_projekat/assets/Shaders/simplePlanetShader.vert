#version 430 core

out vec3 fragPos;
out vec3 normal;

layout(std430, binding = 0) readonly buffer NodeBuffer
{
    ivec4 nodes[];
};

uniform int resolution;
uniform int maxLevel;

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

    // Pozicija temena u okviru cvora
    float u = u0 + (float(gridX) / float(resolution)) * nodeStep;
    float v = v0 + (float(gridY) / float(resolution)) * nodeStep;

    vec3 localUp = LOCAL_UP[face];
    vec3 axisU = vec3(localUp.y, localUp.z, localUp.x);
    vec3 axisV = cross(axisU, localUp);

    vec3 cubePos = localUp + (u * axisU) + (v * axisV);
    vec3 pointOnSphere = SpherifyCube(cubePos);

	vec4 worldPos = model * vec4(pointOnSphere, 1.0f);
	gl_Position = camMat * worldPos;

    normal = normalMatrix * pointOnSphere; 
	fragPos = vec3(worldPos);
}

vec3 SpherifyCube(vec3 p)
{
    vec3 p2 = p * p;

    float x = p.x * sqrt(1.0 - (p2.y + p2.z) * 0.5 + (p2.y * p2.z) / 3.0);
    float y = p.y * sqrt(1.0 - (p2.x + p2.z) * 0.5 + (p2.x * p2.z) / 3.0);
    float z = p.z * sqrt(1.0 - (p2.x + p2.y) * 0.5 + (p2.x * p2.y) / 3.0);

    return vec3(x, y, z);
}