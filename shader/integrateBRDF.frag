#version 400 core

in vec2 TexCoords;
out vec4 colorOutput;


float PI  = 3.14159265359f;

// G-Buffer
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gRoughness;

uniform float materialRoughness;
uniform mat4 view;

float saturate(float f);
vec2 getSphericalCoord(vec3 normalCoord);
vec2 Hammersley(int i, int N);
float GeometryAttenuationGGXSmith(float NdotL, float NdotV, float roughness);
vec3 ImportanceSampleGGX(vec2 Xi, float roughness, vec3 N);
vec2 IntegrateBRDF(float roughness, float NoV);


void main()
{
    vec3 N = normalize(texture(gNormal, TexCoords).rgb);
    vec3 V = normalize(- texture(gPosition, TexCoords).rgb);
    float roughness = texture(gRoughness, TexCoords).r;

    vec2 integratedBRDF = IntegrateBRDF(TexCoords.y, TexCoords.x);
    colorOutput = vec4(integratedBRDF, 0.0f, 1.0f);
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


vec2 IntegrateBRDF(float roughness, float NdotV)
{
    vec3 V;
    V.x = sqrt( 1.0f - NdotV * NdotV );
    V.y = 0.0f;
    V.z = NdotV;

    float A = 0.0f;
    float B = 0.0f;
    const int numSamples = 20;

    vec3 N = vec3(0.0f, 0.0f, 1.0f);

    for(int i = 0; i < numSamples; i++)
    {
        vec2 Xi = Hammersley(i, numSamples);
        vec3 H = ImportanceSampleGGX(Xi, roughness, N);
        vec3 L = normalize(2 * dot(V, H) * H - V);

        float NdotL = saturate(L.z);
        float NdotH = saturate(H.z);
        float VdotH = saturate(dot(V, H));

        if(NdotL > 0.0f)
        {
            float G = GeometryAttenuationGGXSmith(NdotL, NdotV, roughness);
            float G_Vis = (G * VdotH) / (NdotH * NdotV);
            float Fc = pow(1.0f - VdotH, 5.0f);

            A += (1.0f - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }

    return vec2(A, B) / numSamples;
}

