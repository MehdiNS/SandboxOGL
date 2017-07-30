#version 420

layout (location = 0) in vec3 inPos;
layout (location = 0) out vec3 outWorldPos;

uniform mat4 p;
uniform mat4 v;

void main()
{
    outWorldPos = inPos;  

    gl_Position =  p * v * vec4(outWorldPos, 1.0);
}