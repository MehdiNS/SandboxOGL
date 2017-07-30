#pragma once

namespace OGL
{
	class TextureManager;

	class Grass
	{
	public:
		Grass(TextureManager& tm);
		~Grass() {};
		void render(TextureManager& tm);

		u32 m_heightTextureId;
		u32 m_grassTextureId;
	private:
		u32 m_vao;
	};
}