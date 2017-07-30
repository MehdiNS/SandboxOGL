#pragma once
#include "Texture.h"

namespace OGL
{
	class TextureManager : Uncopiable
	{
	private:
		std::vector<Texture>	m_textures;
		std::vector<u32>		m_freelist;

	public:
		TextureManager();
		~TextureManager();
		
		void destroyTextures();
		void destroyTexture(s32 id);
		u32 createTexture(TextureParams& params, const std::string& filename = std::string(), VerticalFlipUse flip = NO_VFLIP);
		s32 getTextureHandle(s32 id);
		void bindTexture(s32 id, GLenum textureUnit);
		u32 size();
	};
}