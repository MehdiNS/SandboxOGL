#version 420

layout (location = 0) in vec2 inTexCoord;
layout (location = 0) out vec4 outFragColor;

uniform layout(binding = 0) sampler2D diffuseBuffer;
uniform layout(binding = 1) sampler2D specularBuffer;

void main () 
{
	vec3 diffuse	= texture2D(diffuseBuffer, inTexCoord).rgb;
	vec3 specular	= texture2D(specularBuffer, inTexCoord).rgb;
    outFragColor = vec4(diffuse + specular, 1.0);
};	



