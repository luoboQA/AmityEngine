#version 460 core
layout(location = 0) in vec2 aPos;       // Position in screen space [-1, 1]
layout(location = 1) in vec2 aTexCoord;  // Texture coordinates [0, 1]

out vec2 TexCoord;

void main()
{
    TexCoord = aTexCoord;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
