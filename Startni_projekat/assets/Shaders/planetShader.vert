#version 430 core

#include "lod_vertex.glsl"

out vec3 fragPos;
out float elevation;
out vec3 objectPos;
out vec3 objectNormal;

uniform mat4 camMat;

void main()
{
	vec3 aPos;
    vec3 aNormal;
	float aEl;
    ComputeLodVertex(aPos, aNormal, aEl);

	vec4 worldPos = model * vec4(aPos, 1.0f);
	gl_Position = camMat * worldPos;

	fragPos = vec3(worldPos);
	elevation = aEl;

	objectPos = aPos;
    objectNormal = aNormal;
}
