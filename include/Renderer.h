#pragma once
#include "Options.h"
#include "Scene.h"
#include "PostEffectManager.h"
#include "TextureManager.h"
#include "ShaderManager.h"
#include "Light.h"
#include "Camera.h"
#include "MeshManager.h"

namespace OGL
{
	class Renderer
	{
	public:
		Renderer();
		~Renderer();

		void initScene();
		void prepareScene(f32 dt, f32 t);
		void renderScene();

		// Deferred
		void renderOpaque();
		void geometryPass();
		void ambientPass();
		void renderSSD();
		void shadingPass();
		void resolvePass();
		void SSRPass();
		void updateUBOLight(Light& light);

		//Forward
		void renderShadow();
		void renderPostEffects();
		void renderPickedBoundingVolume();
		void renderAxis();
		void renderImGui();
		void renderSkybox();
		void finalize();
		void prepareSnow();
		void renderAreaLight();
		void chooseDebugMode();
		void initIBL();

		Camera m_camera;
		PostEffectManager m_postEffectManager;
		LightManager m_lightManager;
		TextureManager m_textureManager;
		MeshManager m_meshManager;
		ShaderManager m_shaderManager;
		std::unique_ptr<Scene> m_scene;
		RenderingOptions m_renderingOptions;
		DebugOptions m_debugOptions;
		AreaLightOptions m_areaLightOptions;
		TAAOptions m_taaOptions;
		SaoOptions m_saoOptions;
		TimerValues m_timerValues;

		glm::vec3 m_sunDir;
		u32 m_uboSky;
		u32 m_uboLight;
		static bool g_firstFrame;
		u32 m_sampleCount;
		const s32 NUM_SAMPLES = 1;
	};
}

