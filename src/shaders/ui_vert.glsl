#version 460 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;

uniform mat4 u_Proj; // Orthographic projection matrix

out vec2 TexCoord;

void main()
{
    TexCoord = aTexCoord;
    gl_Position = u_Proj * vec4(aPos, 0.0, 1.0);
}
