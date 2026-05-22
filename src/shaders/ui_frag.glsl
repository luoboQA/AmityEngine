#version 460 core
in vec2 TexCoord;

uniform sampler2D u_Texture;
uniform vec4 u_Color;
uniform bool u_UseTexture;

out vec4 FragColor;

void main()
{
    if (u_UseTexture)
    {
        // For text rendering, our texture holds alpha/grayscale in the red channel
        float textAlpha = texture(u_Texture, TexCoord).r;
        FragColor = vec4(u_Color.rgb, u_Color.a * textAlpha);
    }
    else
    {
        FragColor = u_Color;
    }
}
