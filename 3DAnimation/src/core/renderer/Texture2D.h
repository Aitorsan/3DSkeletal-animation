#pragma once
#include <string>
class Texture2D
{
public:
	bool Load(const std::string& fileName);
	void Bind();
	unsigned int m_textureID;
	unsigned int m_textureUnit;
};

