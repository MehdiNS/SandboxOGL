#version 420

layout (location = 0) in vec3 inVPosition;
layout (location = 1) in vec3 inWPosition;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec2 inTexCoord;

layout (location = 0) out vec4 DiffuseRT;
layout (location = 1) out vec4 NormalRT;
layout (location = 2) out vec4 PositionRT;

uniform layout(binding = 0) sampler2D gColorMap;
uniform layout(binding = 1) sampler2D gMetalnessMap;
uniform layout(binding = 2) sampler2D gRoughnessMap;
uniform layout(binding = 3) sampler2D gNormalMap;
uniform layout(binding = 4) sampler2D gHeightMap;

uniform bool useParallaxMapping;

layout (binding = 2, std140) uniform matrix_data {
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 viewProjInvMatrix;
	mat4 normalMatrix;
	vec4 cameraPos;
	vec4 sunDir;
	vec4 skyData;
};

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

vec2 parallax_mapping(vec2 texCoords, vec3 viewDir, out float cur_layer_depth)
{ 
	float height_scale = 0.05;
    float height =  texture(gHeightMap, texCoords).r;    
	
	// Simple parallax mapping
    //vec2 p = viewDir.xy / viewDir.z * (height * height_scale);

	// Simple parallax mapping with offset limiting
	//vec2 p = viewDir.xy * (height * height_scale);
   	//return texCoords - p;    

	//Steep Paralax Mapping
	float num_layers = 50;
	float layer_depth = 1.0 / num_layers;
    cur_layer_depth = 0.0;
    vec2 delta_uv = viewDir.xy * height_scale / (viewDir.z * num_layers);
    vec2 cur_uv = texCoords;

    float depth_from_tex = texture(gHeightMap, cur_uv).r;

    for (int i = 0; i < 32; i++) {
        cur_layer_depth += layer_depth;
        cur_uv -= delta_uv;
        depth_from_tex = texture(gHeightMap, cur_uv).r;
        if (depth_from_tex < cur_layer_depth) {
            break;
        }
    }
    //return cur_uv;
	
	// Start of Relief Parallax Mapping
	// decrease shift and height of layer by half
	vec2 deltaTexCoord = delta_uv / 2;
	float deltaHeight = layer_depth / 2;
	// return to the mid point of previous layer
	cur_uv += deltaTexCoord;
	cur_layer_depth -= deltaHeight;
	// binary search to increase precision of Steep Paralax Mapping
	const int numSearches = 5;
	for(int i=0; i<numSearches; i++)
	{
		// decrease shift and height of layer by half
		deltaTexCoord /= 2;
		deltaHeight /= 2;
		// new depth from heightmap
		height = texture(gHeightMap, cur_uv).r;
		// shift along or agains vector V
		if(height > cur_layer_depth) // below the surface
		{
			cur_uv -= deltaTexCoord;
			cur_layer_depth += deltaHeight;
		}
		else // above the surface
		{
			cur_uv += deltaTexCoord;
			cur_layer_depth -= deltaHeight;
		}
	}	
	return cur_uv;
} 

void main()
{
	vec3 N = normalize(inNormal);
	vec3 wViewDir = normalize(cameraPos.xyz - inWPosition);
	vec3 vViewDir = normalize(-inVPosition);
	
	vec2 newTexCoord = inTexCoord;
	float layer_depth = 0.f;
	if(useParallaxMapping)
	{
		mat3 TBN = cotangent_frame(N, -wViewDir, inTexCoord);
		vec3 tangentViewDir = normalize(transpose(TBN) * wViewDir);
		newTexCoord = parallax_mapping(inTexCoord, tangentViewDir, layer_depth);
		if(newTexCoord.x > 1.0 || newTexCoord.y > 1.0 || newTexCoord.x < 0.0 || newTexCoord.y < 0.0)
			discard;
	}

	float wetness = skyData.x;
	
	vec3 albedo = texture(gColorMap, inTexCoord).rgb;
	float roughness = texture(gRoughnessMap, inTexCoord).r;
	float metalness = texture(gMetalnessMap, inTexCoord).r;

	float porosity = saturate((roughness - 0.5) / 0.7 );
	float factor = mix(1, 0.2, (1 - metalness) * porosity);

	albedo    *= mix(1.0, factor, wetness);
	roughness  = mix(0.0, roughness, mix(1.0, factor, wetness));

	DiffuseRT.xyz = mix(vec3(0.0), albedo, 1.0-layer_depth);
	DiffuseRT.w = 1.;
	
	vec3 perturb_N = normalize(perturb_normal(N, wViewDir, newTexCoord));
	NormalRT.xyz = normalize(mix(perturb_N, N, wetness));
	NormalRT.w = roughness;

	PositionRT.xyz = inWPosition;
	PositionRT.w = metalness;
} 