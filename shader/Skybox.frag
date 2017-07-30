#version 420

layout (location = 0) in vec3 inTexCoord;
layout (location = 0) out vec4 outFragColor;

uniform samplerCube skybox;

void main()
{    
	// The skybox is already in linear space
    outFragColor = vec4(texture(skybox, inTexCoord).rgb, 1.);
}