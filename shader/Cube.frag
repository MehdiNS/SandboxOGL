#version 420

layout (location = 0) out vec4 DiffuseRT;
layout (location = 1) out vec4 NormalRT;
//layout (location = 2) out vec4 PositionRT;

uniform layout(binding = 0) sampler2D gColorMap;
uniform layout(binding = 1) sampler2D gMetalnessMap;
uniform layout(binding = 2) sampler2D gRoughnessMap;
uniform layout(binding = 3) sampler2D gNormalMap;
uniform layout(binding = 4) sampler2D gHeightMap;
uniform layout(binding = 5) sampler2D gDepthMap;

layout (binding = 2, std140) uniform matrix_data {
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 viewProjInvMatrix;
	mat4 normalMatrix;
	vec4 cameraPos;
	vec4 sunDir;
	vec4 skyData;
};

uniform mat4 modelMatrix;

#define RED			vec4(1.0f, 0.f, 0.f, 1.f)
#define BLUE		vec4(0.0f, 0.f, 1.f, 1.f)
#define GREEN		vec4(0.0f, 1.f, 0.f, 1.f)
#define BLACK		vec4(0.0f, 0.f, 0.f, 1.f)
#define WHITE		vec4(1.0f, 1.f, 1.f, 1.f)

/////////////////////////////////////////////////////////

// https://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl/17479300#17479300
// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

// Compound versions of the hashing algorithm I whipped together.
uint hash( uvec2 v ) { return hash( v.x ^ hash(v.y)                         ); }
uint hash( uvec3 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z)             ); }
uint hash( uvec4 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w) ); }

// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = uintBitsToFloat( m );       // Range [1:2]
    return f - 1.0;                        // Range [0:1]
}

// Pseudo-random value in half-open range [0:1].
float random( float x ) { return floatConstruct(hash(floatBitsToUint(x))); }
float random( vec2  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec3  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec4  v ) { return floatConstruct(hash(floatBitsToUint(v))); }

void main()
{	
	vec2 uv = gl_FragCoord.xy / vec2(640.f, 480.f);

	// reconstruct world-space position from depth
	float depth = texture(gDepthMap, uv).x;
	vec4 tmp = inverse(projectionMatrix) * vec4((uv.x * 2 - 1.0), (uv.y * 2 - 1.0), (depth * 2 - 1.0), 1.0);
	vec3 positionVS = tmp.xyz / tmp.w;
	vec4 positionWS = inverse(viewMatrix)* vec4(positionVS, 1.0);
	vec4 positionOS = inverse(modelMatrix) * positionWS;
	
	//Perform bounds check
	if ( (0.5 - abs(positionOS.x) < 0.) || (0.5 - abs(positionOS.y) < 0.) || (0.5 - abs(positionOS.z) < 0.) )
	    discard;
	
	//vec2 uvDecal = positionOS.xy + vec2(0.5);
	//uvDecal = 2.0 * uvDecal - 1.0;

    vec3 rgb = vec3(random(modelMatrix[3][0]), random(modelMatrix[3][1]), random(modelMatrix[3][2]));                               
	DiffuseRT.xyz = rgb;

} 



