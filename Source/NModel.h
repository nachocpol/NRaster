#pragma once

/*
  NModel.h
	Defines classes to load 3d models and keep data around in a format NRaster 
	knows how to use.
*/

#include "glm.hpp"

struct Vertex
{
	Vertex()
	{
	}
	Vertex(const Vertex& other)
	{
		Position = other.Position;
		Normal = other.Normal;
		Color = other.Color;
		TexCoord = other.TexCoord;
	}
	Vertex(const glm::vec3& _position) :
		Position(_position.x, _position.y, _position.z, 1.0f)
	{
	}
	Vertex(const glm::vec3& _position, const glm::vec3& _normal) :
		Position(_position.x, _position.y, _position.z, 1.0f)
		, Normal(_normal.x, _normal.y, _normal.z)
	{
	}
	Vertex(const glm::vec3& _position, const glm::vec3& _color, const glm::vec2& _texcoord) :
		Position(_position.x, _position.y, _position.z, 1.0f)
		, Color(_color)
		, TexCoord(_texcoord)
	{
	}
	glm::vec4 Position;
	glm::vec3 Normal;
	glm::vec3 Color;
	glm::vec2 TexCoord;
};

class NModel
{
public:
	NModel();
	~NModel();

	bool LoadFromfile(const char* path);
	Vertex* GetAllVertex()const;
	Vertex* GetVertexAt(uint32_t idx)const;
	uint32_t GetNumVertices()const;

private:
	Vertex* m_vertices;
	uint32_t m_numVertices;
};