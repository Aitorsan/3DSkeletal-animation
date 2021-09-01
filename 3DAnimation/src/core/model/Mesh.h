#pragma once
#include <vector>
#include "core/renderer/Texture2D.h"
#include "Vertex.h"

struct Mesh 
{
	unsigned int m_VAO;
	std::vector<Vertex> m_vertices;
	std::vector<Texture2D> m_textures;
	std::vector<unsigned int> m_indices;

	Mesh() = default;
	Mesh(const std::vector<Vertex>& vertices,
	std::vector<Texture2D> textures,
	std::vector<unsigned int> indices)
		:m_vertices(vertices)
		, m_textures(textures)
		, m_indices(indices)
	{
	}

};

