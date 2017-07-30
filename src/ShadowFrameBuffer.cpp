#include "stdafx.h"
#include "ShadowFrameBuffer.h"
#include "TextureManager.h"

namespace OGL
{
	SMFrameBuffer::SMFrameBuffer()
	{
	}

	SMFrameBuffer::SMFrameBuffer(TextureManager& tm, u32 size) :
		m_fbo{0},
		m_size{size},
		m_shadowMap{}
	{
		Load(tm);
	}

	SMFrameBuffer::~SMFrameBuffer()
	{
		// Destruction des buffers
		glDeleteFramebuffers(1, &m_fbo);
	}

	void SMFrameBuffer::Load(TextureManager& tm)
	{
		// Génération d'un id
		glGenFramebuffers(1, &m_fbo);
		// Verrouillage du Frame Buffer
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

		//Depth
		m_shadowMap = tm.createTexture(FBO_TEXTURE2D_SHADOWMAP);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tm.getTextureHandle(m_shadowMap), 0);

		glDrawBuffer(GL_NONE); // No color buffer is drawn to.

		// Check FBO state
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			glDeleteFramebuffers(1, &m_fbo);
		}

		// Déverrouillage du Frame Buffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void SMFrameBuffer::startFrame()
	{
		glViewport(0, 0, m_size, m_size);
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		glClearDepth(1.0);
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	void SMFrameBuffer::bindForShadowPass()
	{
		// size of the viewport should be equal to size of the shadow map
		glViewport(0, 0, m_size, m_size);
		// set framebuffer for first rendering pass
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		// clear the shadow map with default value
		glClearDepth(1.0);
		glClear(GL_DEPTH_BUFFER_BIT);
		glDrawBuffer(GL_NONE);
	}


	u32 SMFrameBuffer::getFBO() const
	{
		return m_fbo;
	}

	u32 SMFrameBuffer::getSize() const
	{
		return m_size;
	}

	void SMFrameBuffer::setShadowMap(TextureManager& tm, u32 shadowMap)
	{
		m_shadowMap = shadowMap;
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tm.getTextureHandle(m_shadowMap), 0);
	}
}