#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
struct KeyFrame
{
	float TimeStamp;
	glm::mat4 Transform{ 1.0f };
};


//Every joint has its own timestamp and transform
struct JointAnimation
{
	std::string jointName;
	std::vector<KeyFrame> Frames;
};