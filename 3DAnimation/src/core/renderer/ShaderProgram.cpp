#include "ShaderProgram.h"

#include <GL/glew.h>
#include <tuple>
#include <iostream>
#include <fstream>
#include <string>
#include "glm/glm.hpp"

namespace
{
	void DebugProgramLinkError(unsigned int shaderProgramId)
	{
		int Result = GL_FALSE;
		int InfoLogLength;
		glGetProgramiv(shaderProgramId, GL_LINK_STATUS, &Result);
		glGetProgramiv(shaderProgramId, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (!Result)
		{
			char* ProgramErrorMessage = (char*)malloc((InfoLogLength) * sizeof(char));
			glGetProgramInfoLog(shaderProgramId, InfoLogLength, NULL, &(*ProgramErrorMessage));
			std::cerr << "[Shader linking Error]: " << ProgramErrorMessage << std::endl;
			free(ProgramErrorMessage);
		}
	}

	void DebugShaderCompileStatus(unsigned int shaderId, const char* shaderType)
	{
		int succed = GL_FALSE;
		int InfoLogLength;
		glGetShaderiv(shaderId, GL_COMPILE_STATUS, &succed);
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (!succed)
		{
			char* ShaderErrorMessage = (char*)malloc((InfoLogLength) * sizeof(char));
			glGetShaderInfoLog(shaderId, InfoLogLength, NULL, ShaderErrorMessage);
			std::cerr << "[ Compile " << shaderType << " Error]: " << ShaderErrorMessage << std::endl;
			free(ShaderErrorMessage);
		}
	}

}

ShaderProgram::ShaderProgram(const char* vertexShaderSourcePath, const char* fragmentShaderSourcePath)
	: vertexShaderID{}
	, fragmentShaderID{}
	, programID{}
{
	vertexShaderID = loadShader(vertexShaderSourcePath, GL_VERTEX_SHADER);
	fragmentShaderID = loadShader(fragmentShaderSourcePath, GL_FRAGMENT_SHADER);
	programID = createShaderProgram();
}
ShaderProgram::ShaderProgram(const char* vertexShaderSourcePath, const char* fragmentShaderSourcePath, const char* geometryShaderSource)
	: vertexShaderID{}
	, fragmentShaderID{}
	, geometryShaderID{}
	, programID{}
{
	vertexShaderID = loadShader(vertexShaderSourcePath, GL_VERTEX_SHADER);
	fragmentShaderID = loadShader(fragmentShaderSourcePath, GL_FRAGMENT_SHADER);
	geometryShaderID = loadShader(geometryShaderSource, GL_GEOMETRY_SHADER);
	programID = createShaderProgramWithGeometry();
}

ShaderProgram::~ShaderProgram()
{
	stopProgram();
	glDeleteProgram(programID);
}

void ShaderProgram::useProgram()const
{
	glUseProgram(programID);
}

void ShaderProgram::stopProgram() const
{
	glUseProgram(0);
}

int ShaderProgram::getUniformLocation(const char * name) const
{
	return glGetUniformLocation(programID, name);
}

void ShaderProgram::setBool(const char* name, bool value) const
{
	setInt(name, (int)value);
}

void ShaderProgram::setInt(const char* name, int value) const
{
	const GLint& location = glGetUniformLocation(programID, name);

	glUniform1i(location, value);
}

void ShaderProgram::setFloat(const char* name, float value) const
{
	const GLint& location = glGetUniformLocation(programID, name);
	glUniform1f(location, value);
}

void ShaderProgram::setVector3f(const char* name, const glm::vec3& vector) const
{
	const GLint& location = getUniformLocation(name);
	glUniform3f(location, vector.x, vector.y, vector.z);
}

void ShaderProgram::setVector4f(const char* name, const glm::vec4&  vector)const
{
	const GLint& location = getUniformLocation(name);
	glUniform4f(location, vector.x, vector.y, vector.z,vector.w);
}

void ShaderProgram::setMatrix(const char* name, const glm::mat4 & matrix) const
{
	const GLint& location = getUniformLocation(name);
	glUniformMatrix4fv(location, 1, false, &matrix[0][0]);
}


unsigned int ShaderProgram::loadShader(const char* shaderSourcePath,unsigned int type)
{
	unsigned int shaderID{ };
	std::ifstream shaderFile(shaderSourcePath);

	if (shaderFile.is_open())
	{
		std::string shadersrc{ std::istreambuf_iterator<char>(shaderFile), std::istreambuf_iterator<char>() };

		shaderFile.close();

		const char* shaderSourceCode = shadersrc.c_str();
		shaderID = glCreateShader(type);
		glShaderSource(shaderID, 1, &shaderSourceCode, nullptr);
		glCompileShader(shaderID);

		std::string shaderType{ "unknow" };
		switch (type)
		{
		case GL_VERTEX_SHADER:
			shaderType = "vertex";
			break;
		case GL_FRAGMENT_SHADER:
			shaderType = "fragment";
			break;
		case GL_GEOMETRY_SHADER:
			shaderType = "geometry";
			break;
		default:
			break;
		}
		DebugShaderCompileStatus(shaderID, shaderType.data());
	}
	return shaderID;
}
int ShaderProgram::createShaderProgramWithGeometry()
{
	int ShaderProgram = glCreateProgram();

	glAttachShader(ShaderProgram, vertexShaderID);
	glAttachShader(ShaderProgram, fragmentShaderID);
	glAttachShader(ShaderProgram, geometryShaderID);

	glLinkProgram(ShaderProgram);

	glDetachShader(ShaderProgram, vertexShaderID);
	glDetachShader(ShaderProgram, fragmentShaderID);
	glDetachShader(ShaderProgram, geometryShaderID);

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);
	glDeleteShader(geometryShaderID);

	return ShaderProgram;
}

 int ShaderProgram::createShaderProgram()
{
	int ShaderProgram = glCreateProgram();

	glAttachShader(ShaderProgram, vertexShaderID);
	glAttachShader(ShaderProgram, fragmentShaderID);

	glLinkProgram(ShaderProgram);

	glDetachShader(ShaderProgram, vertexShaderID);
	glDetachShader(ShaderProgram, fragmentShaderID);

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	return ShaderProgram;
}
