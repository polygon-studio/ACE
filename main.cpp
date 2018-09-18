#pragma once
#include "chip8.h"
#include <SDL2/SDL.h>

// Screen resolution constants
const int SCREEN_WIDTH	= 64;
const int SCREEN_HEIGHT = 32;

Chip8 myChip8;

int main(int argc, char* args[])
{
	// Emulator customization
	std::string gameFile;
	int cpuSpeed;

	std::cout << "Load rom:";
	std::getline(std::cin, gameFile);

	std::string cpuSpeedStr;
	std::cout << "Enter CPU Speed (in ms):";
	std::getline(std::cin, cpuSpeedStr);
	cpuSpeed = atoi(cpuSpeedStr.c_str());
	std::cout << cpuSpeed << std::endl;
	
	
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cout << "SDL could not initialize! SDL_Error:" << SDL_GetError() << std::endl;
	}
	else
	{
		// Create Window
		SDL_Window* window = SDL_CreateWindow("Chip-8 Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			SCREEN_WIDTH, SCREEN_HEIGHT, 0);
		if (window == NULL)
		{
			std::cout << "Window could not be created! SDL_Error:" << SDL_GetError() << std::endl;
		}
		else
		{
			SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
			SDL_Texture* texture = SDL_CreateTexture(renderer,
				SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, SCREEN_WIDTH, SCREEN_HEIGHT);
			Uint32* pixels = new Uint32[SCREEN_WIDTH * SCREEN_HEIGHT];
			memset(pixels, 100, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(Uint32));

			bool quit = false;
			bool leftMouseButtonDown = false;

			SDL_Event event;

			// Turn on our Chip-8 machine
			myChip8.Initialize();
			// Insert a game into our Chip-8 machine
			//myChip8.LoadGame("invaders.c8");
			myChip8.LoadGame(gameFile.c_str());

			while (!quit)
			{
				SDL_UpdateTexture(texture, NULL, pixels, SCREEN_WIDTH * sizeof(Uint32));
				SDL_PollEvent(&event);
				
				// Check for program exit
				if (event.type == SDL_QUIT)
				{
					quit = true;
				}

				// Emulate one cycle
				myChip8.EmulateCycle();

				// Update Chip-8 display
				if (myChip8.drawFlag) 
				{
					DrawGraphics(pixels);
				}

				// Store key press state
				myChip8.SetKeys(&event);

				SDL_RenderClear(renderer);
				SDL_RenderCopy(renderer, texture, NULL, NULL);
				SDL_RenderPresent(renderer);

				// Cap the Chip-8 cycle rate
				SDL_Delay(cpuSpeed);
			}

			// Destroy SDL
			delete[] pixels;
			SDL_DestroyTexture(texture);
			SDL_DestroyRenderer(renderer);
			SDL_DestroyWindow(window);

			// Quit SDL subsystems
			SDL_Quit();
		}
	}

	return 0;
}
