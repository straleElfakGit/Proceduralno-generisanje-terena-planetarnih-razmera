#version 330 core

out vec4 FragColor;

in vec3 normal;
in vec3 fragPos;
in float height;

uniform vec3 sunDirection; 
uniform vec3 sunColor;     
uniform vec3 ambientColor;

vec3 GetColorByHeight(float h)
{
    if (h < 1.5) 
        return vec3(0.2, 0.4, 0.8);  // Voda / Plava
    if (h < 3.2) 
        return vec3(0.8, 0.7, 0.4);  // Pesak / Žuta
    if (h < 6.0) 
        return vec3(0.2, 0.6, 0.2); // Trava / Zelena
    if (h < 8.5) 
        return vec3(0.4, 0.4, 0.4); // Stena / Siva
    return vec3(0.9, 0.9, 0.9);     // Sneg / Bela
}

void main()
{
	vec3 baseColor = GetColorByHeight(height);

    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(-sunDirection);

    vec3 ambient = ambientColor * baseColor;

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = sunColor * diff * baseColor;

    vec3 finalColor = ambient + diffuse;

    FragColor = vec4(finalColor, 1.0);
}