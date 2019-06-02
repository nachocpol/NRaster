#include "NModel.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <iostream>

NModel::NModel():
	m_vertices(nullptr)
	,m_numVertices(0)
{
}

NModel::~NModel()
{
	if (m_vertices)
	{
		delete m_vertices;
	}
}

bool NModel::LoadFromfile(const char* path)
{
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string error;

	bool success = tinyobj::LoadObj(shapes, materials, error, path);

	if (!error.empty())
	{
		if (success)
		{
			std::cout << "[NModel][LoadFromFile][Warning]: \n";
		}
		else
		{
			std::cout << "[NModel][LoadFromFile][Error]: \n";
		}
		std::cout << error << std::endl;
	}

	if (!success)
	{
		return false;
	}

	// Query loaded vtx:
	for (uint32_t s = 0; s < shapes.size(); ++s)
	{
		m_numVertices += shapes[s].mesh.indices.size();
	}
	m_vertices = new Vertex[m_numVertices];

	// Iterate over shapes -> faces:
	uint32_t curVtx = 0;
	for (uint32_t s = 0; s < shapes.size(); ++s)
	{
		size_t index_offset = 0;
		for (uint32_t f = 0; f < shapes[s].mesh.indices.size(); ++f)
		{
			int fv = shapes[s].mesh.num_vertices[f/3];
			assert(fv == 3);

			unsigned int idx = shapes[s].mesh.indices[f];

			float vx = shapes[s].mesh.positions[3 * idx + 0];
			float vy = shapes[s].mesh.positions[3 * idx + 1];
			float vz = shapes[s].mesh.positions[3 * idx + 2];

			float nx = shapes[s].mesh.normals[3 * idx + 0];
			float ny = shapes[s].mesh.normals[3 * idx + 1];
			float nz = shapes[s].mesh.normals[3 * idx + 2];

			m_vertices[curVtx] = Vertex(glm::vec3(vx, vy, vz),glm::vec3(nx,ny,nz));
			++curVtx;
			index_offset += fv;
		}
	}

	std::cout << "Loaded model. \n";

	return true;
}

Vertex* NModel::GetAllVertex() const
{
	return m_vertices;
}

Vertex* NModel::GetVertexAt(uint32_t idx) const
{
	return &m_vertices[idx];
}

uint32_t NModel::GetNumVertices() const
{
	return m_numVertices;
}
