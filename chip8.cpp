#pragma once
#include "chip8.h"

// 4-bit register identifiers
#define VX V[ (opcode & 0x0F00) >> 8 ]
#define VY V[ (opcode & 0x00F0) >> 4 ]

#define VF V[ 0xF ]	// Carry flag

#define N (opcode & 0x000F)		// 4-bit constant
#define NN (opcode & 0x00FF)	// 8-bit constant

#define NNN (opcode & 0x0FFF)	// Address

unsigned short opcode;				// Stores current opcode
unsigned char memory[4096];			// Emulate 4K Memory
unsigned char V[16];				// CPU registers
unsigned short I;					// Index Register
unsigned short pc;					// Program counter
unsigned char gfx[64 * 32];			// Screen pixels

// Timer registers
unsigned char delay_timer;
unsigned char sound_timer;

unsigned short stack[16];			// The CPU Stack
unsigned short sp;					// Stack pointer
unsigned char key[16];				// Hex keypad 0x0-0xF
unsigned char chip8_fontset[80] =	// Basic fontset
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

// Memory Map
// 0x000-0x1FF Chip 8 interpreter (contains font set in emu)
// 0x050-0x0A0 Used for the built in 4x5 pixel font set
// 0x200-0xFFF Program ROM and work RAM

// Setup random number generator
std::random_device rd;
std::mt19937 rng(rd());
std::uniform_int_distribution<int> rand_int(0, 255);

void Chip8::Initialize()
{
	// Clear the memory, registers, and screen
	// Initialize registers and memory once

	pc = 0x200;	// Program counter starts at 0x200
	opcode = 0;		// Reset current opcode
	I = 0;		// Reset index register
	sp = 0;		// Reset stack pointer

	// Clear display
	for (int i = 0; i < 64 * 32; ++i)
	{
		gfx[i] = 0;
	}
	drawFlag = true;

	// Clear stack
	for (int i = 0; i < 16; ++i)
	{
		stack[i] = 0;
	}

	// Clear registers V0-VF and keys
	for (int i = 0; i < 16; ++i)
	{
		key[i] = V[i] = 0;
	}

	// Clear memory
	for (int i = 0; i < 4096; ++i)
	{
		memory[i] = 0;
	}

	// Load fontset
	for (int i = 0; i < 80; ++i)
	{
		memory[i] = chip8_fontset[i];
	}

	// Reset timers
	delay_timer = 0;
	sound_timer = 0;
}

bool Chip8::LoadGame(const char *filename)
{
	std::ifstream file(filename, std::ios::in | std::ios::binary);
	bool success = false;

	if (file) {
		int i = 0;
		char c;
		while (file.get(c))
		{
			memory[512 + i++] = c;
		}
		std::cout << "Loaded:" << filename << " Size:" << i << " bytes" << std::endl;
		success = true;
	}
	else {
		std::cout << "Failed to load " << filename << std::endl;
	}

	file.close();
	return success;
}

void Chip8::EmulateCycle()
{
	// Emulate one cycle of the system

	// Fetch Opcode
	opcode = memory[pc] << 8 | memory[pc + 1];
	//std::cout << std::hex << std::uppercase << opcode << std::dec << std::nouppercase << std::endl;

	// Decode and Execute Opcode
	switch (opcode & 0xF000)
	{
		// Some opcodes require checking the last four bits
	case 0x0000:
		switch (opcode & 0x00FF)
		{
		case 0x00E0: // Clears the screen
			for (int i = 0; i < 64 * 32; ++i)
			{
				gfx[i] = 0x0;
			}
			drawFlag = true;
			pc += 2;
			break;

		case 0x00EE: // Returns from subroutine
			--sp;
			pc = stack[sp] + 2;
			break;

		default:	// Calls RCA 1802 program at address NNN
			// Explosions go here
			break;
		}
		break;

	case 0x1000: // Jumps to address NNN
		pc = NNN;
		break;

	case 0x2000: // Calls subroutine at address NNN
		stack[sp] = pc;
		++sp;
		pc = NNN;
		break;

	case 0x3000: // Skips the next instruction if VX equals NN.
		if (VX == NN) {
			pc += 4;
		}
		else
		{
			pc += 2;
		}
		break;

	case 0x4000: // Skips the next instruction is VX doesn't equal NN
		if (VX != NN) {
			pc += 4;
		}
		else
		{
			pc += 2;
		}
		break;

	case 0x5000: // Skips the next instruction if VX equals VY
		if (VX == VY) {
			pc += 4;
		}
		else
		{
			pc += 2;
		}
		break;

	case 0x6000: // Sets Vx to NN
		VX = NN;
		pc += 2;
		break;

	case 0x7000: // Adds NN to Vx. (Carry flag is not changed)
		VX += NN;
		pc += 2;
		break;

	case 0x8000:
		switch (opcode & 0x000F)
		{
		case 0x0000: // Sets Vx to the value of Vy
			VX = VY;
			pc += 2;
			break;

		case 0x0001: // Sets Vx to Vx OR Vy (Bitwise OR operation)
			VX |= VY;
			pc += 2;
			break;

		case 0x0002: // Sets Vx to Vx AND Vy (Bitwise AND operation)
			VX &= VY;
			pc += 2;
			break;

		case 0x0003: // Sets Vx to Vx XOR Vy
			VX ^= VY;
			pc += 2;
			break;

		case 0x0004: // Adds the value of Vx to Vy and sets VF to 1 if sum is greater than 255
			if (VY > (0xFF - VX))
			{
				VF = 1; // Carry
			}
			else
			{
				VF = 0;
			}
			VX += VY;
			pc += 2;
			break;

		case 0x0005: // Vy is subtracted from Vx. VF is set to 0 when there's a borrow and 1 when there isn't
			if (VY > VX)
			{
				VF = 0;
			}
			else
			{
				VF = 1;
			}
			VX -= VY;
			pc += 2;
			break;

		case 0x0006: // Stores the least significant bit of Vx in VF and then shifts Vx to the right by 1
			VF = VX & 0x0F;
			//VF = VX & 0x1;
			VX >>= 1;
			pc += 2;
			break;

		case 0x0007: // Sets Vx to Vy minus Vx. VF is set to 0 when there's a borrow and 1 when there isn't
			if (VX > VY)
			{
				VF = 0;
			}
			else
			{
				VF = 1;
			}
			VX = VY - VX;
			pc += 2;
			break;

		case 0x000E: // Stores the most significant bit of Vx in VF and then shifts Vx to the left by 1
			VF = VX & 0xF0 >> 4;
			//VF = VX >> 7;
			VX <<= 1;
			pc += 2;
			break;

		default:
			std::cout << "Unknown opcode [0x8000]:" << opcode << std::endl;
			break;
		}
		break;

	case 0x9000: // Skips the next instruction if Vx doesn't equal Vy
		if (VX != VY) {
			pc += 4;
		}
		else
		{
			pc += 2;
		}
		break;

	case 0xA000: // ANNN: Sets I to the address NNN
		I = NNN;
		pc += 2;
		break;

	case 0xB000: // Jumps to the address NNN plus V0
		pc = NNN + V[0];
		break;

	case 0xC000: // Sets Vx to the result of a bitwise AND operation on a random number (typically:0 to 255) and NN
		VX = rand_int(rng) & NN;
		pc += 2;
		break;

	case 0xD000: // Draws a sprite at coordinate (Vx, Vy)
	{
		unsigned short x = VX;
		unsigned short y = VY;
		unsigned short height = N;
		unsigned short pixel;

		VF = 0;
		for (int yline = 0; yline < height; yline++)
		{
			pixel = memory[I + yline];
			for (int xline = 0; xline < 8; xline++)
			{
				if ((pixel & (0x80 >> xline)) != 0)
				{
					if (gfx[(x + xline + ((y + yline) * 64))] == 1)
					{
						VF = 1;
					}
					gfx[x + xline + ((y + yline) * 64)] ^= 1;
				}
			}
		}

		drawFlag = true;
		pc += 2;
	}
	break;

	case 0xE000:
		switch (opcode & 0x00FF)
		{
			// EX9E: Skips the next instruction
			// if the key stored in VX is pressed
		case 0x009E:
			if (key[VX] == 1)
			{
				pc += 4;
			}
			else
			{
				pc += 2;
			}
			break;

		case 0x00A1:
			if (key[VX] != 1)
			{
				pc += 4;
			}
			else
			{
				pc += 2;
			}
			break;

		default:
			std::cout << "Unknown opcode [0xE000]:" << opcode << std::endl;
			break;
		}
		break;

	case 0xF000:
		switch (opcode & 0x00FF)
		{
		case 0x0007: // Sets Vx to the value of the delay timer
			VX = delay_timer;
			pc += 2;
			break;

		case 0x000A: // A key press is awaited and then stored in VX (Blocking Operation. All instruction halted until next key event)
			for (int i = 0; i < 16; i++)
			{
				if (key[i] == 1)
				{
					VX = i;
					pc += 2;
					break;
				}
			}
			break;

		case 0x0015: // Sets the delay timer to Vx
			delay_timer = VX;
			pc += 2;
			break;

		case 0x0018: // Sets the sound timer to Vx
			sound_timer = VX;
			pc += 2;
			break;

		case 0x001E: // Adds Vx to I
			if (I + VX > 0xFFF)
			{
				VF = 1;
			}
			else
			{
				VF = 0;
			}

			I += VX;
			pc += 2;
			break;

		case 0x0029: // Sets I to the location of the sprite for the character in Vx.
			I = VX * 0x5;
			pc += 2;
			break;

		case 0x0033: // Stores binary coded decimal representation of Vx at adress I, I+1, I+2
			memory[I] = VX / 100;
			memory[I + 1] = (VX / 10) % 10;
			memory[I + 2] = (VX % 100) % 10;
			pc += 2;
			break;

		case 0x0055: // Stores V0 to Vx (including Vx) in memory starting at address I. The offset from I is increased by 1 for each value written, but I istelf is left unmodified
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
			{
				memory[I + i] = V[i];
			}

			// On the original interpreter when the operation is don I = I + Vx + 1
			I += VX + 1;
			pc += 2;
			break;

		case 0x0065: // Fills V0 to Vx (including Vx) with values from memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
			{
				V[i] = memory[I + i];
			}

			// On the original interpreter when the operation is done I = I + Vx + 1
			I += VX + 1;
			pc += 2;
			break;

		default:
			std::cout << "Unknown opcode [0xF000]:" << opcode << std::endl;
			break;
		}
		break;

	default:
		std::cout << "Unknown opcode:" << opcode << std::endl;
		break;
	}


	// Update timers
	if (delay_timer > 0)
	{
		--delay_timer;
	}

	if (sound_timer > 0)
	{
		if (sound_timer == 1)
		{
			std::cout << "BEEP!" << std::endl;
		}
		--sound_timer;
	}
}

void Chip8::SetKeys(SDL_Event *event)
{
	switch (event->type)
	{
	case SDL_KEYDOWN:
		switch (event->key.keysym.sym)
		{
		case SDLK_1: key[0x1] = 1; break;
		case SDLK_2: key[0x2] = 1; break;
		case SDLK_3: key[0x3] = 1; break;
		case SDLK_4: key[0xC] = 1; break;
		case SDLK_q: key[0x4] = 1; break;
		case SDLK_w: key[0x5] = 1; break;
		case SDLK_e: key[0x6] = 1; break;
		case SDLK_r: key[0xD] = 1; break;
		case SDLK_a: key[0x7] = 1; break;
		case SDLK_s: key[0x8] = 1; break;
		case SDLK_d: key[0x9] = 1; break;
		case SDLK_f: key[0xE] = 1; break;
		case SDLK_z: key[0xA] = 1; break;
		case SDLK_x: key[0x0] = 1; break;
		case SDLK_c: key[0xB] = 1; break;
		case SDLK_v: key[0xF] = 1; break;
		}
		break;
	case SDL_KEYUP:
		switch (event->key.keysym.sym)
		{
		case SDLK_1: key[0x1] = 0; break;
		case SDLK_2: key[0x2] = 0; break;
		case SDLK_3: key[0x3] = 0; break;
		case SDLK_4: key[0xC] = 0; break;
		case SDLK_q: key[0x4] = 0; break;
		case SDLK_w: key[0x5] = 0; break;
		case SDLK_e: key[0x6] = 0; break;
		case SDLK_r: key[0xD] = 0; break;
		case SDLK_a: key[0x7] = 0; break;
		case SDLK_s: key[0x8] = 0; break;
		case SDLK_d: key[0x9] = 0; break;
		case SDLK_f: key[0xE] = 0; break;
		case SDLK_z: key[0xA] = 0; break;
		case SDLK_x: key[0x0] = 0; break;
		case SDLK_c: key[0xB] = 0; break;
		case SDLK_v: key[0xF] = 0; break;
		}
		break;
	}
}

void Chip8::DrawGraphics(Uint32 *pixels)
{
	for (int i = 0; i < 64 * 32; i++)
	{
		pixels[i] = gfx[i] * 0xFFFFFF;
	}
}

Chip8::Chip8()
{
}

Chip8::~Chip8()
{
}