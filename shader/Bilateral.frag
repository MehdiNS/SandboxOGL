#version 420

layout (location = 0) vec2 inTexCoord;

uniform layout(binding = 0) sampler2D gSSAO;
uniform layout(binding = 0) sampler2D gDepth;

layout(location = 0, index = 0) out vec4 outFragColor;

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

/** 
  \file SAO_blur.pix
  \author Morgan McGuire and Michael Mara, NVIDIA Research

  \brief 7-tap 1D cross-bilateral blur using a packed depth key

  DX11 HLSL port by Leonardo Zide, Treyarch
  
  Open Source under the "BSD" license: http://www.opensource.org/licenses/bsd-license.php

  Copyright (c) 2011-2012, NVIDIA
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//////////////////////////////////////////////////////////////////////////////////////////////
// Tunable Parameters:

/** Increase to make edges crisper. Decrease to reduce temporal flicker. */
#define EDGE_SHARPNESS     (1.0)

/** Step in 2-pixel intervals since we already blurred against neighbors in the
    first AO pass.  This constant can be increased while R decreases to improve
    performance at the expense of some dithering artifacts. 
    
    Morgan found that a scale of 3 left a 1-pixel checkerboard grid that was
    unobjectionable after shading was applied but eliminated most temporal incoherence
    from using small numbers of sample taps.
    */
#define SCALE               (2)

/** Filter radius in pixels. This will be multiplied by SCALE. */
#define R                   (4)

//////////////////////////////////////////////////////////////////////////////////////////////

// Gaussian coefficients
const float gaussian[] = 
//	{ 0.356642, 0.239400, 0.072410, 0.009869 };
//	{ 0.398943, 0.241971, 0.053991, 0.004432, 0.000134 };  // stddev = 1.0
	{ 0.153170, 0.144893, 0.122649, 0.092902, 0.062970 };  // stddev = 2.0
//	{ 0.111220, 0.107798, 0.098151, 0.083953, 0.067458, 0.050920, 0.036108 }; // stddev = 3.0


/** (1, 0) or (0, 1)*/
uniform ivec2 axis;


void main() 
{
	ivec2 ssC = ivec2(gl_FragCoord.xy);

	float depth = texelFetch(gDepth, ssC, 0).r;

	depth = depth * 2.0 - 1.0;
	depth = 2.0 * near * far / (far + near - depth * (far - near));

	float depth_divide = 1.0 / far;

	depth*=depth_divide;

	float sum = texelFetch(gSSAO, ssC, 0).r;

	// Base weight for depth falloff.  Increase this for more blurriness,
	// decrease it for better edge discrimination
	float BASE = gaussian[0];
	float totalWeight = BASE;
	sum *= totalWeight;

	for (int r = -R; r <= R; ++r) {
		// We already handled the zero case above.  This loop should be unrolled and the branch discarded
		if (r != 0) {
			ivec2 ppos = ssC + axis * (r * SCALE);
			float value = texelFetch(gSSAO, ppos, 0).r;
			float temp_depth = texelFetch(gDepth, ssC, 0).r;

			temp_depth = temp_depth * 2.0 - 1.0;
			temp_depth = 2.0 * near * far / (far + near - temp_depth * (far - near));
			temp_depth *= depth_divide;

			// spatial domain: offset gaussian tap
			float weight = 0.3 + gaussian[abs(r)];

			// range domain (the "bilateral" weight). As depth difference increases, decrease weight.
			weight *= max(0.0, 1.0 - (2000.0 * EDGE_SHARPNESS) * abs(temp_depth - depth));

			sum += value * weight;
			totalWeight += weight;
		}
	}

	const float epsilon = 0.0001;
	outFragColor = vec4(vec3(sum / (totalWeight + epsilon)),1);	
}
