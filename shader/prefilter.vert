#version 420

layout (location = 0) in vec3 pos;
layout (location = 0) out vec3 worldPos;

uniform mat4 p;
uniform mat4 v;

void main()
{
    worldPos = pos;  
    gl_Position =  p * v * vec4(worldPos, 1.0);
}