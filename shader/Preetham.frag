#version 420

layout(location = 0) out vec4 outFragColor;

layout(location = 0) in vec3 inDirection;

layout (binding = 1, std140) uniform sun_data {
	vec4 A;
	vec4 B;
	vec4 C;
	vec4 D;
	vec4 E;
	vec4 Z;
	vec4 SunDirection;
};

vec3 perez(float cos_theta, float gamma, float cos_gamma, vec3 A, vec3 B, vec3 C, vec3 D, vec3 E)
{
    return (1 + A * exp(B / (cos_theta + 0.01))) * (1 + C * exp(D * gamma) + E * cos_gamma * cos_gamma);
}

float clamp(float x, float a, float b)
{
	return (x < a ? a : (x > b ? b : x));
}

vec3 clamp(vec3 v, float a, float b)
{
	return vec3(clamp(v.x, a, b),clamp(v.y, a, b),clamp(v.z, a, b));
}

vec3 degamma(vec3 v) // to srgb
{
    return pow(v, vec3(2.2));
}

vec3 gamma(vec3 v) // to linear
{
    return pow(v, vec3(1. / 2.2));
}

void main()
{
	vec3 V = normalize(inDirection);

	float cos_theta = clamp(V.z, 0, 1);
	float cos_gamma = dot(V, SunDirection.xyz);
	float gamma_ = acos(cos_gamma);

	vec3 R_xyY = Z.xyz * perez(cos_theta, gamma_, cos_gamma, A.xyz , B.xyz , C.xyz , D.xyz , E.xyz );

	vec3 R_XYZ = vec3(R_xyY.x, R_xyY.y, 1 - R_xyY.x - R_xyY.y) * R_xyY.z / R_xyY.y;

	float R_r = dot(vec3( 3.240479, -1.537150, -0.498535), R_XYZ);
	float R_g = dot(vec3(-0.969256,  1.875992,  0.041556), R_XYZ);
	float R_b = dot(vec3( 0.055648, -0.204043,  1.057311), R_XYZ);

	vec3 R = vec3(R_r, R_g, R_b);

	outFragColor = vec4((clamp(R, 0, 1)), 1);
}