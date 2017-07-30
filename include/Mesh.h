#pragma once
#include "Util.h"

namespace OGL
{
#define INVALID_MATERIAL 0xFFFFFFFF
#define POSITION_LOCATION    0
#define NORMAL_LOCATION      1
#define TEX_COORD_LOCATION   2
#define BONE_ID_LOCATION     3
#define BONE_WEIGHT_LOCATION 4

	//typedef  struct {
	//	u32  count;
	//	u32  instanceCount;
	//	u32  firstIndex;
	//	u32  baseVertex;
	//	u32  baseInstance;
	//} DrawElementsIndirectCommand;

	struct Vertex
	{
		glm::vec3 mpos;
		glm::vec2 mtex;
		glm::vec3 mnormal;

		Vertex() {}

		Vertex(const glm::vec3& pos, const glm::vec2& tex, const glm::vec3& normal)
		{
			mpos = pos;
			mtex = tex;
			mnormal = normal;
		}
	};

	class TextureManager;

	class Mesh : Uncopiable
	{
	public:
		Mesh(const std::string& name, TextureManager& tm);
		~Mesh();
		bool loadMesh(const std::string& Filename);

		void render(TextureManager& tm);
		void renderDepth();

		void initMesh(u32 meshIndex,
			const aiMesh* paiMesh,
			std::vector<glm::vec3>& positions,
			std::vector<glm::vec3>& normals,
			std::vector<glm::vec2>& texCoords,
			std::vector<u32>& indices);


		void initMaterials(const std::string& Filename, TextureManager& tm);
		void clear();
		void loadMesh(const std::string& filename, TextureManager& tM);
		void initFromScene(const std::string& Filename, TextureManager& tm);

		void createAABB(std::vector<glm::vec3>& position);
		void updateAABB(const glm::mat4& transform);
		void renderAABB();

		u32 m_vao;
		u32 m_buffers[NB_BUFFER];
		//std::vector<DrawElementsIndirectCommand> Commands;

		struct MeshEntry
		{
			MeshEntry()
			{
				m_numIndices = 0;
				m_baseVertex = 0;
				m_baseIndex = 0;
				m_materialIndex = INVALID_MATERIAL;
			}

			u32 m_numIndices;
			u32 m_baseVertex;
			u32 m_baseIndex;
			u32 m_materialIndex;
		};

		std::vector<MeshEntry> m_entries;
		std::vector<u32> m_albedoMapsId;
		std::vector<u32> m_metalnessMapsId;
		std::vector<u32> m_roughnessMapsId;
		std::vector<u32> m_normalMapsId;
		std::vector<u32> m_heightMapsId;

		// Bounding box stuff
		u32 m_vaoBBox;
		u32 m_vboBBox;
		u32 m_iboBBox;

		struct AABB
		{
			glm::vec3   m_radius;
			glm::vec3 	m_center;
			glm::mat4 	m_transform;
		} m_AABB;

		const aiScene* m_pScene;
	};
}