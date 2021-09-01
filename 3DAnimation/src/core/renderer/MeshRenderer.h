#pragma once
#include "Renderer.h"
class Mesh;

class MeshRenderer final: public Renderer
{
	Mesh* MeshData;
public:
	MeshRenderer(GameObject* parent, ShaderProgram* shader, Mesh* mesh);
	virtual ~MeshRenderer(){}
	void Render() override;
	void SetUp()  override;
	void Update() override;
	void Input()  override;
};

