#version 330 core
layout(location = 0) out vec4 fragColor;

in vec2 vTexCoord;
in vec3 vNormal;

uniform sampler2D fractureTex;

uniform vec3 lightDir;

void main()
{
    vec4 textureColor = texture(fractureTex, vTexCoord);

    vec3 normal = normalize(vNormal);

	if (!gl_FrontFacing)
        normal *= -1.f;

    fragColor = vec4(textureColor.rgb *
            (0.6f + clamp(
                dot(normal, normalize(lightDir)),
                0.f,
                1.f) ),
            textureColor.a);
}

