#version 420

layout (location = 0) in vec3 inPosition;                                             
layout (location = 1) in vec3 inNormal;        
layout (location = 2) in vec2 inTexCoord;                                             

layout (binding = 2, std140) uniform matrix_data {
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 viewProjInvMatrix;
	mat4 normalMatrix;
	vec4 cameraPos;
	vec4 sunDir;
	vec4 skyData;
};

float absTime = skyData.y;
uniform mat4 modelMatrix;

layout (location = 0) out vec3 outWorldPosition;
layout (location = 1) out vec3 outWorldNormal;
layout (location = 2) out vec2 outTexCoord;
layout (location = 3) out vec2 outNormalUV1;
layout (location = 4) out vec2 outNormalUV2;

#define PI 3.14159f
#define TWOPI (2.f*3.14159f)

float saturate(float x)
{
  return max(0.f, min(1.f, x));
}

// 4 waves processed at the same time, can be really slow otherwise
vec3 GerstnerOffset(
	vec2 xzPos, vec4 steepness, vec4 amplitude, vec4 frequency,
	vec4 speed, vec4 dirAB, vec4 dirCD, vec4 time)
{
	vec3 offset;

	vec4 AB = steepness.xxyy * amplitude.xxyy * dirAB.xyzw;
	vec4 CD = steepness.zzww * amplitude.zzww * dirCD.xyzw;

	vec4 dotABCD = frequency.xyzw * vec4(dot(dirAB.xy, xzPos), dot(dirAB.zw, xzPos), dot(dirCD.xy, xzPos), dot(dirCD.zw, xzPos));

	vec4 COS = cos(dotABCD + time * speed);
	vec4 SIN = sin(dotABCD + time * speed);

	offset.x = dot(vec4(AB.xz, CD.xz), COS);
	offset.z = dot(vec4(AB.yw, CD.yw), COS);
	offset.y = dot(amplitude, SIN);

	return offset;
}

vec3 GerstnerNormal(
	vec2 xzPos, vec4 amplitude, vec4 frequency,
	vec4 speed, vec4 dirAB, vec4 dirCD, vec4 time)
{
	vec3 normal = vec3(0.f, 2.f, 0.f);
	vec4 AB = frequency.xxyy * amplitude.xxyy * dirAB.xyzw;
	vec4 CD = frequency.zzww * amplitude.zzww * dirCD.xyzw;
	vec4 dotABCD = frequency.xyzw * vec4(dot(dirAB.xy, xzPos), dot(dirAB.zw, xzPos), dot(dirCD.xy, xzPos), dot(dirCD.zw, xzPos));
	vec4 C = cos(dotABCD + time * speed);
	normal.x -= dot(vec4(AB.xz, CD.xz), C);
	normal.z -= dot(vec4(AB.yw, CD.yw), C);
	normal = normalize(normal);
	return normal;
}

void main() 
{
	vec3 posWS = inPosition;
	vec3 viewWS = posWS - cameraPos.xyz;
	float distanceToCamera = length(viewWS);

	vec2 pos = posWS.xz;
	vec3 sum = vec3(pos.x, 0., pos.y);
	vec3 normal = vec3(0.0, 1.0, 0.0);

	vec4 amplitude1 = vec4(0.01, 0.14, 0.11, 0.13);
	vec4 frequency1 = vec4(1.15, 0.6, 1.0, 1.15);
	vec4 steepness1 = vec4(3.0, 1.7, 4.5, 1.4);
	vec4 speed1 = vec4(-1.0, 0.7, 0.3, 1.0);
	vec4 directionAB1 = vec4(0.27, -0.35, 0.2, 0.15);
	vec4 directionCD1 = vec4(0.47, 0.68, -0.71, -0.2);

	vec4 amplitude2 = vec4(0.05, 0.1, 0.05, 0.03);
	vec4 frequency2 = vec4(3.5, 2.2, 3.0, 4.0);
	vec4 steepness2 = vec4(2.5, 2.0, 4.0, 3.5);
	vec4 speed2 = vec4(0.45, 0.17, 0.35, 1.0);
	vec4 directionAB2 = vec4(-0.44, -0.15, 0.35, 0.15);
	vec4 directionCD2 = vec4(-0.12, -0.78, 0.11, 0.54);

	vec3 gerstnerOffset = GerstnerOffset(posWS.xz, steepness1, amplitude1, frequency1, speed1, directionAB1, directionCD1, vec4(absTime));
	vec3 gerstnerOffset2 = GerstnerOffset(posWS.xz, steepness2, amplitude2, frequency2,	speed2, directionAB2, directionCD2, vec4(absTime));
	
	posWS += gerstnerOffset + gerstnerOffset2;

	vec3 gerstnerNormal = GerstnerNormal(posWS.xz, amplitude1, frequency1, speed1, directionAB1, directionCD1, vec4(absTime));
	vec3 gerstnerNormal2 = GerstnerNormal(posWS.xz, amplitude2, frequency2,	speed2, directionAB2, directionCD2, vec4(absTime));

	normal = gerstnerNormal + gerstnerNormal2;
	normal = normalize(normal);

	vec4 posPS = projectionMatrix * viewMatrix * vec4(posWS, 1.f);
	gl_Position = posPS;
	outWorldPosition = posWS;
	outWorldNormal = normal;
	outTexCoord = inTexCoord;
	vec4 scrollingNormalSpeed = vec4(.5f, -.5f, .6f, .4f);
	outNormalUV1 = posWS.xz * 0.05f + scrollingNormalSpeed.xy * absTime * 0.01f;
	outNormalUV2 = posWS.xz * 0.05f + scrollingNormalSpeed.zw * absTime * 0.01f;
}
