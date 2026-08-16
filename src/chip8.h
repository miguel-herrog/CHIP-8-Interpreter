#ifndef CHIP8_H
#define CHIP8_H
#include <stdint.h>

struct Chip8
{
    uint8_t RAM[4096];
    uint16_t I;
    uint8_t V[16]; // Resgisters (Vx - 0 through F)
    uint16_t PC;   // Program Counter
    uint16_t stack[16];
    uint8_t SP; // Stack Pointer
    uint8_t delayTimer;
    uint8_t soundTimer;
    uint8_t gfx[64 * 32];
    uint8_t keypad[16];
};

extern uint8_t fontset[80];
extern void initialize(struct Chip8 *cpu);
extern int load_rom(struct Chip8 *cpu, const char *filename);
extern void emulate_cycle(struct Chip8 *cpu);

#endif