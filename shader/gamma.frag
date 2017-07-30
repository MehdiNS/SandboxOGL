#version 420

layout (location = 0) in vec2 inTexCoord;

layout (location = 0) out vec4 outFragColor;

uniform layout(binding = 0) sampler2D g_texture;

uniform float exposure;

const float A = 0.15;
const float B = 0.50;
const float C = 0.10;
const float D = 0.20;
const float E = 0.02;
const float F = 0.30;
const float W = 11.2;

vec3 Uncharted2Tonemap(vec3 x)
{
    return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

void main()
{
    vec3 hdrColor = texture(g_texture, inTexCoord).rgb;
  
    // Reinhard tone mapping
    //vec3 ldrColor = hdrColor / (hdrColor + vec3(1.0));

	hdrColor *= exposure;
	float ExposureBias = 2.0f;
	vec3 curr = Uncharted2Tonemap(vec3(ExposureBias)*hdrColor);
	vec3 whiteScale = vec3(1.0f)/Uncharted2Tonemap(vec3(W));
	vec3 color = curr*whiteScale;
	vec3 ldrColor = pow(color,vec3(1./2.2));
    outFragColor = vec4(ldrColor, 1.0);
}
