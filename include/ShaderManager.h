#pragma once
#include "Shader.h"

namespace OGL
{
	class ShaderManager : Uncopiable
	{
	public:
		ShaderManager();
		~ShaderManager();
		Shader& getShader(const std::string& filename);
		void cleanup();
		void printAll();
		void updateAll();

		using ShaderMap_t = std::unordered_map<std::string, std::shared_ptr<Shader>>;
		ShaderMap_t  m_shaders;
	};
}