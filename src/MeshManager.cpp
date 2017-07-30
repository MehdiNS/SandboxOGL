#include "stdafx.h"
#include "MeshManager.h"

namespace OGL
{
	MeshManager::MeshManager()
	{
	}

	MeshManager::~MeshManager()
	{
		cleanup();
	}

	Mesh* MeshManager::getMesh(const std::string& filename, TextureManager& textureManager)
	{
		auto search = m_meshes.find(filename);
		if (search != m_meshes.end()) 
		{
			return search->second.get();
		}
		// If not, create it and
		// Add it to the mesh map
		m_meshes[filename] = std::make_shared<Mesh>(filename, textureManager);
		return m_meshes[filename].get();
	}


	void MeshManager::cleanup()
	{
		m_meshes.clear();
	}

	void MeshManager::printAll()
	{
#ifdef _DEBUG
		for (const auto& elt : m_meshes) 
		{
			DEBUG_ONLY(std::cout << "Key:[" << elt.first << "] Id:[" << elt.second.use_count() << "]\n");
		}
#endif
	}
}