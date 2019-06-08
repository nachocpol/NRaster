#pragma once

/*
  NRaster.h
	Takes care off triangle rasterization, binning etc.
*/

#include "glm.hpp"
#include "NModel.h"
#include <vector>

namespace tthread
{
	class thread;
};


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

struct BinnedTriangle
{
	Vertex Verts[3];
	float MinDepth;
};

class NRaster
{
private:
	NRaster();
	NRaster(const NRaster& other);
	~NRaster();

public:
	static NRaster* Instance();
	bool Initialize();

	void SetViewport(int x, int y, int w, int h);
	void SetRenderTarget(PixelRGBA32* data);
	void SetDepthBuffer(float* data);
	void SetShaders(VertexShaderFn vertexShader, PixelShaderFn pixelShader);
	void Draw(Vertex* data, uint32_t numVertices);

private:

	static float EdgeTest(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);
	static void RasterTriangle(const RenderState& renderState, Vertex* vtx);
	static bool PointInsideRect(const glm::vec2& p, const glm::vec4& rect);

	struct RasterContextMT
	{
		RasterContextMT(const RenderState& _state,const glm::ivec4& _rect, glm::vec3 _debugCol) :
			  MTState(_state)
			, Rect(_rect)
			, DebugColour(_debugCol)
		{};

		const RenderState& MTState;
		const glm::ivec4& Rect;
		glm::vec3 DebugColour;
	};
	static void RasterTraingleMT(void* renderContext);

	int m_numBinsWidth;
	int m_numBinsHeight;

	int m_binWidth;
	int m_binHeight;

	std::vector <tthread::thread*> m_rasterThreads;
	RenderState m_renderState;
	static NRaster* m_instance;
};