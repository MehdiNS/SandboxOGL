#pragma once

namespace OGL
{
	class TextureManager;

	class SMFrameBuffer
	{
	public:
		SMFrameBuffer();
		SMFrameBuffer(TextureManager& tm, u32 size);
		~SMFrameBuffer();

		void Load(TextureManager& tm);

		void startFrame();
		void bindForShadowPass();

		u32 getFBO() const;
		u32 getSize() const;
		void setShadowMap(TextureManager& tm, u32 shadowMap);

		u32 m_size;

		u32 m_fbo;
		u32 m_shadowMap;
	};
}