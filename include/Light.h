#pragma once
#include "Quad.h"

namespace OGL
{
	struct LightDataLayout
	{
		glm::vec4 v1;	// Color.x		Color.y			Color.z			AmbientIntensity
		glm::vec4 v2;	// Pos.x(Dir.x)	Pos.y(Dir.y)	Pos.z(Dir.z)	DiffuseIntensity
		glm::vec4 v3;	// Constant		Linear			Exp				EMPTY
		glm::vec4 v4;	// EMPTY		EMPTY			EMPTY			EMPTY

		LightDataLayout()
		{
			v1 = glm::vec4(0);
			v2 = glm::vec4(0);
			v3 = glm::vec4(0);
			v4 = glm::vec4(0);
		}

		LightDataLayout(glm::vec3 color, f32 amb, f32 dif, glm::vec3 pos, glm::vec3 attenuation)
		{
			v1.x = color.x; v1.y = color.y; v1.z = color.z;
			v1.w = amb;
			v2.x = pos.x; v2.y = pos.y; v2.z = pos.z;
			v2.w = dif;
			v3.x = attenuation.x; v3.y = attenuation.y; v3.z = attenuation.z;
			v3.w = 0.f;
			v4 = glm::vec4();
		}
	};

	struct AreaLight
	{
		glm::mat4 m_transform;
		Quad m_quad;
		f32 m_scale;
		// TODO : the two following texture should be class member and not instance member
		u32 m_ltcAmpId;
		u32 m_ltcMatId;
		u32 m_lightTextureId;
	};

	class TextureManager;

	class Light
	{
	public:
		Light();
		Light(const LightDataLayout& layout);
		void setLightPos(const glm::vec3& newPos);
		glm::vec3 getLightPos();
		bool isPonctual();
		bool isDirectionnal();

		LightDataLayout m_lightLayout;
		glm::mat4 m_LightSpaceMatrix;
	};

	class LightManager
	{
	public:
		static const u32 MAX_LIGHTS = 4;

		LightManager();
		void AddLight(Light pl, TextureManager& texM);

		Light m_lightsInScene[MAX_LIGHTS];
		u32 m_shadowMap[MAX_LIGHTS];
		u32 m_nbLights;
	};

};