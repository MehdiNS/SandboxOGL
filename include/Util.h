#pragma once

namespace OGL
{

#define NB_ELEMENTS_IN_ARRAY(a) (sizeof(a)/sizeof(a[0]))
#define INDEX_BUFFER	0    
#define POS_VB			1
#define NORMAL_VB		2
#define TEXCOORD_VB		3
#define NB_BUFFER		4   
//#define INDIRECT_BUFFER 5   

	enum
	{
		USE_GEOM = 1,
		USE_TESS
	};

	// CS util
	std::string randomstring(size_t length);
	u32 reverse(u32 x);	// Reverses (reflects) bits in a 32-bit word.
	u32 crc32(u8 *message);

	// Math util
	glm::vec3 spherical(f32 theta, f32 phi);
	std::pair<f32, f32> computeSunPosition(f64 pTime, f64 pDay, f64 pLat, f64 pLong, f64 GMT_offset); //pDay [0,365]
	glm::mat4 computeBillboardRotationMatrix(glm::vec3 cameraPos, glm::vec3 objPos);
	f64 wrapAngle(f64 angle, f64 max);
	f32 lerp(f32 w, f32 a, f32 b);
	f32 linearDepthWS(f32 depth, f32 nearP, f32 farP);
	f32 halton(s32 i, s32 base);
	template <std::size_t N, s32 base1, s32 base2>	auto generateHaltonSequence();
	f32 fract(f32 f);
	glm::vec3 orthogonal(glm::vec3 v);

	// GL util 
	void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity,
		GLsizei length, const GLchar *message, void *userParam);
	void printOglError();
	void clearBindings();
	void drawQuad();
	void drawCube();	// drawCube() renders a 1x1 cube in NDC.

#ifdef _DEBUG
#  define GL(line) do {                      \
       line;                                 \
       printOglError();						 \
   } while(0)
#else
#  define GL(line) line
#endif

	// C++ util
	template <class F, class ... Args>	auto genericTimer(F f, Args&& ... args);
	bool isCompressedTexture(std::string filename);
	bool isHdrTexture(std::string filename);
	bool isCubemapTexture(std::string filename);
	bool isValidFile(const fs::path& p, fs::file_status s = fs::file_status{});

#ifdef _DEBUG
#define DEBUG_ONLY(line) do {              \
       line;                               \
   } while(0)
#else
#define DEBUG_ONLY(line)  
#endif

	template <class T, std::size_t N>
	class ConstexprArray {
	public:
		using value_type = T;
		using size_type = std::size_t;
	private:
		value_type elts[N];
	public:
		constexpr ConstexprArray() : elts{ {} }
		{
		}
		constexpr const value_type& operator[](size_type n) const
		{
			return elts[n];
		}
		constexpr value_type& operator[](size_type n)
		{
			return elts[n];
		}
	};
}
