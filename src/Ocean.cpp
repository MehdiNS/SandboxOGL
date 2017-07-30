#include "stdafx.h"
#include "Ocean.h"
#include "TextureManager.h"
#include "Util.h"

namespace OGL
{
	Ocean::Ocean(TextureManager& tm)
	{
		buildOcean(500, 100, 500);
		m_oceanNormalMap = tm.createTexture(ASSET_TEXTURE2D_MIP_ON, "asset/water_normal.png");
	}

	void Ocean::buildOcean(u32 rads, u32 angs, f32 radius)
	{
		std::vector<glm::vec3>		vertices;
		std::vector<glm::vec3>		normals;
		std::vector<glm::vec2>		texCoords;
		std::vector<u32>			indices;

		f32 epsilon = 0.001f;
		const f32 PI = glm::pi<f32>();

		// Compute vertices position in a polar grid
		for (f32 rad = 0.0f; rad < radius + epsilon; rad += radius / (f32)rads)
		{
			for (f32 angle = 0.0f; angle < 2.0f * PI + epsilon; angle += (2.0f * PI) / angs)
			{
				glm::vec3 vertex = glm::vec3(rad * std::cos(angle), 0.f, rad * std::sin(angle));
				glm::vec3 normal = glm::vec3(0.f, 1.f, 0.f);
				glm::vec2 texCoord = glm::vec2(vertex.x, vertex.z);

				vertices.emplace_back(vertex);
				normals.emplace_back(normal);
				texCoords.emplace_back(texCoord);
			}
		}

		u32 pos = std::find(std::begin(vertices), std::end(vertices), glm::vec3(0.f)) - std::begin(vertices);
		f32 maxVal = 0.f;

		// Compute indices
		for (u32 radNumber = 0; radNumber < rads; radNumber++)
		{
			for (u32 angNumber = 0; angNumber < angs; angNumber++)
			{
				u32 first = (radNumber * (angs + 1)) + angNumber;
				u32 second = first + angs + 1;

				indices.emplace_back(first);
				indices.emplace_back(first + 1);
				indices.emplace_back(second);

				indices.emplace_back(second);
				indices.emplace_back(first + 1);
				indices.emplace_back(second + 1);
			}
		}

		nbIndices = indices.size();

		glGenVertexArrays(1, &m_vao);
		glBindVertexArray(m_vao);

		glGenBuffers(NB_ELEMENTS_IN_ARRAY(m_buffers), m_buffers);
	
		glBindBuffer(GL_ARRAY_BUFFER, m_buffers[POS_VB]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, m_buffers[NORMAL_VB]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(normals[0]) * normals.size(), &normals[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, m_buffers[TEXCOORD_VB]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords[0]) * texCoords.size(), &texCoords[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[INDEX_BUFFER]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) *  nbIndices, &indices[0], GL_STATIC_DRAW);

		glBindVertexArray(0);
	}

	void Ocean::render(TextureManager& tm)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		tm.bindTexture(m_oceanNormalMap, GL_TEXTURE0);
		glBindVertexArray(m_vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[INDEX_BUFFER]);

		glDrawElements(
			GL_TRIANGLES,
			nbIndices,
			GL_UNSIGNED_INT,
			(void*)0
			);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void Ocean::renderDepth()
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(m_vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[INDEX_BUFFER]);

		glDrawElements(
			GL_TRIANGLES,
			nbIndices,
			GL_UNSIGNED_INT,
			(void*)0
			);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
}