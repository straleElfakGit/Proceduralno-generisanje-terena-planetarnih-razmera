#version 430 core

uniform mat4 viewRot; 
uniform mat4 projection;
uniform vec3 sunDir;
uniform float quadHalfSize;

out vec2 vUV;

const vec2 CORNERS[4] = vec2[4](
    vec2(-1.0, -1.0),
    vec2( 1.0, -1.0),
    vec2(-1.0,  1.0),
    vec2( 1.0,  1.0)
);

void main()
{
    vec2 c = CORNERS[gl_VertexID];

    vec3 helper = (abs(sunDir.y) < 0.99) ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    vec3 right = normalize(cross(helper, sunDir));
    vec3 up = cross(sunDir, right);

    vec3 dir = sunDir + (c.x * right + c.y * up) * quadHalfSize;

    vec4 clip = projection * viewRot * vec4(dir, 1.0);
    gl_Position = clip.xyww;

    vUV = c;
}
