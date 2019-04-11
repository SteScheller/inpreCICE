#version 330 core
layout(location = 0) out vec4 fragColor;

in vec2 vTexCoord;

uniform sampler2D sampleTex;
uniform sampler2D tfTex;

uniform float tfMin;
uniform float tfMax;

void main()
{
    float value = texture(sampleTex, vTexCoord).r;

    fragColor = vec4(
        texture(tfTex,
            vec2((value - tfMin) / (tfMax - tfMin), 0.5f)).rgb,
            1.f);

    //fragColor = vec4(vec3(value), 1.f);

    //fragColor = vec4(texture(tfTex, vTexCoord).rgb, 1.f);

}
