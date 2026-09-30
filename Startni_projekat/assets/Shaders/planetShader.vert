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
out float elevation;
out vec3 objectPos;

uniform int resolution;
uniform mat4 model;
uniform mat4 camMat;
uniform mat3 normalMatrix;

uniform NoiseSettings noise;

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

const vec3 GRAD3[12] = vec3[12] (
    vec3( 1.0,  1.0,  0.0), vec3(-1.0,  1.0,  0.0), vec3( 1.0, -1.0,  0.0), vec3(-1.0, -1.0,  0.0),
    vec3( 1.0,  0.0,  1.0), vec3(-1.0,  0.0,  1.0), vec3( 1.0,  0.0, -1.0), vec3(-1.0,  0.0, -1.0),
    vec3( 0.0,  1.0,  1.0), vec3( 0.0, -1.0,  1.0), vec3( 0.0,  1.0, -1.0), vec3( 0.0, -1.0, -1.0)
);

const float F3 = 1.0 / 3.0;
const float G3 = 1.0 / 6.0;

const float NORMAL_EPS = 0.01;

vec4 Noised(vec3 x);
void CalculatePosition(out vec3 outPos, out vec3 outNormal);
void ElevatedPosition(vec3 localUp, vec3 axisU, vec3 axisV, float u, float v, out vec3 outPos, out vec3 outNormal);
vec3 SpherifyCube(vec3 p);

void main()
{
	vec3 aPos;
    vec3 aNormal;

    CalculatePosition(aPos, aNormal);

	vec4 worldPos = model * vec4(aPos, 1.0f);
	gl_Position = camMat * worldPos;

    normal = normalMatrix * aNormal; 
	fragPos = vec3(worldPos);
}


#include "position_calculation.glsl"
