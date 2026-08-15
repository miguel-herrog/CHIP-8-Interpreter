#include "chip8.h"
#include <stdio.h>
#include <string.h> 
#include <stdlib.h>

uint8_t fontset[80] = {
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

void initialize(struct Chip8* cpu){
    cpu->PC =  0x200;
    memset(cpu->RAM, 0, sizeof(cpu->RAM));
    memset(cpu->gfx, 0, sizeof(cpu->gfx));
    memset(cpu->keypad, 0, sizeof(cpu->keypad));
    for (int i = 0; i < 80; i++){
        cpu->RAM[0x50 + i] = fontset[i];
    }
}

int load_rom(struct Chip8* cpu, const char* filename){
    FILE *fptr = fopen(filename, "rb");
    if (!fptr) return 0;
    
    fseek(fptr, 0, SEEK_END);
    long size = ftell(fptr);
    rewind(fptr);
    if (size> 3584) {printf("ROM to big\n"); fclose(fptr); return 0;}

    fread(&cpu->RAM[0x200], 1, size, fptr);
    fclose(fptr);
    return 1;
}

void emulate_cycle(struct Chip8* cpu){
    // Fetch
    uint16_t opcode = (cpu->RAM[cpu->PC] << 8) | cpu->RAM[cpu->PC+ 1];
    cpu->PC += 2;
    // Decode
    uint16_t NNN = opcode & 0x0FFF; // 12 Bits
    uint16_t NN = opcode & 0x00FF; // 8 Bits
    uint16_t N = opcode & 0x000F; // 4 Bits

    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;

    switch(opcode & 0xF000){
        case 0x0000:{
            if (opcode == 0x00E0){
                memset(cpu->gfx, 0, sizeof(cpu->gfx));
            } 
            else if (opcode == 0x00EE){
                cpu->SP -= 1;
                cpu->PC = cpu->stack[cpu->SP];
            }
            break;  
        }    
        case 0x1000: 
            cpu->PC = NNN;
            break;
        case 0x2000:
            cpu->stack[cpu->SP] = cpu->PC;
            cpu->SP += 1;
            cpu->PC = NNN;
            break;
        case 0x3000: 
            if (cpu->V[x] == NN){
                cpu->PC += 2;
            }
            break;
        case 0x4000:
            if (cpu->V[x] != NN){
                cpu->PC += 2;
            }
            break;
        case 0x5000: 
            if (cpu->V[x] == cpu->V[y]){
                cpu->PC += 2;
            }
            break;
        case 0x6000: 
            cpu->V[x] = NN;
            break;
        case 0x7000: 
            cpu->V[x] += NN;
            break;
        case 0x8000: 
            switch(N){
                case 0x0: 
                    cpu->V[x] = cpu->V[y];
                    break;
                case 0x1: 
                    cpu->V[x] = cpu->V[x] | cpu->V[y];
                    break;
                case 0x2: 
                    cpu->V[x] = cpu->V[x] & cpu->V[y];
                    break;
                case 0x3: 
                    cpu->V[x] = cpu->V[x] ^ cpu->V[y];
                    break;
                case 0x4: 
                    {
                        uint16_t sum = cpu->V[x] + cpu->V[y]; 
                        if (sum > 255){
                            cpu->V[0xF] = 1;
                        } 
                        else {
                            cpu->V[0xF] = 0;
                        }
                        cpu->V[x] = sum;
                        break;
                    }
                case 0x5: 
                    if (cpu->V[x] >= cpu->V[y]){
                        cpu->V[0xF] = 1;
                    } 
                    else {
                        cpu->V[0xF] = 0;
                    }     
                    cpu->V[x] -= cpu->V[y];      
                    break;
                case 0x6: 
                    {
                        uint8_t carry = (cpu->V[x] & 0x01);
                        cpu->V[x] >>= 1;
                        cpu->V[0xF] = carry;
                        break;
                    }
                case 0x7: 
                    if (cpu->V[x] <= cpu->V[y]){
                        cpu->V[0xF] = 1;
                    } 
                    else {
                        cpu->V[0xF] = 0;
                    }     
                    cpu->V[x] = cpu->V[y] - cpu->V[x];      
                    break;
                case 0xE:
                    {
                        uint8_t carry = (cpu->V[x] & 0x80) >> 7;
                        cpu->V[x] <<= 1;
                        cpu->V[0xF] = carry;
                        break;
                    }
                default: printf("Unknown subtipe or not implemented: 0x%X\n", opcode);
                    break;
            }
            break;
        case 0x9000: 
            if (cpu->V[x] != cpu->V[y]){
                cpu->PC += 2;
            }
            break;
        case 0xA000:
            cpu->I = NNN; 
            break;
        case 0xB000: 
            cpu->PC = NNN + cpu->V[0];
            break;
        case 0xC000: 
            cpu->V[x] = (rand() % 256) & NN;
            break;
        case 0xD000: 
            cpu->V[0xF] = 0;
            int startX = cpu->V[x] % 64;
            int startY = cpu->V[y] % 32;

            for (int row = 0; row < N; row++){
                uint8_t spriteByte = cpu->RAM[cpu->I + row];

                for (int col = 0; col < 8; col++){

                    if (spriteByte & (0x80 >> col)){
                        int pX = startX + col;
                        int pY = startY + row;

                        if (pX >= 64 || pY >= 32){
                            continue;
                        }
                        int indice = (pY * 64) + pX;

                        if (cpu->gfx[indice] == 1){
                            cpu->V[0xF] = 1;
                        }
                        cpu->gfx[indice] ^= 1;
                    }
                }
            }
            break;
        case 0xE000: 
            switch(NN){
                case 0x9E:
                    if (cpu->keypad[cpu->V[x]] == 1){
                        cpu->PC += 2;
                    }
                    break;
                case 0xA1:
                    if (cpu->keypad[cpu->V[x]] == 0){
                        cpu->PC += 2;
                    }
                    break;
            }
            break;
        case 0xF000: 
            switch(NN){
                case 0x07:
                    cpu->V[x] = cpu->delayTimer;
                    break;
                case 0x0A:
                    int i;
                    for (i = 0; i < 16; i++){
                        if (cpu->keypad[i] == 1){
                            cpu->V[x] = i;
                            break;
                        }
                    }
                    if (i == 16) { cpu->PC -= 2; }
                    break;
                case 0x15:
                    cpu->delayTimer = cpu->V[x];
                    break;
                case 0x18:
                    cpu->soundTimer = cpu->V[x];
                    break;
                case 0x1E:
                    cpu->I += cpu->V[x];
                    break;
                case 0x29:
                    cpu->I = 0x50 + (cpu->V[x] * 5);
                    break;
                case 0x33:
                    cpu->RAM[cpu->I] = cpu->V[x] / 100;
                    cpu->RAM[cpu->I + 1] = cpu->V[x] % 100 / 10;
                    cpu->RAM[cpu->I + 2] = cpu->V[x] % 10;
                    break;
                case 0x55:
                    for (int i = 0; i <= x; i++){
                        cpu->RAM[cpu->I + i] = cpu->V[i];
                    }
                    cpu->I += (cpu->V[x] + 1);
                    break;
                case 0x65:
                    for (int i = 0; i <= x; i++){
                        cpu->V[i] = cpu->RAM[cpu->I + i];
                    }
                    cpu->I += (cpu->V[x] + 1);
                    break;
                default:
                    printf("Unknown FX opcode: 0x%X\n", opcode);
                    break;
            }
            break;
        default:
            printf("Unknown Opcode or not implemented: 0x%X\n", opcode);
            break;
    }
}