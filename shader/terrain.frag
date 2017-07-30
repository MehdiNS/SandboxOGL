#version 420

uniform layout(binding = 0) sampler2D gColorMap;
uniform layout(binding = 1) sampler2D gMetalnessMap;
uniform layout(binding = 2) sampler2D gRoughnessMap;
uniform layout(binding = 3) sampler2D gNormalMap;
uniform layout(binding = 4) sampler2D gHeightMap;

layout (location = 0) in vec3 inVPosition;
layout (location = 1) in vec3 inWPosition;
layout (location = 2) in vec3 inWNormal;
layout (location = 3) in vec2 inTexCoord;

layout (location = 0) out vec4 DiffuseRT;
layout (location = 1) out vec4 NormalRT;
layout (location = 2) out vec4 PositionRT;

layout (binding = 2, std140) uniform matrix_data {
    mat4 projectionMatrix;
    mat4 viewMatrix;
	mat4 viewProjInvMatrix;
    mat4 normalMatrix;
    vec4 cameraPos;
    vec4 sunDir;
	vec4 skyData;
};

#define RED			vec4(1.0f, 0.f, 0.f, 1.f)
#define BLUE		vec4(0.0f, 0.f, 1.f, 1.f)
#define GREEN		vec4(0.0f, 1.f, 0.f, 1.f)
#define BLACK		vec4(0.0f, 0.f, 0.f, 1.f)
#define WHITE		vec4(1.0f, 1.f, 1.f, 1.f)
#define GREY		vec4(0.5f, 0.5f, 0.5f, 1.f)

float saturate(float f){return clamp(f, 0.0f, 1.0f);}

mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv)
{
    // get edge vectors of the pixel triangle
    vec3 dp1 = dFdx(p);
    vec3 dp2 = dFdy(p);
    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);
 
    // solve the linear system
    vec3 dp2perp = cross(dp2, N);
    vec3 dp1perp = cross(N, dp1);
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
 
    // construct a scale-invariant frame 
    float invmax = inversesqrt(max(dot(T,T), dot(B,B)));
    return mat3(T * invmax, B * invmax, N);
}

vec3 perturb_normal(vec3 N, vec3 V, vec2 texcoord)
{
    // assume N, the interpolated vertex normal and 
    // V, the view vector (vertex to eye)
    vec3 map = texture(gNormalMap, texcoord).xyz;
    map = map * 2. - 1.;
    mat3 TBN = cotangent_frame(N, -V, texcoord);
    return normalize(TBN * map);
}

/////////////////////////////////////////////////////////

void main()
{	
	vec3 N = normalize(inWNormal);
	vec3 wViewDir = normalize(cameraPos.xyz - inWPosition);
	vec3 vViewDir = normalize(-inVPosition);
	float wetness = skyData.x;
	
	vec3 albedo = texture(gColorMap, inTexCoord).rgb;
	float roughness = texture(gRoughnessMap, inTexCoord).r;
	float metalness = texture(gMetalnessMap, inTexCoord).r;

	float porosity = saturate((roughness - 0.5) / 0.7 );
	float factor = mix(1, 0.2, (1 - metalness) * porosity);
	
	albedo    *= mix(1.0, factor, wetness);
	roughness  = mix(0.0, roughness, mix(1.0, factor, wetness));

	DiffuseRT.xyz = albedo;
	DiffuseRT.w = 1.;
	
	vec3 perturb_N = normalize(perturb_normal(N, wViewDir, inTexCoord));
	NormalRT.xyz = normalize(mix(perturb_N, N, wetness));
	NormalRT.w = roughness;

	PositionRT.xyz = inWPosition;
	PositionRT.w = 0.f;
} 

