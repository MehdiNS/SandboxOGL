#version 420

layout (location = 0) in vec3 inWPosition;
layout (location = 1) in vec3 inWNormal;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in vec2 inWNormalUV1;
layout (location = 4) in vec2 inWNormalUV2;

layout (location = 0) out vec4 DiffuseRT;
layout (location = 1) out vec4 NormalRT;
layout (location = 2) out vec4 PositionRT;

uniform layout(binding = 0) sampler2D gNormalMap;

layout (binding = 2, std140) uniform matrix_data {
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 viewProjInvMatrix;
	mat4 normalMatrix;
	vec4 cameraPos;
	vec4 sunDir;
	vec4 skyData;
};

void main()
{
	vec3 color = vec3(0.1f, 0.2f, 0.25f);

	DiffuseRT.xyz = color;
	DiffuseRT.w = 1.f;

	vec3 wViewDir = normalize(cameraPos.xyz - inWPosition);
	vec3 N = normalize(inWNormal);
	
	float dist = length(cameraPos.xyz - inWPosition);
	float normalMapAttenuation = 0.1;
	vec3 normal1 = normalize(texture(gNormalMap, inWNormalUV1) * 2.0 - 1.0).xyz;
	vec3 normal2 = normalize(texture(gNormalMap, inWNormalUV2) * 2.0 - 1.0).xyz;
	N = normalize(N + normalize(normal1 + normal2) * normalMapAttenuation);

	NormalRT.xyz = N;
	NormalRT.w = 0.;//roughness;

	PositionRT.xyz = inWPosition;
	PositionRT.w = 0.f;//metalness;
} 