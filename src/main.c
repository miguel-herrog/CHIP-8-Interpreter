#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <SDL.h>

struct Chip8 {
    uint8_t RAM[4096];
    uint16_t I;
    uint8_t V[16]; // Resgisters (Vx - 0 through F)
    uint16_t PC; // Program Counter
    uint16_t stack[16];
    uint8_t SP; // Stack Pointer
    uint8_t delayTimer;
    uint8_t soundTimer;
    uint8_t gfx[64 * 32];
    uint8_t keypad[16];
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
    memset(cpu->gfx, 0, sizeof(cpu->gfx));
    memset(cpu->keypad, 0, sizeof(cpu->keypad));
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
                    break;
                case 0x65:
                    for (int i = 0; i <= x; i++){
                        cpu->V[i] = cpu->RAM[cpu->I + i];
                    }
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


int main(int argc, char* argv[]) {
    // Inicializar SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Error al inicializar SDL: %s", SDL_GetError());
        return 1;
    }

    // Crear la Ventana (1024x512, centrada)
    SDL_Window* window = SDL_CreateWindow(
        "Chip-8 Emulator",
        SDL_WINDOWPOS_CENTERED, // x 
        SDL_WINDOWPOS_CENTERED, // y
        1024, 512, // x, y
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        SDL_Log("Error al crear la ventana: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Crear el Renderer (GPU)
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED
    );

    if (!renderer) {
        SDL_Log("Error al crear el renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Crear la Textura ( RGBA, Streaming, 64x32 nativos)
    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        64,  // Ancho nativo CHIP-8
        32   // Alto nativo CHIP-8
    );

    if (!texture) {
        SDL_Log("Error al crear la textura: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // --- El Bucle Infinito ---
    struct Chip8 myConsole;
    initialize(&myConsole);
    load_rom(&myConsole, "tetris.ch8"); 

    int quit = 0;
    SDL_Event event;

    while (!quit) {
        // Procesar eventos del SO
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            } else if(event.type == SDL_KEYDOWN){
                switch (event.key.keysym.sym){
                    case SDLK_1:
                        myConsole.keypad[0x1] = 1;
                        break;
                    case SDLK_2:
                        myConsole.keypad[0x2] = 1;
                        break;
                    case SDLK_3:
                        myConsole.keypad[0x3] = 1;
                        break;
                    case SDLK_4:
                        myConsole.keypad[0xC] = 1;
                        break;
                    case SDLK_q:
                        myConsole.keypad[0x4] = 1;
                        break;
                    case SDLK_w:
                        myConsole.keypad[0x5] = 1;
                        break;
                    case SDLK_e:
                        myConsole.keypad[0x6] = 1;
                        break;
                    case SDLK_r:
                        myConsole.keypad[0xD] = 1;
                        break;
                    case SDLK_a:
                        myConsole.keypad[0x7] = 1;
                        break;
                    case SDLK_s:
                        myConsole.keypad[0x8] = 1;
                        break;
                    case SDLK_d:
                        myConsole.keypad[0x9] = 1;
                        break;
                    case SDLK_f:
                        myConsole.keypad[0xE] = 1;
                        break;
                    case SDLK_z:
                        myConsole.keypad[0xA] = 1;
                        break;
                    case SDLK_x:
                        myConsole.keypad[0x0] = 1;
                        break;
                    case SDLK_c:
                        myConsole.keypad[0xB] = 1;
                        break;
                    case SDLK_v:
                        myConsole.keypad[0xF] = 1;
                        break;
                    default:
                        break;
                }
            } else if(event.type == SDL_KEYUP){
                switch (event.key.keysym.sym){
                    case SDLK_1:
                        myConsole.keypad[0x1] = 0;
                        break;
                    case SDLK_2:
                        myConsole.keypad[0x2] = 0;
                        break;
                    case SDLK_3:
                        myConsole.keypad[0x3] = 0;
                        break;
                    case SDLK_4:
                        myConsole.keypad[0xC] = 0;
                        break;
                    case SDLK_q:
                        myConsole.keypad[0x4] = 0;
                        break;
                    case SDLK_w:
                        myConsole.keypad[0x5] = 0;
                        break;
                    case SDLK_e:
                        myConsole.keypad[0x6] = 0;
                        break;
                    case SDLK_r:
                        myConsole.keypad[0xD] = 0;
                        break;
                    case SDLK_a:
                        myConsole.keypad[0x7] = 0;
                        break;
                    case SDLK_s:
                        myConsole.keypad[0x8] = 0;
                        break;
                    case SDLK_d:
                        myConsole.keypad[0x9] = 0;
                        break;
                    case SDLK_f:
                        myConsole.keypad[0xE] = 0;
                        break;
                    case SDLK_z:
                        myConsole.keypad[0xA] = 0;
                        break;
                    case SDLK_x:
                        myConsole.keypad[0x0] = 0;
                        break;
                    case SDLK_c:
                        myConsole.keypad[0xB] = 0;
                        break;
                    case SDLK_v:
                        myConsole.keypad[0xF] = 0;
                        break;
                    default:
                        break;
                }
            }
        }
        // Ejecutar un ciclo de la CPU
        emulate_cycle(&myConsole);

        uint32_t pixels[64 * 32];
        for (int i = 0; i < 64 * 32; i++){
            if(myConsole.gfx[i] == 1){
                pixels[i] = 0xFFFFFFFF;
            } else{
                pixels[i] = 0x000000FF;
            }
        }

        SDL_UpdateTexture(
            texture,
            NULL,
            pixels, 
            64 * sizeof(uint32_t)
        );

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_Delay(2);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}