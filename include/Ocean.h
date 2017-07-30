#pragma once

namespace OGL
{
	class TextureManager;

	class Ocean
	{
	public:

		Ocean(TextureManager& tm);
		~Ocean() {};

		void buildOcean(u32 rads, u32 angs, f32 radius);
		void render(TextureManager& tm);
		void renderDepth();

		u32 nbIndices;
		u32 m_vao;
		u32 m_buffers[4];
		u32 m_oceanNormalMap;
	};
}