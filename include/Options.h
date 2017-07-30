#pragma once

namespace OGL
{
	struct RenderingOptions
	{
		bool m_renderPostFx;
		bool m_renderImGui;
		bool m_renderShadow;
		bool m_renderOcean;
		bool m_renderGrass;
		bool m_renderTerrain;
		bool m_renderSSAO;
	};

	struct TimerValues
	{
		f32 m_frame;
		f32 m_shadow;
		f32 m_opaque;
		f32 m_geometry;
		f32 m_decal;
		f32 m_light;
		f32 m_areaLight;
		f32 m_postFX;
		f32 m_fxaa;
		f32 m_sky;
		f32 m_ssao;
		f32 m_ssr;
		f32 m_resolve;
		f32 m_physicalAL;
		f32 m_uboUpdate;
		f32 m_tonemap;
		f32 m_combine;
		f32 m_prepareSnow;
		f32 m_heightSnow;
		f32 m_blurSnow;
	};

	struct SceneOptions
	{
		f32 m_hour;
		f32 m_wetness;
		f32 m_exposure;
	};

	struct AreaLightOptions
	{
		f32 m_color[3];
		f32 m_intensity;
	};

	enum DebugMode
	{
		FinalFrame = 0,
		WorldNormal,
		AO,
		AOBlurred,
		ShadowMap,
		Diffuse,
		Specular,
		SSR,
		WorldPosition,
		Albedo,
		CurrentFrame,
		LastFrame,
		SnowHeightmap,
		NB_DEBUGMODE
	};

	const std::string& translateDebugMode(DebugMode debugMode);

	struct DebugOptions
	{
		u32 m_pickedObj;
		DebugMode m_debugMode;
		bool m_leftClicking;
		bool m_rightClicking;
	};

	struct TAAOptions
	{
		f32 m_jitterAASigma;
	};

	struct SaoOptions
	{
		f32 m_radius;
		f32 m_bias;
		f32 m_intensity;
	};
}

