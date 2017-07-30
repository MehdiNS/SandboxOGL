#include "stdafx.h"
#include "Terrain.h"
#include "TextureManager.h"
#include "Util.h"

namespace OGL
{
#define NB_ELEMENTS_IN_ARRAY(a) (sizeof(a)/sizeof(a[0]))
#define INDEX_BUFFER 0    
#define POS_VB       1
#define NORMAL_VB    2
#define TEXCOORD_VB  3
#define TANGENT_VB   4
#define BITANGENT_VB 5

	Terrain::Terrain(TextureManager& tm)
	{
		m_normalTextureId = tm.createTexture(ASSET_TEXTURE2D_MIP_ON, "asset/rockNormal.png");
		m_albedoTextureId = tm.createTexture(ASSET_TEXTURE2D_MIP_ON, "asset/rockAlbedo.png");
		m_roughnessTextureId = tm.createTexture(ASSET_TEXTURE2D_MIP_ON, "asset/rockRoughness.png");
		m_metalnessTextureId = tm.createTexture(ASSET_TEXTURE2D_MIP_ON, "asset/rockMetalness.png");
		m_heightTextureId = tm.createTexture(ASSET_TEXTURE2D_MIP_ON, "asset/heightmap.png");

		m_position.resize(nbVertexH*nbVertexW);
		m_normal.resize(nbVertexH*nbVertexW);
		m_uvCoord.resize(nbVertexH*nbVertexW);
	
		std::fill(std::begin(m_position), std::end(m_position), glm::vec3(0));
		std::fill(std::begin(m_normal), std::end(m_normal), glm::vec3(0));
		std::fill(std::begin(m_uvCoord), std::end(m_uvCoord), glm::vec2(0));
		std::fill(std::begin(m_index), std::end(m_index), 0);

		buildTerrain();
		computeNormals();
		computeTexCoord();
		buildIndex();

		// Create the VAO
		GL(glGenVertexArrays(1, &m_vao));
		GL(glBindVertexArray(m_vao));

		// Create the buffers for the vertices attributes
		GL(glGenBuffers(NB_ELEMENTS_IN_ARRAY(m_buffers), m_buffers));
		GL(glBindVertexArray(m_vao));

		// Generate and populate the buffers with vertex attributes and the indices
		GL(glBindBuffer(GL_ARRAY_BUFFER, m_buffers[POS_VB]));
		GL(glBufferData(GL_ARRAY_BUFFER, sizeof(m_position[0]) * nbVertexH*nbVertexW, &m_position[0], GL_STATIC_DRAW));
		GL(glEnableVertexAttribArray(0));
		GL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0));
		GL(glBindBuffer(GL_ARRAY_BUFFER, 0));

		GL(glBindBuffer(GL_ARRAY_BUFFER, m_buffers[NORMAL_VB]));
		GL(glBufferData(GL_ARRAY_BUFFER, sizeof(m_normal[0]) * nbVertexH*nbVertexW, &m_normal[0], GL_STATIC_DRAW));
		GL(glEnableVertexAttribArray(1));
		GL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0));
		GL(glBindBuffer(GL_ARRAY_BUFFER, 0));

		GL(glBindBuffer(GL_ARRAY_BUFFER, m_buffers[TEXCOORD_VB]));
		GL(glBufferData(GL_ARRAY_BUFFER, sizeof(m_uvCoord[0]) * nbVertexH*nbVertexW, &m_uvCoord[0], GL_STATIC_DRAW));
		GL(glEnableVertexAttribArray(2));
		GL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0));
		GL(glBindBuffer(GL_ARRAY_BUFFER, 0));

		GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[INDEX_BUFFER]));
		GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_index[0]) * nbIdx, &m_index[0], GL_STATIC_DRAW));

		GL(glBindVertexArray(0));
	}

	void Terrain::render(TextureManager& tm)
	{
		GL(glBindVertexArray(m_vao));
		GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[INDEX_BUFFER]));

		//Bind terrain texture
		tm.bindTexture(m_albedoTextureId, GL_TEXTURE0);
		tm.bindTexture(m_metalnessTextureId, GL_TEXTURE1);
		tm.bindTexture(m_roughnessTextureId, GL_TEXTURE2);
		tm.bindTexture(m_normalTextureId, GL_TEXTURE3);
		tm.bindTexture(m_heightTextureId, GL_TEXTURE4);

		GL(glDrawElements(
			GL_TRIANGLES,      // mode
			nbIdx,			   // count
			GL_UNSIGNED_INT,   // type
			(void*)0           // element array buffer offset
			));

		GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));    
		GL(glBindVertexArray(0));
	}

	void Terrain::renderDepth()
	{
		GL(glBindVertexArray(m_vao));
		GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[INDEX_BUFFER]));

		GL(glDrawElements(
			GL_TRIANGLES,      // mode
			nbIdx,			   // count
			GL_UNSIGNED_INT,   // type
			(void*)0           // element array buffer offset
			));

		GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
		GL(glBindVertexArray(0));
	}
	void Terrain::computeTexCoord()
	{
		for (int row = 0; row < nbVertexH; row++)
		{
			for (int col = 0; col < nbVertexW; col++)
			{
				f32 x = (f32)row / (f32)nbVertexH;
				f32 z = (f32)col / (f32)nbVertexW;
				z = 1 - z;
				x *= 10;
				z *= 10;//repeat the texture 10 times
				m_uvCoord[nbVertexW * row + col] = glm::vec2(x, z);
			}
		}
	}

	glm::vec3 Terrain::computeNormal(int x, int z)
	{
		glm::vec3 n1;
		glm::vec3 n2;
		glm::vec3 n3;
		glm::vec3 n4;

		glm::vec3 v1;
		glm::vec3 v2;
		glm::vec3 v3;
		glm::vec3 v4;

		n1 = n2 = n3 = n4 = glm::vec3(0.f, 1.f, 0.f);

		auto& v = getPosition(x, z);
		if (z < nbVertexH - 1) 	v1 = getPosition(x, z + 1) - v;
		if (x < nbVertexW - 1)	v2 = getPosition(x + 1, z) - v;
		if (z > 0)		v3 = getPosition(x, z - 1) - v;
		if (x > 0)	v4 = getPosition(x - 1, z) - v;

		// les produits vectoriels
		if (z < nbVertexH - 1 && x < nbVertexW - 1) n1 = glm::cross(v1, v2);
		if (z > 0 && x < nbVertexW - 1) n2 = glm::cross(v2, v3);
		if (z > 0 && x > 0) n3 = glm::cross(v3, v4);
		if (z < nbVertexH - 1 && x > 0) n4 = glm::cross(v4, v1);

		n1 = n1 + n2 + n3 + n4;

		n1 = glm::normalize(n1);
		glm::vec3 resultat;

		resultat = n1;

		return resultat;
	}

	glm::vec3& Terrain::getPosition(int x, int z)
	{
		return m_position[z*nbVertexW + x];
	}

	void Terrain::buildTerrain()
	{
		const f32 nbVertHPerUnit = (f32)nbVertexH / (f32)hTerrain;	// nbVert H per world unit
		const f32 nbVertWPerUnit = (f32)nbVertexW / (f32)wTerrain;	// nbVert W per world unit
		const f32 halfTerrainH = 0.5 * hTerrain;
		const f32 halfTerrainW = 0.5 * wTerrain;
		for (int x = 0; x < nbVertexW; ++x)
		{
			for (int z = 0; z < nbVertexH; ++z)
			{
				//f32 t = static_cast<f32>(m_altitude[z*nbVertexW + x]) / 255.f;
				//f32 vMin = 0.f;
				//f32 vMax = 5.f;
				//f32 y =  (1.f - t)*vMin + t*vMax;
				glm::vec3 position = glm::vec3(f32(x) / nbVertHPerUnit - halfTerrainH, 0, f32(z) / nbVertWPerUnit - halfTerrainW);
				glm::vec3 normal = glm::vec3(0.f, 0.f, 0.f);
				m_position[z*nbVertexW + x] = position;
				m_normal[z*nbVertexW + x] = normal;
			}
		}
	}

	void Terrain::computeNormals()
	{
		for (int x = 0; x < nbVertexW; ++x)
		{
			for (int z = 0; z < nbVertexH; ++z)
			{
				m_normal[nbVertexW * z + x] = computeNormal(x, z);
			}
		}
	}

	void Terrain::buildIndex()
	{
		int k = 0;

		for (int z = 0; z < nbVertexH - 1; ++z)
		{
			for (int x = 0; x < nbVertexW - 1; ++x)
			{
				m_index[k++] = z*nbVertexW + x;
				m_index[k++] = (z + 1) * nbVertexW + (x + 1);
				m_index[k++] = z*nbVertexW + (x + 1);
				m_index[k++] = z*nbVertexW + x;
				m_index[k++] = (z + 1)* nbVertexW + x;
				m_index[k++] = (z + 1)* nbVertexW + (x + 1);
			}
		}
	}

	void Terrain::computeTangentBasis(
		// inputs
		std::vector<glm::vec3> & vertices,
		std::vector<glm::vec2> & uvs,
		std::vector<glm::vec3> & normals,
		// outputs
		std::vector<glm::vec3> & tangents,
		std::vector<glm::vec3> & bitangents
		) {

		for (u32 i = 0; i < nbIdx; i += 3) {

			auto i0 = m_index[i];
			auto i1 = m_index[i + 1];
			auto i2 = m_index[i + 2];

			// Shortcuts for vertices
			glm::vec3& v0 = vertices[i0];
			glm::vec3& v1 = vertices[i1];
			glm::vec3& v2 = vertices[i2];

			// Shortcuts for UVs
			glm::vec2& uv0 = uvs[i0];
			glm::vec2& uv1 = uvs[i1];
			glm::vec2& uv2 = uvs[i2];

			// Edges of the triangle : postion delta
			glm::vec3 deltaPos1 = v1 - v0;
			glm::vec3 deltaPos2 = v2 - v0;

			// UV delta
			glm::vec2 deltaUV1 = uv1 - uv0;
			glm::vec2 deltaUV2 = uv2 - uv0;

			f32 r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
			glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r;
			glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x)*r;

			// Set the same tangent for all three vertices of the triangle.
			// They will be merged later, in vboindexer.cpp
			tangents[i0] += tangent;
			tangents[i1] += tangent;
			tangents[i2] += tangent;

			// Same thing for binormals
			bitangents[i0] += bitangent;
			bitangents[i1] += bitangent;
			bitangents[i2] += bitangent;
		}

		// See "Going Further"
		for (u32 i = 0; i < vertices.size(); i += 1)
		{
			glm::vec3& n = normals[i];
			glm::vec3& t = tangents[i];
			glm::vec3& b = bitangents[i];

			// Gram-Schmidt orthogonalize
			t = glm::normalize(t - n * glm::dot(n, t));

			// Calculate handedness
			if (glm::dot(glm::cross(n, t), b) < 0.0f) {
				t = t * -1.0f;
			}
		}
	}
}