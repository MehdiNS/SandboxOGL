#version 420

const float Rg = 6360.0;
const float Rt = 6420.0;
const float RL = 6421.0;

const int TRANSMITTANCE_W = 256;
const int TRANSMITTANCE_H = 64;

const int SKY_W = 64;
const int SKY_H = 16;

const int RES_R = 32;
const int RES_MU = 128;
const int RES_MU_S = 32;
const int RES_NU = 8;

const float AVERAGE_GROUND_REFLECTANCE = 0.1;

// Rayleigh
const float HR = 8.0;
const vec3 betaR = vec3(5.8e-3, 1.35e-2, 3.31e-2);

// Mie
// DEFAULT
const float HM = 1.2;
const vec3 betaMSca = vec3(4e-3);
const vec3 betaMEx = betaMSca / 0.9;
const float mieG = 0.8;
// CLEAR SKY
/*const float HM = 1.2;
const vec3 betaMSca = vec3(20e-3);
const vec3 betaMEx = betaMSca / 0.9;
const float mieG = 0.76;*/
// PARTLY CLOUDY
/*const float HM = 3.0;
const vec3 betaMSca = vec3(3e-3);
const vec3 betaMEx = betaMSca / 0.9;
const float mieG = 0.65;*/

// ----------------------------------------------------------------------------
// NUMERICAL INTEGRATION PARAMETERS
// ----------------------------------------------------------------------------

const int TRANSMITTANCE_INTEGRAL_SAMPLES = 500;
const int INSCATTER_INTEGRAL_SAMPLES = 50;
const int IRRADIANCE_INTEGRAL_SAMPLES = 32;
const int INSCATTER_SPHERICAL_INTEGRAL_SAMPLES = 16;

const float mPI = 3.141592657;

// ----------------------------------------------------------------------------
// PARAMETERIZATION OPTIONS
// ----------------------------------------------------------------------------

#define TRANSMITTANCE_NON_LINEAR
#define INSCATTER_NON_LINEAR


layout (location = 0) in vec3 inPosition;   
layout (location = 1) in vec2 inTexCoord;    

layout (location = 0) out vec3 outPosition;
layout (location = 1) out vec2 outTexCoord;

uniform sampler3D deltaSRSampler;
uniform sampler3D deltaSMSampler;
uniform float first;

void main () 
{
	outTexCoord = inTexCoord;
	outPosition = inPosition;
	gl_Position = vec4 (inPosition, 1.0f);
};