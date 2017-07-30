#pragma once

namespace OGL
{
	class Shader
	{
		enum ShaderType
		{
			VERTEX = 0,
			FRAGMENT,
			GEOMETRY,
			//TESS_CONTROL,
			//TESS_EVALUATION,
			SHADER_NUMBER
		};

		static std::tuple<GLenum, const std::string> shaderInfo[];

	public:
		Shader(const std::string& filename);
		~Shader();

		void bind();
		void unbind();
		void update();
		void retriveUniformLocation();
		u32 getUniformLocation(const std::string& s);
		bool isValidUBO(const std::string& s);
		static void printShaderInfoLog(u32 shaderId);
		static u32 loadShaderFromFile(std::string path, GLenum shaderType);

	public:
		u32 m_program;
		std::array<u32, SHADER_NUMBER> m_shaders;
		std::string m_name;
		std::unordered_map<std::string, s32> m_uniformMap;
		std::vector<std::string> m_uniformBlockList;
	};
}