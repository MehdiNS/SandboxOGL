#version 420

layout (location = 0) in vec2 inTexCoord;
layout (location = 0) out vec4 outFragColor;

uniform layout(binding = 0) sampler2D normalBuffer;
uniform layout(binding = 1) sampler2D depthBuffer;
uniform layout(binding = 2) sampler2D finalBuffer;
uniform layout(binding = 3) sampler2D positionBuffer;

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


vec3 GetWorldPosFromDepth_v0(vec2 texCoord)
{
	float depth = texture(depthBuffer, texCoord).x;
	vec4 tmp = inverse(projectionMatrix) * vec4((texCoord.xy * 2.f - 1.f), (depth * 2.f - 1.f), 1.f);
	vec3 posVS = tmp.xyz / tmp.w;
	vec4 posWS = inverse(viewMatrix)* vec4(posVS, 1.f);
	return posWS;
}

vec3 GetWorldPosFromDepth_v1(vec2 texCoord)
{
    float depth = texture(depthBuffer, texCoord).x;
    vec4 spos = vec4(texCoord * 2.0f - 1.0f, depth * 2.0f - 1.0f, 1.0f);
    spos = viewProjInvMatrix * spos;
    return spos.xyz / spos.w;
}
 
vec3 GetViewPosFromDepth_v0(vec2 texCoord)
{
	float depth = ( texture(depthBuffer, texCoord).x );
	float x = texCoord.x * 2.f - 1.f;
	float y = texCoord.y * 2.f - 1.f;
	vec4 projectedPos = vec4(x, y, depth, 1.f);
	vec4 posVS = inverse(projectionMatrix) * projectedPos;
	return posVS.xyz / posVS.w;
}

float linearizeDepth(float z)
{
	float n = cameraPos.w; // camera z near
	float f = sunDir.w; // camera z far
	return (2.0 * n) / (f + n - z * (f - n));	
}

bool SSR(in vec3 wsPosition, in vec3 wsNormal, in vec3 wsReflectionVector, out vec4 reflectedColor)
{
    float factor = dot(wsReflectionVector, wsNormal);

    // Variables
    reflectedColor = vec4(0);
    vec2 pixelsize = 1.0/vec2(640.f, 480.f);

    // Get texture informations
    vec4 csPosition = projectionMatrix * viewMatrix * vec4(wsPosition, 1.0);
    vec3 ndcsPosition = csPosition.xyz / csPosition.w;
    vec3 ssPosition = 0.5 * ndcsPosition + 0.5;

    // Project reflected vector into screen space
    wsReflectionVector += wsPosition;
    vec4 csReflectionVector = projectionMatrix * viewMatrix * vec4(wsReflectionVector, 1.0);
    vec3 ndcsReflectionVector = csReflectionVector.xyz / csReflectionVector.w;
    vec3 ssReflectionVector = 0.5 * ndcsReflectionVector + 0.5;
    ssReflectionVector = normalize(ssReflectionVector - ssPosition);

    vec3 lastSamplePosition;
    vec3 currentSamplePosition;

    float initalStep;
    float pixelStepSize;

    int sampleCount = max(int(640.f), int(480.f));
    int count = 0;
    // Ray trace
    initalStep = max(pixelsize.x, pixelsize.y);
    pixelStepSize = 1.0f;
    ssReflectionVector *= initalStep;

    lastSamplePosition = ssPosition + ssReflectionVector;
    currentSamplePosition = lastSamplePosition + ssReflectionVector;

    while(count < sampleCount)
    {
        // Out of screen space
        if(currentSamplePosition.x < 0.0 || currentSamplePosition.x > 1.0 ||
           currentSamplePosition.y < 0.0 || currentSamplePosition.y > 1.0 ||
           currentSamplePosition.z < 0.0 || currentSamplePosition.z > 1.0)
        {
            return false;
        }
        
        vec2 samplingPosition = currentSamplePosition.xy;
        float sampledDepth = linearizeDepth( texture(depthBuffer, samplingPosition).x );
        float currentDepth = linearizeDepth(currentSamplePosition.z);

        if(currentDepth > sampledDepth)
        {   
            float delta = abs(currentDepth - sampledDepth);
            if(delta <= 0.001f)
            {
                float f = currentDepth;
                reflectedColor = texture2D(finalBuffer, vec2(samplingPosition.x, samplingPosition.y));
                // Fading to screen edges
				vec2 fadeToScreenEdge = vec2(1.0);
				fadeToScreenEdge.x = distance(lastSamplePosition.x , 1.0);
				fadeToScreenEdge.x *= distance(lastSamplePosition.x, 0.0) * 4.0;
				fadeToScreenEdge.y = distance(lastSamplePosition.y, 1.0);
				fadeToScreenEdge.y *= distance(lastSamplePosition.y, 0.0) * 4.0;
				reflectedColor *= fadeToScreenEdge.x * fadeToScreenEdge.y;
				return true;  
            }
        }
        else
        {
            // Step ray
            lastSamplePosition = currentSamplePosition;
            currentSamplePosition = lastSamplePosition + ssReflectionVector * pixelStepSize;    
        } 
        count++;
    }
    return false;
}

void main()
{
	vec3 posWS = texture(positionBuffer, inTexCoord).xyz;
	vec3 N = normalize(texture(normalBuffer, inTexCoord).xyz);
	vec3 V = normalize(cameraPos.xyz - posWS);
	vec3 R = normalize(reflect(-V,N));
	vec4 res;
	bool b = SSR(posWS, N, R, res);
	outFragColor = b ? res : vec4(0,0,0,1);
}