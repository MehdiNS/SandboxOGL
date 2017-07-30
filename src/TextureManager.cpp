#include "stdafx.h"
#include "TextureManager.h"
#include "Util.h"

namespace OGL
{
	TextureManager::TextureManager()
	{
	}

	TextureManager::~TextureManager()
	{
		// Textures ids are put a -1, ie marked for proper delete
		destroyTextures();
	}

	void TextureManager::destroyTextures()
	{
		std::for_each(std::begin(m_textures),
			std::end(m_textures),
			[](Texture& t) {t.m_id = -1; });
	}

	void TextureManager::destroyTexture(s32 id)
	{
		m_textures[id].m_id = -1;
		m_freelist.emplace_back(id);
	}

	u32 TextureManager::createTexture(TextureParams& params, const std::string& filename, VerticalFlipUse flip)
	{
		if (!m_freelist.empty())
		{
			u32 free = m_freelist.back();
			m_freelist.pop_back();
			m_textures[free].m_id = free;
			return free;
		}
		else
		{
			u32 id = m_textures.size();
			m_textures.push_back(Texture(params, filename, flip, id));
			return id;
		}
	}

	s32 TextureManager::getTextureHandle(s32 id)
	{
		if (m_textures[id].m_id == -1)
			return 0;
		else
			return m_textures[id].m_textureObj;
	}

	void TextureManager::bindTexture(s32 id, GLenum textureUnit)
	{
		if (m_textures[id].m_id != -1)
			m_textures[id].bind(textureUnit);
		else
			assert(false && "No valid texture at given id");
	}

	u32 TextureManager::size()
	{
		return static_cast<u32>(m_textures.size());
	}

}