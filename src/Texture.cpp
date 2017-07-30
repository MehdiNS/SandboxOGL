#include "stdafx.h"
#include "Texture.h"
#include "Util.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace OGL
{
	TextureParams ASSET_TEXTURE2D_MIP_ON{ GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, NO_SRGB, USE_MIPMAP };
	TextureParams ASSET_TEXTURE2D_MIP_OFF{ GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, NO_SRGB, NO_MIPMAP };
	TextureParams ASSET_TEXTURE2D_MIP_OFF_FLOAT{ GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 0, 0, GL_RGBA, GL_FLOAT, NO_SRGB, NO_MIPMAP };
	TextureParams FBO_TEXTURE2D_RGBA{ GL_TEXTURE_2D, 0, GL_RGBA8, 640, 480, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, NO_SRGB, NO_MIPMAP };
	TextureParams FBO_TEXTURE2D_RGBA512{ GL_TEXTURE_2D, 0, GL_RGBA8, 512, 512, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, NO_SRGB, NO_MIPMAP };
	TextureParams FBO_TEXTURE2D_R{ GL_TEXTURE_2D, 0, GL_RED, 640, 480, 0, 0, GL_RGB, GL_UNSIGNED_BYTE, NO_SRGB, NO_MIPMAP };
	TextureParams FBO_TEXTURE2D_FLOAT{ GL_TEXTURE_2D, 0, GL_RGBA32F, 640, 480, 0, 0, GL_RGBA, GL_FLOAT, NO_SRGB, NO_MIPMAP };
	TextureParams FBO_TEXTURE2D_FLOAT256{ GL_TEXTURE_2D, 0, GL_RGBA32F, 256, 256, 0, 0, GL_RGBA, GL_FLOAT, NO_SRGB, NO_MIPMAP };
	TextureParams FBO_TEXTURE2D_SHADOWMAP{ GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 512, 512, 0, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NO_SRGB, NO_MIPMAP };
	TextureParams FBO_TEXTURE2D_DEPTH_STENCIL{ GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 640, 480, 0, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NO_SRGB, NO_MIPMAP };
	TextureParams FBO_TEXTURE2D_DEPTH_STENCIL256{ GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 256, 256, 0, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NO_SRGB, NO_MIPMAP };
	TextureParams CUBEMAP{ GL_TEXTURE_CUBE_MAP, 0, GL_RGB, 0, 0, 0, 0, GL_RGB, GL_UNSIGNED_BYTE, NO_SRGB, NO_MIPMAP };
	TextureParams HDR_CUBEMAP{ GL_TEXTURE_CUBE_MAP, 0, GL_RGB32F, 0, 0, 0, 0, GL_RGB, GL_FLOAT, NO_SRGB, NO_MIPMAP };
	TextureParams EQUIRECTANGLE{ GL_TEXTURE_2D, 0, GL_RGB, 0, 0, 0, 0, GL_RGB, GL_UNSIGNED_BYTE, NO_SRGB, NO_MIPMAP };
	TextureParams HDR_EQUIRECTANGLE{ GL_TEXTURE_2D, 0, GL_RGB32F, 0, 0, 0, 0, GL_RGB, GL_FLOAT, NO_SRGB, NO_MIPMAP };
	TextureParams FBO_CUBEMAP_512{ GL_TEXTURE_CUBE_MAP, 0, GL_RGB, 512, 512, 0, 0, GL_RGB, GL_UNSIGNED_BYTE, NO_SRGB, NO_MIPMAP };
	TextureParams FBO_HDR_CUBEMAP_512{ GL_TEXTURE_CUBE_MAP, 0, GL_RGB32F, 512, 512, 0, 0, GL_RGB, GL_FLOAT, NO_SRGB, NO_MIPMAP };
	TextureParams FBO_HDR_CUBEMAP_128{ GL_TEXTURE_CUBE_MAP, 0, GL_RGB32F, 128, 128, 0, 0, GL_RGB, GL_FLOAT, NO_SRGB, USE_MIPMAP };
	TextureParams FBO_HDR_CUBEMAP_32{ GL_TEXTURE_CUBE_MAP, 0, GL_RGB32F, 32, 32, 0, 0, GL_RGB, GL_FLOAT, NO_SRGB, NO_MIPMAP };
	TextureParams FBO_CUBEMAP_32{ GL_TEXTURE_CUBE_MAP, 0, GL_RGB32F, 32, 32, 0, 0, GL_RGB, GL_FLOAT, NO_SRGB, NO_MIPMAP };
	TextureParams FBO_TEXTURE2D_512_2CH{ GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, 0, GL_RG, GL_FLOAT, NO_SRGB, NO_MIPMAP };

	Texture::Texture(TextureParams& params, const std::string& filename, VerticalFlipUse flip, s32 id)
	{
		m_id = id;
		m_params = params;
		m_filename = filename;

		// TODO : test on strings are bad. Only done at startup but still...
		if (filename.empty())
			loadFramebufferTexture();
		else if (isCompressedTexture(filename))
			loadCompressedTextureFromFile();
		else if (isHdrTexture(filename))
			loadHdr();
		else if (isCubemapTexture(filename))
			loadCubemap();
		else
			loadTextureFromFile(flip);

		GL(glBindTexture(m_params.m_target, m_textureObj));
	}

	Texture::Texture(const std::string& filename, s32 id)
	{
		m_id = -1;
		m_filename = filename;
		loadHdr();
		GL(glBindTexture(m_params.m_target, m_textureObj));
	}

	Texture::~Texture()
	{
		/*
		Textures are stored in a vector in the texture manager
		Emplace_back function can reallocate elements meaning destructing them
		We need to delete the texture only when it is no longer in the manager
		And that happens when id is -1
		*/
		if (m_id == -1)
			glDeleteTextures(1, &m_textureObj);
	}

	void Texture::loadTextureFromFile(VerticalFlipUse flip)
	{
		// Image loading
		s32 x, y, n;

		if (flip == DO_VFLIP)
			stbi_set_flip_vertically_on_load(true);
		else
			stbi_set_flip_vertically_on_load(false);

		u8* image_data = stbi_load(m_filename.c_str(), &x, &y, &n, 0);
		if (!image_data)
		{
			DEBUG_ONLY(std::cerr << "ERROR : could not load " << m_filename << std::endl);
			return;
		}

		m_params.m_width = x;
		m_params.m_height = y;

		// Power of 2 check, may screw the mipmap if not
		if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0)
			DEBUG_ONLY(std::cerr << "WARNING : " << m_filename << " is not power-of-2 dimensions" << std::endl);

		// This condition is necessary because of the way some textures are authored, 
		// combined with the fact that I don't wanna force the number of channels
		if (n == 1)
		{
			m_params.m_internalFormat = GL_RED;
			m_params.m_format = GL_RED;
		}
		else if (n == 2)
		{
			m_params.m_internalFormat = GL_RG;
			m_params.m_format = GL_RG;
		}
		else if (n == 3)
		{
			m_params.m_internalFormat = GL_RGB;
			m_params.m_format = GL_RGB;
		}
		else // n == 4
		{
			m_params.m_internalFormat = GL_RGBA;
			m_params.m_format = GL_RGBA;
		}

		GL(glGenTextures(1, &m_textureObj));
		GL(glBindTexture(m_params.m_target, m_textureObj));
		GL(glTexImage2D(m_params.m_target,
			m_params.m_level,
			m_params.m_internalFormat,
			m_params.m_width,
			m_params.m_height,
			m_params.m_border,
			m_params.m_format,
			m_params.m_type,
			image_data));

		if (m_params.m_mips)
			GL(glGenerateMipmap(m_params.m_target));

		// Trilinear filtering.
		GL(glTexParameteri(m_params.m_target, GL_TEXTURE_WRAP_S, GL_REPEAT));
		GL(glTexParameteri(m_params.m_target, GL_TEXTURE_WRAP_T, GL_REPEAT));
		GL(glTexParameteri(m_params.m_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		GL(glTexParameteri(m_params.m_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));

		// Set the maximum anisotropic filtering level allowed (perf?)
		f32 max_aniso = 0.0f;
		GL(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso));
		GL(glTexParameterf(m_params.m_target, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso));

		// No longer need the loaded image 
		stbi_image_free(image_data);
	}

	void Texture::loadCubemap()
	{
		GLenum cubesides[6] =
		{
			GL_TEXTURE_CUBE_MAP_POSITIVE_X,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
		};

		std::string cubepaths[6] =
		{
			m_filename + "/posx.jpg",
			m_filename + "/negx.jpg",
			m_filename + "/posy.jpg",
			m_filename + "/negy.jpg",
			m_filename + "/posz.jpg",
			m_filename + "/negz.jpg"
		};

		// create new cube texture and set its parameters; very similar to 2D texture
		GL(glGenTextures(1, &m_textureObj));
		GL(glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureObj));
		GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
		GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_GENERATE_MIPMAP, GL_TRUE));

		for (u32 i = 0; i < 6; i++)
		{
			s32 x, y, n;
			s32 force_channels = 3;
			u8* image_data = stbi_load(cubepaths[i].c_str(), &x, &y, &n, force_channels);
			if (!image_data)
			{
				DEBUG_ONLY(std::cerr << "ERROR: could not load : " << cubepaths[i] << std::endl);
				return;
			}

			m_params.m_width = x;
			m_params.m_height = y;

			GL(glTexImage2D(
				cubesides[i],
				0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data
				));

			stbi_image_free(image_data);
		}
		GL(glGenerateMipmap(GL_TEXTURE_CUBE_MAP));
	}

	void Texture::loadCompressedTextureFromFile()
	{
		// TODO : Investigate why I can't load some dds of sponza with gli
		gli::texture tex = gli::load(m_filename);
		if (tex.empty())
		{
			DEBUG_ONLY(std::cout << "ERROR : " << m_filename << " is empty" << std::endl);
			return;
		}
		//tex = gli::flip<gli::texture>(tex);
		gli::gl gl(gli::gl::PROFILE_GL33);
		gli::gl::format format = gl.translate(tex.format(), tex.swizzles());
		glm::ivec3 size = tex.extent(0);
		GL(glGenTextures(1, &m_textureObj));
		GL(glBindTexture(GL_TEXTURE_2D, m_textureObj));
		gli::texture::size_type texSize = tex.size(0);
		void* data = tex.data(0, 0, 0);

		if (gli::is_compressed(tex.format()))
			GL(glCompressedTexImage2D(GL_TEXTURE_2D, 0, format.Internal, size.x, size.y, 0, texSize, data));
		else
			GL(glTexImage2D(GL_TEXTURE_2D, 0, format.Internal, size.x, size.y, 0, format.External, format.Type, data));

		GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
	}

	void Texture::loadFramebufferTexture()
	{
		GL(glGenTextures(1, &m_textureObj));
		GL(glBindTexture(m_params.m_target, m_textureObj));

		if (m_params.m_target == GL_TEXTURE_2D)
		{
			GL(glTexImage2D(m_params.m_target,
				m_params.m_level,
				m_params.m_internalFormat,
				m_params.m_width,
				m_params.m_height,
				m_params.m_border,
				m_params.m_format,
				m_params.m_type,
				nullptr));

			GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

			if (m_params.m_internalFormat == GL_DEPTH_COMPONENT)
			{
				GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE));
				GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL));
			}

			GL(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			GL(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

			if (m_params.m_mips == USE_MIPMAP)
			{
				GL(glGenerateMipmap(GL_TEXTURE_2D));
			}
		}
		else //GL_TEXTURE_3D
		{
			for (u32 i = 0; i < 6; ++i)
			{
				GL(glTexImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					m_params.m_level,
					m_params.m_internalFormat,
					m_params.m_width,
					m_params.m_height,
					m_params.m_border,
					m_params.m_format,
					m_params.m_type,
					nullptr));
			}
			GL(glTexParameteri(m_params.m_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			GL(glTexParameteri(m_params.m_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
			GL(glTexParameteri(m_params.m_target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
			GL(glTexParameteri(m_params.m_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

			if (m_params.m_mips == USE_MIPMAP)
			{
				GL(glTexParameteri(m_params.m_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
				GL(glGenerateMipmap(GL_TEXTURE_CUBE_MAP));
			}
			else
			{
				GL(glTexParameteri(m_params.m_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			}
		}
	}

	void Texture::loadHdr()
	{
		stbi_set_flip_vertically_on_load(true);
		s32 x, y, n;
		f32* image_data = stbi_loadf(m_filename.c_str(), &x, &y, &n, 0);
		if (image_data)
		{
			m_params.m_width = x;
			m_params.m_height = y;

			GL(glGenTextures(1, &m_textureObj));
			GL(glBindTexture(m_params.m_target, m_textureObj));

			GL(glTexImage2D(m_params.m_target,
				m_params.m_level,
				m_params.m_internalFormat,
				m_params.m_width,
				m_params.m_height,
				m_params.m_border,
				m_params.m_format,
				m_params.m_type,
				image_data));

			//glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

			GL(glTexParameteri(m_params.m_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			GL(glTexParameteri(m_params.m_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
			GL(glTexParameteri(m_params.m_target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
			GL(glTexParameteri(m_params.m_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			GL(glTexParameteri(m_params.m_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

			stbi_image_free(image_data);
		}
		else
		{
			DEBUG_ONLY(std::cerr << "Error : failed to load hdr image " << m_filename << std::endl);
		}
	}

	void Texture::loadLdr()
	{
		s32 x, y, n;
		s32 force_channels = 3;
		stbi_set_flip_vertically_on_load(true);
		u8* image_data = stbi_load(m_filename.c_str(), &x, &y, &n, force_channels);
		if (!image_data)
		{
			DEBUG_ONLY(std::cerr << "ERROR: could not load : " << m_filename << std::endl);
			return;
		}

		m_params.m_width = x;
		m_params.m_height = y;

		GL(glGenTextures(1, &m_textureObj));
		GL(glBindTexture(m_params.m_target, m_textureObj));

		GL(glTexImage2D(m_params.m_target,
			m_params.m_level,
			m_params.m_internalFormat,
			m_params.m_width,
			m_params.m_height,
			m_params.m_border,
			m_params.m_format,
			m_params.m_type,
			image_data));

		GL(glTexParameteri(m_params.m_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GL(glTexParameteri(m_params.m_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		GL(glTexParameteri(m_params.m_target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
		GL(glTexParameteri(m_params.m_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GL(glTexParameteri(m_params.m_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

		stbi_image_free(image_data);
	}

	void Texture::bind(GLenum textureUnit)
	{
		GL(glActiveTexture(textureUnit));
		GL(glBindTexture(m_params.m_target, m_textureObj));
	}
}