#pragma once
#include <glm/glm.hpp>
#include "../core/renderer/ShaderProgram.h"

class PositionalLight
{
	glm::vec3 Position;
	glm::vec3 Color;
	float Attenuation;
public:
	PositionalLight(const glm::vec3& position) 
		:Position(position)
		, Color(1.0f)
		, Attenuation (1.0f)
	{

	}

	void SetUniforms(ShaderProgram& program)
	{
		program.useProgram();
		program.setVector3f("light_color", Color);
		program.setVector3f("light_position",Position);
		program.setFloat("light_attenuation",Attenuation);
	}

};