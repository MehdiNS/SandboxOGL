#pragma once
#include "Texture.h"
#include "TextureManager.h"

namespace OGL
{
	class Skybox
	{
	public:
		GLuint skyboxVAO, skyboxVBO;

		Skybox(TextureManager& tm);
		Skybox();
		~Skybox();
		void Render();
	};
}
