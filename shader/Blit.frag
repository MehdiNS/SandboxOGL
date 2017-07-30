#version 420

uniform layout(binding = 0) sampler2D InputBuffer;

layout (location = 0) in vec2 inTexCoord;
layout (location = 0) out vec4 outFragColor;

void main()
{
	vec3 color = texture(InputBuffer, inTexCoord).xyz;
 	outFragColor = vec4(color, 1.);
}