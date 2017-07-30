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

layout (location = 0) out vec3 outVPosition;
layout (location = 1) out vec3 outWPosition;
layout (location = 2) out vec3 outWNormal;
layout (location = 3) out vec2 outTexCoord;

uniform layout(binding = 0) sampler2D gColorMap;
uniform layout(binding = 1) sampler2D gMetalnessMap;
uniform layout(binding = 2) sampler2D gRoughnessMap;
uniform layout(binding = 3) sampler2D gNormalMap;
uniform layout(binding = 4) sampler2D gHeightMap;

#define TERRAIN_DIMENSION vec2(32.f)

void main() {
	vec2 xz = (inPosition.xz + vec2(16.f)) / TERRAIN_DIMENSION;
	float t = texture2D(gHeightMap,xz).x;
	float vMin = -1.f;
	float vMax = 8.f;
	float y = (1.f - t)*vMin + t*vMax;

    vec4 posWS		 = vec4(inPosition.x, y, inPosition.z, 1.f);
	vec4 posVS		 = viewMatrix * posWS;
    gl_Position		 = projectionMatrix * posVS;
	outTexCoord      = inTexCoord;
	outWPosition	 = posWS.xyz;
	outVPosition	 = posVS.xyz;
	outWNormal		 = normalize(inNormal);
}

 