#version 420

layout (location = 0) in vec3 inPosition;

layout (binding = 2, std140) uniform matrix_data {
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 normalMatrix;
	vec3 cameraPos;
	float wetness;
};

//layout (binding = 3, std140) uniform mesh_data {
//	mat4 modelMatrix;
//};
uniform mat4 modelMatrix;

layout (location = 0) out vec3 outDirection;

void main()
{
    outDirection = normalize(inPosition);
    vec4 PosWS = modelMatrix * vec4(inPosition, 1.f);
	gl_Position = projectionMatrix * (viewMatrix * PosWS);
}