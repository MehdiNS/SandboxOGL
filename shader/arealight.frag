#version 420

layout (location = 0) in vec2 inTexCoord;
layout (location = 0) out vec4 outFragColor;

layout (binding = 2, std140) uniform matrix_data {
    mat4 projectionMatrix;
    mat4 viewMatrix;
    mat4 viewProjInvMatrix;
    mat4 normalMatrix;
    vec4 cameraPos;
    vec4 sunDir;
    vec4 skyData;
}
;
#define RED			vec4(1.0f, 0.f, 0.f, 1.f)
#define BLUE		vec4(0.0f, 0.f, 1.f, 1.f)
#define GREEN		vec4(0.0f, 1.f, 0.f, 1.f)
#define BLACK		vec4(0.0f, 0.f, 0.f, 1.f)
#define WHITE		vec4(1.0f, 1.f, 1.f, 1.f)

uniform float intensity;
uniform vec3 color;
/////////////////////////////////////////////////////////

void main() 
{
	vec3 lightColor = pow(intensity * color, vec3(2.2));
    outFragColor = vec4(lightColor, 1);
}
