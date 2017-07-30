#include "stdafx.h"
#include "Line.h"
#include "Util.h"

namespace OGL
{
	Line::Line() :
		m_points{ glm::vec3(0) },
		m_color{ glm::vec3(1) }
	{
		glGenBuffers(1, &m_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, m_points, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	Line::Line(glm::vec3 a, glm::vec3 b, glm::vec3 color) :
		m_points{ a,b },
		m_color{ color }
	{
		glGenBuffers(1, &m_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, m_points, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void Line::update(glm::vec3 a, glm::vec3 b)
	{
		m_points[0] = a;
		m_points[1] = b;
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, m_points, GL_DYNAMIC_DRAW);
	}

	void Line::render(ShaderManager& sm)
	{
		auto& shader = sm.getShader("Line");
		shader.bind();
		s32 program = shader.m_program;
		
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glUniform3fv(shader.getUniformLocation("color"), 1, glm::value_ptr(m_color));

		glDrawArrays(GL_LINES, 0, 2);
		
	}
}