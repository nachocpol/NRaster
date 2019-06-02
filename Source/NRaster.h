#pragma once

/*
  NRaster.h
	Takes care off triangle rasterization, binning etc.
*/

#include "glm.hpp"

struct Vertex;

typedef glm::vec4(*VertexShaderFn)(const Vertex& vertex);
typedef glm::vec4(*PixelShaderFn)(const Vertex& vertex);

struct PixelRGBA32
{
	uint8_t A;
	uint8_t B;
	uint8_t G;
	uint8_t R;
};

struct DepthTest
{
	enum T
	{
		LessThan,
		Count
	};
};

struct WindingOrder
{
	enum T
	{
		CCW,
		Count
	};
};

struct PixelFormat
{
	enum T
	{
		RGBA32,
		Count
	};
};

struct RenderState
{
	PixelRGBA32* RenderTarget;
	float* DepthBuffer;
	DepthTest::T DTest;
	WindingOrder::T WOrder;
	glm::ivec4 ScreenRect;	// x,y,w,h
	VertexShaderFn VertexShader;
	PixelShaderFn PixelShader;
};

class NRaster
{
private:
	NRaster();
	NRaster(const NRaster& other);
	~NRaster();

public:
	static NRaster* Instance();
	void SetViewport(int x, int y, int w, int h);
	void SetRenderTarget(PixelRGBA32* data);
	void SetDepthBuffer(float* data);
	void SetShaders(VertexShaderFn vertexShader, PixelShaderFn pixelShader);
	void Draw(Vertex* data, uint32_t numVertices);

private:

	static float EdgeTest(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);
	static void RasterTriangle(const RenderState& renderState, Vertex* vtx);

	RenderState m_renderState;
	static NRaster* m_instance;
};