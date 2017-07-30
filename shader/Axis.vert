#version 420

layout (location = 0) in vec3 inPosition;                                             
layout (location = 1) in vec3 inColor;                                             
layout (location = 0) out vec3 outColor;

layout (binding = 2, std140) uniform matrix_data {
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 viewProjInvMatrix;
	mat4 normalMatrix;
	vec4 cameraPos;
	vec4 sunDir;
	vec4 skyData;
};

uniform mat4 modelMatrix;

void main() 
{
    gl_Position	= projectionMatrix * viewMatrix * modelMatrix * vec4(inPosition, 1.f);; 
	outColor	= inColor;
}
