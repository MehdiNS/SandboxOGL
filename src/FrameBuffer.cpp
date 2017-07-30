#include "stdafx.h"
#include "FrameBuffer.h"
#include "Util.h"
#include "TextureManager.h"

namespace OGL
{
	FrameBuffer::FrameBuffer() :
		m_fbo{ 0 },
		m_width{ 0 },
		m_heigth{ 0 },
		m_texturesId{},
		m_depthTextureId{ 0 },
		m_diffuseTextureId{ 0 },
		m_specularTextureId{ 0 },
		m_finalTextureCurrentId{ 0 },
		m_finalTextureLastId{ 0 }
	{
	}

	FrameBuffer::FrameBuffer(TextureManager& tm, u32 width, u32 heigth, bool useStencil) :
		m_fbo{ 0 },
		m_width{ width },
		m_heigth{ heigth },
		m_texturesId{},
		m_depthTextureId{ 0 },
		m_diffuseTextureId{ 0 },
		m_specularTextureId{ 0 },
		m_finalTextureCurrentId{ 0 },
		m_finalTextureLastId{ 0 }
	{
		load(tm);
	}

	FrameBuffer::~FrameBuffer()
	{
		glDeleteFramebuffers(1, &m_fbo);
	}

	void FrameBuffer::load(TextureManager& tm)
	{
		GL(glGenFramebuffers(1, &m_fbo));
		GL(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));

		// G-Buffer & depth texture creation
		//Color
		m_texturesId[GBUFFER_RT0] = tm.createTexture(FBO_TEXTURE2D_RGBA);
		GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tm.getTextureHandle(m_texturesId[GBUFFER_RT0]), 0));

		//Normal
		m_texturesId[GBUFFER_RT1] = tm.createTexture(FBO_TEXTURE2D_FLOAT);
		GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, tm.getTextureHandle(m_texturesId[GBUFFER_RT1]), 0));

		//Position
		m_texturesId[GBUFFER_RT2] = tm.createTexture(FBO_TEXTURE2D_FLOAT);
		GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, tm.getTextureHandle(m_texturesId[GBUFFER_RT2]), 0));

		//Depth
		m_depthTextureId = tm.createTexture(FBO_TEXTURE2D_DEPTH_STENCIL);
		GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, tm.getTextureHandle(m_depthTextureId), 0));

		//SSAO
		m_aoTextureId = tm.createTexture(FBO_TEXTURE2D_RGBA);
		m_aoTextureBlurred1Id = tm.createTexture(FBO_TEXTURE2D_RGBA);
		m_aoTextureBlurred2Id = tm.createTexture(FBO_TEXTURE2D_RGBA);
		GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, tm.getTextureHandle(m_aoTextureBlurred2Id), 0));

		//Diffuse lighting
		m_diffuseTextureId = tm.createTexture(FBO_TEXTURE2D_FLOAT);
		GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, tm.getTextureHandle(m_diffuseTextureId), 0));

		//Speular lighting
		m_specularTextureId = tm.createTexture(FBO_TEXTURE2D_FLOAT);
		GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, tm.getTextureHandle(m_specularTextureId), 0));

		//Final
		m_finalTextureCurrentId = tm.createTexture(FBO_TEXTURE2D_FLOAT);
		m_finalTextureLastId = tm.createTexture(FBO_TEXTURE2D_FLOAT);
		m_finalTextureWithHudId = tm.createTexture(FBO_TEXTURE2D_FLOAT);
		GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_2D, tm.getTextureHandle(m_finalTextureCurrentId), 0));

		//SSR
		m_ssrTextureId = tm.createTexture(FBO_TEXTURE2D_FLOAT);
		GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT7, GL_TEXTURE_2D, tm.getTextureHandle(m_ssrTextureId), 0));

		// Check FBO state
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			glDeleteFramebuffers(1, &m_fbo);
		}

		// Unlock Frame Buffer
		unbind();
	}

	void FrameBuffer::startFrame(TextureManager& tm)
	{
		GL(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
		GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_2D, tm.getTextureHandle(m_finalTextureCurrentId), 0));
		clearBindings();
		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7 };
		GL(glDrawBuffers(8, drawBuffers));
		GL(glClearColor(0.f, 0.f, 0.f, 1.f));
		GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));

		GLenum drawBuffersSAO[] = { GL_COLOR_ATTACHMENT3 };
		GL(glDrawBuffers(1, drawBuffersSAO));
		GL(glClearColor(1.f, 1.f, 1.f, 1.f));
		GL(glClear(GL_COLOR_BUFFER_BIT));
	}

	void FrameBuffer::bindForGeometryPass()
	{
		GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo));
		clearBindings();
		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		GL(glDrawBuffers(3, drawBuffers));
		GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
		GL(glClearColor(0.f, 0.f, 0.f, 1.0f));
	}

	void FrameBuffer::bindForSSAOPass(TextureManager& tm)
	{
		//GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo));
		clearBindings();
		GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, tm.getTextureHandle(m_aoTextureId), 0));
		GL(glDrawBuffer(GL_COLOR_ATTACHMENT3));
		tm.bindTexture(m_texturesId[GBUFFER_RT1], GL_TEXTURE0);
		tm.bindTexture(m_depthTextureId, GL_TEXTURE1);
	}

	void FrameBuffer::bindForSSAOBlurPass(TextureManager& tm, s32 pass)
	{
		clearBindings();
		if (pass == 0)
		{
			GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, tm.getTextureHandle(m_aoTextureBlurred1Id), 0));
			tm.bindTexture(m_aoTextureId, GL_TEXTURE0);
		}
		else // pass == 1
		{
			GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, tm.getTextureHandle(m_aoTextureBlurred2Id), 0));
			tm.bindTexture(m_aoTextureBlurred1Id, GL_TEXTURE0);
		}
		GL(glDrawBuffer(GL_COLOR_ATTACHMENT3));
		tm.bindTexture(m_depthTextureId, GL_TEXTURE1);
	}

	void FrameBuffer::bindForDecalPass(TextureManager& tm)
	{
		//GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo));
		clearBindings();
		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		GL(glDrawBuffers(2, drawBuffers));
		//for (u32 i = 0; i < GBUFFER_SIZE; i++) {
		//	tm.bindTexture(m_texturesId[GBUFFER_RT0 + i], 3 + GL_TEXTURE0 + i);
		//}
		tm.bindTexture(m_depthTextureId, GL_TEXTURE5);
	}

	void FrameBuffer::bindForShadingPass(TextureManager& tm, u32 irradianceMapId, u32 prefilterMapId, u32 brdfLUTId)
	{
		GL(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
		clearBindings();
		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 };
		GL(glDrawBuffers(2, drawBuffers));
		for (u32 i = 0; i < GBUFFER_SIZE; i++) {
			tm.bindTexture(m_texturesId[GBUFFER_RT0 + i], GL_TEXTURE0 + i);
		}
		tm.bindTexture(m_depthTextureId, GL_TEXTURE0 + GBUFFER_SIZE);
		// Slot [GL_TEXTURE0 + GBUFFER_SIZE + 1] is reserved for a shadowmap
		tm.bindTexture(irradianceMapId, GL_TEXTURE0 + GBUFFER_SIZE + 2);
		tm.bindTexture(prefilterMapId, GL_TEXTURE0 + GBUFFER_SIZE + 3);
		tm.bindTexture(brdfLUTId, GL_TEXTURE0 + GBUFFER_SIZE + 4);
		tm.bindTexture(m_aoTextureBlurred2Id, GL_TEXTURE0 + GBUFFER_SIZE + 5);
	}

	void FrameBuffer::bindForAreaLightingPass(TextureManager& tm, u32 ltcAmpId, u32 ltcMatId)
	{
		//GL(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
		clearBindings();
		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 };
		GL(glDrawBuffers(2, drawBuffers));
		for (u32 i = 0; i < GBUFFER_SIZE; i++) 
			tm.bindTexture(m_texturesId[GBUFFER_RT0 + i], GL_TEXTURE0 + i);
		tm.bindTexture(m_depthTextureId, GL_TEXTURE0 + GBUFFER_SIZE);
		tm.bindTexture(ltcAmpId, GL_TEXTURE0 + GBUFFER_SIZE + 1);
		tm.bindTexture(ltcMatId, GL_TEXTURE0 + GBUFFER_SIZE + 2);
	}


	void FrameBuffer::bindForSSRPass(TextureManager& tm)
	{
		//GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo));
		clearBindings();
		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT7 };
		GL(glDrawBuffers(1, drawBuffers));
		tm.bindTexture(m_texturesId[GBUFFER_RT1], GL_TEXTURE0); // Normal
		tm.bindTexture(m_depthTextureId, GL_TEXTURE1);
		tm.bindTexture(m_finalTextureCurrentId, GL_TEXTURE2);
		tm.bindTexture(m_texturesId[GBUFFER_RT2], GL_TEXTURE3); // Position
	}

	void FrameBuffer::bindForFinalPass(TextureManager& tm)
	{
		clearBindings();
		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT6 };
		GL(glDrawBuffers(1, drawBuffers));
		tm.bindTexture(m_diffuseTextureId, GL_TEXTURE0);
		tm.bindTexture(m_specularTextureId, GL_TEXTURE1);
	}

	void FrameBuffer::unbind()
	{
		GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}

	u32 FrameBuffer::getFBO() const
	{
		return m_fbo;
	}

	u32 FrameBuffer::getWidth() const
	{
		return m_width;
	}

	u32 FrameBuffer::getHeigth() const
	{
		return m_heigth;
	}
}