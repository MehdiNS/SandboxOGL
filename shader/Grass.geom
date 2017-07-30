#version 420
layout (points) in;
layout (triangle_strip, max_vertices = 9) out;

#define RED			vec4(1.0f, 0.f, 0.f, 1.f)
#define BLUE		vec4(0.0f, 0.f, 1.f, 1.f)
#define GREEN		vec4(0.0f, 1.f, 0.f, 1.f)
#define BLACK		vec4(0.0f, 0.f, 0.f, 1.f)
#define WHITE		vec4(1.0f, 1.f, 1.f, 1.f)
#define GREY		vec4(0.5f, 0.5f, 0.5f, 1.f)

layout (binding = 2, std140) uniform matrix_data {
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 viewProjInvMatrix;
	mat4 normalMatrix;
	vec4 cameraPos;
	vec4 sunDir;
	vec4 skyData;
};

in int grassMagic[];

out vec2 TexCoord;
out vec3 WorldPos;
out float Magic;
out vec3 Normal;

vec3 Hermite(vec3 p0, vec3 p1, vec3 v0, vec3 v1, float t)
{
	// calculate blending functions
	float b0 =  2*t*t*t - 3*t*t + 1;
	float b1 = -2*t*t*t + 3*t*t;
	float b2 = t*t*t - 2*t*t + t;
	float b3 = t*t*t - t*t;
	
	// calculate the x,y and z of the curve point
	float x = b0*p0.x + 
			  b1*p1.x + 
			  b2*v0.x + 
			  b3*v1.x ;
	
	float y = b0*p0.y + 
			  b1*p1.y + 
			  b2*v0.y + 
			  b3*v1.y ;
	
	float z = b0*p0.z + 
			  b1*p1.z + 
			  b2*v0.z + 
			  b3*v1.z ;
	
	return vec3(x,y,z);
}


float hash(float n) { return fract(sin(n) * 1e4); }
float hash(vec2 p) { return fract(1e4 * sin(17.0 * p.x + p.y * 0.1) * (0.1 + abs(sin(p.y * 13.0 + p.x)))); }

float noise(float x) {
    float i = floor(x);
    float f = fract(x);
    float u = f * f * (3.0 - 2.0 * f);
    return mix(hash(i), hash(i + 1.0), u);
}

float noise(vec2 x) {
    vec2 i = floor(x);
    vec2 f = fract(x);

    // Four corners in 2D of a tile
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    // Same code, with the clamps in smoothstep and common subexpressions
    // optimized away.
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

float parity(float n)
{
	if (n > 0.5) return -1.f;
	else return 1.f;
}

vec3 reverseNormalIfNotInFront(vec3 normalA, vec3 A, vec3 B)
{
    vec3 AB = B - A;
    double dotP = dot(AB, normalA);
    bool inFront = (dotP > 0);
	if (!inFront)
		return -normalA;
	else 
		return normalA;
}

const float pi = 3.14159;
const float twoPi = 2. * pi; 

void main() {   

	int magic = grassMagic[0];
	
	//Origin of the particle
	vec3 s0 = gl_in[0].gl_Position.xyz;

	float rx = fract(noise(magic * s0.xz));
	float rz = fract(noise(magic * s0.zx));

	float angle = twoPi * fract(noise(magic));
	vec3 quadDir = normalize( vec3(cos(angle), 0, sin(angle)));

	float dx = parity(rx) * 0.1 *rx;
	float dz = parity(rz) * 0.1 *rz;
	//float dx = parity(rx) * clamp(rx, 0.5, 0.10);
	//float dy = parity(ry) * clamp(ry, 0.5, 0.10);
	float dy = clamp(0.20 * abs(fract(noise(magic))), 0.15, 0.55);

	//Animation
	float time = skyData.y;
	dx *= cos(quadDir.x*time*dy);
	dz *= cos(quadDir.y*time*dy);
	//dx *= cos(time*dy);

	//End of the particle
	vec3 s1 = gl_in[0].gl_Position.xyz 	+ vec3(dx, dy, dz);
	vec3 v0 = vec3(0, 0.1, 0);
	vec3 v1 = vec3(0, -0.05, 0);

	float bladeHalfWidth = 0.01f;
	quadDir *= bladeHalfWidth;

	vec3 p0 = Hermite(s0, s1, v0, v1, 0) - quadDir;
	vec3 p1 = Hermite(s0, s1, v0, v1, 0) + quadDir;
	vec3 p2 = Hermite(s0, s1, v0, v1, 0.5) - quadDir*0.7;
	vec3 p3 = Hermite(s0, s1, v0, v1, 0.5) + quadDir*0.7;
	vec3 p4 = Hermite(s0, s1, v0, v1, 0.6) - quadDir*0.5;
	vec3 p5 = Hermite(s0, s1, v0, v1, 0.6) + quadDir*0.5;
	vec3 p6 = Hermite(s0, s1, v0, v1, 0.75) - quadDir*0.15;
	vec3 p7 = Hermite(s0, s1, v0, v1, 0.75) + quadDir*0.15;
	vec3 p8 = Hermite(s0, s1, v0, v1, 1) + vec3(0,quadDir.y,0);
	vec3 n0 = reverseNormalIfNotInFront(normalize(cross(p1-p0, p2-p0)), p0, cameraPos.xyz); 
	vec3 n1 = n0;
	vec3 n2 = reverseNormalIfNotInFront(normalize(cross(p3-p2, p4-p2)), p2, cameraPos.xyz); 
	vec3 n3 = n2;
	vec3 n4 = reverseNormalIfNotInFront(normalize(cross(p5-p4, p6-p4)), p4, cameraPos.xyz);  
	vec3 n5 = n4;
	vec3 n6 = reverseNormalIfNotInFront(normalize(cross(p7-p6, p8-p6)), p6, cameraPos.xyz);  
	vec3 n7 = n6;
	vec3 n8 = n6;

	mat4 pv = projectionMatrix * viewMatrix;
	vec3 worldPos;

	worldPos = p0;
	gl_Position = pv * vec4(worldPos, 1);
	WorldPos = worldPos;
	TexCoord = vec2(0,0);
	Magic = magic;
	Normal = n0;
	EmitVertex();

	worldPos = p1;
	gl_Position = pv * vec4(worldPos, 1);
	WorldPos = worldPos;
	TexCoord = vec2(1,0);
	Magic = magic;
	Normal = n1;
	EmitVertex();
	
	worldPos = p2;
	gl_Position = pv * vec4(worldPos, 1);
	WorldPos = worldPos;
	TexCoord = vec2(0,0.25);
	Magic = magic;
	Normal = n2;
	EmitVertex();

	worldPos = p3;
	gl_Position = pv * vec4(worldPos, 1);
	WorldPos = worldPos;
	TexCoord = vec2(1,0.25);
	Magic = magic;
	Normal = n3;
	EmitVertex();	

	worldPos = p4;
	gl_Position = pv * vec4(worldPos, 1);
	WorldPos = worldPos;
	TexCoord = vec2(0,0.5);
	Magic = magic;
	Normal = n4;
	EmitVertex();

	worldPos = p5;
	gl_Position = pv * vec4(worldPos, 1);
	WorldPos = worldPos;
	TexCoord = vec2(1,0.5);
	Magic = magic;
	Normal = n5;
	EmitVertex();	
	
	worldPos = p6;
	gl_Position = pv * vec4(worldPos, 1);
	WorldPos = worldPos;
	TexCoord = vec2(0,0.75);
	Magic = magic;
	Normal = n6;
	EmitVertex();

	worldPos = p7;
	gl_Position = pv * vec4(worldPos, 1);
	WorldPos = worldPos;
	TexCoord = vec2(1,0.75);
	Magic = magic;
	Normal = n7;
	EmitVertex();		

	worldPos = p8;
	gl_Position = pv * vec4(worldPos, 1);
	WorldPos = worldPos;
	TexCoord = vec2(0.5,1.);
	Magic = magic;
	Normal = n8;
	EmitVertex();
	
	EndPrimitive();
}  