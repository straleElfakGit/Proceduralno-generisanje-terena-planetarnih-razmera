#version 430 core

struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

struct DirLight {
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  

out vec4 FragColor;

in vec3 fragPos;
in float elevation;
in vec3 objectPos;
in vec3 objectNormal; 

uniform Material material;
uniform DirLight dirLight;
uniform vec3 viewPos;
uniform mat3 normalMatrix;

uniform vec2 heightRange;

uniform sampler2D texWater;
uniform sampler2D noiseMap;     
uniform sampler2DArray diffuseMap;
uniform sampler2DArray normalMap;

uniform float textureScale = 20.0; 
uniform float normalStrength = 1.0;
uniform vec2 waterOffset;

// ---------------------------------------------------------------- konstante

const int NUM_LAYERS = 8;

const int L_DIRT = 0;
const int L_GRASS = 1;
const int L_SAND = 2;
const int L_ROCK = 3;
const int L_ROCKICE = 4;
const int L_SNOW = 5;
const int L_COBBLE = 6;
const int L_SANDROCK = 7;

const float NORMAL_Y_SIGN[NUM_LAYERS] = float[NUM_LAYERS](1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);

const float TEX_SIZE = 1024.0;
const float BOMB_NOISE_SCALE = 0.0025;
const float BOMB_MAX_TEXELS = 0.0;
const float NORMAL_FADE_START = 2.0;      
const float NORMAL_FADE_END = 10.0;

const float COAST_WIDTH = 0.015;
const float HEIGHT_BLEND_CONTRAST = 0.8;
const float HEIGHT_BLEND_DEPTH = 0.2;

const float PROJ_MIN_WEIGHT  = 0.03;  
const float LAYER_MIN_WEIGHT = 0.04;
const float NORMAL_MIN_SHARE = 0.15;

// ---------------------------------------------------------------- projekcije (biplanar)

struct Projection
{
    ivec3 axes; 
    vec2  uv;
    vec2  dx;
    vec2  dy;
    float weight;
    vec2  offA;
    vec2  offB;
    float bombW;
};

Projection gProj[2];
float gTexelsPerPixel = 0.0;
int gProjCount = 2;

vec3 CalcDirLight(DirLight light, vec3 norm, vec3 viewDir, vec3 baseColor, float specFactor);
vec3 GetOceanColor(float depthFactor);

void SetupProjections(vec3 p, vec3 dpdx, vec3 dpdy, vec3 n)
{
    vec3 an = abs(n);

    int axMajor = (an.x > an.y && an.x > an.z) ? 0 : ((an.y > an.z) ? 1 : 2);
    int a1 = (axMajor + 1) % 3;
    int a2 = (axMajor + 2) % 3;
    int axMid = (an[a1] >= an[a2]) ? a1 : a2;

    int axis[2] = int[2](axMajor, axMid);

    vec2 w = clamp((vec2(an[axMajor], an[axMid]) - 0.5773) / (1.0 - 0.5773), 0.0, 1.0) + 1e-3;
    w /= (w.x + w.y);

    w.y = max(w.y - PROJ_MIN_WEIGHT, 0.0);
    w /= (w.x + w.y);
    gProjCount = (w.y > 0.0) ? 2 : 1;

    for (int i = 0; i < 2; i++)
    {
        int a = axis[i];
        ivec3 ax = ivec3(a, (a + 1) % 3, (a + 2) % 3);

        gProj[i].axes   = ax;
        gProj[i].uv     = vec2(p[ax.y], p[ax.z]);
        gProj[i].dx     = vec2(dpdx[ax.y], dpdx[ax.z]);
        gProj[i].dy     = vec2(dpdy[ax.y], dpdy[ax.z]);
        gProj[i].weight = w[i];
        gProj[i].offA   = vec2(0.0);
        gProj[i].offB   = vec2(0.0);
        gProj[i].bombW  = 0.0;
    }

    gTexelsPerPixel = max(length(gProj[0].dx), length(gProj[0].dy)) * TEX_SIZE;

    if (gTexelsPerPixel < BOMB_MAX_TEXELS)
    {
        for (int i = 0; i < gProjCount; i++)
        {
            float k  = textureLod(noiseMap, gProj[i].uv * BOMB_NOISE_SCALE, 0.0).r;
            float l  = k * 8.0;
            float ia = floor(l + 0.5);
            float ib = floor(l);

            gProj[i].offA = sin(vec2(3.0, 7.0) * ia);
            gProj[i].offB = sin(vec2(3.0, 7.0) * ib);

            if (ia != ib)
            {
                float f = (1.0 - fract(l)) * 2.0;
                gProj[i].bombW = smoothstep(0.2, 0.8, f);
            }
        }
    }
}

vec4 SampleBombed(sampler2DArray tex, int p, int layer)
{
    Projection pr = gProj[p];
    float L = float(layer);

    if (pr.bombW < 0.001)
        return textureGrad(tex, vec3(pr.uv + pr.offA, L), pr.dx, pr.dy);
    if (pr.bombW > 0.999)
        return textureGrad(tex, vec3(pr.uv + pr.offB, L), pr.dx, pr.dy);

    vec4 a = textureGrad(tex, vec3(pr.uv + pr.offA, L), pr.dx, pr.dy);
    vec4 b = textureGrad(tex, vec3(pr.uv + pr.offB, L), pr.dx, pr.dy);
    return mix(a, b, pr.bombW);
}

vec4 SampleDiffuse(int layer)
{
    vec4 c = vec4(0.0);
    for (int i = 0; i < gProjCount; ++i)
        c += SampleBombed(diffuseMap, i, layer) * gProj[i].weight;
    return c;
}

vec3 SampleNormal(int layer, vec3 geoN, float strength)
{
    vec3 result = vec3(0.0);

    for (int i = 0; i < gProjCount; ++i)
    {
        ivec3 ax = gProj[i].axes;

        vec3 tn = SampleBombed(normalMap, i, layer).xyz * 2.0 - 1.0;
        tn.y *= NORMAL_Y_SIGN[layer];
        tn.xy *= strength;

        vec3 v;
        v[ax.x] = abs(tn.z) * geoN[ax.x];
        v[ax.y] = tn.x + geoN[ax.y];
        v[ax.z] = tn.y + geoN[ax.z];

        result += v * gProj[i].weight;
    }
    return result;
}

// ---------------------------------------------------------------- tezine slojeva

void ComputeLayerWeights(float t, float steep, out float w[NUM_LAYERS])
{
    float flatness = 1.0 - steep;

    float shore   = 1.0 - smoothstep(0.00, 0.03, t);
    float beach   = 1.0 - smoothstep(0.02, 0.07, t);
    float lowland = smoothstep(0.03, 0.09, t) * (1.0 - smoothstep(0.22, 0.38, t));
    float hills   = smoothstep(0.20, 0.34, t) * (1.0 - smoothstep(0.42, 0.58, t));
    float rocks   = smoothstep(0.40, 0.55, t) * (1.0 - smoothstep(0.66, 0.80, t));
    float ice     = smoothstep(0.62, 0.74, t) * (1.0 - smoothstep(0.84, 0.95, t));
    float snow    = smoothstep(0.78, 0.92, t);

    float cliff      = steep * smoothstep(0.05, 0.20, t);
    float coastCliff = steep * (1.0 - smoothstep(0.05, 0.20, t));

    w[L_SAND]     = beach * flatness;
    w[L_COBBLE]   = shore * 0.6;
    w[L_SANDROCK] = coastCliff;
    w[L_GRASS]    = lowland * flatness;
    w[L_DIRT]     = hills * flatness + lowland * steep * 0.3;
    w[L_ROCK]     = rocks + cliff + 0.02;
    w[L_ROCKICE]  = ice;
    w[L_SNOW]     = snow * (1.0 - 0.85 * steep);
}

vec3 ShadeLand(float landT, float steep, vec3 geoN, out vec3 outN)
{
    float w[NUM_LAYERS];
    ComputeLayerWeights(landT, steep, w);

    int idx[3];
    float wt[3];
    for (int k = 0; k < 3; ++k)
    {
        int best = 0;
        for (int i = 1; i < NUM_LAYERS; ++i)
            if (w[i] > w[best])
                best = i;

        idx[k]  = best;
        wt[k]   = w[best];
        w[best] = -1.0;
    }

    vec4  d[3];
    float s[3];
    float maxS = -1e9;

    for (int k = 0; k < 3; ++k)
    {
        if (wt[k] > LAYER_MIN_WEIGHT)
        {
            d[k] = SampleDiffuse(idx[k]);
            s[k] = wt[k] + d[k].a * HEIGHT_BLEND_CONTRAST;
        }
        else
        {
            d[k] = vec4(0.0);
            s[k] = -1.0;
        }
        maxS = max(maxS, s[k]);
    }

    float b[3];
    float bSum = 0.0;
    for (int k = 0; k < 3; ++k)
    {
        b[k] = max(s[k] - (maxS - HEIGHT_BLEND_DEPTH), 0.0);
        bSum += b[k];
    }
    bSum = max(bSum, 1e-4);

    vec3 albedo = vec3(0.0);
    for (int k = 0; k < 3; ++k)
        albedo += d[k].rgb * (b[k] / bSum);

    float nFade = 1.0 - smoothstep(NORMAL_FADE_START, NORMAL_FADE_END, gTexelsPerPixel);

    outN = geoN;
    if (nFade > 0.001)
    {
        vec3 nSum = vec3(0.0);
        for (int k = 0; k < 3; ++k)
            if (b[k] > NORMAL_MIN_SHARE * bSum)
                nSum += SampleNormal(idx[k], geoN, normalStrength * nFade) * b[k];

        outN = normalize(nSum);
    }

    return albedo;
}

// ---------------------------------------------------------------- voda

vec3 ShadeWater(float seaT)
{
    vec3 tex = vec3(0.0);
    for (int i = 0; i < gProjCount; ++i)
        tex += textureGrad(texWater, gProj[i].uv + waterOffset, gProj[i].dx, gProj[i].dy).rgb * gProj[i].weight;

    vec3 tint = GetOceanColor(1.0 - seaT);
    return pow(clamp(tex * tint * 2.0, 0.0, 1.0), vec3(2.2));
}

// ---------------------------------------------------------------- main

void main()
{
    vec3 geoN = normalize(objectNormal);
    vec3 viewDir = normalize(viewPos - fragPos);

    vec3 p = objectPos * textureScale;
    vec3 dpdx = dFdx(p);
    vec3 dpdy = dFdy(p);
    SetupProjections(p, dpdx, dpdy, geoN);

    float hMax = max(heightRange.y,  1e-4);
    float hMin = min(heightRange.x, -1e-4);

    float landT = clamp(elevation / hMax, 0.0, 1.0);
    float seaT  = clamp(elevation / hMin, 0.0, 1.0);

    float blendWaterLand = smoothstep(0.0, COAST_WIDTH, elevation / hMax);

    float slope = 1.0 - clamp(dot(geoN, normalize(objectPos)), 0.0, 1.0);
    float steep = smoothstep(0.08, 0.30, slope);

    vec3 waterColor = vec3(0.0);
    if (blendWaterLand < 0.999)
        waterColor = ShadeWater(seaT);

    vec3 landColor = vec3(0.0);
    vec3 landN = geoN;
    if (blendWaterLand > 0.001)
        landColor = ShadeLand(landT, steep, geoN, landN);

    vec3 baseColor = mix(waterColor, landColor, blendWaterLand);

    vec3 nObj = normalize(mix(geoN, landN, blendWaterLand));
    vec3 norm = normalize(normalMatrix * nObj);

    float specFactor = mix(8.0, 4.0, blendWaterLand);

    vec3 result = CalcDirLight(dirLight, norm, viewDir, baseColor, specFactor);

    FragColor = vec4(pow(clamp(result, 0.0, 1.0), vec3(1.0 / 2.2)), 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 norm, vec3 viewDir, vec3 baseColor, float specFactor)
{
    vec3 lightDir = normalize(-light.direction);

    // diffuse shading
    float diff = max(dot(norm, lightDir), 0.0);

    // specular shading
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess * specFactor);

    // combine results
    vec3 ambient  = light.ambient  * baseColor;
    vec3 diffuse  = light.diffuse  * (diff * baseColor);
    vec3 specular = light.specular * (spec * material.specular);

    return ambient + diffuse + specular;
}

vec3 GetOceanColor(float depthFactor)
{
    vec3 c0 = vec3(0.005, 0.015, 0.150);
    vec3 c1 = vec3(0.020, 0.080, 0.400);
    vec3 c2 = vec3(0.000, 0.250, 0.600);
    vec3 c3 = vec3(0.000, 0.450, 0.550);

    if (depthFactor < 0.35)
        return mix(c0, c1, depthFactor / 0.35);
    if (depthFactor < 0.75)
        return mix(c1, c2, (depthFactor - 0.35) / (0.75 - 0.35));
    return mix(c2, c3, (depthFactor - 0.75) / (1.00 - 0.75));
}
