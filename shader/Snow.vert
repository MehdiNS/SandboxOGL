#version 420

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;        
layout (location = 2) in vec2 inTexCoord;                                             

layout (location = 0) out vec3 outWorldPosition;
layout (location = 1) out vec3 outWorldNormal;
layout (location = 2) out vec2 outTexCoord;

layout (binding = 2, std140) uniform matrix_data {
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 viewProjInvMatrix;
	mat4 normalMatrix;
	vec4 cameraPos;
	vec4 sunDir;
	vec4 skyData;
};

uniform mat4 gMVP;
uniform mat4 modelMatrix;

void main()
{
	vec4 wPos = modelMatrix * vec4(inPosition, 1.0);
    outWorldPosition = wPos.xyz;
	gl_Position	= gMVP * wPos;
    outTexCoord = inTexCoord;
    outWorldNormal = normalize((modelMatrix * vec4(inNormal, 0.0f)).xyz);
}
