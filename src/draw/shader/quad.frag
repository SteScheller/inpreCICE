#version 330 core
layout(location = 0) out vec4 fragColor;

in vec2 vTexCoord;

uniform sampler2D renderTex;    //!< texture that contains the 2D result of
                                //!< the rendering

void main()
{
    fragColor = texture(renderTex, vTexCoord);
}

