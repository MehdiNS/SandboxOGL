#include "stdafx.h"
#include "PostEffectManager.h"
#include "Util.h"
#include "Timer.h"
#include "ShaderManager.h"
#include "TextureManager.h"
#include "FrameBuffer.h"
#include "Options.h"
#include "Renderer.h"

namespace OGL
{
	PostEffectManager::PostEffectManager()
	{
	}

	void PostEffectManager::init(TextureManager& tm, u32 finalTextureObj)
	{
		// Create Fx Framebuffer Object
		glGenFramebuffers(1, &fxFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fxFbo);
		fxDrawBuffers[0] = GL_COLOR_ATTACHMENT0;
		glDrawBuffers(1, fxDrawBuffers);

		// Create Fx textures
		glGenTextures(FX_TEXTURE_COUNT, fxTexturesId);
		for (u32 i = 0; i < FX_TEXTURE_COUNT; ++i)
		{
			fxTexturesId[i] = tm.createTexture(FBO_TEXTURE2D_FLOAT);
		}

		// Attach first fx texture to framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tm.getTextureHandle(fxTexturesId[0]), 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			DEBUG_ONLY(std::cerr << "Error when building postFX framebuffer\n" << std::endl);
			exit(EXIT_FAILURE);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}


	PostEffectManager::~PostEffectManager()
	{
		// Texture destructed by the manager
	}

	void PostEffectManager::renderAll(ShaderManager& sm, TextureManager& tm, FrameBuffer& fbo, TimerValues& timerValues, SceneOptions& sceneOptions)
	{
		GL(glBindFramebuffer(GL_FRAMEBUFFER, fxFbo));
		GL(glDisable(GL_DEPTH_TEST));

		fxTexturesId[0] = fbo.m_finalTextureCurrentId;
		u32 depth = tm.getTextureHandle(fbo.m_depthTextureId);
		u32 normal = tm.getTextureHandle(fbo.m_texturesId[FrameBuffer::GBUFFER_RT1]);

		{
			// Tonemap/Gamma
			Timer timer{ timerValues.m_tonemap };
			GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tm.getTextureHandle(fxTexturesId[1]), 0));
			GL(glClear(GL_COLOR_BUFFER_BIT));
			auto& shader = sm.getShader("Gamma");
			shader.bind();
			GL(glUniform1fv(shader.getUniformLocation("exposure"), 1, &(sceneOptions.m_exposure)));
			tm.bindTexture(fxTexturesId[0], GL_TEXTURE0);
			drawQuad();
		}

		{
			// FXAA
			Timer timer{ timerValues.m_fxaa };
			GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tm.getTextureHandle(fxTexturesId[0]), 0));
			GL(glClear(GL_COLOR_BUFFER_BIT));
			auto& shader = sm.getShader("Fxaa");
			shader.bind();
			tm.bindTexture(fxTexturesId[1], GL_TEXTURE0);
			drawQuad();
		}

		if(Renderer::g_firstFrame)
		{
			Timer timer{ timerValues.m_combine };
			GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tm.getTextureHandle(fbo.m_finalTextureWithHudId), 0));
			GL(glClear(GL_COLOR_BUFFER_BIT));
			auto& shader = sm.getShader("Blit");
			shader.bind();
			tm.bindTexture(fxTexturesId[0], GL_TEXTURE0); // Current
			drawQuad();
		}
		else
		{
			Timer timer{ timerValues.m_combine };
			GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tm.getTextureHandle(fbo.m_finalTextureWithHudId), 0));
			GL(glClear(GL_COLOR_BUFFER_BIT));
			auto& shader = sm.getShader("Combine");
			shader.bind();
			tm.bindTexture(fxTexturesId[0], GL_TEXTURE0); // Current
			tm.bindTexture(fbo.m_finalTextureLastId, GL_TEXTURE1); // Last
			drawQuad();
		}

		{
			GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tm.getTextureHandle(fbo.m_finalTextureLastId), 0));
			GL(glClear(GL_COLOR_BUFFER_BIT));
			auto& shader = sm.getShader("Blit");
			shader.bind();
			tm.bindTexture(fbo.m_finalTextureWithHudId, GL_TEXTURE0); // Current
			drawQuad();
		}		
		GL(glBindFramebuffer(GL_FRAMEBUFFER, fbo.m_fbo));
	}
}