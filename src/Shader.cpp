#include "stdafx.h"
#include "Shader.h"
#include "Util.h"

namespace OGL
{
	// TODO : must have been really tired when I thought it could be a good idea...
	std::tuple<GLenum, const std::string> Shader::shaderInfo[] =
	{
		std::make_tuple(GL_VERTEX_SHADER,".vert"),
		std::make_tuple(GL_FRAGMENT_SHADER,".frag"),
		std::make_tuple(GL_GEOMETRY_SHADER,".geom"),
		//std::make_tuple(GL_TESS_CONTROL_SHADER,".tc"),
		//std::make_tuple(GL_TESS_EVALUATION_SHADER,".te") 
	};

	void Shader::printShaderInfoLog(u32 shaderID)
	{
		s32 max_length = 2048;
		s32 actual_length = 0;
		s8 log[2048];
		glGetShaderInfoLog(shaderID, max_length, &actual_length, log);
		DEBUG_ONLY(std::cerr << "Shader info log for GL index " << shaderID << " : \n" << log << std::endl);
	}

	u32 Shader::loadShaderFromFile(std::string path, GLenum shaderType)
	{
		//Open file 
		u32 shaderID = 0;
		std::string shaderString;
		std::ifstream sourceFile(path);

		//Source file loaded 
		if (sourceFile) {
			//Get shader source 
			shaderString.assign((std::istreambuf_iterator<s8>(sourceFile)), std::istreambuf_iterator<s8>());
			//Create shader ID 
			shaderID = glCreateShader(shaderType);
			//Set shader source 
			const s8* shaderSource = shaderString.c_str();
			glShaderSource(shaderID, 1, (const s8**)&shaderSource, NULL);
			//Compile shader source 
			glCompileShader(shaderID);
			//Check for compile errors
			s32 params = -1;
			glGetShaderiv(shaderID, GL_COMPILE_STATUS, &params);
			if (GL_TRUE != params) {
				DEBUG_ONLY(std::cout << "ERROR: GL shader did not compile :  " << path.c_str() << std::endl);
				printShaderInfoLog(shaderID);
				return false;
			}
		}
		return shaderID;
	}

	Shader::Shader(const std::string& fileName)
	{
		m_name = fileName;
		m_program = glCreateProgram();
		const std::string folderName = "shader/";
		for (u8 i = 0; i < SHADER_NUMBER; ++i)
		{
			auto& shaderName = folderName + fileName + std::get<1>(shaderInfo[i]);
			m_shaders[i] = isValidFile(shaderName) ? loadShaderFromFile(shaderName, std::get<0>(shaderInfo[i])) : 0;
			if (m_shaders[i])
				GL(glAttachShader(m_program, m_shaders[i]));
		}
		assert(m_shaders[0] != 0);
		GL(glLinkProgram(m_program));
		GL(glValidateProgram(m_program));
		retriveUniformLocation();
	}

	Shader::~Shader()
	{
		// The shader manager have the responsability to clean the mess
		//for (u32 i = 0; i < SHADER_NUMBER; ++i)
		//{
		//	glDetachShader(m_program, m_shaders[i]));
		//	glDeleteShader(m_shaders[i]));
		//}
		//glDeleteProgram(m_program));
	}

	void Shader::bind()
	{
		GL(glUseProgram(m_program));
	}

	void Shader::unbind()
	{
		GL(glUseProgram(0));
	}

	void Shader::update()
	{
		const std::string folderName = "shader/";

		for (u32 i = 0; i < SHADER_NUMBER; ++i)
		{
			if (m_shaders[i])
			{
				GL(glDetachShader(m_program, m_shaders[i]));
				GL(glDeleteShader(m_shaders[i]));
			}

			auto& shaderName = folderName + m_name + std::get<1>(shaderInfo[i]);
			m_shaders[i] = isValidFile(shaderName) ? loadShaderFromFile(folderName + m_name + std::get<1>(shaderInfo[i]), std::get<0>(shaderInfo[i])) : 0;

			if (m_shaders[i])
			{
				DEBUG_ONLY(std::cout << "Recompiling " << shaderName << "\n");
				GL(glAttachShader(m_program, m_shaders[i]));
				GL(glLinkProgram(m_program));
				GL(glValidateProgram(m_program));
				retriveUniformLocation();
			}
		}
	}

	void Shader::retriveUniformLocation()
	{
		GLint numUniforms = 0;
		GL(glGetProgramInterfaceiv(m_program, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms));

		GLenum properties[] = { GL_NAME_LENGTH, GL_TYPE, GL_LOCATION, GL_BLOCK_INDEX };

		for (int i = 0; i < numUniforms; ++i) {
			s32 results[4];
			GL(glGetProgramResourceiv(m_program, GL_UNIFORM, i, 4, properties, 4, NULL, results));

			//if (results[3] != -1) continue;  // Skip uniforms in blocks
			s32 nameBufSize = results[0] + 1;
			s8* name = new s8[nameBufSize];
			glGetProgramResourceName(m_program, GL_UNIFORM, i, nameBufSize, NULL, name);
			u32 location = glGetUniformLocation(m_program, name);
			m_uniformMap[name] = location;
			delete[] name;
		}

		GLint uniformBlocks;
		GLint uniformBlockMaxNameLength;
		GLint uniformMaxNameLength;

		GL(glGetProgramiv(m_program, GL_ACTIVE_UNIFORM_BLOCKS, &uniformBlocks));
		GL(glGetProgramiv(m_program, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &uniformBlockMaxNameLength));
		GL(glGetProgramiv(m_program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniformMaxNameLength));

		std::vector<GLchar> blockName(uniformBlockMaxNameLength);
		std::vector<GLchar> uniformName(uniformMaxNameLength);

		for (GLint i = 0; i < uniformBlocks; ++i)
		{
			GLsizei length;
			//s32 blockBinding;
			//u32 blockIndex;

			glGetActiveUniformBlockName(m_program, i, uniformBlockMaxNameLength, &length, blockName.data());
			//blockIndex = glGetUniformBlockIndex(m_program, blockName.data());
			//glGetActiveUniformBlockiv(m_program, blockIndex, GL_UNIFORM_BLOCK_BINDING, &blockBinding);
			//std::cout << blockName.data() << std::endl;
			m_uniformBlockList.emplace_back(blockName.data());
		}
	}

	u32 Shader::getUniformLocation(const std::string& s)
	{
		return m_uniformMap[s];
	}

	bool Shader::isValidUBO(const std::string& s)
	{
		return std::find(std::begin(m_uniformBlockList),
			std::end(m_uniformBlockList),
			s)
			!= std::end(m_uniformBlockList);
	}
}