#pragma once

namespace OGL
{
	const u32 nbVertexH = 128;	// nbVertex h
	const u32 nbVertexW = 128;	// nbVertex w
	const u32 hTerrain = 32;	// h in world unit
	const u32 wTerrain = 32;	// w in world unit
	const u32 nbIdx = nbVertexH * nbVertexW * 6;

#define COUNT_VB	 6

	class TextureManager;

	class Terrain
	{
	protected:
		std::vector<glm::vec3> m_position;
		std::vector<glm::vec3> m_normal;
		std::vector<glm::vec2> m_uvCoord;
		std::vector<glm::vec3> m_tangent;
		std::vector<glm::vec3> m_bitangent;
		u32 m_index[nbIdx];
		u8* m_altitude;

	public:

		Terrain(TextureManager& tm);
		~Terrain() {};

		glm::vec3& getPosition(s32 x, s32 y);
		glm::vec3 computeNormal(s32 x, s32 y);
		void buildTerrain();
		void computeNormals();
		void computeTangentBasis(
			std::vector<glm::vec3> & vertices,
			std::vector<glm::vec2> & uvs,
			std::vector<glm::vec3> & normals,
			// outputs
			std::vector<glm::vec3> & tangents,
			std::vector<glm::vec3> & bitangents
			);
		void buildIndex();
		void computeTexCoord();
		void render(TextureManager& tm);
		void renderDepth();

		u32 m_vao;
		u32 m_buffers[COUNT_VB];
		u32 m_albedoTextureId;
		u32 m_roughnessTextureId;
		u32 m_metalnessTextureId;
		u32 m_heightTextureId;
		u32 m_normalTextureId;
	};
}