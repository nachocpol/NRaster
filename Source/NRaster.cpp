#include "NRaster.h"
#include "NModel.h"

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
	for (uint32_t i = 0; i < numVertices; i += 3)
	{
		Vertex triangle[3] = { data[i + 0], data[i + 2], data[i + 1] };

		// Vertex shader:
		triangle[0].Position = m_renderState.VertexShader(triangle[0]);
		triangle[1].Position = m_renderState.VertexShader(triangle[1]);
		triangle[2].Position = m_renderState.VertexShader(triangle[2]);

		// Normalize:
		triangle[0].Position /= triangle[0].Position.w;
		triangle[1].Position /= triangle[1].Position.w;
		triangle[2].Position /= triangle[2].Position.w;

		// Raster triangle:
		NRaster::RasterTriangle(m_renderState, triangle);
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

	// [CCW]
	glm::vec3 v0 = vtx[0].Position;
	glm::vec3 v1 = vtx[1].Position;
	glm::vec3 v2 = vtx[2].Position;

	// Convert into screen position (note that we flip the y)
	glm::vec3 rasterv0((v0.x * 0.5f + 0.5f) * width, (1.0f - (v0.y * 0.5f + 0.5f)) * height, v0.z);
	glm::vec3 rasterv1((v1.x * 0.5f + 0.5f) * width, (1.0f - (v1.y * 0.5f + 0.5f)) * height, v1.z);
	glm::vec3 rasterv2((v2.x * 0.5f + 0.5f) * width, (1.0f - (v2.y * 0.5f + 0.5f)) * height, v2.z);

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
