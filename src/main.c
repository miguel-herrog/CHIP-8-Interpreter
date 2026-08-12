#include <stdio.h>
#include <stdint.h>

struct Chip8 {
    uint8_t RAM[4096];
    uint16_t I;
    uint8_t V[16]; // Resgisters (Vx - 0 through F)
    uint16_t PC; // Program Counter
    uint16_t stack[16];
    uint8_t SP; // Stack Pointer
    uint8_t delayTimer;
    uint8_t soundTimer;
};

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

    for (int i = 0; i < 80; i++){
        cpu->RAM[0x50 + i] = fontset[i];
    }
}

void load_rom(struct Chip8* cpu, const char* filename){
    FILE *fptr = fopen(filename, "rb");
    if (!fptr) return;
    
    fseek(fptr, 0, SEEK_END);
    long size = ftell(fptr);
    rewind(fptr);
    if (size> 3584) {printf("ROM to big\n"); fclose(fptr); return;}

    fread(&cpu->RAM[0x200], 1, size, fptr);

    fclose(fptr);
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
        case 0x0000: 
            break;      
        case 0x1000: 
            cpu->PC = NNN;
            break;
        case 0x2000: 
            break;
        case 0x3000: 
            break;
        case 0x4000: 
            break;
        case 0x5000: 
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
            break;
        case 0xA000:
            cpu->I = NNN;
            break;
        case 0xB000: 
            break;
        case 0xC000: 
            break;
        case 0xD000: 
            break;
        case 0xE000: 
            break;
        case 0xF000: 
            break;
        default:
            printf("Unknown Opcode or not implemented: 0x%X\n", opcode);
            break;
    }
}

int main() {
    struct Chip8 myConsole;
    initialize(&myConsole);
    return 0;
}