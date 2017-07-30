#version 420

layout (location = 0) in vec3 inPosition;
uniform mat4 gMVP;
uniform mat4 modelMatrix;

void main(void)
{
	gl_Position = gMVP*modelMatrix*vec4(inPosition,1);
}