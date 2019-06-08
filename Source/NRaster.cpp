#include "NRaster.h"
#include "tinythread.h"
#include <iostream>

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
	m_numBinsWidth = numCores * 2;

	return false;
}

void NRaster::SetViewport(int x, int y, int w, int h)
{
	m_renderState.ScreenRect = glm::vec4(x, y, w, h);
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
	int width = m_renderState.ScreenRect.z;
	int height = m_renderState.ScreenRect.w;

	for (uint32_t i = 0; i < numVertices; i += 3)
	{
		BinnedTriangle triangle;
		triangle.Verts[0] = data[i + 0];
		triangle.Verts[1] = data[i + 2];
		triangle.Verts[2] = data[i + 1];

		// Vertex shader:
		triangle.Verts[0].Position = m_renderState.VertexShader(triangle.Verts[0]);
		triangle.Verts[1].Position = m_renderState.VertexShader(triangle.Verts[1]);
		triangle.Verts[2].Position = m_renderState.VertexShader(triangle.Verts[2]);

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
		int binWidth = width / m_numBinsWidth;
		int binHeight = height / m_numBinsHeight;
		glm::vec2 p0(triangle.Verts[0].Position.x, triangle.Verts[0].Position.y);
		glm::vec2 p1(triangle.Verts[1].Position.x, triangle.Verts[1].Position.y);
		glm::vec2 p2(triangle.Verts[2].Position.x, triangle.Verts[2].Position.y);
		for (int by = 0; by < m_numBinsHeight; ++by)
		{
			for (int bx = 0; bx < m_numBinsWidth; ++bx)
			{
				glm::vec4 brect = glm::vec4(bx * binWidth, by * binHeight, binWidth, binHeight);
				int cnt = PointInsideRect(p0, brect) ? 1 : 0;
				cnt += PointInsideRect(p1, brect) ? 1 : 0;
				cnt += PointInsideRect(p2, brect) ? 1 : 0;
				if (cnt == 3)
				{

				}
				else if (cnt > 0)
				{

				}
			}
		}


		// Raster triangle:
		NRaster::RasterTriangle(m_renderState, triangle.Verts);
	}
}

inline float NRaster::EdgeTest(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
	return ((c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x));
}

void NRaster::RasterTriangle(const RenderState& renderState, Vertex* vtx)
{
	int width = renderState.ScreenRect.z;
	int height = renderState.ScreenRect.w;
	PixelRGBA32* pixels = renderState.RenderTarget;
	float* depthBuffer = renderState.DepthBuffer;

	// [CCW] already in raster space
	glm::vec3 rasterv0 = vtx[0].Position;
	glm::vec3 rasterv1 = vtx[1].Position;
	glm::vec3 rasterv2 = vtx[2].Position;

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
	int minX = (int)glm::min(glm::min(rasterv0.x, rasterv1.x), rasterv2.x);
	int minY = (int)glm::min(glm::min(rasterv0.y, rasterv1.y), rasterv2.y);
	int maxX = (int)glm::max(glm::max(rasterv0.x, rasterv1.x), rasterv2.x);
	int maxY = (int)glm::max(glm::max(rasterv0.y, rasterv1.y), rasterv2.y);

	// Pre calc 1 over area of the tri:
	float areaRcp = 1.0f / EdgeTest(rasterv0, rasterv1, rasterv2);

	for (int sy = minY; sy <= maxY; ++sy)
	{
		for (int sx = minX; sx <= maxX; ++sx)
		{
			glm::vec3 rasterPixel((float)sx + 0.5f, (float)sy + 0.5f, 0.0f); // +0.5->center pixel

			// Areas of the parallelograms [rastervx, rastervy, rasterPixel]
			float w0 = EdgeTest(rasterv1, rasterv2, rasterPixel);
			float w1 = EdgeTest(rasterv2, rasterv0, rasterPixel);
			float w2 = EdgeTest(rasterv0, rasterv1, rasterPixel);

			if ((w0 >= 0.0f) && (w1 >= 0.0f) && (w2 >= 0.0f))
			{
				// Barycentric coordinates. Ratio between the area of the triangle 
				// and ratio of the area of each vx,vy,pixel. Note that we do not divide by 2, as it cancels out.
				w0 *= areaRcp;
				w1 *= areaRcp;
				w2 *= areaRcp;

				// Depth test [LESS_THAN]:
				float pixelDepth = 1.0f / (rasterv0.z * w0 + rasterv1.z * w1 + rasterv2.z * w2);
				if (pixelDepth < depthBuffer[sy * width + sx])
				{
					// Update depth buffer:
					depthBuffer[sy * width + sx] = pixelDepth;

					// Perspective correct attributes:
					glm::vec2 TexCoord = (rasterTexCoord0 * w0 + rasterTexCoord1 * w1 + rasterTexCoord2 * w2) * pixelDepth;
					glm::vec3 Normal = (rasterNormal0 * w0 + rasterNormal1 * w1 + rasterNormal2 * w2) * pixelDepth;

					glm::vec3 ToLight = -glm::vec3(1.0, 0.5, 0.0f);
					float NdotL = glm::dot(glm::normalize(ToLight), glm::normalize(Normal));

					// Pixel shader:
					Vertex interpolatedData;
					interpolatedData.Normal = Normal;
					interpolatedData.TexCoord = TexCoord;
					glm::vec4 pixel = renderState.PixelShader(interpolatedData);

					// Pixel color:
					PixelRGBA32* cur = &pixels[sy * width + sx];

					PixelRGBA32 newPixel;
					newPixel.R = uint8_t(pixel.x * 255.0f);
					newPixel.G = uint8_t(pixel.y * 255.0f);
					newPixel.B = uint8_t(pixel.z * 255.0f);
					newPixel.A = 0xff;

					*cur = newPixel;
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

void NRaster::RasterTraingleMT(void* renderContext)
{
	RasterContextMT* context = (RasterContextMT*)renderContext;
	if (!context)
	{
		return;
	}

	PixelRGBA32* output = context->MTState.RenderTarget;

	PixelRGBA32 col;
	col.R = uint8_t(255.0f * context->DebugColour.x);
	col.G = uint8_t(255.0f * context->DebugColour.y);
	col.B = uint8_t(255.0f * context->DebugColour.z);
	col.A = 0xff;

	int x = context->Rect.x;
	int y = context->Rect.y;
	int w = context->Rect.z;
	int h = context->Rect.w;

	int rtWidth = context->MTState.ScreenRect.z;
	int rtHeight = context->MTState.ScreenRect.w;

	for (int sy = y; sy <= h; ++sy)
	{
		for (int sx = x; sx <= h; ++sx)
		{
			output[sy * rtWidth + sx] = col;
		}
	}
}
