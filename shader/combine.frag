#version 420

uniform layout(binding = 0) sampler2D InputBuffer0;
uniform layout(binding = 1) sampler2D InputBuffer1;

layout (location = 0) in vec2 inTexCoord;
layout (location = 0) out vec4 outFragColor;

void main()
{
	vec3 color0 = texture(InputBuffer0, inTexCoord).xyz;
	vec3 color1 = texture(InputBuffer1, inTexCoord).xyz;
 	//outFragColor = vec4(0.5*(color0+color1), 1.);
	outFragColor = vec4(color0, 1.);
}