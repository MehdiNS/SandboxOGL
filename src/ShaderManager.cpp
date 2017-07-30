#include "stdafx.h"
#include "ShaderManager.h"
#include "Util.h"

namespace OGL
{
	ShaderManager::ShaderManager()
	{
	}

	ShaderManager::~ShaderManager()
	{
		cleanup();
	}

	Shader& ShaderManager::getShader(const std::string& filename)
	{
		auto search = m_shaders.find(filename);
		if (search != m_shaders.end()) {
			return *search->second.get();
		}
		// If not, create it and
		// Add it to the texture map
		m_shaders[filename] = std::make_unique<Shader>(filename);
		return *m_shaders[filename].get();
	}


	void ShaderManager::cleanup()
	{
		m_shaders.clear();
	}


	void ShaderManager::printAll()
	{
		// Iterate and print keys and values of unordered_map
		for (const auto& elt : m_shaders)
		{
			DEBUG_ONLY(std::cout << "Key:[" << elt.first << "] Id:[" << elt.second.use_count() << "]\n");
		}
	}

	void ShaderManager::updateAll()
	{
		for (auto& shader : m_shaders)
		{
			(*shader.second).update();
		}
	}
}