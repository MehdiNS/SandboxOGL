#include "stdafx.h"
#include "Quad.h"

namespace OGL
{
	Quad::Quad()
	{
		glGenVertexArrays(1, &m_vao);
		glBindVertexArray(m_vao);

		f32 points[] =
		{
			// first triangle
			-1.f,  1.f, 0.0f,  // top left 
			-1.f, -1.f, 0.0f,  // bottom left
			1.f, -1.f,  0.0f,  // bottom right
			// second triangle
			-1.f,  1.f, 0.0f,   // top left
			1.f, -1.f,  0.0f,  // bottom right
			1.f,  1.f,  0.0f  // top right
		};

		glGenBuffers(1, &m_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		f32 texcoords[] =
		{
			0.0f, 1.0f,
			0.0f, 0.0f,
			1.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 0.0f,
			1.0f, 1.0f
		};

		glGenBuffers(1, &m_vbot);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbot);
		glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	Quad::~Quad()
	{
		//glDeleteBuffers(1, &m_vbo);
		//glDeleteBuffers(1, &m_vbot);
		//if (m_vao != 0) 
		//{
		//	glDeleteVertexArrays(1, &m_vao);
		//	m_vao = 0;
		//}
	}

	void Quad::render()
	{
		//glDisable(GL_CULL_FACE);
		glBindVertexArray(m_vao);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		//glEnable(GL_CULL_FACE);
	}
}