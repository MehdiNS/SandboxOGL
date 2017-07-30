#version 420

layout (location = 0) in vec3 pos;
layout (location = 0) out vec3 worldPos;

uniform mat4 p;
uniform mat4 v;

float clamp(float x, float a, float b){	return (x < a ? a : (x > b ? b : x));}
vec3 clamp(vec3 v, float a, float b){return vec3(clamp(v.x, a, b),clamp(v.y, a, b),clamp(v.z, a, b));}

void main()
{
    worldPos = pos;  
    gl_Position =  p * v * vec4(pos, 1.0);
}
l