#pragma once
#include <string>
#include <SDL.h>
#include <iostream>
#include <random>
#include <fstream>

class Chip8
{
public:
	bool drawFlag;

	void Initialize();
	void EmulateCycle();
	void SetKeys			(SDL_Event	*event);
	void DrawGraphics		(Uint32		*pixels);
	bool LoadGame			(const char	*filename);

	Chip8();
	~Chip8();
};