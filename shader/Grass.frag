#version 420

in vec2 TexCoord; 
in vec3 WorldPos;
in float Magic;
in vec3 Normal;

layout (location = 0) out vec4 DiffuseRT;
layout (location = 1) out vec4 NormalRT;
layout (location = 2) out vec4 PositionRT;

uniform layout(binding = 0) sampler2D gColorMap;
uniform layout(binding = 1) sampler2D gMetalnessMap;
uniform layout(binding = 2) sampler2D gRoughnessMap;
uniform layout(binding = 3) sampler2D gNormalMap;
uniform layout(binding = 4) sampler2D gHeightTexture;
uniform layout(binding = 5) sampler2D gGrassTexture;

layout (binding = 2, std140) uniform matrix_data {
    mat4 projectionMatrix;
    mat4 viewMatrix;
	mat4 viewProjInvMatrix;
    mat4 normalMatrix;
    vec4 cameraPos;
    vec4 sunDir;
	vec4 skyData;
};

float hash(float n) { return fract(sin(n) * 1e4); }
float hash(vec2 p) { return fract(1e4 * sin(17.0 * p.x + p.y * 0.1) * (0.1 + abs(sin(p.y * 13.0 + p.x)))); }

float noise(float x) {
    float i = floor(x);
    float f = fract(x);
    float u = f * f * (3.0 - 2.0 * f);
    return mix(hash(i), hash(i + 1.0), u);
}

void main() 
{
	vec3 color = texture2D(gGrassTexture,TexCoord).xyz;
	//float t = fract(noise(Magic));
	//if(t < 0.33f)
	//	color.r = color.r - (30.f/255.f);
	//else if(t < 0.66f)
	//	color.b = color.b + (20.f/255.f);

	//vec3 lightDir = sunDir;
	//vec3 V = normalize(-(WorldPos-cameraPos));
	//vec3 L = normalize(lightDir);  
	//vec3 H = normalize(V+L);
	//float NdotL = dot(Normal,L);
	//float NdotH = dot(Normal,H);
	//
	//vec3 IAmbiant = vec3(0.3);
	//vec3 IDiffuse = vec3(max(0.f,NdotL));

	float AO = mix(0.0, 0.7, WorldPos.y / 0.07);
	//color *= AO;

	DiffuseRT.xyz = color;
	DiffuseRT.w = 0.f;
	NormalRT.xyz = normalize(Normal);
	NormalRT.w = 0.4f;//roughness;
	PositionRT.xyz = WorldPos;
	PositionRT.w = 0.f;//metalness;


}