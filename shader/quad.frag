#version 420

layout (location = 0) in vec2 inTexCoord;

layout (location = 0) out vec4 outFragColor;

uniform layout(binding = 0) sampler2D hdrBuffer;

#define RED			vec4(1.0f, 0.f, 0.f, 1.f)
#define BLUE		vec4(0.0f, 0.f, 1.f, 1.f)
#define GREEN		vec4(0.0f, 1.f, 0.f, 1.f)
#define BLACK		vec4(0.0f, 0.f, 0.f, 1.f)
#define WHITE		vec4(1.0f, 1.f, 1.f, 1.f)

/////////////////////////////////////////////////////////

void main () 
{
    outFragColor = vec4(texture2D(hdrBuffer, inTexCoord).rgb, 1.0);
};	

