#pragma once

class PreethamSky
{
public:
	glm::vec4 A, B, C, D, E;
	glm::vec4 Z;

	static PreethamSky compute(float sunTheta, float turbidity, float normalizedSunY);
};

