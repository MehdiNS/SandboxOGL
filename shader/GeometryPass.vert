#version 420

layout (location = 0) in vec3 inPosition;                                             
layout (location = 1) in vec3 inNormal;        
layout (location = 2) in vec2 inTexCoord;

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
layout (location = 1) out vec3 outWorldPosition;
layout (location = 2) out vec3 outWorldNormal;
layout (location = 3) out vec2 outTexCoord;

void main()
{
	vec4 wPos = modelMatrix * vec4(inPosition, 1.0);
    outWorldPosition = wPos.xyz;
	vec4 vPos = viewMatrix * wPos;
	outViewPosition = vPos.xyz;
	gl_Position	= projectionMatrix * vPos;
	float scale = 1.;//modelMatrix[0][0];
    outTexCoord = scale * inTexCoord;
    outWorldNormal = normalize((modelMatrix * vec4(inNormal, 0.0f)).xyz);
}