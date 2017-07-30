#pragma once

namespace OGL
{
	class TextureManager;

	class SnowFrameBuffer
	{
	public:
		SnowFrameBuffer();
		SnowFrameBuffer(TextureManager& tm, u32 width, u32 heigth);
		~SnowFrameBuffer();

		void load(TextureManager& tm);
		void bindForSnowPass(TextureManager& tm);
		void bindForBlurPass(TextureManager& tm);
		void clear(TextureManager& tm);

		u32 getFBO() const;
		u32 getWidth() const;
		u32 getHeigth() const;

		u32 m_width;
		u32 m_heigth;

		u32 m_size;
		
		u32 m_fbo;
		u32 m_heightmapId1;
		u32 m_heightmapId2;
		u32 m_depthTextureId;
	};
}