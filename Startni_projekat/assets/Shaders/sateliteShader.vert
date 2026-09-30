#version 430 core

layout (location = 0) in vec3 aPos;

uniform mat4 projview;
uniform mat4 model;
uniform float cubeScale;

void main()
{
	vec3 scalePos = aPos * cubeScale;
	vec4 pos = projview * model * vec4(scalePos, 1.0f);
	gl_Position = vec4(pos);
}