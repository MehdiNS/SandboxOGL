#pragma once

namespace OGL
{
	class TextureManager;

	class FrameBuffer
	{
	public:
		FrameBuffer();
		FrameBuffer(TextureManager& tm, u32 width, u32 heigth, bool useStencil = false);
		~FrameBuffer();
		void load(TextureManager& tm);
		void startFrame(TextureManager& tm);
		void bindForGeometryPass();
		void bindForSSAOPass(TextureManager& tm);
		void bindForSSAOBlurPass(TextureManager& tm, s32 pass);
		void bindForDecalPass(TextureManager& tm);
		void bindForShadingPass(TextureManager& tm, u32 irradianceMapId, u32 prefilterMapId, u32 brdfLUTId);
		void bindForAreaLightingPass(TextureManager& tm, u32 ltcAmpId, u32 ltcMatId);
		void bindForSSRPass(TextureManager& tm);
		void bindForFinalPass(TextureManager& tm);
		void unbind();
		u32 getFBO() const;
		u32 getWidth() const;
		u32 getHeigth() const;

		enum
		{
			GBUFFER_RT0 = 0,		//Color
			GBUFFER_RT1,			//Normal
			GBUFFER_RT2,			//Position
			GBUFFER_SIZE
		};

		s32 m_texturesId[GBUFFER_SIZE];
		s32 m_depthTextureId;
		s32 m_aoTextureId;
		s32 m_aoTextureBlurred1Id;
		s32 m_aoTextureBlurred2Id;
		s32 m_diffuseTextureId;
		s32 m_specularTextureId;
		s32 m_ssrTextureId;
		s32 m_finalTextureCurrentId;
		s32 m_finalTextureLastId;
		s32 m_finalTextureWithHudId;
		u32 m_fbo;
	
	private:
		u32 m_width;
		u32 m_heigth;
	};
}