#ifndef JOINT_HPP
#define JOINT_HPP

#include <vector>
#include <glm/glm.hpp>

struct Joint
{
	int ID;
	std::string Name;
	std::string Channel;
	glm::mat4 InverseTransform;
	glm::mat4 localBindTransform;
	std::vector<Joint*> Children;

	~Joint()
	{
		for (Joint* ptr : Children)
		{
			delete ptr;
			ptr = nullptr;
		}
	}

	inline void CalculateInverseBindTransform(const glm::mat4& parentBindTransform)
	{
		glm::mat4 bindTransform = parentBindTransform * localBindTransform;
		InverseTransform = glm::inverse(bindTransform);

		std::for_each(Children.begin(), Children.end(),[&](Joint* c) {
			c->CalculateInverseBindTransform(bindTransform); 
		});
	}
};

#endif //JOINT_HPP