# ACE
*Another Chip-8 Emulator (Written in C++)*

This program emulates the [Chip-8](https://en.wikipedia.org/wiki/CHIP-8) *interpreted programming language* and can run c8 compatible roms. The program will ask for for a file and then speed of the CPU in ms, *a speed of 1ms provides the fastest execution while still being readable*.

### Requirements
* OpenGL
* SDL2

### Todo
* Add a GUI for loading roms and changing emulator settings like cpu cycles
* Add sound support
* Add options to change resolution
* *Switch graphics API from OpenGL to Vulkan*
* *Add support for other hardware like the Sega Saturn*
* *Rename this program*

#### Resources
[Opcode table handy for writing your own c8 programs.](https://en.wikipedia.org/wiki/CHIP-8#Opcode_table)

[Chip-8 assembler](https://bisqwit.iki.fi/jutut/kuvat/programming_examples/chip8/asm.cc)
