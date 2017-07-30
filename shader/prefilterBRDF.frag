#version 420
in vec2 TexCoords;
in vec3 envMapCoords;
out vec4 colorOutput;


float PI  = 3.14159265359f;

// G-Buffer
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gRoughness;

uniform sampler2D envMap;
uniform float materialRoughness;
uniform mat4 view;

float saturate(float f);
vec2 getSphericalCoord(vec3 normalCoord);
vec2 Hammersley(int i, int N);
vec3 ImportanceSampleGGX(vec2 Xi, float roughness, vec3 N);
vec3 PrefilterEnvMap(float roughness, vec3 R);


void main()
{
    vec3 N = normalize(texture(gNormal, TexCoords).rgb);
    vec3 V = normalize(- texture(gPosition, TexCoords).rgb);
    vec3 R = 2 * dot( V, N ) * N - V;
    float roughness = texture(gRoughness, TexCoords).r;

    vec3 prefilteredColor = PrefilterEnvMap(roughness, R);
    colorOutput = vec4(prefilteredColor, 1.0f);
}



float saturate(float f)
{
    return clamp(f, 0.0f, 1.0f);
}


vec2 getSphericalCoord(vec3 normalCoord)
{
    float phi = acos(-normalCoord.y);
    float theta = atan(1.0f * normalCoord.x, -normalCoord.z) + PI;

    return vec2(theta / (2.0f * PI), phi / PI);
}


vec2 Hammersley(int i, int N)
{
  return vec2( float(i) / float(N), float(bitfieldReverse(i)) * 2.3283064365386963e-10 );
}


float GeometryAttenuationGGXSmith(float NdotL, float NdotV, float roughness)
{
    float NdotL2 = NdotL * NdotL;
    float NdotV2 = NdotV * NdotV;
    float kRough2 = roughness * roughness + 0.0001f;

    float ggxL = (2.0f * NdotL) / (NdotL + sqrt(NdotL2 + kRough2 * (1.0f - NdotL2)));
    float ggxV = (2.0f * NdotV) / (NdotV + sqrt(NdotV2 + kRough2 * (1.0f - NdotV2)));

    return ggxL * ggxV;
}


vec3 ImportanceSampleGGX(vec2 Xi, float roughness, vec3 N)
{
    float a = roughness * roughness;

    float Phi = 2 * PI * Xi.x;
    float CosTheta = sqrt((1.0f - Xi.y) / (1.0f + (a*a - 1.0f) * Xi.y));
    float SinTheta = sqrt(1.0f - CosTheta * CosTheta);

    vec3 H;
    H.x = SinTheta * cos(Phi);
    H.y = SinTheta * sin(Phi);
    H.z = CosTheta;

    vec3 UpVector = abs(N.z) < 0.999f ? vec3(0.0f, 0.0f, 1.0f) : vec3(1.0f, 0.0f, 0.0f);
    vec3 TangentX = normalize(cross(UpVector, N));
    vec3 TangentY = cross(N, TangentX);

    return normalize(TangentX * H.x + TangentY * H.y + N * H.z);
}


vec3 PrefilterEnvMap(float roughness, vec3 R)
{
    vec3 N = R;
    vec3 V = R;
    vec3 prefilteredColor = vec3(0.0f);
    const int NumSamples = 20;
    float totalWeight = 0.0f;

    for(int i = 0; i < NumSamples; i++)
    {
        vec2 Xi = Hammersley(i, NumSamples);
        vec3 H = ImportanceSampleGGX(Xi, roughness, N);
        vec3 L = 2 * dot(V, H) * H - V;

        float NdotL = saturate(dot(N, L));

        if(NdotL > 0.0f)
        {
            prefilteredColor += textureLod(envMap, getSphericalCoord(L * mat3(view)), 0).rgb * NdotL;
            totalWeight += NdotL;
        }
    }

    return prefilteredColor / totalWeight;
}
