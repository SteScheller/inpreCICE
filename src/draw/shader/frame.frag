#version 330 core
layout(location = 0) out vec4 frag_color;

in vec3 vTexCoord;

uniform vec4 linecolor;

void main()
{
    frag_color = linecolor;
}
