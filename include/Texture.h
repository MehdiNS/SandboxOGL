#pragma once

namespace OGL
{
	enum MipMapUse : bool 
	{
		NO_MIPMAP = false,
		USE_MIPMAP = true,
	};

	enum SrgbUse : bool
	{
		NO_SRGB = false,
		USE_SRGB = true,
	};

	enum VerticalFlipUse : bool
	{
		NO_VFLIP = false,
		DO_VFLIP = true,
	};

	struct TextureParams
	{
		GLenum		m_target;
		GLint		m_level;
		GLint		m_internalFormat;
		GLsizei		m_width;
		GLsizei		m_height;
		GLsizei		m_depth;
		GLint		m_border;
		GLenum		m_format;
		GLenum		m_type;
		SrgbUse		m_sRGB;
		MipMapUse	m_mips;
	};

	// TODO : Need to find something to replace this. It's ugly. 
	// Other options would be to pass all the parameters each time (not expressive, very verbose), 
	// or maybe use a macro instead of this
	extern TextureParams ASSET_TEXTURE2D_MIP_ON;
	extern TextureParams ASSET_TEXTURE2D_MIP_OFF;
	extern TextureParams ASSET_TEXTURE2D_MIP_OFF_FLOAT;
	extern TextureParams FBO_TEXTURE2D_RGBA;
	extern TextureParams FBO_TEXTURE2D_RGBA512;
	extern TextureParams FBO_TEXTURE2D_R;
	extern TextureParams FBO_TEXTURE2D_FLOAT;
	extern TextureParams FBO_TEXTURE2D_FLOAT256;
	extern TextureParams FBO_TEXTURE2D_SHADOWMAP;
	extern TextureParams FBO_TEXTURE2D_DEPTH_STENCIL;
	extern TextureParams FBO_TEXTURE2D_DEPTH_STENCIL256;
	extern TextureParams CUBEMAP;
	extern TextureParams HDR_CUBEMAP;
	extern TextureParams FBO_CUBEMAP_512;
	extern TextureParams FBO_HDR_CUBEMAP_512;
	extern TextureParams FBO_HDR_CUBEMAP_128;
	extern TextureParams FBO_HDR_CUBEMAP_32;
	extern TextureParams FBO_CUBEMAP_32;
	extern TextureParams EQUIRECTANGLE;
	extern TextureParams HDR_EQUIRECTANGLE;
	extern TextureParams FBO_TEXTURE2D_512_2CH;

	class Texture
	{
	public:
		Texture(TextureParams& params, const std::string& filename = {}, VerticalFlipUse flip = NO_VFLIP, s32 id = -1);
		Texture(const std::string& filename = {}, s32 id = -1);

		// "Holy Trinity"
		Texture(const Texture& other) :
			m_id{ other.m_id },
			m_filename{ other.m_filename },
			m_params{ other.m_params },
			m_textureObj{ other.m_textureObj }
		{
		}
		void swap(Texture &other)
		{
			using std::swap;
			swap(m_id, other.m_id);
			swap(m_filename, other.m_filename);
			swap(m_params, other.m_params);
			swap(m_textureObj, other.m_textureObj);
		}
		Texture& operator=(const Texture&other)
		{
			Texture{ other }.swap(*this);
			return *this;
		}
		~Texture();

		void loadTextureFromFile(VerticalFlipUse flip);
		void loadCompressedTextureFromFile();
		void loadFramebufferTexture();
		void loadHdr();
		void loadLdr();
		void loadCubemap();

		void bind(GLenum TextureUnit);
		const std::string& getName() { return m_filename; }

	public:
		TextureParams m_params;
		s32 m_id;
		u32 m_textureObj;
		std::string m_filename;
	};
}
