#version 420

layout (location = 0) out vec3 outTexCoord;

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
	float x = -1.0 + float((gl_VertexID & 1) << 2);
    float y = -1.0 + float((gl_VertexID & 2) << 1);
    vec4 pos = vec4(x, y, 1, 1);

	// TODO : Get rid of it
	mat4 invView = inverse(viewMatrix);
	mat4 invProj = inverse(projectionMatrix);
	
	outTexCoord = mat3(invView) * (invProj * pos).xyz;
    
	gl_Position = pos;
}