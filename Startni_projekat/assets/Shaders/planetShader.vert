#version 430 core

struct NoiseSettings {
    float strength;
    int numberOfOctaves;
    float baseRoughness;
    float roughness;
    float persistance;
    float minValue;
    vec3 center;
};

out vec3 normal;
out vec3 fragPos;

uniform int resolution;
uniform mat4 model;
uniform mat4 camMat;
uniform mat3 normalMatrix;

uniform int perm[512];
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

vec3 SpherifyCube(vec3 p);
int Perm(int i);
float SimplexNoise(vec3 p);
vec3 ElevatedPosition(vec3 localUp, vec3 axisU, vec3 axisV, float u, float v);

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

	vec3 aPos = ElevatedPosition(localUp, axisU, axisV, u, v);

	vec3 pu = ElevatedPosition(localUp, axisU, axisV, u + NORMAL_EPS, v);
	vec3 pv = ElevatedPosition(localUp, axisU, axisV, u, v + NORMAL_EPS);

	vec3 aNormal = normalize(cross(pu - aPos, pv - aPos));

	if (dot(aNormal, aPos) < 0.0)
		aNormal = -aNormal;

	vec4 worldPos = model * vec4(aPos, 1.0f);
	gl_Position = camMat * worldPos;

	normal = normalMatrix * aNormal;
	fragPos = vec3(worldPos);

}

vec3 ElevatedPosition(vec3 localUp, vec3 axisU, vec3 axisV, float u, float v)
{
	vec3 cubePos = localUp + (u * axisU) + (v * axisV);
	vec3 pointOnSphere = SpherifyCube(cubePos);

	float noiseValue = 0.0f;
    float frequency = noise.baseRoughness;
    float amplitude = 1.0f;

    for(int i = 1; i <= noise.numberOfOctaves; i++)
    {
        float val = SimplexNoise((pointOnSphere * frequency) + noise.center);
        noiseValue += (val + 1.0f) * 0.5f * amplitude;
        frequency *= noise.roughness;
        amplitude *= noise.persistance;
    }

    noiseValue = max(0.0, noiseValue - noise.minValue);
    float elevation = 1.0f + (noiseValue * noise.strength);
    return pointOnSphere * elevation;
}

vec3 SpherifyCube(vec3 p) 
{
    vec3 p2 = p * p;

    float x = p.x * sqrt(1.0 - (p2.y + p2.z) * 0.5 + (p2.y * p2.z) / 3.0);
    float y = p.y * sqrt(1.0 - (p2.x + p2.z) * 0.5 + (p2.x * p2.z) / 3.0);
    float z = p.z * sqrt(1.0 - (p2.x + p2.y) * 0.5 + (p2.x * p2.y) / 3.0);
    
    return vec3(x, y, z);
}

int Perm(int i)
{
	return perm[i & 511];
}

float SimplexNoise(vec3 p)
{
    float s = (p.x + p.y + p.z) * F3;

    int i = int(floor(p.x + s));
    int j = int(floor(p.y + s));
    int k = int(floor(p.z + s));

    float t = float(i + j + k) * G3;

    float x0 = p.x - (float(i) - t);
    float y0 = p.y - (float(j) - t);
    float z0 = p.z - (float(k) - t);

    int i1, j1, k1;
    int i2, j2, k2;

    if (x0 >= y0)
    {
        if (y0 >= z0)      { i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 1; k2 = 0; }
        else if (x0 >= z0) { i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 0; k2 = 1; }
        else               { i1 = 0; j1 = 0; k1 = 1; i2 = 1; j2 = 0; k2 = 1; }
    }
    else
    {
        if (y0 < z0)      { i1 = 0; j1 = 0; k1 = 1; i2 = 0; j2 = 1; k2 = 1; }
        else if (x0 < z0) { i1 = 0; j1 = 1; k1 = 0; i2 = 0; j2 = 1; k2 = 1; }
        else              { i1 = 0; j1 = 1; k1 = 0; i2 = 1; j2 = 1; k2 = 0; }
    }

    float x1 = x0 - float(i1) + G3;
    float y1 = y0 - float(j1) + G3;
    float z1 = z0 - float(k1) + G3;

    float x2 = x0 - float(i2) + F3;
    float y2 = y0 - float(j2) + F3;
    float z2 = z0 - float(k2) + F3;

    float x3 = x0 - 0.5;
    float y3 = y0 - 0.5;
    float z3 = z0 - 0.5;

    int ii = i & 255;
    int jj = j & 255;
    int kk = k & 255;

    float n0 = 0.0, n1 = 0.0, n2 = 0.0, n3 = 0.0;

    float t0 = 0.6 - x0*x0 - y0*y0 - z0*z0;
    if (t0 > 0.0)
    {
        t0 *= t0;
        int gi0 = Perm(ii + Perm(jj + Perm(kk))) % 12;
        n0 = t0 * t0 * dot(GRAD3[gi0], vec3(x0, y0, z0));
    }

    float t1 = 0.6 - x1*x1 - y1*y1 - z1*z1;
    if (t1 > 0.0)
    {
        t1 *= t1;
        int gi1 = Perm(ii + i1 + Perm(jj + j1 + Perm(kk + k1))) % 12;
        n1 = t1 * t1 * dot(GRAD3[gi1], vec3(x1, y1, z1));
    }

    float t2 = 0.6 - x2*x2 - y2*y2 - z2*z2;
    if (t2 > 0.0)
    {
        t2 *= t2;
        int gi2 = Perm(ii + i2 + Perm(jj + j2 + Perm(kk + k2))) % 12;
        n2 = t2 * t2 * dot(GRAD3[gi2], vec3(x2, y2, z2));
    }

    float t3 = 0.6 - x3*x3 - y3*y3 - z3*z3;
    if (t3 > 0.0)
    {
        t3 *= t3;
        int gi3 = Perm(ii + 1 + Perm(jj + 1 + Perm(kk + 1))) % 12;
        n3 = t3 * t3 * dot(GRAD3[gi3], vec3(x3, y3, z3));
    }

    return (n0 + n1 + n2 + n3) * 32.0;
}