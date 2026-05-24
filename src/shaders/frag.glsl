#version 460 core

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;
in vec4 MaterialColor;

uniform sampler2D u_Texture;
uniform int u_HasTexture;

out vec4 FragColor;

void main()
{
    vec4 baseColor = MaterialColor;
    if (u_HasTexture == 1)
    {
        baseColor = texture(u_Texture, TexCoord);
    }

    vec3 ambient = 0.4 * baseColor.rgb;
    vec3 LightDir = -vec3(-1.0, -0.5, -1.0);
    vec3 diffuse = max(dot(normalize(LightDir), normalize(Normal)), 0.0) * baseColor.rgb * 3.0;

    FragColor = vec4(diffuse + ambient, baseColor.a);
}
