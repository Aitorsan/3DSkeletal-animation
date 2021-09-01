#pragma once

#include "glm/glm.hpp"

class ShaderProgram
{
	int vertexShaderID;
	int fragmentShaderID;
	int geometryShaderID;
	int programID;
public:
	ShaderProgram() = default;
	ShaderProgram(const char* vertexShaderSourcePath, const char* fragmentShaderSourcePath);
	ShaderProgram(const char* vertexShaderSourcePath, const char* fragmentShaderSourcePath, const char* geometryShaderSource);

	virtual ~ShaderProgram();
	int createShaderProgram();
	int createShaderProgramWithGeometry();
	unsigned int loadShader(const char* ShaderSourcePath,unsigned int type);
	void useProgram() const;
	void stopProgram()const;
	// utility uniform functions
	int getUniformLocation(const char* name)const;
	void setBool(const char* name, bool value) const;
	void setInt(const char* name, int value) const;
	void setFloat(const char* name, float value) const;
	void setVector3f(const char* name, const glm::vec3&  vector)const;
	void setVector4f(const char* name, const glm::vec4&  vector)const;

	void setMatrix(const char* name, const glm::mat4& matrix) const;
};

