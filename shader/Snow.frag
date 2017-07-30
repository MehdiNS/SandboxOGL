#version 420

layout (location = 0) in vec3 inWPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoord;

layout (location = 0) out vec4 nextHeightmap;

uniform layout(binding = 0) sampler2D lastHeightmap;

float LinearizeDepth(float depth)
{
	float n = 0.01f; // camera z near
	float f = 0.10f; // camera z far
	depth = depth*2.f -1.f;
	return (2.f * n) / (f + n - depth * (f - n));	
}

float saturate(float f)
{
    return clamp(f, 0.0f, 1.0f);
}

void main()
{
	vec2 uv = gl_FragCoord.xy / vec2(256.f);
	float currentLinearDepth = 1.f-(gl_FragCoord.z);
	float temp = texture(lastHeightmap,uv).x;
	if(texture(lastHeightmap,uv).x < currentLinearDepth)
		temp = currentLinearDepth;
	nextHeightmap = vec4(vec3(saturate(temp)), 1);
}