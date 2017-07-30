#pragma once
#include "Ocean.h"
#include "Grass.h"
#include "Object.h"
#include "FrameBuffer.h"
#include "ShadowFrameBuffer.h"
#include "SnowFrameBuffer.h"
#include "Options.h"
#include "Light.h"
#include "Terrain.h"

namespace OGL
{
	class TextureManager;
	class ShaderManager;
	class MeshManager;
	class ShaderManager;
	class Camera;

	struct MatrixData
	{
		glm::mat4 projection;
		glm::mat4 view;
		glm::mat4 viewProjInv;
		glm::mat4 normal;
		glm::vec4 cameraPos;		// Camera position(xyz), CameraNear(w) 
		glm::vec4 sunDir;			// Sun direction(xyz), CameraFar(w)
		glm::vec4 skyData;			// Wetness(x) 
	};

	struct PointLightData
	{
		LightDataLayout pointLight;
	};

	class Scene
	{
	public:
		Scene(LightManager& lm, TextureManager& tm, MeshManager& mm, ShaderManager& sm);
		~Scene();
		void createAssets(LightManager& lm, TextureManager& tm, MeshManager& mm, ShaderManager& sm);
		void prepareFrame();
		void updateUBOStatic(LightManager& lm, Camera& camera);
		void generateUniformBuffers(LightManager & lm, ShaderManager & sm);

		MatrixData m_matrixData;
		PointLightData m_pointLightData;
		u32 m_uboStatic;
		u32 m_uboLight;
		//u32 m_uboLightArray;
		u32 m_uboSky;
		u32 heightmapSnowId;

		struct IBLData 
		{
			u32 m_envCubeMapId;
			u32 m_irrCubeMapId;
			u32 m_prefilterMapId;
			u32 m_brdfLUTId;
		} m_IBLData;

		std::unique_ptr<FrameBuffer> m_frameBuffer;
		std::unique_ptr<SMFrameBuffer> m_shadowFrameBuffer;
		std::unique_ptr<SnowFrameBuffer> m_snowFrameBuffer;
		std::vector<Object> m_objectList;
		std::vector<Object> m_decalList;
		std::vector<AreaLight> m_areaLightList;
		std::shared_ptr<Terrain> m_terrain;
		std::unique_ptr<Ocean> m_ocean;
		std::unique_ptr<Grass> m_grass;

		SceneOptions m_sceneOptions;
	};
}