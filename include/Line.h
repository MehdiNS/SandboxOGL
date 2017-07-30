#pragma once
#include "ShaderManager.h"

namespace OGL
{

	//Draw a line between two points a and b
	struct Line
	{
		Line();
		Line(glm::vec3 a, glm::vec3 b, glm::vec3 color = glm::vec3{ 1.f });
		void update(glm::vec3 a, glm::vec3 b);
		void render(ShaderManager& sm);

		glm::vec3 m_points[2];
		glm::vec3 m_color;
		u32 m_vbo;
	};
}