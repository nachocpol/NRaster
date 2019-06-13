#include <chrono>
#include <smmintrin.h>
#include <iostream>
#include <SDL.h>

#include "NModel.h"
#include "NRaster.h"

#include "glm.hpp"
#include "matrix.hpp"
#include "gtc/matrix_transform.hpp"

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
bool PollEvents();

void RenderScene(PixelRGBA32* pixels, int width, int height);

NModel cubeModel;

int main(int, char**)
{
	InitSDL();
	InitWindowAndRenderer();

	cubeModel.LoadFromfile("../../Data/teapot.obj");

	NRaster::Instance()->Initialize();

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
			for (int y = 0; y < gContext.Height; ++y)
			{
				for (int x = 0; x < gContext.Width; ++x)
				{
					PixelRGBA32* cur = (PixelRGBA32*)pData;
					cur += y * gContext.Width + x;
					
					PixelRGBA32 clear;
					clear.R = 0x32;
					clear.G = 0x32;
					clear.B = 0x32;
					clear.A = 0;
					*cur = clear;
				}
			}

			auto start = std::chrono::system_clock::now();
			
			RenderScene((PixelRGBA32*)pData, gContext.Width, gContext.Height);

			auto end = std::chrono::system_clock::now();
			std::chrono::duration<double> diff = end - start;
			std::cout << diff.count() * 1000.0 << "ms.\n";
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
					debugDepthPixel.R = uint8_t(gContext.DepthBuffer[y * gContext.Width + x] * 255.0f * 1.9f);
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
			target.w = gContext.Width / 3;
			target.h = gContext.Height / 3;
			SDL_RenderCopy(gContext.Renderer, gContext.DepthBufferDebug, NULL, &target);
		}

		NRaster::Instance()->DebugDraw(gContext.Renderer);

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
		tri0[0] = Vertex(glm::vec3(-0.5f, 0.5f, 0.4f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f));	// Top-left
		tri0[1] = Vertex(glm::vec3(-0.5f, -0.5f, 0.4f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f));	// Bot-left
		tri0[2] = Vertex(glm::vec3(0.5f, -0.5f, 0.4f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f));	// Bot-right
	}

	Vertex tri1[3];
	{
		tri1[0] = Vertex(glm::vec3(-0.5f, 0.5f, 0.4f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f));	// Top-lef
		tri1[1] = Vertex(glm::vec3(0.5f, -0.5f, 0.4f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f));	// Bot-right
		tri1[2] = Vertex(glm::vec3(0.5f, 0.5f, 0.4f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f));	// Top-right
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

static float curtime = 0.0f;
glm::vec4 MyVertexShader(const Vertex& vertex)
{
	auto worldFromObject = glm::mat4();
	worldFromObject = glm::scale(worldFromObject, glm::vec3(0.02f, 0.02f, 0.02f));
	worldFromObject = glm::rotate(worldFromObject, curtime, glm::vec3(0.0f, 1.0f, 0.0f));

	//worldFromObject = glm::mat4();

	auto viewFromWorld = glm::lookAtLH(glm::vec3(0.0f, 2.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	auto screenFromView = glm::perspectiveFovLH(glm::radians(75.0f), (float)gContext.Width, (float)gContext.Height, 0.05f, 10.0f);

	return screenFromView * viewFromWorld * worldFromObject * vertex.Position;
}

glm::vec4 MyPixelShader(const Vertex& vertex)
{
	float NdotL = glm::clamp(glm::dot(glm::normalize(vertex.Normal), glm::vec3(1.0f, 0.5f, 0.0f)),0.1f,1.0f);
	const int M = 20;
	float p = (fmod(vertex.TexCoord.x * M, 1.0) > 0.5) ^ (fmod(vertex.TexCoord.y * M, 1.0) < 0.5);
	return glm::vec4(1.0f,1.0f,1.0f,1.0f) * p * NdotL;
}

void RenderScene(PixelRGBA32* pixels, int width, int height)
{
	NRaster::Instance()->SetDepthBuffer(gContext.DepthBuffer);
	NRaster::Instance()->SetRenderTarget(pixels);
	NRaster::Instance()->SetViewport(0, 0, gContext.Width, gContext.Height);
	NRaster::Instance()->SetShaders(MyVertexShader, MyPixelShader);
	NRaster::Instance()->Draw(cubeModel.GetAllVertex(), cubeModel.GetNumVertices());

	curtime += 0.014f;
}
