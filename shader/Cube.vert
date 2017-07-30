#version 420

layout (location = 0) in vec3 inPosition;                                             
layout (location = 1) in vec2 inTexCoord;                                             
layout (location = 2) in vec3 inNormal;        

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

layout (location = 0) out vec3 outViewPosition;
layout (location = 1) out vec3 outViewNormal;
layout (location = 2) out vec2 outTexCoord;


void main() {
	vec4 pos		= viewMatrix * (modelMatrix  * vec4(inPosition, 1.f));
	outViewPosition	= pos.xyz;
    gl_Position		= projectionMatrix * pos; 
}