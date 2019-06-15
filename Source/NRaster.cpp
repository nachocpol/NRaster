#include "NRaster.h"
#include "NProfiler.h"
#include "tinythread.h"
#include "SDL.h" // for debug rendering
#include <iostream>

// #define MULTICORE

NRaster::NRaster()
{
}

NRaster::NRaster(const NRaster& other)
{
	assert(false);
}

NRaster::~NRaster()
{
}

NRaster* NRaster::Instance()
{
	static NRaster* kInstance = nullptr;
	if (!kInstance)
	{
		kInstance = new NRaster();
	}
	return kInstance;
}

bool NRaster::Initialize()
{
	uint32_t numCores = tthread::thread::hardware_concurrency();
	std::cout << "[NRaster][Initialize][Info]: The number of detected CPU cores is: " << numCores << std::endl;

	m_numBinsHeight = numCores;
	m_numBinsWidth = numCores;

	m_bins = new std::vector<BinnedTriangle>[m_numBinsWidth * m_numBinsHeight];

	return false;
}

void NRaster::SetViewport(int x, int y, int w, int h)
{
	m_renderState.ScreenRect = glm::vec4(x, y, w, h);

	m_binWidth = w / m_numBinsWidth;
	m_binHeight = h / m_numBinsHeight;
}

void NRaster::SetRenderTarget(PixelRGBA32* data)
{
	m_renderState.RenderTarget = data;
}

void NRaster::SetDepthBuffer(float* data)
{
	m_renderState.DepthBuffer = data;
}

void NRaster::SetShaders(VertexShaderFn vertexShader, PixelShaderFn pixelShader)
{
	m_renderState.VertexShader = vertexShader;
	m_renderState.PixelShader = pixelShader;
}

void NRaster::Draw(Vertex* data, uint32_t numVertices)
{
	// Before starting a new drawcall, clear the bins. This #ISN´T thread safe
	for (int by = 0; by < m_numBinsHeight; ++by)
	{
		for (int bx = 0; bx < m_numBinsWidth; ++bx)
		{
			m_bins[by * m_numBinsWidth + bx].clear();
		}
	}

	int width = m_renderState.ScreenRect.z;
	int height = m_renderState.ScreenRect.w;

	VertexRenderData vtxRenderData;
	vtxRenderData.Projection = m_curProjection;
	vtxRenderData.View = m_curView;
	vtxRenderData.Transform = m_curTransform;

	uint64_t acumCycles = 0;
	for (uint32_t i = 0; i < numVertices; i += 3)
	{
		BinnedTriangle triangle;
		triangle.Verts[0] = data[i + 0];
		triangle.Verts[1] = data[i + 2];
		triangle.Verts[2] = data[i + 1];

		// Vertex shader:
		triangle.Verts[0].Position = m_renderState.VertexShader(triangle.Verts[0], vtxRenderData);
		triangle.Verts[1].Position = m_renderState.VertexShader(triangle.Verts[1], vtxRenderData);
		triangle.Verts[2].Position = m_renderState.VertexShader(triangle.Verts[2], vtxRenderData);

		// Normalize:
		triangle.Verts[0].Position /= triangle.Verts[0].Position.w;
		triangle.Verts[1].Position /= triangle.Verts[1].Position.w;
		triangle.Verts[2].Position /= triangle.Verts[2].Position.w;

		// Convert to screen position
		triangle.Verts[0].Position = glm::vec4((triangle.Verts[0].Position.x * 0.5f + 0.5f) * width, (1.0f - (triangle.Verts[0].Position.y * 0.5f + 0.5f)) * height, triangle.Verts[0].Position.z, 1.0f);
		triangle.Verts[1].Position = glm::vec4((triangle.Verts[1].Position.x * 0.5f + 0.5f) * width, (1.0f - (triangle.Verts[1].Position.y * 0.5f + 0.5f)) * height, triangle.Verts[1].Position.z, 1.0f);
		triangle.Verts[2].Position = glm::vec4((triangle.Verts[2].Position.x * 0.5f + 0.5f) * width, (1.0f - (triangle.Verts[2].Position.y * 0.5f + 0.5f)) * height, triangle.Verts[2].Position.z, 1.0f);

		// Min depth
		triangle.MinDepth = glm::min(glm::min(triangle.Verts[0].Position.z, triangle.Verts[1].Position.z), triangle.Verts[2].Position.z);

		// Add to bin:
#if defined(MULTICORE)
		int binWidth = width / m_numBinsWidth;
		int binHeight = height / m_numBinsHeight;
		glm::vec3 p0(triangle.Verts[0].Position.x, triangle.Verts[0].Position.y,0.0f);
		glm::vec3 p1(triangle.Verts[1].Position.x, triangle.Verts[1].Position.y,0.0f);
		glm::vec3 p2(triangle.Verts[2].Position.x, triangle.Verts[2].Position.y,0.0f);
		glm::vec4 triBounds = GetBounds(p0, p1, p2);
		for (int by = 0; by < m_numBinsHeight; ++by)
		{
			for (int bx = 0; bx < m_numBinsWidth; ++bx)
			{
				float x = bx * binWidth;
				float y = by * binHeight;
				glm::vec4 bquad = glm::vec4(x, y, x + binWidth, y + binHeight);
				if (RectInsideRect(bquad, triBounds))
				{
					m_bins[by * m_numBinsWidth + bx].push_back(triangle);
				}

			}
		}
#else
		auto tstart = NProfilerGet()->Now();

		// Raster triangle:
		m_renderState.RtSize = m_renderState.ScreenRect; // the size of the rt should be inside the texture
		NRaster::RasterTriangle(m_renderState, triangle.Verts);

		auto tend = NProfilerGet()->Now();
		acumCycles += NProfilerGet()->TimeMSToCycles(NProfilerGet()->TimeDiffMS(tstart, tend));
#endif
	}
	
	//uint64_t avgCycles = acumCycles / numVertices;
	//std::cout << "[NRaster::Draw] " << avgCycles << std::endl;

	// Schedule jobs
#if defined(MULTICORE)
	std::vector<tthread::thread*> debugThreads;
	std::vector<RasterContextMT*> threadContexts;
	for (int by = 0; by < m_numBinsHeight; ++by)
	{
		for (int bx = 0; bx < m_numBinsWidth; ++bx)
		{
			if (m_bins[by * m_numBinsWidth + bx].empty())
			{
				continue;
			}
			// if ((by == 2) && (bx == 3))
			{
				glm::vec4 threadZone(bx * m_binWidth, by * m_binHeight, m_binWidth, m_binHeight);
				threadContexts.push_back(new RasterContextMT(m_renderState, m_bins[by * m_numBinsWidth + bx], threadZone, glm::vec4(0, 0, 1, 1)));
				debugThreads.emplace_back(new tthread::thread(NRaster::RasterTraingleMT, (void*)threadContexts[threadContexts.size() - 1]));
			}
		}
	}
	// Wait for all to be done and release memory
	for (uint32_t tidx = 0; tidx != debugThreads.size(); ++tidx)
	{
		debugThreads[tidx]->join();
		delete debugThreads[tidx];
		delete threadContexts[tidx];
	}
	debugThreads.clear();
	threadContexts.clear();
#endif
}

void NRaster::SetTransforms(glm::mat4 transform, glm::mat4 view, glm::mat4 projection)
{
	m_curTransform = transform;
	m_curView = view;
	m_curProjection = projection;
}

void NRaster::DebugDraw(SDL_Renderer* renderer)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xff);
	for (int bx = 0; bx < m_numBinsWidth; ++bx)
	{
		int x = (bx * m_binWidth);
		SDL_RenderDrawLine(renderer,x,0,x,m_renderState.ScreenRect.w);
	}
	for (int by = 0; by < m_numBinsHeight; ++by)
	{
		int y = (by * m_binHeight);
		SDL_RenderDrawLine(renderer, 0, y, m_renderState.ScreenRect.z, y);
	}
}

inline float NRaster::EdgeTest(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
	return ((c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x));
}

void NRaster::RasterTriangle(const RenderState& renderState, Vertex* vtx)
{
	int width = renderState.RtSize.z;
	int height = renderState.RtSize.w;
	PixelRGBA32* pixels = renderState.RenderTarget;
	float* depthBuffer = renderState.DepthBuffer;

	// [CCW] already in raster space
	glm::vec3 rasterv0 = vtx[0].Position;
	glm::vec3 rasterv1 = vtx[1].Position;
	glm::vec3 rasterv2 = vtx[2].Position;

	// Pre calc 1 over area of the tri:
	float areaRcp = 1.0f / EdgeTest(rasterv0, rasterv1, rasterv2);
	// Skip back facing triangles:
	if (areaRcp <= 0.0f)
	{
		return;
	}

	// We use 1 / V.z to calculate the current pixel depth
	//	1 / P.z =  (1 / V0.z) * D0 + (1 / V1.z) * D1 + (1 / V2.z) * D2
	rasterv0.z = 1.0f / rasterv0.z;
	rasterv1.z = 1.0f / rasterv1.z;
	rasterv2.z = 1.0f / rasterv2.z;

	// Attribute correct interpolation:
	//	1) att0 /= raster0.z
	//  2) Find cur attribute using bary coords
	//  3) Finally, mult by z

	// TexCoords
	glm::vec2 rasterTexCoord0 = vtx[0].TexCoord * rasterv0.z;
	glm::vec2 rasterTexCoord1 = vtx[1].TexCoord * rasterv1.z;
	glm::vec2 rasterTexCoord2 = vtx[2].TexCoord * rasterv2.z;
	// Normals
	glm::vec3 rasterNormal0 = vtx[0].Normal * rasterv0.z;
	glm::vec3 rasterNormal1 = vtx[1].Normal * rasterv1.z;
	glm::vec3 rasterNormal2 = vtx[2].Normal * rasterv2.z;

	// Triangle bounding quad:
	glm::vec4 bounds = GetBounds(rasterv0, rasterv1, rasterv2);

	for (int sy = bounds.y; sy <= bounds.w; ++sy)
	{
		
		uint32_t rowOffset = sy * width;
		PixelRGBA32* curPixelRow = &pixels[rowOffset];
		float* curDepthRow = &depthBuffer[rowOffset];

		for (int sx = bounds.x; sx <= bounds.z; ++sx)
		{
			// Clip if its outside the bounds
			if (!PointInsideRect(glm::vec2(sx, sy), renderState.ScreenRect))
			{
				continue;
			}
			glm::ivec3 rasterPixel(sx,sy,0); // +0.5->center pixel

			// Areas of the parallelograms [rastervx, rastervy, rasterPixel]
			float w0 = EdgeTest(rasterv1, rasterv2, rasterPixel);
			float w1 = EdgeTest(rasterv2, rasterv0, rasterPixel);
			float w2 = EdgeTest(rasterv0, rasterv1, rasterPixel);

			if ((w0 > 0.0f) && (w1 > 0.0f) && (w2 > 0.0f))
			{
				// Barycentric coordinates. Ratio between the area of the triangle 
				// and ratio of the area of each vx,vy,pixel. Note that we do not divide by 2, as it cancels out.
				w0 *= areaRcp;
				w1 *= areaRcp;
				w2 *= areaRcp;

				// Depth test [LESS_THAN]:
				float pixelDepth = 1.0f / (rasterv0.z * w0 + rasterv1.z * w1 + rasterv2.z * w2);
				if (pixelDepth < curDepthRow[sx])
				{
					// Update depth buffer:
					curDepthRow[sx] = pixelDepth;

					// Perspective correct attributes:
					Vertex interpolatedData;
					interpolatedData.Normal = (rasterNormal0 * w0 + rasterNormal1 * w1 + rasterNormal2 * w2) * pixelDepth;
					interpolatedData.TexCoord = (rasterTexCoord0 * w0 + rasterTexCoord1 * w1 + rasterTexCoord2 * w2) * pixelDepth;

					// Pixel shader:
					glm::vec4 pixel = renderState.PixelShader(interpolatedData);

					// Pixel color:
					curPixelRow[sx].R = (uint8_t)(pixel.r * 255.0f);
					curPixelRow[sx].G = (uint8_t)(pixel.g * 255.0f);
					curPixelRow[sx].B = (uint8_t)(pixel.b * 255.0f);
					curPixelRow[sx].A = (uint8_t)(pixel.a * 255.0f);
				}
			}
		}
	}
}

bool NRaster::PointInsideRect(const glm::vec2& p, const glm::vec4& rect)
{
	if (p.x >= rect.x &&
		p.x <= (rect.x + rect.z) &&
		p.y >= rect.y &&
		p.y <= (rect.y + rect.w))
	{
		return true;
	}
	return false;
}

bool NRaster::RectInsideRect(const glm::vec4& a, const glm::vec4& b)
{
	glm::vec4 ra = glm::vec4(a.x, a.y, a.z - a.x, a.w - a.y);
	glm::vec4 rb = glm::vec4(b.x, b.y, b.z - b.x, b.w - b.y);
	//	x  y    z       w
	// [x, y, width, height]
	if (ra.x + ra.z >= rb.x &&
		ra.x <= rb.x + rb.z &&
		ra.y + ra.w >= rb.y && 
		ra.y <= rb.y + rb.w)
	{
		return true;
	}
	return false;
}

glm::vec4 NRaster::GetBounds(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
	glm::vec4 bounds;
	bounds.x = (int)glm::min(glm::min(a.x, b.x), c.x);
	bounds.y = (int)glm::min(glm::min(a.y, b.y), c.y);
	bounds.z = (int)glm::max(glm::max(a.x, b.x), c.x);
	bounds.w = (int)glm::max(glm::max(a.y, b.y), c.y);
	return bounds;
}

void NRaster::RasterTraingleMT(void* renderContext)
{
	RasterContextMT* context = (RasterContextMT*)renderContext;
	if (!context)
	{
		return;
	}
	context->MTState.RtSize = context->MTState.ScreenRect;
	context->MTState.ScreenRect = context->Rect;

	// Sort triangles... sadly makes it slower :(
	//std::sort(context->MTTriangles.begin(), context->MTTriangles.end(), [](BinnedTriangle& a, BinnedTriangle& b) {
	//	return a.MinDepth > b.MinDepth;
	//});

	for (uint32_t i = 0; i != context->MTTriangles.size(); ++i)
	{
		NRaster::RasterTriangle(context->MTState, (Vertex*)context->MTTriangles[i].Verts);
	}
}
