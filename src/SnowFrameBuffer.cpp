#include "stdafx.h"
#include "SnowFrameBuffer.h"
#include "Util.h"
#include "TextureManager.h"

namespace OGL
{
	SnowFrameBuffer::SnowFrameBuffer() :
		m_fbo{ 0 },
		m_width{ 0 },
		m_heigth{ 0 },
		m_heightmapId1{ 0 },
		m_heightmapId2{ 0 }
	{
	}

	SnowFrameBuffer::SnowFrameBuffer(TextureManager& tm, u32 width, u32 heigth) :
		m_fbo{ 0 },
		m_width{ width },
		m_heigth{ heigth },
		m_heightmapId1{ 0 },
		m_heightmapId2{ 0 }
	{
		load(tm);
	}

	SnowFrameBuffer::~SnowFrameBuffer()
	{
		// Destruction des buffers
		glDeleteFramebuffers(1, &m_fbo);
		//m_texturesId.clear();
	}

	void SnowFrameBuffer::load(TextureManager& tm)
	{
		glGenFramebuffers(1, &m_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

		//Heightmap
		m_heightmapId1 = tm.createTexture(FBO_TEXTURE2D_FLOAT256);
		m_heightmapId2 = tm.createTexture(FBO_TEXTURE2D_FLOAT256);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tm.getTextureHandle(m_heightmapId1), 0);

		//Depth
		m_depthTextureId = tm.createTexture(FBO_TEXTURE2D_DEPTH_STENCIL256);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, tm.getTextureHandle(m_depthTextureId), 0);

		// Check FBO state
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			glDeleteFramebuffers(1, &m_fbo);

		// Unlock Frame Buffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	u32 SnowFrameBuffer::getFBO() const
	{
		return m_fbo;
	}

	u32 SnowFrameBuffer::getWidth() const
	{
		return m_width;
	}

	u32 SnowFrameBuffer::getHeigth() const
	{
		return m_heigth;
	}

	void SnowFrameBuffer::bindForSnowPass(TextureManager& tm)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tm.getTextureHandle(m_heightmapId2), 0);
		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, drawBuffers);
		tm.bindTexture(m_heightmapId1, GL_TEXTURE0);
	}

	void SnowFrameBuffer::bindForBlurPass(TextureManager& tm)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tm.getTextureHandle(m_heightmapId1), 0);
		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, drawBuffers);
		tm.bindTexture(m_heightmapId2, GL_TEXTURE0);
	}

	void SnowFrameBuffer::clear(TextureManager& tm)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tm.getTextureHandle(m_heightmapId2), 0);
		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, drawBuffers);
		glClear(GL_COLOR_BUFFER_BIT);
		GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		GL(glClearColor(0.f, 0.f, 0.f, 1.0f));
	}
}