#version 330 core
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;

uniform mat4 projMX;    //!< projection matrix

out vec2 vTexCoord;     //!< coordinates for mapping the image to the quad

void main()
{
    gl_Position = projMX * vec4(inPosition, 0.f, 1.f);
    vTexCoord = inTexCoord;
}
