#include "stdafx.h"
#include "Mesh.h"
#include "TextureManager.h"

namespace OGL
{
	Mesh::Mesh(const std::string& name, TextureManager& tM)
	{
		m_vao = 0;
		m_vaoBBox = 0;
		for (auto& b : m_buffers) b = 0;
		m_vboBBox = m_iboBBox = 0;
		loadMesh(name, tM);
	}

	Mesh::~Mesh()
	{
		clear();
	}

	void Mesh::clear()
	{
		if (m_buffers[0] != 0)
		{
			glDeleteBuffers(NB_ELEMENTS_IN_ARRAY(m_buffers), m_buffers);
		}

		glDeleteBuffers(1, &m_vboBBox);
		glDeleteBuffers(1, &m_iboBBox);

		if (m_vao != 0)
		{
			glDeleteVertexArrays(1, &m_vao);
			m_vao = 0;
		}
		if (m_vaoBBox != 0)
		{
			glDeleteVertexArrays(1, &m_vaoBBox);
			m_vaoBBox = 0;
		}
	}

	void Mesh::loadMesh(const std::string& filename, TextureManager& tM)
	{
		// Create the bunding box VAO
		glGenVertexArrays(1, &m_vaoBBox);
		glBindVertexArray(m_vaoBBox);
		glGenBuffers(1, &m_vboBBox);
		glGenBuffers(1, &m_iboBBox);

		// Create the mesh VAO
		glGenVertexArrays(1, &m_vao);
		glBindVertexArray(m_vao);

		// Create the buffers for the vertices attributes
		glGenBuffers(NB_ELEMENTS_IN_ARRAY(m_buffers), m_buffers);

		Assimp::Importer Importer;
		m_pScene = Importer.ReadFile(filename.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);

		if (!m_pScene || m_pScene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !m_pScene->mRootNode)
		{
			DEBUG_ONLY(std::cout << "ERROR (assimp) " << Importer.GetErrorString() << std::endl);
			return;
		}
		else
		{
			initFromScene(filename, tM);
		}

		glBindVertexArray(0);

		return;
	}

	void Mesh::initFromScene(const std::string& filename, TextureManager& tM)
	{
		m_entries.resize(m_pScene->mNumMeshes);
		m_albedoMapsId.resize(m_pScene->mNumMaterials);
		m_metalnessMapsId.resize(m_pScene->mNumMaterials);
		m_roughnessMapsId.resize(m_pScene->mNumMaterials);
		m_normalMapsId.resize(m_pScene->mNumMaterials);
		m_heightMapsId.resize(m_pScene->mNumMaterials);

		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> texCoords;
		std::vector<u32> indices;

		u32 nbVertices = 0;
		u32 numIndices = 0;

		// Count the number of vertices and indices
		for (u32 i = 0; i < m_entries.size(); i++) {
			m_entries[i].m_materialIndex = m_pScene->mMeshes[i]->mMaterialIndex;
			m_entries[i].m_numIndices = m_pScene->mMeshes[i]->mNumFaces * 3;
			m_entries[i].m_baseVertex = nbVertices;
			m_entries[i].m_baseIndex = numIndices;

			nbVertices += m_pScene->mMeshes[i]->mNumVertices;
			numIndices += m_entries[i].m_numIndices;
		}

		// Reserve space in the vectors for the vertex attributes and indices
		positions.reserve(nbVertices);
		normals.reserve(nbVertices);
		texCoords.reserve(nbVertices);
		indices.reserve(nbVertices);
		//Commands.resize(m_entries.size());

		// Initialize the meshes in the scene one by one
		for (u32 i = 0; i < m_entries.size(); i++) {
			const aiMesh* paiMesh = m_pScene->mMeshes[i];
			initMesh(i, paiMesh, positions, normals, texCoords, indices);
			//Commands[i].count = m_entries[i].m_numIndices;
			//Commands[i].instanceCount = 1;
			//Commands[i].firstIndex = m_entries[i].m_baseIndex;
			//Commands[i].baseVertex = m_entries[i].m_baseVertex;
			//Commands[i].baseInstance = 0;
		}

		createAABB(positions);

		// Load textures
		initMaterials(filename, tM);

		glBindVertexArray(m_vao);

		glEnableVertexAttribArray(POSITION_LOCATION);
		glEnableVertexAttribArray(NORMAL_LOCATION);
		glEnableVertexAttribArray(TEX_COORD_LOCATION);

		// Generate and populate the buffers with vertex attributes and the indices
		glBindBuffer(GL_ARRAY_BUFFER, m_buffers[POS_VB]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(positions[0]) * positions.size(), &positions[0], GL_STATIC_DRAW);
		glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, m_buffers[NORMAL_VB]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(normals[0]) * normals.size(), &normals[0], GL_STATIC_DRAW);
		glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, m_buffers[TEXCOORD_VB]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords[0]) * texCoords.size(), &texCoords[0], GL_STATIC_DRAW);
		glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[INDEX_BUFFER]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		//glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_buffers[INDIRECT_BUFFER]);
		//glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(Commands[0]) * Commands.size(), &Commands[0], GL_STATIC_DRAW);
		//glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

		glBindVertexArray(0);

		// Bounding box stuff
		glBindVertexArray(m_vaoBBox);
		glEnableVertexAttribArray(0);

		// Cube 1x1x1, centered on origin
		f32 vertices[] =
		{
			-0.5, -0.5, -0.5, 1.0,
			0.5, -0.5, -0.5, 1.0,
			0.5,  0.5, -0.5, 1.0,
			-0.5,  0.5, -0.5, 1.0,
			-0.5, -0.5,  0.5, 1.0,
			0.5, -0.5,  0.5, 1.0,
			0.5,  0.5,  0.5, 1.0,
			-0.5,  0.5,  0.5, 1.0,
		};

		glBindBuffer(GL_ARRAY_BUFFER, m_vboBBox);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		u16 elements[] = {
			0, 1, 2, 3,
			4, 5, 6, 7,
			0, 4, 1, 5, 2, 6, 3, 7
		};

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_iboBBox);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
		glBindVertexArray(0);

		return;
	}

	void Mesh::initMesh(u32 meshIndex,
		const aiMesh* paiMesh,
		std::vector<glm::vec3>& positions,
		std::vector<glm::vec3>& normals,
		std::vector<glm::vec2>& texCoords,
		std::vector<u32>& indices)
	{
		const aiVector3D zero(0.0f, 0.0f, 0.0f);

		// Populate the vertex attribute vectors
		for (u32 i = 0; i < paiMesh->mNumVertices; i++) {
			const aiVector3D* pPos = &(paiMesh->mVertices[i]);
			const aiVector3D* pNormal = &(paiMesh->mNormals[i]);
			const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &zero;

			positions.push_back(glm::vec3(pPos->x, pPos->y, pPos->z));
			normals.push_back(glm::vec3(pNormal->x, pNormal->y, pNormal->z));
			texCoords.push_back(glm::vec2(pTexCoord->x, pTexCoord->y));
		}

		// Populate the index buffer
		for (u32 i = 0; i < paiMesh->mNumFaces; i++) {
			const aiFace& face = paiMesh->mFaces[i];
			assert(face.mNumIndices == 3);
			indices.push_back(face.mIndices[0]);
			indices.push_back(face.mIndices[1]);
			indices.push_back(face.mIndices[2]);
		}
	}

	void Mesh::initMaterials(const std::string& filename, TextureManager& tM)
	{
		// Extract the directory part from the file name
		std::string::size_type slashIndex = filename.find_last_of("/");
		std::string dir;

		if (slashIndex == std::string::npos)
			dir = ".";
		else if (slashIndex == 0)
			dir = "/";
		else
			dir = filename.substr(0, slashIndex);

		bool ret = true;

		// Initialize the materials
		for (u32 i = 0; i < m_pScene->mNumMaterials; i++) {
			const aiMaterial* pMaterial = m_pScene->mMaterials[i];
			aiString Path;

			if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0
				&& (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS))
			{
				std::string fullPath = dir + "/" + Path.data;
				//m_albedoMapsId[i] = tM.createTexture(fullPath.c_str(), GL_SRGB_ALPHA, GL_RGBA, GL_UNSIGNED_BYTE);
				m_albedoMapsId[i] = tM.createTexture(ASSET_TEXTURE2D_MIP_ON, fullPath.c_str());
				DEBUG_ONLY(printf("Loaded base color '%s'\n", fullPath.c_str()));
			}
			else
			{
				std::string fullPath = dir + "/../error.png";
				//m_albedoMapsId[i] = tM.createTexture(fullPath.c_str(), GL_SRGB_ALPHA, GL_RGBA, GL_UNSIGNED_BYTE);
				m_albedoMapsId[i] = tM.createTexture(ASSET_TEXTURE2D_MIP_ON, fullPath.c_str());
				DEBUG_ONLY(printf("Loaded base color '%s'\n", fullPath.c_str()));
			}

			//Metalness
			if (pMaterial->GetTextureCount(aiTextureType_AMBIENT) > 0
				&& (pMaterial->GetTexture(aiTextureType_AMBIENT, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS))
			{
				std::string fullPath = dir + "/" + Path.data;
				m_metalnessMapsId[i] = tM.createTexture(ASSET_TEXTURE2D_MIP_ON, fullPath.c_str());
				DEBUG_ONLY(printf("Loaded metal. texture '%s'\n", fullPath.c_str()));
			}
			else
			{
				std::string fullPath = dir + "/../black.png";
				m_metalnessMapsId[i] = tM.createTexture(ASSET_TEXTURE2D_MIP_ON, fullPath.c_str());
				DEBUG_ONLY(printf("Loaded metal. texture '%s'\n", fullPath.c_str()));
			}

			//Roughness
			if (pMaterial->GetTextureCount(aiTextureType_SPECULAR) > 0
				&& (pMaterial->GetTexture(aiTextureType_SPECULAR, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS))
			{
				std::string fullPath = dir + "/" + Path.data;
				m_roughnessMapsId[i] = tM.createTexture(ASSET_TEXTURE2D_MIP_ON, fullPath.c_str());
				DEBUG_ONLY(printf("Loaded rough. texture '%s'\n", fullPath.c_str()));
			}
			else
			{
				std::string fullPath = dir + "/../black.png";
				m_roughnessMapsId[i] = tM.createTexture(ASSET_TEXTURE2D_MIP_ON, fullPath.c_str());
				DEBUG_ONLY(printf("Loaded rough. texture '%s'\n", fullPath.c_str()));
			}

			//Normal map
			if (pMaterial->GetTextureCount(aiTextureType_HEIGHT) > 0
				&& (pMaterial->GetTexture(aiTextureType_HEIGHT, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS))
			{
				std::string fullPath = dir + "/" + Path.data;
				m_normalMapsId[i] = tM.createTexture(ASSET_TEXTURE2D_MIP_ON, fullPath.c_str());
				DEBUG_ONLY(printf("Loaded normal map texture '%s'\n", fullPath.c_str()));
			}
			else
			{
				std::string fullPath = dir + "/../black.png";
				m_normalMapsId[i] = tM.createTexture(ASSET_TEXTURE2D_MIP_ON, fullPath.c_str());
				DEBUG_ONLY(printf("Loaded normal map texture '%s'\n", fullPath.c_str()));
			}
		}
		return;
	}

	void Mesh::render(TextureManager& tm)
	{
		GL(glBindVertexArray(m_vao));
		GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[INDEX_BUFFER]));

		for (u32 i = 0; i < m_entries.size(); ++i)
		{
			const u32 m_materialIndex = m_entries[i].m_materialIndex;

			tm.bindTexture(m_albedoMapsId[m_materialIndex], GL_TEXTURE0);
			tm.bindTexture(m_metalnessMapsId[m_materialIndex], GL_TEXTURE1);
			tm.bindTexture(m_roughnessMapsId[m_materialIndex], GL_TEXTURE2);
			tm.bindTexture(m_normalMapsId[m_materialIndex], GL_TEXTURE3);
			if(m_heightMapsId[m_materialIndex])
				tm.bindTexture(m_heightMapsId[m_materialIndex], GL_TEXTURE4);

			GL(glDrawElementsBaseVertex(GL_TRIANGLES,
				m_entries[i].m_numIndices,
				GL_UNSIGNED_INT,
				(void*)(sizeof(u32) * m_entries[i].m_baseIndex),
				m_entries[i].m_baseVertex));
		}

		//for (u32 i = 0; i < m_entries.size(); ++i)
		//{
		//	const u32 m_materialIndex = m_entries[i].m_materialIndex;
		//
		//	tm.bindTexture(m_albedoMapsId[m_materialIndex], GL_TEXTURE0);
		//	tm.bindTexture(m_metalnessMapsId[m_materialIndex], GL_TEXTURE1);
		//	tm.bindTexture(m_roughnessMapsId[m_materialIndex], GL_TEXTURE2);
		//	tm.bindTexture(m_normalMapsId[m_materialIndex], GL_TEXTURE3);
		//}
		//glMultiDrawElementsIndirect(GL_TRIANGLES,
		//	GL_UNSIGNED_INT,
		//	&commands[0],
		//	nbDraw,
		//	0);		

		GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));  
		GL(glBindVertexArray(0));
	}

	void Mesh::renderDepth()
	{
		GL(glBindVertexArray(m_vao));
		GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[INDEX_BUFFER]));
		u32 nbDraw = m_entries.size();

		//glMultiDrawElementsIndirect(GL_TRIANGLES,
		//	GL_UNSIGNED_INT,
		//	Commands.data(),
		//	nbDraw,
		//	0);
		for (u32 i = 0; i < nbDraw; ++i)
		{
			GL(glDrawElementsBaseVertex(GL_TRIANGLES,
				m_entries[i].m_numIndices,
				GL_UNSIGNED_INT,
				(void*)(sizeof(u32) * m_entries[i].m_baseIndex),
				m_entries[i].m_baseVertex));
		}
		GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
		GL(glBindVertexArray(0));
	}

	void Mesh::createAABB(std::vector<glm::vec3>& position)
	{
		f32 minX, maxX, minY, maxY, minZ, maxZ;
		minX = (glm::vec4(position[0], 1.0f)).x;
		maxX = (glm::vec4(position[0], 1.0f)).x;
		minY = (glm::vec4(position[0], 1.0f)).y;
		maxY = (glm::vec4(position[0], 1.0f)).y;
		minZ = (glm::vec4(position[0], 1.0f)).z;
		maxZ = (glm::vec4(position[0], 1.0f)).z;

		auto minMaxX = std::minmax_element(std::begin(position), std::end(position), [this](const glm::vec3& v0, const glm::vec3& v1) { return (glm::vec4(v0, 1.0f)).x < (glm::vec4(v1, 1.0f)).x; });
		auto minMaxY = std::minmax_element(std::begin(position), std::end(position), [this](const glm::vec3& v0, const glm::vec3& v1) { return (glm::vec4(v0, 1.0f)).y < (glm::vec4(v1, 1.0f)).y; });
		auto minMaxZ = std::minmax_element(std::begin(position), std::end(position), [this](const glm::vec3& v0, const glm::vec3& v1) { return (glm::vec4(v0, 1.0f)).z < (glm::vec4(v1, 1.0f)).z; });
		minX = std::min(minX, (glm::vec4((*minMaxX.first), 1.0f)).x);
		maxX = std::max(maxX, (glm::vec4((*minMaxX.second), 1.0f)).x);
		minY = std::min(minY, (glm::vec4((*minMaxY.first), 1.0f)).y);
		maxY = std::max(maxY, (glm::vec4((*minMaxY.second), 1.0f)).y);
		minZ = std::min(minZ, (glm::vec4((*minMaxZ.first), 1.0f)).z);
		maxZ = std::max(maxZ, (glm::vec4((*minMaxZ.second), 1.0f)).z);

		glm::vec3 size = glm::vec3(maxX - minX, maxY - minY, maxZ - minZ);
		m_AABB.m_radius = 0.5f * glm::vec3(maxX - minX, maxY - minY, maxZ - minZ);
		m_AABB.m_center = 0.5f * glm::vec3(minX + maxX, minY + maxY, minZ + maxZ);
		m_AABB.m_transform = glm::translate(glm::mat4(1), m_AABB.m_center) * glm::scale(glm::mat4(1), size);
	}

	void Mesh::updateAABB(const glm::mat4& transform)
	{
		AABB other;
		for (int i = 0; i < 3; i++) {
			other.m_center[i] = transform[3][i];
			other.m_radius[i] = 0.0f;
			for (int j = 0; j < 3; j++) 
			{
				other.m_center[i] += transform[i][j] * m_AABB.m_center[j];
				other.m_radius[i] += std::abs(transform[i][j]) * m_AABB.m_radius[j];
			}
		}
		m_AABB.m_transform = glm::translate(other.m_center)* glm::scale(2.f*other.m_radius);
	}

	void Mesh::renderAABB()
	{
		glBindVertexArray(m_vaoBBox);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_iboBBox);

		glDrawElementsBaseVertex(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, 0, 0);
		glDrawElementsBaseVertex(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, (GLvoid*)(4 * sizeof(u16)), 0);
		glDrawElementsBaseVertex(GL_LINES, 8, GL_UNSIGNED_SHORT, (GLvoid*)(8 * sizeof(u16)), 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); 
		glBindVertexArray(0);
	}

}