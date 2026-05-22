#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Proj;
uniform vec4 u_MaterialColor;
uniform mat3 u_NormalMatrix;

out vec3 Normal;
out vec3 FragPos;
out vec4 MaterialColor;
out vec2 TexCoord;

void main()
{
    // pass absolute position to frag
    vec4 v = vec4(aPos, 1.0);
    FragPos = vec3(u_Model * v);

    TexCoord = aTexCoord;
    MaterialColor = u_MaterialColor;
    
    Normal = u_NormalMatrix * aNormal; // world space of normal

    gl_Position = u_Proj * u_View * u_Model * v;
}
