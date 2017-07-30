#include "stdafx.h"
#include "Util.h"

namespace OGL
{
	// from https://learnopengl.com/#!In-Practice/Debugging
	void APIENTRY glDebugOutput(GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const GLchar *message,
		void *userParam)
	{
		// ignore non-significant error/warning codes
		if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

		if (severity != GL_DEBUG_SEVERITY_HIGH)
			return;

		DEBUG_ONLY(std::cout << "---------------" << std::endl);
		DEBUG_ONLY(std::cout << "Debug message (" << id << "): " << message << std::endl);

		switch (source)
		{
		case GL_DEBUG_SOURCE_API:             DEBUG_ONLY(std::cout << "Source: API"); break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   DEBUG_ONLY(std::cout << "Source: Window System"); break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: DEBUG_ONLY(std::cout << "Source: Shader Compiler"); break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:     DEBUG_ONLY(std::cout << "Source: Third Party"); break;
		case GL_DEBUG_SOURCE_APPLICATION:     DEBUG_ONLY(std::cout << "Source: Application"); break;
		case GL_DEBUG_SOURCE_OTHER:           DEBUG_ONLY(std::cout << "Source: Other"); break;
		} DEBUG_ONLY(std::cout << std::endl);

		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR:               DEBUG_ONLY(std::cout << "Type: Error"); break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: DEBUG_ONLY(std::cout << "Type: Deprecated Behaviour"); break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  DEBUG_ONLY(std::cout << "Type: Undefined Behaviour"); break;
		case GL_DEBUG_TYPE_PORTABILITY:         DEBUG_ONLY(std::cout << "Type: Portability"); break;
		case GL_DEBUG_TYPE_PERFORMANCE:         DEBUG_ONLY(std::cout << "Type: Performance"); break;
		case GL_DEBUG_TYPE_MARKER:              DEBUG_ONLY(std::cout << "Type: Marker"); break;
		case GL_DEBUG_TYPE_PUSH_GROUP:          DEBUG_ONLY(std::cout << "Type: Push Group"); break;
		case GL_DEBUG_TYPE_POP_GROUP:           DEBUG_ONLY(std::cout << "Type: Pop Group"); break;
		case GL_DEBUG_TYPE_OTHER:               DEBUG_ONLY(std::cout << "Type: Other"); break;
		} DEBUG_ONLY(std::cout << std::endl);

		switch (severity)
		{
		case GL_DEBUG_SEVERITY_HIGH:         DEBUG_ONLY(std::cout << "Severity: high"); break;
		case GL_DEBUG_SEVERITY_MEDIUM:       DEBUG_ONLY(std::cout << "Severity: medium"); break;
		case GL_DEBUG_SEVERITY_LOW:          DEBUG_ONLY(std::cout << "Severity: low"); break;
		case GL_DEBUG_SEVERITY_NOTIFICATION: DEBUG_ONLY(std::cout << "Severity: notification"); break;
		} DEBUG_ONLY(std::cout << std::endl);
		DEBUG_ONLY(std::cout << std::endl);
		//__debugbreak();
	}

	std::string randomstring(size_t length)
	{
		auto randchar = []() -> char
		{
			const char charset[] =
				"0123456789"
				"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"abcdefghijklmnopqrstuvwxyz";
			const size_t max_index = (sizeof(charset) - 1);
			return charset[rand() % max_index];
		};
		std::string str(length, 0);
		std::generate_n(str.begin(), length, randchar);
		return str;
	}

	// Reverses (reflects) bits in a 32-bit word.
	unsigned reverse(unsigned x)
	{
		x = ((x & 0x55555555) << 1) | ((x >> 1) & 0x55555555);
		x = ((x & 0x33333333) << 2) | ((x >> 2) & 0x33333333);
		x = ((x & 0x0F0F0F0F) << 4) | ((x >> 4) & 0x0F0F0F0F);
		x = (x << 24) | ((x & 0xFF00) << 8) |
			((x >> 8) & 0xFF00) | (x >> 24);
		return x;
	}

	u32 crc32(u8 *message)
	{
		int i, j;
		u32 byte, crc;

		i = 0;
		crc = 0xFFFFFFFF;
		while (message[i] != 0) {
			byte = message[i];            // Get next byte.
			byte = reverse(byte);         // 32-bit reversal.
			for (j = 0; j <= 7; j++) {    // Do eight times.
				if ((int)(crc ^ byte) < 0)
					crc = (crc << 1) ^ 0x04C11DB7;
				else crc = crc << 1;
				byte = byte << 1;          // Ready next msg bit.
			}
			i = i + 1;
		}
		return reverse(~crc);
	}

	glm::vec3 spherical(f32 theta, f32 phi)
	{
		return glm::vec3(glm::cos(phi) * glm::sin(theta), glm::cos(theta), glm::sin(phi) * glm::sin(theta));
	}

	void printOglError()
	{
		std::string error;
		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR)
		{
			switch (err)
			{
			case GL_INVALID_OPERATION:
				error = "INVALID_OPERATION";
				break;
			case GL_INVALID_ENUM:
				error = "INVALID_ENUM";
				break;
			case GL_INVALID_VALUE:
				error = "INVALID_VALUE";
				break;
			case GL_OUT_OF_MEMORY:
				error = "OUT_OF_MEMORY";
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				error = "INVALID_FRAMEBUFFER_OPERATION";
				break;
			}
			DEBUG_ONLY(std::cout << error << std::endl);
		}
	}

#define PI (3.14159265358979f)
#define TWOPI (2.f*PI)
#define FOURPI (4.f*PI)

	std::pair<f32, f32> computeSunPosition(f64 pTime, f64 pDay, f64 pLat,
		f64 pLong, f64 GMT_offset)
	{
		f64 latRad = glm::radians(pLat);
		f64 longRad = glm::radians(pLong);

		//f64 solarTime = pTime + +0.170 * sin(4.0 * PI * (pDay - 80.0) / 373.0) -
		//	0.129 * sin(2.0 * PI * (pDay - 8.0) / 355.0) +
		//	12.0 * (((GMT_offset / 12.0) * PI) - longRad) / PI;
		//f64 solarTime2 = pTime +
		//	(0.170*sin(4 * PI*(pDay - 80) / 373) - 0.129*sin(2 * PI*(pDay - 8) / 355)) +
		//	(30 - longRad) / 15.0;
		//f64 solarTime3 = PI / 12 * (pTime
		//	+ 0.17 * sin(4 * PI * (pDay - 80) / 373)
		//	- 0.129 * sin(2 * PI * (pDay - 8) / 355));

		auto varLT = pTime;
		auto varX = glm::radians((360 * (140 - 1)) / 365.242);
		auto varEOT = 0.258 * cos(varX) - 7.416 * sin(varX) - 3.648 * cos(2 * varX) - 9.228 * sin(2 * varX);
		auto varLC = (15 * 2 - 2.2) / 15;
		auto varDST = 0;
		f64 solarTime = varLT + (varEOT / 60) - varLC - varDST;

		f64 solarDeclination = 0.4093 * sin(2.0 * PI * (pDay - 81.0) / 368.0);

		f64 sinL = std::sin(latRad);
		f64 cosL = std::cos(latRad);

		f64 sinD = std::sin(solarDeclination);
		f64 cosD = std::cos(solarDeclination);

		f64 solarTimeRad = PI * solarTime / 12.0;
		f64 sinSTR = std::sin(solarTimeRad);
		f64 cosSTR = std::cos(solarTimeRad);

		f64 theta = (PI / 2.) - std::asin((sinL * sinD) - (cosL * cosD * cosSTR));
		f64 temp1 = -(cosD * sinSTR);
		f64 temp2 = (cosL * sinD) - (sinL * cosD * cosSTR);
		f64 phi = std::atan2(temp1, temp2);
		return std::make_pair<f32, f32>(static_cast<f32>(theta), static_cast<f32>(phi));
	}

	template <class F, class ... Args>
	auto genericTimer(F f, Args && ... args)
	{
		auto before = systemclock::now();
		f(forward<Args>(args)...);
		auto after = systemclock::now();
		return 0.001f * duration_cast<microseconds>(after - before).count();
	}

	glm::mat4 computeBillboardRotationMatrix(glm::vec3 cameraPos, glm::vec3 objPos)
	{
		glm::vec3 lookAtDirection = glm::normalize(cameraPos - objPos);
		glm::vec3 x, y, z;
		z = lookAtDirection;
		glm::vec3 crossVector = glm::vec3(1, 0, 0);
		f32 absX = glm::abs(lookAtDirection.x);
		f32 absY = glm::abs(lookAtDirection.y);
		f32 absZ = glm::abs(lookAtDirection.z);
		if ((absY <= absX) && (absY <= absZ))
			crossVector = glm::vec3(0.0f, 1.0f, 0.0f);
		else if ((absZ <= absX) && (absZ <= absY))
			crossVector = glm::vec3(1.0f, 0.0f, 0.0f);
		x = glm::normalize(glm::cross(z, crossVector));
		// third base vector equals crossing first and second base vector 
		y = glm::normalize(glm::cross(z, x));
		glm::mat4 rotationMatrix = glm::mat4(
			glm::vec4(x, 0.f),
			glm::vec4(y, 0.f),
			glm::vec4(z, 0.f),
			glm::vec4(0.f, 0.f, 0.f, 1.f)
			);
		return rotationMatrix;
	}

	void clearBindings()
	{
#ifdef _DEBUG
		for (u32 i = 0; i < 8; ++i)
		{
			// TODO : (perf) Remember the clean way to do that without so many gl calls
			GL(glActiveTexture(GL_TEXTURE0 + i));
			GL(glBindTexture(GL_TEXTURE_2D, 0));
			GL(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
		}
#endif
	}

	bool isCompressedTexture(std::string filename)
	{
		std::string extension;

		size_t i = filename.rfind('.', filename.length());
		if (i != std::string::npos)
		{
			auto& extension = filename.substr(i + 1, filename.length() - i);
			if (extension == "dds")
				return true;
			else
				return false;
		}
		else
		{
			DEBUG_ONLY(std::cout << "ERROR (isCompressedTexture) : filename have no extension" << std::endl);
		}
		return false;
	}

	bool isHdrTexture(std::string filename)
	{
		std::string extension;

		size_t i = filename.rfind('.', filename.length());
		if (i != std::string::npos)
		{
			auto& extension = filename.substr(i + 1, filename.length() - i);
			if (extension == "hdr")
				return true;
			else
				return false;
		}
		else
		{
			DEBUG_ONLY(std::cout << "ERROR (isHdrTexture) : filename have no extension" << std::endl);
		}
		return false;
	}

	bool isCubemapTexture(std::string filename)
	{
		size_t i = filename.rfind('.', filename.length());
		return (i == std::string::npos);
	}

	void drawQuad()
	{
		// /!\ Need to bind the quad shader before running this function
		// See : https://rauwendaal.net/2014/06/14/rendering-a-screen-covering-triangle-in-opengl/
		GL(glDrawArrays(GL_TRIANGLES, 0, 3));
	}

	f64 wrapAngle(f64 angle, f64 max)
	{
		return angle - max * floor(angle / max);
	}

	f32 lerp(f32 w, f32 a, f32 b)
	{
		return a + w*(b - a);
	}

	f32 linearDepthWS(f32 depth, f32 nearP, f32 farP)
	{
		return (nearP*farP / (nearP - farP)) / (depth - farP / (farP - nearP));
	}

	f32 halton(s32 i, s32 base)
	{
		f32 x = 1.f / base;
		f32 v = 0.f;
		while (i > 0)
		{
			v += x * (i % base);
			i = std::floor(i / base);
			x /= base;
		}
		return v;
	}

	template <std::size_t N, s32 base1, s32 base2>
	auto generateHaltonSequence() {
		ConstexprArray<glm::vec2, N> res;
		for (std::size_t i = 1; i != N + 1; ++i)
			res[i - 1] = glm::vec2(halton(i, base1), halton(i, base2));
		return res;
	}

	void drawCube()
	{
		static u32 cubeVAO;
		static u32 cubeVBO;
		if (cubeVAO == 0)
		{
			f32 vertices[] = {
				// back face
				-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
				1.0f, -1.0f,  1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
				1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
				1.0f, -1.0f,  1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
				-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
				-1.0f, -1.0f,  1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
																	  // front face
				-1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
				1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
				1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
				1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
				-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
				-1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
																	  // left face
				-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
				-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
				-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
				-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
				-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
				-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
																	  // right face
				1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
				1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
				1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
				1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
				1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
				1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
																	 // bottom face
				-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
				1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
				1.0f,  1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
				1.0f,  1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
				-1.0f,  1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
				-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
																	  // top face
				-1.0f, -1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
				1.0f,  1.0f,  1.0f , 0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
				1.0f, -1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
				1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
				-1.0f, -1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
				-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left   
			};
			GL(glGenVertexArrays(1, &cubeVAO));
			GL(glGenBuffers(1, &cubeVBO));
			// Fill buffer
			GL(glBindBuffer(GL_ARRAY_BUFFER, cubeVBO));
			GL(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));
			// Link vertex attributes
			GL(glBindVertexArray(cubeVAO));
			GL(glEnableVertexAttribArray(0));
			GL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (GLvoid*)0));
			GL(glEnableVertexAttribArray(1));
			GL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (GLvoid*)(3 * sizeof(f32))));
			GL(glEnableVertexAttribArray(2));
			GL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (GLvoid*)(6 * sizeof(f32))));
			GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
			GL(glBindVertexArray(0));
		}
		// Render Cube
		GL(glBindVertexArray(cubeVAO));
		GL(glDrawArrays(GL_TRIANGLES, 0, 36));
		GL(glBindVertexArray(0));
	}

	bool isValidFile(const fs::path& p, fs::file_status s)
	{
		return fs::status_known(s) ? fs::exists(s) : fs::exists(p);
	}

	f32 fract(f32 f) { return f - (s32)f; }

	// From http://lolengine.net/blog/2013/09/21/picking-orthogonal-vector-combing-coconuts
	/* Requires the input to be normalised.
	* Doesn’t normalise the output. */
	glm::vec3 orthogonal(glm::vec3 v)
	{
		f32 k = fract(std::abs(v.x) + 0.5);
		return glm::vec3(-v.y, v.x - k * v.z, k * v.y);
	}

}