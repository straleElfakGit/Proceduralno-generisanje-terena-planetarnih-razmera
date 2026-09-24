#version 430 core

out vec3 normal;
out vec3 fragPos;

uniform int resolution;
uniform mat4 model;
uniform mat4 camMat;
uniform mat3 normalMatrix;

const vec2 QUAD_OFFSET[6] = vec2[6] (
	vec2(0.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 1.0),
    vec2(0.0, 0.0),

    vec2(1.0, 1.0),
    vec2(1.0, 0.0)
);

const vec3 LOCAL_UP[6] = vec3[6] (
	vec3( 1.0,  0.0,  0.0),
    vec3(-1.0,  0.0,  0.0),
    vec3( 0.0,  1.0,  0.0),
    vec3( 0.0, -1.0,  0.0),
	vec3( 0.0,  0.0,  1.0),
    vec3( 0.0,  0.0, -1.0)
);

vec3 PositionOnSphere();
vec3 SpherifyCube(vec3 p);

void main()
{
	vec3 aPos = PositionOnSphere();
	vec3 aNormal = aPos;

	vec4 worldPos = model * vec4(aPos, 1.0f);
	gl_Position = camMat * worldPos;

	normal = normalMatrix * aNormal;
	fragPos = vec3(worldPos);
}

vec3 PositionOnSphere()
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
	vec3 axis_u = vec3(localUp.y, localUp.z, localUp.x);
	vec3 axis_v = cross(axis_u, localUp);

	vec3 cubePos = localUp + (u * axis_u) + (v * axis_v);
	vec3 spherePos = SpherifyCube(cubePos);
	return spherePos;
}

vec3 SpherifyCube(vec3 p) 
{
    vec3 p2 = p * p;

    float x = p.x * sqrt(1.0 - (p2.y + p2.z) * 0.5 + (p2.y * p2.z) / 3.0);
    float y = p.y * sqrt(1.0 - (p2.x + p2.z) * 0.5 + (p2.x * p2.z) / 3.0);
    float z = p.z * sqrt(1.0 - (p2.x + p2.y) * 0.5 + (p2.x * p2.y) / 3.0);
    
    return vec3(x, y, z);
}