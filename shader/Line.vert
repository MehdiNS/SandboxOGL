#version 420

layout (location = 0) in vec3 inPosition;                                    

layout (binding = 2, std140) uniform matrix_data {
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 viewProjInvMatrix;
	mat4 normalMatrix;
	vec4 cameraPos;
	vec4 sunDir;
	vec4 skyData;
};

//layout (binding = 3, std140) uniform mesh_data {
//	mat4 modelMatrix;
//};
uniform mat4 modelMatrix;

void main() 
{
    gl_Position	= projectionMatrix * viewMatrix * vec4(inPosition, 1.f); 
}
