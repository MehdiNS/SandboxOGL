#version 420

layout (location = 0) in vec2 inTexCoord;

uniform layout(binding = 0) sampler2D ColorBuffer;
uniform layout(binding = 1) sampler2D NormalBuffer;
uniform layout(binding = 2) sampler2D PositionBuffer;
uniform layout(binding = 3) sampler2D DepthBuffer;
uniform layout(binding = 4) sampler2DShadow ShadowMapBuffer;
uniform layout(binding = 5) samplerCube EnvMapBuffer;
uniform layout(binding = 6) samplerCube PrefilterMap;
uniform layout(binding = 7) sampler2D BrdfLUT;
uniform layout(binding = 8) sampler2D AoMap;
 
layout(location = 0) out vec4 outDiffuseColor;
layout(location = 1) out vec4 outSpecularColor;

layout (binding = 2, std140) uniform matrix_data {
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 viewProjInvMatrix;
	mat4 normalMatrix;
	vec4 cameraPos;
	vec4 sunDir;
	vec4 skyData;
};

#define MAX_NUM_TOTAL_LIGHTS 10
struct Light {
  vec4 A;
  vec4 B;
  vec4 C;
  vec4 D;
  mat4 lightSpaceMat;
};

vec3 GetLightColor(Light l) { return l.A.xyz; }
vec3 GetLightPos(Light l) { return l.B.xyz; }
vec3 GetLightAtt(Light l) { return l.C.xyz; }

layout (binding = 5, std140) uniform point_light_data {
	Light light;
};

struct shading
{
	vec3 diffuse;
	vec3 specular;
};

float saturate(float x) {  return max(0, min(1, x));}
vec3 saturate(vec3 v) {  return vec3(max(0, min(1, v.x)),max(0, min(1, v.y)),max(0, min(1, v.z)));}

float LinearizeDepth(float depth)
{
	float n = cameraPos.w; // camera z near
	float f = sunDir.w; // camera z far
	return (2.0 * n) / (f + n - depth * (f - n));	
}

#define PI 3.1415f
#define RED			vec4(1.0f, 0.f, 0.f, 1.f)
#define BLUE		vec4(0.0f, 0.f, 1.f, 1.f)
#define GREEN		vec4(0.0f, 1.f, 0.f, 1.f)
#define BLACK		vec4(0.0f, 0.f, 0.f, 1.f)
#define WHITE		vec4(1.0f, 1.f, 1.f, 1.f)

vec2 poissonDisk[16] = vec2[](	 vec2(-0.94201624, -0.39906216)
								,vec2(0.94558609, -0.76890725)
								,vec2(-0.094184101, -0.92938870)
								,vec2(0.34495938, 0.29387760)
								,vec2(-0.91588581, 0.45771432)
								,vec2(-0.81544232, -0.87912464)
								,vec2(-0.38277543, 0.27676845)
								,vec2(0.97484398, 0.75648379)
								,vec2(0.44323325, -0.97511554)
								,vec2(0.53742981, -0.47373420)
								, vec2(-0.26496911, -0.41893023)
								, vec2(0.79197514, 0.19090188)
								, vec2(-0.24188840, 0.99706507)
								, vec2(-0.81409955, 0.91437590)
								, vec2(0.19984126, 0.78641367)
								,vec2(0.14383161, -0.14100790));

vec3 BlinnPhong(vec3 N, vec3 positionVS)
{
	vec3 lightDir;
	vec3 lightColor = light.A.xyz;
	float light_distance;
	float attenuation;

	lightDir = (viewMatrix * vec4(sunDir.xyz, 0.f)).xyz;
	attenuation = 1.f;
	
	// Blinn Phong
	vec3 V = normalize(-positionVS);
	vec3 L = normalize(lightDir);  
	vec3 H = normalize(V+L);
	float NdotL = dot(N,L);
	float NdotH = dot(N,H);
	
	vec3 diffuse = vec3(max(0.0f,NdotL));
	vec3 spec = vec3(max(0.f,pow(max(0.f,NdotH), 10.f)));
	
	vec3 finalColor = vec3(0);
	if (NdotL > 0.f && attenuation > 0.f)
		finalColor = lightColor * attenuation * (diffuse + spec);
	return finalColor;
}

// Microfacet specular = D*G*F / (4*NoL*NoV) = D*Vis*F
// Vis = G / (4*NoL*NoV)

float rcp(float x) { return 1./x; }

vec3 Diffuse_Lambert( vec3 DiffuseColor )
{
	return DiffuseColor * (1 / PI);
}

vec3 Diffuse_Burley( vec3 DiffuseColor, float Roughness, float NoV, float NoL, float VoH )
{
	float FD90 = 0.5 + 2 * VoH * VoH * Roughness;
	float FdV = 1 + (FD90 - 1) * pow( 1 - NoV, 5);
	float FdL = 1 + (FD90 - 1) * pow( 1 - NoL, 5);
	return DiffuseColor * ( (1 / PI) * FdV * FdL );
}

vec3 Diffuse_OrenNayar( vec3 DiffuseColor, float Roughness, float NoV, float NoL, float VoH )
{
	float a = Roughness * Roughness;
	float s = a;// / ( 1.29 + 0.5 * a );
	float s2 = s * s;
	float VoL = 2 * VoH * VoH - 1;		// double angle identity
	float Cosri = VoL - NoV * NoL;
	float C1 = 1 - 0.5 * s2 / (s2 + 0.33);
	float C2 = 0.45 * s2 / (s2 + 0.09) * Cosri * ( Cosri >= 0 ? rcp( max( NoL, NoV ) ) : 1 );
	return DiffuseColor / PI * ( C1 + C2 ) * ( 1 + Roughness * 0.5 );
}

// GGX / Trowbridge-Reitz
// [Walter et al. 2007, "Microfacet models for refraction through rough surfaces"]
float D_GGX( float Roughness, float NoH )
{
	float a = Roughness * Roughness;
	float a2 = a * a;
	float d = ( NoH * a2 - NoH ) * NoH + 1;	// 2 mad (= NoH^2 * a2 - NoH^2 = NoH^2  * (a2 - 1))
	return a2 / ( PI*d*d );					// 4 mul, 1 rcp
}

// Appoximation of joint Smith term for GGX
// [Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"]
float Vis_SmithJointApprox( float Roughness, float NoV, float NoL )
{
	float a = Roughness * Roughness;
	float Vis_SmithV = NoL * ( NoV * ( 1 - a ) + a )	+ 0.0001; //(= 
	float Vis_SmithL = NoV * ( NoL * ( 1 - a ) + a )	+ 0.0001;
	// Note: will generate NaNs with Roughness = 0.  MinRoughness is used to prevent this
	return 0.5 * rcp( Vis_SmithV + Vis_SmithL );
}

// [Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"]
vec3 F_Schlick( vec3 SpecularColor, float VoH )
{
	float Fc = pow( 1 - VoH, 5 );					// 1 sub, 3 mul
	//return Fc + (1 - Fc) * SpecularColor;		// 1 add, 3 mad
	
	// Anything less than 2% is physically impossible and is instead considered to be shadowing
	return saturate( 50.0 * SpecularColor.g ) * Fc + (1 - Fc) * SpecularColor;	
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}  

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return nom / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}


shading BRDF(vec3 baseColor, vec3 specColor, float metalness, float roughness, vec3 L, vec3 V, vec3 N )
{
	shading s;
	vec3 H = normalize(V + L);
	float NoH = saturate( dot(N, H) );
	float VoH = saturate( dot(V, H) );
	float NoL = saturate( dot(N, L) );
	float NoV = saturate( dot(N, V) );


	// Generalized microfacet specular
	// Microfacet specular = D*G*F / (4*NoL*NoV) = D*Vis*F
	// Vis = G / (4*NoL*NoV)
	float D = D_GGX( roughness, NoH );
	float Vis = Vis_SmithJointApprox( roughness, NoV, NoL );
	vec3 F = F_Schlick( specColor, VoH );

	// Energy conservation
    vec3 kS = F;
    vec3 kD = vec3(1.0f) - kS;
	kD *= 1.0f - metalness;

	//vec3 diffuse = kD * Diffuse_Burley( baseColor, roughness, NoV, NoL, VoH );
	vec3 diffuse = kD * Diffuse_Lambert( baseColor );
	vec3 spec = kS * (D * Vis) * F;

	NoL = max(NoL, 0);

	float illuminance = 10.*NoL;

	s.diffuse  = diffuse * NoL * illuminance;
	s.specular = spec  * NoL * illuminance;
	return s;
}

/////////////////////////////////////////////////////////

float ShadowCalculation(vec4 fragPosLightSpace)
{
	// [-1;1] to [0;1]
    // perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    float currentDepth = projCoords.z;
	float visibility = 1.0;

	vec2 texelSize = 1.0 / textureSize(ShadowMapBuffer, 0);
	float bias = 0.005;
    //float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;
	// Sample the shadow map 4 times
	int nbSample = 16;
	float shadowStep = 1./ nbSample;
	for (int i=0; i<nbSample; ++i)
	{
		visibility -= shadowStep*(1-texture( ShadowMapBuffer, vec3(projCoords.xy + poissonDisk[i]/500.0,  (projCoords.z-bias))));
	}

	return visibility;
}  

vec3 degamma(vec3 v) // to srgb
{
    return pow(v, vec3(2.2));
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}   

void main()
{
	vec2 uv = inTexCoord;
	vec2 uvShadow = gl_FragCoord.xy / vec2(512.f);

	//float depth = texture(DepthBuffer, uv).x;

	vec4 pb = texture(PositionBuffer, uv);
	vec3 positionWS = pb.xyz;
	vec4 positionLS = light.lightSpaceMat * vec4(positionWS,1.f);
	float metalness = pb.w;
	
	vec4 cb = texture2D(ColorBuffer, uv);
	vec3 baseColor =  degamma(cb.xyz);
	vec3 diffuseColor = baseColor - baseColor * metalness;

	vec4 nb = texture(NormalBuffer, uv);
	vec3 N = normalize(nb.xyz);
	float roughness = nb.w;
	vec3 F0 = mix(vec3(0.04f), baseColor, metalness);
	
	vec3 V = normalize(cameraPos.xyz-positionWS.xyz);
	vec3 L = normalize(sunDir.xyz);  
    float visibility = ShadowCalculation(positionLS);
	
	shading direct = BRDF(diffuseColor, F0, metalness, roughness, L, V, N );
	
	// TODO : The IBL is done at the same time than the directional light right now because of 
	//	1) my laziness
	//  2) my GPU is a potato and I can't really afford a separate pass without killing framerate  
	vec3 R = normalize(reflect(-V, N));  
	vec3 kS = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness); 
	vec3 kD = 1.0 - kS;
	kD *= 1.0f - metalness;
	vec3 irradiance = texture(EnvMapBuffer, N).rgb;
	vec3 iDiffuse  = irradiance * baseColor;
	vec3 diffuseIndirect   = (kD * iDiffuse); 
	
	//vec3 R_env = normalize(cameraPos.xyz + R*5000.f); 
	// trying ARM parallax corrected cubemap 
	vec3 localPosWS = positionWS;
	vec3 intersectMaxPointPlanes = (500 - localPosWS) / R;
	vec3 intersectMinPointPlanes = (-500 - localPosWS) / R;
	vec3 largestRayParams = max(intersectMaxPointPlanes, intersectMinPointPlanes);
	float distToIntersect = min(min(largestRayParams.x, largestRayParams.y), largestRayParams.z);
	vec3 intersectPositionWS = localPosWS + R * distToIntersect;
	vec3 _EnviCubeMapPos = vec3(cameraPos);
	vec3 R_env = intersectPositionWS - _EnviCubeMapPos;
	
	const float MAX_REFLECTION_LOD = 4.0;
	vec3 prefilteredColor = textureLod(PrefilterMap, R_env,  roughness * MAX_REFLECTION_LOD).rgb;   
	vec2 envBRDF  = texture(BrdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
	vec3 specularIndirect = prefilteredColor * (kS * envBRDF.x + envBRDF.y);
	
	float ao = texture(AoMap, uv).x;
	outDiffuseColor = vec4(visibility * direct.diffuse + ao*diffuseIndirect, 1.f);
	outSpecularColor = vec4(visibility * direct.specular + ao*specularIndirect, 1.f);
	//outDiffuseColor = vec4(diffuseIndirect, 1.f);
	//outDiffuseColor = vec4(visibility * direct.diffuse, 1.f);
	//outSpecularColor = vec4(visibility * direct.specular, 1.f);
	//outSpecularColor = vec4(specularIndirect, 1.f);
	//outDiffuseColor = vec4(vec3(0), 1.f);
	//outSpecularColor = vec4(vec3(0.), 1.f);
}