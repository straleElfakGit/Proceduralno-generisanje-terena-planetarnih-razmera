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

in vec3 normal;
in vec3 fragPos;
in float elevation;
in vec3 objectPos;

uniform Material material;
uniform DirLight dirLight;
uniform vec3 viewPos;
uniform float maxElevation;

uniform sampler2D texWater;
uniform sampler2D texSand;
uniform sampler2D texGrass;
uniform sampler2D texStonyGrass;
uniform sampler2D texRock;
uniform sampler2D texMountain;
uniform sampler2D texSnow;

uniform float textureScale = 20.0; 
uniform vec2 waterOffset;

const float PI = 3.14159265359;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 baseColor, float specFactor); 
vec3 GetOceanColor(float depthFactor);
vec3 GetTriplanar(sampler2D tex, vec3 pos, vec3 blend, float scale, vec2 offset);

void main()
{
    vec3 norm = normalize(normal);
    vec3 viewDir = normalize(viewPos - fragPos);

    vec3 blendAxis = abs(norm);
    blendAxis = pow(blendAxis, vec3(4.0));
    blendAxis /= (blendAxis.x + blendAxis.y + blendAxis.z);

    vec3 colorWater      = GetTriplanar(texWater, objectPos, blendAxis, textureScale, waterOffset);
    vec3 colorSand       = GetTriplanar(texSand, objectPos, blendAxis, textureScale, vec2(0.0));
    vec3 colorGrass      = GetTriplanar(texGrass, objectPos, blendAxis, textureScale, vec2(0.0));
    vec3 colorStonyGrass = GetTriplanar(texStonyGrass, objectPos, blendAxis, textureScale, vec2(0.0));
    vec3 colorRock       = GetTriplanar(texRock, objectPos, blendAxis, textureScale, vec2(0.0));
    vec3 colorMountain   = GetTriplanar(texMountain, objectPos, blendAxis, textureScale, vec2(0.0));
    vec3 colorSnow       = GetTriplanar(texSnow, objectPos, blendAxis, textureScale, vec2(0.0));

    float maxOceanDepth = -maxElevation * 0.5;     
    float depthFactor = smoothstep(maxOceanDepth, 0.0, elevation);
    vec3 waterTint = GetOceanColor(depthFactor);
    vec3 finalWaterColor = colorWater * waterTint * 2.0;

    float blendSandGrass    = smoothstep(maxElevation * 0.01, maxElevation * 0.04, elevation);
    float blendGrassStony   = smoothstep(maxElevation * 0.08, maxElevation * 0.12, elevation);
    float blendStonyRock    = smoothstep(maxElevation * 0.16, maxElevation * 0.26, elevation);
    float blendRockMountain = smoothstep(maxElevation * 0.32, maxElevation * 0.36, elevation);
    float blendMountainSnow = smoothstep(maxElevation * 0.38, maxElevation * 0.60, elevation);

    vec3 landColor = mix(colorSand, colorGrass, blendSandGrass);
    landColor = mix(landColor, colorStonyGrass, blendGrassStony);
    landColor = mix(landColor, colorRock, blendStonyRock);
    landColor = mix(landColor, colorMountain, blendRockMountain);
    landColor = mix(landColor, colorSnow, blendMountainSnow);

    float blendWaterLand = smoothstep(0.0, maxElevation * 0.01, elevation);
    vec3 baseColor = mix(finalWaterColor, landColor, blendWaterLand);

    float specFactor = mix(8.0, 4.0, blendWaterLand);

    vec3 result = CalcDirLight(dirLight, norm, viewDir, baseColor, specFactor);
    FragColor = vec4(result, 1.0f);
}

vec3 GetTriplanar(sampler2D tex, vec3 pos, vec3 blend, float scale, vec2 offset)
{
    vec2 uvX = pos.yz * scale + offset;
    vec2 uvY = pos.xz * scale + offset;
    vec2 uvZ = pos.xy * scale + offset;

    vec3 colX = texture(tex, uvX).rgb;
    vec3 colY = texture(tex, uvY).rgb;
    vec3 colZ = texture(tex, uvZ).rgb;

    return colX * blend.x + colY * blend.y + colZ * blend.z;
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
    
    return (ambient + diffuse + specular);
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