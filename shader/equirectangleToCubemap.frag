#version 420

layout (location = 0) in vec3 inWPosition;  
layout (location = 0) out vec3 fragColor;

uniform layout(binding = 0) sampler2D equirectangularMap;

#define RED			vec4(1.0f, 0.f, 0.f, 1.f)
#define BLUE		vec4(0.0f, 0.f, 1.f, 1.f)
#define GREEN		vec4(0.0f, 1.f, 0.f, 1.f)
#define BLACK		vec4(0.0f, 0.f, 0.f, 1.f)
#define WHITE		vec4(1.0f, 1.f, 1.f, 1.f)

#define PI 3.1415
#define TWO_PI (3.1415*2.)

//vec2 SampleSphericalMap(vec3 dir)
//{
//	vec2 uv;
//	uv.x = atan( -dir.z, dir.x );
//	uv.y = acos( -dir.y );
//	uv /= vec2( TWO_PI, PI );
//	return uv;
//}

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{		
    vec2 uv = SampleSphericalMap(normalize(inWPosition));
    vec3 color = texture(equirectangularMap, uv).rgb;
    
    fragColor = color;
}


