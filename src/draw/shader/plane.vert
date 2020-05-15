#version 330 core
layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 tex_coords;

uniform mat4 pvmMx;

out vec2 vTexCoord;
out vec3 vNormal;

void main()
{
    gl_Position = pvmMx * vec4(in_position, 0.f, 1.f);
    vTexCoord = tex_coords;
    vNormal = (pvmMx * vec4(0.f, 0.f, 1.f, 0.f)).xyz;
}
