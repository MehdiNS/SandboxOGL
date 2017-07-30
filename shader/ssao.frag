#version 420

uniform layout(binding = 0) sampler2D gNormal;
uniform layout(binding = 1) sampler2D gDepth;

layout(location = 0) in vec2 inTexCoord;
layout(location = 0) out vec4 outFragColor;

layout (binding = 2, std140) uniform matrix_data {
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 viewProjInvMatrix;
	mat4 normalMatrix;
	vec4 cameraPos;
	vec4 sunDir;
	vec4 skyData;
};

float near = cameraPos.w;
float far = sunDir.w;
ivec2 resolution = ivec2(640,480);

/**
 \file SAO_AO.pix
 \author Morgan McGuire and Michael Mara, NVIDIA Research

 Reference implementation of the Scalable Ambient Obscurance (SAO) screen-space ambient obscurance algorithm. 
 
 The optimized algorithmic structure of SAO was published in McGuire, Mara, and Luebke, Scalable Ambient Obscurance,
 <i>HPG</i> 2012, and was developed at NVIDIA with support from Louis Bavoil.

 The mathematical ideas of AlchemyAO were first described in McGuire, Osman, Bukowski, and Hennessy, The 
 Alchemy Screen-Space Ambient Obscurance Algorithm, <i>HPG</i> 2011 and were developed at 
 Vicarious Visions.  
 
 DX11 HLSL port by Leonardo Zide of Treyarch

 <hr>

  Open Source under the "BSD" license: http://www.opensource.org/licenses/bsd-license.php

  Copyright (c) 2011-2012, NVIDIA
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  */

// Total number of direct samples to take at each pixel
#define NUM_SAMPLES (21)

// If using depth mip levels, the log of the maximum pixel offset before we need to switch to a lower 
// miplevel to maintain reasonable spatial locality in the cache
// If this number is too small (< 3), too many taps will land in the same pixel, and we'll get bad variance that manifests as flashing.
// If it is too high (> 5), we'll get bad performance because we're not using the MIP levels effectively
#define LOG_MAX_OFFSET 3

// This must be less than or equal to the MAX_MIP_LEVEL defined in SSAO.cpp
#define MAX_MIP_LEVEL 5

/** Used for preventing AO computation on the sky (at infinite depth) and defining the CS Z to bilateral depth key scaling. 
    This need not match the real far plane*/
#define FAR_PLANE_Z (300.0)

// This is the number of turns around the circle that the spiral pattern makes.  This should be prime to prevent
// taps from lining up.  This particular choice was tuned for NUM_SAMPLES == 9
#define NUM_SPIRAL_TURNS (7)

//////////////////////////////////////////////////

/** The height in pixels of a 1m object if viewed from 1m away.  
    You can compute it from your projection matrix.  The actual value is just
    a scale factor on radius; you can simply hardcode this to a constant (~500)
    and make your radius value unitless (...but resolution dependent.)  */
float projScale = 579.41125497f;

uniform vec3 saoData;
/** World-space AO radius in scene units (r).  e.g., 1.0m */
float radius = saoData.x;

/** Bias to avoid AO in smooth corners, e.g., 0.01m */
float bias = saoData.y;

/** Darkending factor, e.g., 1.0 */
float intensity = saoData.z;

/**  vec4(-2.0f / (width*P[0][0]), 
          -2.0f / (height*P[1][1]),
          ( 1.0f - P[0][2]) / P[0][0], 
          ( 1.0f + P[1][2]) / P[1][1])
    
    where P is the projection matrix that maps camera space points 
    to [-1, 1] x [-1, 1].  That is, GCamera::getProjectUnit(). */
vec4 projInfo = vec4(
		-2.0f / (resolution.x * projectionMatrix[0][0]),
		-2.0f / (resolution.y * projectionMatrix[1][1]),
		( 1.0f - projectionMatrix[0][2]) / projectionMatrix[0][0],
		( 1.0f - projectionMatrix[1][2]) / projectionMatrix[1][1]
	);

float radius2 = radius * radius;

/////////////////////////////////////////////////////////

vec3 reconstructCSPosition(vec2 S, float z) {
    return vec3((S.xy * projInfo.xy + projInfo.zw) * z, z);
}

vec3 getPosition(ivec2 ssP) {
    vec3 P;
    P.z = texelFetch(gDepth, ssP, 0).r;

    P.z = P.z * 2.0 - 1.0;
    P.z = 2.0 * near * far / (far + near - P.z * (far - near));
    P.z = -P.z;

    // Offset to pixel center
    P = reconstructCSPosition(vec2(ssP) + vec2(0.5), P.z);
    return P;
}

/** Read the camera-space position of the point at screen-space pixel ssP + unitOffset * ssR.  Assumes length(unitOffset) == 1 */
vec3 getOffsetPosition(ivec2 ssC, vec2 unitOffset, float ssR) {
    // Derivation:
    //  mipLevel = floor(log(ssR / MAX_OFFSET));
	int mipLevel = clamp(int(floor(log2(ssR))) - LOG_MAX_OFFSET, 0, MAX_MIP_LEVEL);

	ivec2 ssP = ivec2(ssR * unitOffset) + ssC;

	vec3 P;

	// We need to divide by 2^mipLevel to read the appropriately scaled coordinate from a MIP-map.
	// Manually clamp to the texture size because texelFetch bypasses the texture unit
	ivec2 mipP = clamp(ssP >> mipLevel, ivec2(0), (resolution >> mipLevel) - ivec2(1));

	//read from depth buffer
	P.z = texelFetch(gDepth, mipP, 0).r;
	P.z = P.z * 2.0 - 1.0;
	P.z = 2.0 * near * far / (far + near - P.z * (far - near));
	P.z = -P.z;

	// Offset to pixel center
	P = reconstructCSPosition(vec2(ssP) + vec2(0.5), P.z);

	return P;
}

/** Reconstructs screen-space unit normal from screen-space position */
vec3 reconstructCSFaceNormal(vec3 C) {
	return normalize(cross(dFdy(C), dFdx(C)));
}

/** Returns a unit vector and a screen-space radius for the tap on a unit disk (the caller should scale by the actual disk radius) */
vec2 tapLocation(int sampleNumber, float spinAngle, out float ssR){
	// Radius relative to ssR
	float alpha = float(sampleNumber + 0.5) * (1.0 / NUM_SAMPLES);
	float angle = alpha * (NUM_SPIRAL_TURNS * 6.28) + spinAngle;

	ssR = alpha;
	return vec2(cos(angle), sin(angle));
}
 
/** Compute the occlusion due to sample with index \a i about the pixel at \a uv that corresponds
    to camera-space point \a C with unit normal \a n_C, using maximum screen-space sampling radius \a ssDiskRadius */
float sampleAO(ivec2 ssC, vec3 C, vec3 n_C, float ssDiskRadius, int tapIndex, float randomPatternRotationAngle) {
	// Offset on the unit disk, spun for this pixel
	float ssR;
	vec2 unitOffset = tapLocation(tapIndex, randomPatternRotationAngle, ssR);
	ssR *= ssDiskRadius;

	// The occluding point in camera space
	vec3 Q = getOffsetPosition(ssC, unitOffset, ssR);

	vec3 v = Q - C;

	float vv = dot(v, v);
	float vn = dot(v, n_C);

    const float epsilon = 0.01;
    float f = max(radius2 - vv, 0.0); 
	return f * f * f * max((vn - bias) / (epsilon + vv), 0.0);
}

void main()
{
	// Pixel being shaded
	ivec2 ssC = ivec2(gl_FragCoord.xy);

	vec3 C = getPosition(ssC);
	//vec3 n_C = reconstructCSFaceNormal(C);
	//n_C = -n_C;
	vec3 normalWS = normalize(texelFetch(gNormal,ssC,0).xyz);
	vec3 n_C =  normalize((viewMatrix * vec4(normalWS, 0)).xyz);

	// Hash function used in the HPG12 AlchemyAO paper
	float randomPatternRotationAngle = (3 * ssC.x ^ ssC.y + ssC.x * ssC.y) * 10;

	// Choose the screen-space sample radius
	// proportional to the projected area of the sphere
	float ssDiskRadius = projScale * radius / C.z;

	float sum = 0.0;
	for (int i = 0; i < NUM_SAMPLES; ++i)
	     sum += sampleAO(ssC, C, n_C, ssDiskRadius, i, randomPatternRotationAngle);

    float temp = radius2 * radius;
    sum /= temp * temp;
	float A  = max(0.0, 1.0 - sum * intensity * (5.0 / NUM_SAMPLES));
	
	// Bilateral box-filter over a quad for free, respecting depth edges
	// (the difference that this makes is subtle)
	if (abs(dFdx(C.z)) < 0.02) {
		A -= dFdx(A) * (float(ssC.x & 1) - 0.5);
	}
	if (abs(dFdy(C.z)) < 0.02) {
		A -= dFdy(A) * (float(ssC.y & 1) - 0.5);
	}

	float visibility = A;

	outFragColor = vec4(vec3(visibility), 1);
}