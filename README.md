# CHIP-8 Emulator in C

![CHIP-8 Gameplay Demo](gameplay_demo.gif)

## Overview
A custom CHIP-8 virtual machine written from scratch in pure C. Built to master low-level programming, manual memory management, bitwise operations, and hardware architecture. 

## Features
* **Full Instruction Set:** Complete implementation of all standard CHIP-8 opcodes.
* **Hardware-Accelerated Rendering:** Graphics pipeline built using SDL2.
* **Accurate Emulation:** Precise 60Hz timing for delay and sound timers independent of CPU cycles.
* **Memory Quirks Handled:** Includes historical hardware behaviors for memory load/store operations.
* **Modern Controls:** Original 16-key hexpad mapped to a standard QWERTY layout.
* **Modular Architecture:** Source code separated into headers and implementations, managed by a custom Makefile.

## Dependencies
To compile and run this emulator from the source code, you will need:
* **GCC / MinGW-w64** (C Compiler)
* **SDL2** (Simple DirectMedia Layer development libraries)

## Build Instructions
Navigate to the project's root directory and run the build system:

`make`
*(Note: Use mingw32-make if you are running MinGW on Windows)*

## Usage
Execute the compiled binary from the command line, passing the path to the CHIP-8 ROM as an argument:

`./chip8_emulator.exe roms/space_invaders.ch8`

## Controls Mapping
The CHIP-8 originally used a 16-key hexadecimal keypad. This emulator maps those keys to the left side of a modern keyboard to ensure playability:

| Original CHIP-8 Pad | Modern QWERTY Mapping |
| :---: | :---: |
| 1 2 3 C | 1 2 3 4 |
| 4 5 6 D | Q W E R |
| 7 8 9 E | A S D F |
| A 0 B F | Z X C V |
