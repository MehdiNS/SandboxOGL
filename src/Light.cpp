#include "stdafx.h"
#include "Light.h"
#include "TextureManager.h"

namespace OGL
{
	Light::Light() { m_LightSpaceMatrix = glm::mat4(1); }
	Light::Light(const LightDataLayout& layout) { m_lightLayout = layout; m_LightSpaceMatrix = glm::mat4(1); }
	void Light::setLightPos(const glm::vec3& newPos) { m_lightLayout.v2.xyz = newPos; }
	glm::vec3 Light::getLightPos() { return m_lightLayout.v2.xyz; }
	bool Light::isPonctual() { return (m_lightLayout.v2.w == 0); }
	bool Light::isDirectionnal() { return (m_lightLayout.v2.w == 1); }

	LightManager::LightManager()
	{
		m_nbLights = 0;
		std::fill(std::begin(m_shadowMap), std::end(m_shadowMap), 0);
	};

	void LightManager::AddLight(Light pl, TextureManager& texM)
	{
		if (m_nbLights < MAX_LIGHTS)
		{
			m_lightsInScene[m_nbLights] = pl;
			m_shadowMap[m_nbLights] = texM.createTexture(FBO_TEXTURE2D_SHADOWMAP);
			m_nbLights++;
		}
	}
}