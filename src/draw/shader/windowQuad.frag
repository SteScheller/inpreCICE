#version 330 core
layout(location = 0) out vec4 fragColor;

in vec2 vTexCoord;

uniform sampler2D renderTex;    //!< texture that contains the rendering result

void main()
{
    fragColor = texture(renderTex, vTexCoord);
}

