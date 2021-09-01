#include "MeshRenderer.h"
#include <GL/glew.h>
#include "core/model/Mesh.h"
#include "core/scene/GameObject.h"
#include "core/renderer/ShaderProgram.h"

MeshRenderer::MeshRenderer(GameObject* parent, ShaderProgram * shader, Mesh * mesh)
	: Renderer(parent,shader)
	, MeshData(mesh)
{
}

void MeshRenderer::Render()
{
	Shader->useProgram();
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VertexArrayObject);
	glDrawElements(GL_TRIANGLES, MeshData->m_indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	Shader->stopProgram();
}


void MeshRenderer::SetUp()
{
	// create the VAO
	glGenVertexArrays(1, &VertexArrayObject);
	glBindVertexArray(VertexArrayObject);
	
	glGenBuffers(1, &VertexBufferObject);
	glGenBuffers(1, &ElementBufferObject);

	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)* MeshData->m_vertices.size(), &MeshData->m_vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, MeshData->m_indices.size() * sizeof(unsigned int), &MeshData->m_indices, GL_STATIC_DRAW);

}

void MeshRenderer::Update()
{
}

void MeshRenderer::Input()
{
}

