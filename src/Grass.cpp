#include "stdafx.h"
#include "Grass.h"
#include "TextureManager.h"

namespace OGL
{
	Grass::Grass(TextureManager& tm)
	{
		m_heightTextureId = tm.createTexture(ASSET_TEXTURE2D_MIP_ON, "asset/heightmap.png");
		m_grassTextureId = tm.createTexture(ASSET_TEXTURE2D_MIP_ON, "asset/blade.png");
		glEnable(GL_PROGRAM_POINT_SIZE);
	}

	void Grass::render(TextureManager& tm)
	{
		tm.bindTexture(m_heightTextureId, GL_TEXTURE4);
		tm.bindTexture(m_grassTextureId, GL_TEXTURE5);
		u32 nbBlades = static_cast<u32>(pow(2, 16));
		u32 index;
		glDrawElementsInstanced(GL_POINTS, 1, GL_UNSIGNED_INT, &index, nbBlades);
	}
}