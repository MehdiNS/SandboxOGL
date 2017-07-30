#pragma once
#include "Mesh.h"

namespace OGL
{
	class TextureManager;

	class MeshManager : Uncopiable
	{
	public:
		MeshManager();
		~MeshManager();
		Mesh* getMesh(const std::string& filename, TextureManager& textureManager);
		void cleanup();
		void printAll();

	private:
		using MeshMap_t = std::unordered_map<std::string, std::shared_ptr<Mesh>>;
		MeshMap_t m_meshes;
	};
}