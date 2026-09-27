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

out vec3 fragPos;
out vec3 normal;

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

float SimplexNoise2(vec3 p)
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

vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
  return mod289(((x * 34.0) + 1.0) * x);
}

vec4 taylorInvSqrt(vec4 r) {
  return 1.79284291400159 - 0.85373472095314 * r;
}

float SimplexNoise(vec3 v) { 
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 = v - i + dot(i, C.xxx) ;

  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  vec3 x1 = x0 - i1 + C.xxx;
  vec3 x2 = x0 - i2 + C.yyy;
  vec3 x3 = x0 - D.yyy;

  i = mod289(i); 
  vec4 p = permute( permute( permute( 
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

  float n_ = 0.142857142857;
  vec3  ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z * ns.z);

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

  vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), dot(p2,x2), dot(p3,x3) ) );
}