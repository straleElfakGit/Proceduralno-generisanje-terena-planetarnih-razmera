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
in vec3 objectPos;
in float biomePercent;
in float elevation;

uniform Material material;
uniform DirLight dirLight;
uniform vec3 viewPos;

uniform vec2 elevationMinMax;
uniform sampler2D colorGradient;
uniform int biomeCount;

vec3 GetBaseColor();
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 baseColor, float shininessFactor); 
float invLerp(float a, float b, float v);
float GetPositiveElevationPercent(float elev, float maxElev);
float GetNegativeElevationPercent(float elev, float minElev);

void main()
{
    vec3 norm = normalize(normal);
    vec3 viewDir = normalize(viewPos - fragPos);
    
    float landPercent = invLerp(0.0, elevationMinMax.y, elevation);
    float seaPercent  = invLerp(elevationMinMax.x, 0.0, elevation);
    float isLand = floor(seaPercent);
    float u = mix(mix(0.0, 0.5, seaPercent), mix(0.5, 1.0, landPercent), isLand);

    float n = float(max(biomeCount, 1));
    float v = (biomePercent * (n - 1.0) + 0.5) / n;

    vec3 baseColor = texture(colorGradient, vec2(u, v)).rgb;
    float shininessFactor = 1.0 + isLand * 7.0;


    vec3 result = CalcDirLight(dirLight, norm, viewDir, baseColor, shininessFactor);
    FragColor = vec4(result, 1.0f);
}

vec3 GetBaseColor()
{
    float maxValue = GetPositiveElevationPercent(elevation, elevationMinMax.y);
    float maxFactor = mix(0.5, 1.0, maxValue);
    float minValue = GetNegativeElevationPercent(elevation, elevationMinMax.x);
    float minFactor = mix(0.0, 0.5, minValue);
    float floorMinValue = floor(minValue);

    float res = (floorMinValue * maxFactor) + (1.0 - floorMinValue) * minFactor; 
    return texture(colorGradient, vec2(res, biomePercent)).rgb;
}

vec3 CalcDirLight(DirLight light, vec3 norm, vec3 viewDir, vec3 baseColor, float shininessFactor)
{
    vec3 lightDir = normalize(-light.direction);
    
    // diffuse shading
    float diff = max(dot(norm, lightDir), 0.0);
    
    // specular shading
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess * shininessFactor);
    
    // combine results
    vec3 ambient  = light.ambient  * baseColor;
    vec3 diffuse  = light.diffuse  * (diff * baseColor);
    vec3 specular = light.specular * (spec * material.specular);
    
    return (ambient + diffuse + specular);
} 

float invLerp(float a, float b, float v) 
{
    return clamp((v - a) / (b - a), 0.0, 1.0);
}

float GetPositiveElevationPercent(float elev, float maxElev) 
{
    return invLerp(0.0, maxElev, elev);
}

float GetNegativeElevationPercent(float elev, float minElev) 
{
    return invLerp(minElev, 0.0, elev); 
}