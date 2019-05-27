#include <iostream>
#include <SDL.h>
#include "NMath.h"

using namespace math;

struct PixelRGBA32
{ 
	uint8_t A;
	uint8_t B;
	uint8_t G;
	uint8_t R;
};

struct Vertex
{
	Vec3 V0;
	Vec3 V1;
	Vec3 V2;
};

struct GraphicsContext
{
	SDL_Window* Window;
	SDL_Renderer* Renderer;
	SDL_Texture* Framebuffer;
	SDL_Texture* DepthBufferDebug;
	float* DepthBuffer;

	int Width = 1024;
	int Height = 720;
}gContext;


bool InitSDL();
bool InitWindowAndRenderer();
void CleanUp();

void TestSDL(PixelRGBA32* pixels, int width, int height);
void TestRaster(PixelRGBA32* pixels, int width, int height);
void RasterTriangle(PixelRGBA32* pixels,Vec3* positions, int width, int height);
bool PollEvents();

int main(int, char**)
{
	InitSDL();
	InitWindowAndRenderer();

	bool exit = false;
	while (!exit)
	{
		exit = PollEvents();

		// Clear depth buffer
		{
			for (int y = 0; y < gContext.Height; ++y)
			{
				for (int x = 0; x < gContext.Width; ++x)
				{
					gContext.DepthBuffer[y * gContext.Width + x] = 1.0f;
				}
			}
		}
		
		// Rendering.
		SDL_SetRenderDrawColor(gContext.Renderer, 128, 128, 170, 255);
		SDL_RenderClear(gContext.Renderer);
		void* pData = nullptr;
		int pitch = 0;
		SDL_LockTexture(gContext.Framebuffer, NULL, &pData, &pitch);
		{
			// TestSDL((PixelRGBA32*)pData, gContext.Width, gContext.Height);
			// TestRaster((PixelRGBA32*)pData, gContext.Width, gContext.Height);

			Vec3 pts[3];
			{
				pts[0] = Vec3(0.0f, 0.5f, 0.3f);
				pts[1] = Vec3(-0.5f, -0.5f, 0.3f);
				pts[2] = Vec3(0.5f, -0.5f, 0.3f);
			}
			Vec3 pts2[3];
			{
				pts2[0] = Vec3(0.0f, 0.25f, 0.2f);
				pts2[1] = Vec3(-0.25f, -0.65f, 0.2f);
				pts2[2] = Vec3(0.25f, -0.65f, 0.9f);
			}

			RasterTriangle((PixelRGBA32*)pData, pts, gContext.Width, gContext.Height);
			RasterTriangle((PixelRGBA32*)pData, pts2, gContext.Width, gContext.Height);


		}
		SDL_UnlockTexture(gContext.Framebuffer);
		SDL_RenderCopy(gContext.Renderer, gContext.Framebuffer, NULL, NULL);

		{
			void* debugDepth = nullptr;
			int pitch;
			SDL_LockTexture(gContext.DepthBufferDebug, NULL, &debugDepth, &pitch);
			for (int y = 0; y < gContext.Height; ++y)
			{
				for (int x = 0; x < gContext.Width; ++x)
				{
					PixelRGBA32 debugDepthPixel;
					debugDepthPixel.R = uint8_t(gContext.DepthBuffer[y * gContext.Width + x] * 255.0f);
					debugDepthPixel.G = 0;
					debugDepthPixel.B = 0;
					debugDepthPixel.A = 255;
					((PixelRGBA32*)debugDepth)[y * gContext.Width + x] = debugDepthPixel;
				}
			}

			SDL_UnlockTexture(gContext.DepthBufferDebug);
			SDL_Rect target;
			target.x = 0;
			target.y = 0;
			target.w = gContext.Width / 4;
			target.h = gContext.Width/ 4;
			SDL_RenderCopy(gContext.Renderer, gContext.DepthBufferDebug, NULL, &target);
		}

		// Present.
		SDL_RenderPresent(gContext.Renderer);
	}

	CleanUp();
	return 0;
}

bool InitSDL()
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		return false;
	}
}

bool InitWindowAndRenderer()
{
	gContext.Window = SDL_CreateWindow("NRaster", 100, 100, 1024, 720, SDL_WINDOW_SHOWN);
	if (gContext.Window == nullptr)
	{
		CleanUp();
		return false;
	}

	gContext.Renderer = SDL_CreateRenderer(gContext.Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (gContext.Renderer == nullptr)
	{
		CleanUp();
		return false;
	}

	gContext.Framebuffer = SDL_CreateTexture(gContext.Renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, gContext.Width, gContext.Height);
	if (!gContext.Framebuffer)
	{
		return false;
	}

	gContext.DepthBufferDebug = SDL_CreateTexture(gContext.Renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, gContext.Width, gContext.Height);
	if (!gContext.DepthBufferDebug)
	{
		return false;
	}

	gContext.DepthBuffer = new float[gContext.Width * gContext.Height];
}

void CleanUp()
{
	SDL_Quit();
}

void TestSDL(PixelRGBA32* pixels, int width, int height)
{
	int pixelSize = sizeof(PixelRGBA32);
	for (int h = 0; h < gContext.Height; ++h)
	{
		for (int w = 0; w < gContext.Width; ++w)
		{
			float u = (float)w / gContext.Width;
			float v = (float)h / gContext.Height;
			int idx = (h * (width * pixelSize)) + (w * pixelSize);

			PixelRGBA32* cur = pixels;
			cur += h * width + w;

			PixelRGBA32 newPixel;
			newPixel.R = (uint8_t)(u * 255.0f);
			newPixel.G = (uint8_t)(v * 255.0f);
			newPixel.B = 0x0;
			newPixel.A = 0xff;

			*cur = newPixel;
		}
	}
}


float EdgeTest(const Vec3& a, const Vec3& b, const Vec3& c)
{
	return ((c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x));
}

void TestRaster(PixelRGBA32* pixels, int width, int height)
{
	Vec3 pts[3];

	// Triangle in NDC space [-1,1]
	pts[0] = Vec3(0.0f, 0.5f, 0.0f);
	pts[1] = Vec3(-0.5f, -0.5f, 0.0f);
	pts[2] = Vec3(0.5f, -0.5f, 0.0f);

	RasterTriangle(pixels, pts, width, height);
}

void RasterTriangle(PixelRGBA32* pixels, Vec3* positions, int width, int height)
{
	Vec3 v0 = positions[0];
	Vec3 v1 = positions[1];
	Vec3 v2 = positions[2];

	// Convert into screen position (note that we flip the y)
	Vec3 rasterv0((v0.x * 0.5f + 0.5f) * width, (1.0f - (v0.y * 0.5f + 0.5f)) * height, v0.z);
	Vec3 rasterv1((v1.x * 0.5f + 0.5f) * width, (1.0f - (v1.y * 0.5f + 0.5f)) * height, v1.z);
	Vec3 rasterv2((v2.x * 0.5f + 0.5f) * width, (1.0f - (v2.y * 0.5f + 0.5f)) * height, v2.z);

	// Attribute correct interpolation:
	//	color0 /= raster0.z
	// ...
	//  Then use the bary coords as usual.
	//  Finally, mult interpolated result by pixel z.

	// We use 1 / V.z to calculate the current pixel depth
	//	1 / P.z =  (1 / V0.z) * D0 + (1 / V1.z) * D1 + (1 / V2.z) * D2
	float depthRcp0 = 1.0f / rasterv0.z;
	float depthRcp1 = 1.0f / rasterv1.z;
	float depthRcp2 = 1.0f / rasterv2.z;

	float area = EdgeTest(rasterv0, rasterv1, rasterv2);
	for (int sy = 0; sy < height; ++sy)
	{
		for (int sx = 0; sx < width; ++sx)
		{
			Vec3 rasterPixel(sx, sy, 0.0f);

			// Areas of the parallelograms [rastervx, rastervy, rasterPixel]
			float w0 = EdgeTest(rasterv0, rasterv1, rasterPixel);
			float w1 = EdgeTest(rasterv1, rasterv2, rasterPixel);
			float w2 = EdgeTest(rasterv2, rasterv0, rasterPixel);

			if ((w0 >= 0.0f) && (w1 >= 0.0f) && (w2 >= 0.0f))
			{
				// Barycentric coordinates. Ratio between the area of the triangle 
				// and ratio of the area of each vx,vy,pixel. Note that we do not divide by 2, as it cancels out.
				w0 /= area;
				w1 /= area;
				w2 /= area;

				// Depth test [LESS_THAN]:
				float pixelDepth = 1.0f / (depthRcp0 * w0 + depthRcp1 * w1 + depthRcp2 * w2);
				if (pixelDepth < gContext.DepthBuffer[sy * width + sx])
				{
					gContext.DepthBuffer[sy * gContext.Width + sx] = pixelDepth;
				
					// Pixel shader :)
					PixelRGBA32* cur = pixels;
					cur += sy * width + sx;

					PixelRGBA32 newPixel;
					newPixel.R = uint8_t(w0 * 255.0f);
					newPixel.G = uint8_t(w1 * 255.0f);
					newPixel.B = uint8_t(w2 * 255.0f);
					newPixel.A = 0xff;

					*cur = newPixel;
				}
			}
		}
	}
}

bool PollEvents()
{
	SDL_Event sdlEvent;
	while (SDL_PollEvent(&sdlEvent) != 0)
	{
		if (sdlEvent.type == SDL_QUIT)
		{
			return true;
		}
	}
	return false;
}
