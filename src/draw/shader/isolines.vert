#version 330 core
layout(location = 0) in vec2 inPosition;

uniform mat3 pvmMx;

void main()
{
    gl_Position = vec4((pvmMx * vec3(inPosition, 1.f)).xy, 0.f, 1.f);
}
