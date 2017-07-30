// vertex shader
#version 420

layout (binding = 2, std140) uniform matrix_data {
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 viewProjInvMatrix;
	mat4 normalMatrix;
	vec4 cameraPos;
	vec4 sunDir;
	vec4 skyData;
};

uniform layout(binding = 0) sampler2D gColorMap;
uniform layout(binding = 1) sampler2D gMetalnessMap;
uniform layout(binding = 2) sampler2D gRoughnessMap;
uniform layout(binding = 3) sampler2D gNormalMap;
uniform layout(binding = 4) sampler2D gHeightTexture;
uniform layout(binding = 5) sampler2D gGrassTexture;

out int grassMagic;

int extract(int value, int begin, int end)
{
    int mask = (1 << (end - begin)) - 1;
    return (value >> begin) & mask;
}

float rand(vec2 n) { 
    return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

float noise(vec2 p){
    vec2 ip = floor(p);
    vec2 u = fract(p);
    u = u*u*(3.0-2.0*u);

    float res = mix(
        mix(rand(ip),rand(ip+vec2(1.0,0.0)),u.x),
        mix(rand(ip+vec2(0.0,1.0)),rand(ip+vec2(1.0,1.0)),u.x),u.y);
    return res*res;
}

float clamp(float x, float a, float b)
{
  return max(a, min(b, x));
}

#define TERRAIN_DIMENSION vec2(10.f)

void main() 
{
	int maxi = 16;
	float x = extract(gl_InstanceID, 0, maxi/2);
	float z = extract(gl_InstanceID, maxi/2, maxi);
	float dx = noise(vec2(x,z));
	float dz = noise(vec2(z,x));
	x = (x+dx)/(2<<( maxi/2 -1));
	z = (z+dz)/(2<<( maxi/2 -1));
	//float t = texture2D(gHeightTexture,vec2(x,z)).x;
	//float vMin = 0.f;
	//float vMax = 5.f;
	//float alt = (1.f - t)*vMin + t*vMax;
	vec2 temp =  vec2(x,z)*TERRAIN_DIMENSION-(0.5*TERRAIN_DIMENSION);
	gl_Position = vec4(temp.x, 0, temp.y, 1);
	grassMagic = gl_InstanceID; 
	//gl_PointSize = 5.f;
}