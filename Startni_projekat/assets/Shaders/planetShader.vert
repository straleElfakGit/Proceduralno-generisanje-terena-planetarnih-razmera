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

vec4 noised(vec3 x);
void ElevatedPosition(vec3 localUp, vec3 axisU, vec3 axisV, float u, float v, out vec3 outPos, out vec3 outNormal);
vec3 SpherifyCube(vec3 p);

void main()
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

	vec4 worldPos = model * vec4(aPos, 1.0f);
	gl_Position = camMat * worldPos;

    normal = normalMatrix * aNormal; 
	fragPos = vec3(worldPos);
}

void ElevatedPosition(vec3 localUp, vec3 axisU, vec3 axisV, float u, float v, out vec3 outPos, out vec3 outNormal)
{
	vec3 cubePos = localUp + (u * axisU) + (v * axisV);
	vec3 pointOnSphere = SpherifyCube(cubePos);

    vec3 noiseGrad = vec3(0.0);

	float noiseValue = 0.0f;
    float frequency = noise.baseRoughness;
    float amplitude = 1.0f;

    for(int i = 1; i <= noise.numberOfOctaves; i++)
    {
        vec4 val = noised((pointOnSphere * frequency) + noise.center);
        noiseValue += val.x * amplitude;
        noiseGrad += val.yzw * amplitude * frequency;
        frequency *= noise.roughness;
        amplitude *= noise.persistance;
    }

    float mask = step(noise.minValue, noiseValue);
    noiseValue = max(0.0, noiseValue - noise.minValue);
    noiseGrad *= mask;
    
    float elevation = 1.0f + (noiseValue * noise.strength);
    outPos = pointOnSphere * elevation;

    vec3 tangentGrad = noiseGrad - pointOnSphere * dot(noiseGrad, pointOnSphere);
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

float hash(float n) 
{
    uint p = floatBitsToUint(n);
    p ^= noise.seed;
    p ^= p >> 15;
    p *= 0x85ebca6bU;
    p ^= p >> 13;
    p *= 0xc2b2ae35U;
    p ^= p >> 16;
    return float(p) * 2.3283064365386963e-10;
    //return fract(sin(n)*753.5453123); 
}

vec4 noised(vec3 x)
{
    vec3 p = floor(x);
    vec3 w = fract(x);
    vec3 u = w*w*(3.0-2.0*w);
    vec3 du = 6.0*w*(1.0-w);
    
    float n = p.x + p.y*157.0 + 113.0*p.z;
    
    float a = hash(n+  0.0);
    float b = hash(n+  1.0);
    float c = hash(n+157.0);
    float d = hash(n+158.0);
    float e = hash(n+113.0);
    float f = hash(n+114.0);
    float g = hash(n+270.0);
    float h = hash(n+271.0);
    
    float k0 =   a;
    float k1 =   b - a;
    float k2 =   c - a;
    float k3 =   e - a;
    float k4 =   a - b - c + d;
    float k5 =   a - c - e + g;
    float k6 =   a - b - e + f;
    float k7 = - a + b + c - d + e - f - g + h;

    return vec4( k0 + k1*u.x + k2*u.y + k3*u.z + k4*u.x*u.y + k5*u.y*u.z + k6*u.z*u.x + k7*u.x*u.y*u.z, 
                 du * (vec3(k1,k2,k3) + u.yzx*vec3(k4,k5,k6) + u.zxy*vec3(k6,k4,k5) + k7*u.yzx*u.zxy ));
}