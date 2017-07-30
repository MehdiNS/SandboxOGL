#version 420

layout (location = 0) in vec2 inTexCoord;

uniform layout(binding = 0) sampler2D ColorBuffer;
uniform layout(binding = 1) sampler2D NormalBuffer;
uniform layout(binding = 2) sampler2D PositionBuffer;
uniform layout(binding = 3) sampler2D DepthBuffer;
uniform layout(binding = 4) sampler2D LtcAmp;
uniform layout(binding = 5) sampler2D LtcMat;
uniform layout(binding = 6) sampler2D LightTexture;
 
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

// Adapted from http://blog.selfshadow.com/sandbox/ltc.html
float saturate(float x) {  return max(0, min(1, x));}
vec3 saturate(vec3 v) {  return vec3(max(0, min(1, v.x)),max(0, min(1, v.y)),max(0, min(1, v.z)));}
vec3 PowVec3(vec3 v, float p){ return vec3(pow(v.x, p), pow(v.y, p), pow(v.z, p));}
const float gamma = 2.2;
vec3 ToLinear(vec3 v) { return PowVec3(v,     gamma); }
vec3 ToSRGB(vec3 v)   { return PowVec3(v, 1.0/gamma); }

float LinearizeDepth(float depth)
{
	float n = cameraPos.w; // camera z near
	float f = sunDir.w; // camera z far
	return (2.0 * n) / (f + n - depth * (f - n));	
}

const float PI = 3.1415f;
const float LUT_SIZE  = 32.0;
const float LUT_SCALE = (LUT_SIZE - 1.0)/LUT_SIZE;
const float LUT_BIAS  = 0.5/LUT_SIZE;

struct shading
{
	vec3 diffuse;
	vec3 specular;
};

uniform mat4 modelMatrix;
uniform vec4 pointsList[4];
uniform float intensity;
uniform vec3 color;
/////////////////////////////////////////////////////////

vec2 LTC_Coords(float cosTheta, float roughness)
{
    float theta = acos(cosTheta);
    vec2 coords = vec2(roughness, theta/(0.5*PI));

    // scale and bias coordinates, for correct filtered lookup
    coords = coords*(LUT_SIZE - 1.0)/LUT_SIZE + 0.5/LUT_SIZE;

    return coords;
}

mat3 LTC_Matrix(sampler2D texLSDMat, vec2 coord)
{
    // load inverse matrix
    vec4 t = texture2D(texLSDMat, coord);
    mat3 Minv = mat3(
        vec3(1,     0, t.y),
        vec3(  0, t.z,   0),
        vec3(t.w,   0, t.x)
    );
    return Minv;
}

float IntegrateEdge(vec3 v1, vec3 v2)
{
    float cosTheta = dot(v1, v2);
    cosTheta = clamp(cosTheta, -0.9999, 0.9999);

    float theta = acos(cosTheta);    
    float res = cross(v1, v2).z * theta / sin(theta);

    return res;
}

void ClipQuadToHorizon(inout vec3 L[5], out int n)
{
    // detect clipping config
    int config = 0;
    if (L[0].z > 0.0) config += 1;
    if (L[1].z > 0.0) config += 2;
    if (L[2].z > 0.0) config += 4;
    if (L[3].z > 0.0) config += 8;

    // clip
    n = 0;

    if (config == 0)
    {
        // clip all
    }
    else if (config == 1) // V1 clip V2 V3 V4
    {
        n = 3;
        L[1] = -L[1].z * L[0] + L[0].z * L[1];
        L[2] = -L[3].z * L[0] + L[0].z * L[3];
    }
    else if (config == 2) // V2 clip V1 V3 V4
    {
        n = 3;
        L[0] = -L[0].z * L[1] + L[1].z * L[0];
        L[2] = -L[2].z * L[1] + L[1].z * L[2];
    }
    else if (config == 3) // V1 V2 clip V3 V4
    {
        n = 4;
        L[2] = -L[2].z * L[1] + L[1].z * L[2];
        L[3] = -L[3].z * L[0] + L[0].z * L[3];
    }
    else if (config == 4) // V3 clip V1 V2 V4
    {
        n = 3;
        L[0] = -L[3].z * L[2] + L[2].z * L[3];
        L[1] = -L[1].z * L[2] + L[2].z * L[1];
    }
    else if (config == 5) // V1 V3 clip V2 V4) impossible
    {
        n = 0;
    }
    else if (config == 6) // V2 V3 clip V1 V4
    {
        n = 4;
        L[0] = -L[0].z * L[1] + L[1].z * L[0];
        L[3] = -L[3].z * L[2] + L[2].z * L[3];
    }
    else if (config == 7) // V1 V2 V3 clip V4
    {
        n = 5;
        L[4] = -L[3].z * L[0] + L[0].z * L[3];
        L[3] = -L[3].z * L[2] + L[2].z * L[3];
    }
    else if (config == 8) // V4 clip V1 V2 V3
    {
        n = 3;
        L[0] = -L[0].z * L[3] + L[3].z * L[0];
        L[1] = -L[2].z * L[3] + L[3].z * L[2];
        L[2] =  L[3];
    }
    else if (config == 9) // V1 V4 clip V2 V3
    {
        n = 4;
        L[1] = -L[1].z * L[0] + L[0].z * L[1];
        L[2] = -L[2].z * L[3] + L[3].z * L[2];
    }
    else if (config == 10) // V2 V4 clip V1 V3) impossible
    {
        n = 0;
    }
    else if (config == 11) // V1 V2 V4 clip V3
    {
        n = 5;
        L[4] = L[3];
        L[3] = -L[2].z * L[3] + L[3].z * L[2];
        L[2] = -L[2].z * L[1] + L[1].z * L[2];
    }
    else if (config == 12) // V3 V4 clip V1 V2
    {
        n = 4;
        L[1] = -L[1].z * L[2] + L[2].z * L[1];
        L[0] = -L[0].z * L[3] + L[3].z * L[0];
    }
    else if (config == 13) // V1 V3 V4 clip V2
    {
        n = 5;
        L[4] = L[3];
        L[3] = L[2];
        L[2] = -L[1].z * L[2] + L[2].z * L[1];
        L[1] = -L[1].z * L[0] + L[0].z * L[1];
    }
    else if (config == 14) // V2 V3 V4 clip V1
    {
        n = 5;
        L[4] = -L[0].z * L[3] + L[3].z * L[0];
        L[0] = -L[0].z * L[1] + L[1].z * L[0];
    }
    else if (config == 15) // V1 V2 V3 V4
    {
        n = 4;
    }
    
    if (n == 3)
        L[3] = L[0];
    if (n == 4)
        L[4] = L[0];
}

vec3 FetchDiffuseFilteredTexture(sampler2D texLightFiltered, vec3 p1_, vec3 p2_, vec3 p3_, vec3 p4_)
{
    // area light plane basis
    vec3 V1 = p2_ - p1_;
    vec3 V2 = p4_ - p1_;
    vec3 planeOrtho = (cross(V1, V2));
    float planeAreaSquared = dot(planeOrtho, planeOrtho);
    float planeDistxPlaneArea = dot(planeOrtho, p1_);
    // orthonormal projection of (0,0,0) in area light space
    vec3 P = planeDistxPlaneArea * planeOrtho / planeAreaSquared - p1_;

    // find tex coords of P
    float dot_V1_V2 = dot(V1,V2);
    float inv_dot_V1_V1 = 1.0 / dot(V1, V1);
    vec3 V2_ = V2 - V1 * dot_V1_V2 * inv_dot_V1_V1;
    vec2 Puv;
    Puv.y = dot(V2_, P) / dot(V2_, V2_);
    Puv.x = dot(V1, P)*inv_dot_V1_V1 - dot_V1_V2*inv_dot_V1_V1*Puv.y ;

    // LOD
    float d = abs(planeDistxPlaneArea) / pow(planeAreaSquared, 0.75);

    return texture2DLod(texLightFiltered, vec2(0.125, 0.125) + 0.75 * Puv, log(2048.0*d)/log(3.0) ).rgb;
}

vec3 LTC_Evaluate(vec3 N, vec3 V, vec3 P, mat3 Minv, vec4 points[4], bool twoSided)
{
    // construct orthonormal basis around N
    vec3 T1, T2;
    T1 = normalize(V - N*dot(V, N));
    T2 = cross(N, T1);

    // rotate area light in (T1, T2, R) basis
    Minv = Minv * transpose(mat3(T1, T2, N));

    // polygon (allocate 5 vertices for clipping)
    vec3 L[5];
    L[0] = Minv * (points[0].xyz - P);
    L[1] = Minv * (points[1].xyz - P);
    L[2] = Minv * (points[2].xyz - P);
    L[3] = Minv * (points[3].xyz - P);
    L[4] = L[3]; // avoid warning

    vec3 textureLight = color;
	//textureLight = FetchDiffuseFilteredTexture(LightTexture, L[0], L[1], L[2], L[3]);
    int n;
    ClipQuadToHorizon(L, n);
    
    if (n == 0)
        return vec3(0, 0, 0);

    // project onto sphere
    L[0] = normalize(L[0]);
    L[1] = normalize(L[1]);
    L[2] = normalize(L[2]);
    L[3] = normalize(L[3]);
    L[4] = normalize(L[4]);

    // integrate
    float sum = 0.0;

    sum += IntegrateEdge(L[0], L[1]);
    sum += IntegrateEdge(L[1], L[2]);
    sum += IntegrateEdge(L[2], L[3]);
    if (n >= 4)
        sum += IntegrateEdge(L[3], L[4]);
    if (n == 5)
        sum += IntegrateEdge(L[4], L[0]);

    // note: negated due to winding order
    sum = twoSided ? abs(sum) : max(0.0, -sum);

    vec3 Lo_i = vec3(sum, sum, sum);

    // scale by filtered light color
    Lo_i *= textureLight;

    return Lo_i;
}

void main()
{
	vec2 uv = inTexCoord;

	// reconstruct world-space position from depth
	float depth = texture(DepthBuffer, uv).x;

	vec4 pb = texture(PositionBuffer, uv);
	vec3 positionWS = pb.xyz;
	float metalness = pb.w;
	
	vec4 cb = texture2D(ColorBuffer, uv);
	vec3 albedo =  ToLinear(cb.xyz);
	vec3 diffuseColor = albedo - albedo * metalness;

	vec4 nb = texture(NormalBuffer, uv);
	vec3 N = normalize(nb.xyz);
	float roughness = nb.w;
	//roughness = pow(pow(roughness, 4.0) + pow(0.498, 4.0), 0.25);
    //roughness = max(roughness, 0.03);
	vec3 V = normalize(cameraPos.xyz-positionWS.xyz);
	bool twoSided = true;

    vec3 diff = LTC_Evaluate(N, V, positionWS, mat3(1), pointsList, twoSided); 
	
	 // scale by light intensity
    diff *= intensity;
    // scale by diffuse albedo
    diff *= diffuseColor;
    // normalize
    diff /= 2.0f * PI;

	vec2 coords = LTC_Coords(dot(N, V), roughness);
	mat3 Minv = LTC_Matrix(LtcMat, coords);
    vec3 spec = LTC_Evaluate(N, V, positionWS, Minv, pointsList, twoSided);
    spec *= intensity;
	
	// apply BRDF scale terms (BRDF magnitude and Schlick Fresnel)
    vec2 schlick = texture2D(LtcAmp, coords).xy;
	vec3 specColor = mix(vec3(0.04), albedo, metalness);
    spec *= specColor*schlick.x + (1.0 - specColor)*schlick.y;
	spec /= 2.0f * PI;

	outDiffuseColor = vec4(diff, 1.f);
	outSpecularColor = vec4(spec, 1.f);
	//outDiffuseColor = vec4(vec3(0), 1.f);
	//outSpecularColor = vec4(vec3(0), 1.f);
}