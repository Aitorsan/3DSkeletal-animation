#pragma once
#include "core/scene/Component.h"
class ShaderProgram;

class Renderer: public Component
{
protected:
	ShaderProgram* Shader;
	unsigned int VertexArrayObject;
	unsigned int VertexBufferObject;
	unsigned int ElementBufferObject;
public:
	Renderer(GameObject* parent, ShaderProgram* shader):Component(parent),Shader(shader) {};
	virtual ~Renderer() {}
	virtual void Render() = 0;
};