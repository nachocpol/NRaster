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
	Vertex()
	{
	}
	Vertex(const Vec3& _position) : 
		  Position(_position) 
	{
	}
	Vertex(const Vec3& _position, const Vec3& _color, const Vec2& _texcoord) : 
		  Position(_position) 
		, Color(_color)
		, TexCoord(_texcoord)
	{
	}
	Vec3 Position;
	Vec3 Color;
	Vec2 TexCoord;
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
void RasterTriangle(PixelRGBA32* pixels, Vertex* vtx, int width, int height);
bool PollEvents();

void RenderScene(PixelRGBA32* pixels, int width, int height);

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
		SDL_SetRenderDrawColor(gContext.Renderer, 255, 255, 255, 255);
		SDL_RenderClear(gContext.Renderer);
		void* pData = nullptr;
		int pitch = 0;
		SDL_LockTexture(gContext.Framebuffer, NULL, &pData, &pitch);
		{
			// TestSDL((PixelRGBA32*)pData, gContext.Width, gContext.Height);
			TestRaster((PixelRGBA32*)pData, gContext.Width, gContext.Height);

		}
		SDL_UnlockTexture(gContext.Framebuffer);
		SDL_RenderCopy(gContext.Renderer, gContext.Framebuffer, NULL, NULL);

		// Debug depth buffer:
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
			target.w = gContext.Width / 5;
			target.h = gContext.Height / 5;
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
	return true;
}

bool InitWindowAndRenderer()
{
	gContext.Window = SDL_CreateWindow("NRaster", 100, 100, gContext.Width, gContext.Height, SDL_WINDOW_SHOWN);
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
	return true;
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
	/*
		Lets render a quad:
			
			(-0.5, 0.5)--(0.5, 0.5)
				 |			|
				 |			|
			(-0.5,-0.5)--(0.5,-0.5)
	*/

	Vertex tri0[3];
	{
		tri0[0] = Vertex(Vec3(-0.5f, 0.5f, 0.4f), Vec3(1.0f, 0.0f, 0.0f), Vec2(0.0f, 0.0f));	// Top-left
		tri0[1] = Vertex(Vec3(-0.5f, -0.5f, 0.4f), Vec3(0.0f, 1.0f, 0.0f), Vec2(0.0f, 1.0f));	// Bot-left
		tri0[2] = Vertex(Vec3(0.5f, -0.5f, 0.4f), Vec3(0.0f, 0.0f, 1.0f), Vec2(1.0f, 1.0f));	// Bot-right
	}

	Vertex tri1[3];
	{
		tri1[0] = Vertex(Vec3(-0.5f, 0.5f, 0.4f), Vec3(1.0f, 0.0f, 0.0f), Vec2(0.0f, 0.0f));	// Top-lef
		tri1[1] = Vertex(Vec3(0.5f, -0.5f, 0.4f), Vec3(0.0f, 1.0f, 0.0f), Vec2(1.0f, 1.0f));	// Bot-right
		tri1[2] = Vertex(Vec3(0.5f, 0.5f, 0.4f), Vec3(0.0f, 0.0f, 1.0f), Vec2(1.0f, 0.0f));		// Top-right
	}

	RasterTriangle(pixels, tri0, gContext.Width, gContext.Height);
	RasterTriangle(pixels, tri1, gContext.Width, gContext.Height);
}

void RasterTriangle(PixelRGBA32* pixels, Vertex* vtx, int width, int height)
{
	// [CCW]
	Vec3 v0 = vtx[0].Position;
	Vec3 v1 = vtx[1].Position;
	Vec3 v2 = vtx[2].Position;

	// Convert into screen position (note that we flip the y)
	Vec3 rasterv0((v0.x * 0.5f + 0.5f) * width, (1.0f - (v0.y * 0.5f + 0.5f)) * height, v0.z);
	Vec3 rasterv1((v1.x * 0.5f + 0.5f) * width, (1.0f - (v1.y * 0.5f + 0.5f)) * height, v1.z);
	Vec3 rasterv2((v2.x * 0.5f + 0.5f) * width, (1.0f - (v2.y * 0.5f + 0.5f)) * height, v2.z);

	// We use 1 / V.z to calculate the current pixel depth
	//	1 / P.z =  (1 / V0.z) * D0 + (1 / V1.z) * D1 + (1 / V2.z) * D2
	float depthRcp0 = 1.0f / rasterv0.z;
	float depthRcp1 = 1.0f / rasterv1.z;
	float depthRcp2 = 1.0f / rasterv2.z;
	
	// Attribute correct interpolation:
	//	att0 /= raster0.z
	// ...
	//  Then use the bary coords as usual.
	//  Finally, mult interpolated result by pixel z.
	
	// TexCoords (we mult z as z = 1/z)
	Vec2 rasterTexCoord0 = Vec2(vtx[0].TexCoord.x * depthRcp0, vtx[0].TexCoord.y * depthRcp0);
	Vec2 rasterTexCoord1 = Vec2(vtx[1].TexCoord.x * depthRcp1, vtx[1].TexCoord.y * depthRcp1);
	Vec2 rasterTexCoord2 = Vec2(vtx[2].TexCoord.x * depthRcp2, vtx[2].TexCoord.y * depthRcp2);

	float area = EdgeTest(rasterv0, rasterv1, rasterv2);
	for (int sy = 0; sy < height; ++sy)
	{
		for (int sx = 0; sx < width; ++sx)
		{
			Vec3 rasterPixel((float)sx + 0.5f, (float)sy + 0.5f, 0.0f); // +0.5->center pixel

			// Areas of the parallelograms [rastervx, rastervy, rasterPixel]
			float w0 = EdgeTest(rasterv1, rasterv2, rasterPixel);
			float w1 = EdgeTest(rasterv2, rasterv0, rasterPixel);
			float w2 = EdgeTest(rasterv0, rasterv1, rasterPixel);

			if ((w0 >= 0.0f) && (w1 >= 0.0f) && (w2 >= 0.0f))
			{
				// Barycentric coordinates. Ratio between the area of the triangle 
				// and ratio of the area of each vx,vy,pixel. Note that we do not divide by 2, as it cancels out.
				w0 /= area;
				w1 /= area;
				w2 /= area;

				// Depth test [LESS_THAN]:
				float pixelDepth = 1.0f / (depthRcp0 * w0 + depthRcp1 * w1 + depthRcp2 * w2);
				// nah... pixelDepth = pixelDepth <= 0.0f ? 0.001f : pixelDepth;
				if (pixelDepth < gContext.DepthBuffer[sy * width + sx])
				{
					// Update depth buffer:
					gContext.DepthBuffer[sy * gContext.Width + sx] = pixelDepth;

					// Perspective correct TexCoords:
					Vec2 intTexCoord;
					intTexCoord.x = (rasterTexCoord0.x * w0 + rasterTexCoord1.x * w1 + rasterTexCoord2.x * w2) * pixelDepth;
					intTexCoord.y = (rasterTexCoord0.y * w0 + rasterTexCoord1.y * w1 + rasterTexCoord2.y * w2) * pixelDepth;

					// Pixel color:
					PixelRGBA32* cur = pixels;
					cur += sy * width + sx;

					const int M = 10;
					float checker = (fmod(intTexCoord.x * M, 1.0) > 0.5) ^ (fmod(intTexCoord.y * M, 1.0) < 0.5);

					PixelRGBA32 newPixel;
					newPixel.R = uint8_t(checker * 255.0f);
					newPixel.G = uint8_t(checker * 255.0f);
					newPixel.B = uint8_t(checker * 255.0f);
					newPixel.A = 0xff;

					*cur = newPixel;
				}
				else
				{
					// Pixel color:
					PixelRGBA32* cur = pixels;
					cur += sy * width + sx;

					PixelRGBA32 newPixel;
					newPixel.R = 0xdd;
					newPixel.G = 0xdd;
					newPixel.B = 0xdd;
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

void RenderScene(PixelRGBA32 * pixels, int width, int height)
{
}
