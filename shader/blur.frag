#version 420

layout (location = 0) in vec2 inTexCoord;

uniform layout(binding = 0) sampler2D gTexture;

layout(location = 0, index = 0) out vec4 outFragColor;

vec2 poissonDisk[4] = vec2[](
  vec2( -0.94201624, -0.39906216 ),
  vec2( 0.94558609, -0.76890725 ),
  vec2( -0.094184101, -0.92938870 ),
  vec2( 0.34495938, 0.29387760 )
);

layout (binding = 2, std140) uniform matrix_data {
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 viewProjInvMatrix;
	mat4 normalMatrix;
	vec4 cameraPos;
	vec4 sunDir;
	vec4 skyData;
};

float saturate(float f)
{
    return clamp(f, 0.0f, 1.0f);
}

void main()
{
    float color = 0;
    for(int i=-0;i<=4;++i)
		color += texture( gTexture, inTexCoord + poissonDisk[i]/60.).x;
	color = color/4.;

	// In case it's snowing :)
	//float dt = cameraPos.w;
	//color -= 0.1*dt;
	//color = clamp(color,0.,0.20);
    
	outFragColor = vec4(vec3(color), 1.0);
}