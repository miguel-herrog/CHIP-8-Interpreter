#include "chip8.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <SDL.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Use it from terminal: ./main.exe <ruta_a_la_rom>");
        return 1;
    }

    // Inicializar SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        SDL_Log("Error initializing SDL: %s", SDL_GetError());
        return 1;
    }

    // Crear la Ventana (1024x512, centrada)
    SDL_Window *window = SDL_CreateWindow(
        "Chip-8 Emulator",
        SDL_WINDOWPOS_CENTERED, // x
        SDL_WINDOWPOS_CENTERED, // y
        1024, 512,              // x, y
        SDL_WINDOW_SHOWN);

    if (!window)
    {
        SDL_Log("Error creating the window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Crear el Renderer (GPU)
    SDL_Renderer *renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED);

    if (!renderer)
    {
        SDL_Log("Error creating the renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Crear la Textura ( RGBA, Streaming, 64x32 nativos)
    SDL_Texture *texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        64, // Ancho nativo CHIP-8
        32  // Alto nativo CHIP-8
    );

    if (!texture)
    {
        SDL_Log("Error creating the texture: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // --- El Bucle Infinito ---
    struct Chip8 myConsole;
    initialize(&myConsole);
    if (load_rom(&myConsole, argv[1]) == 0)
    {
        SDL_Quit();
        return 1;
    }

    int quit = 0;
    SDL_Event event;
    uint32_t lastTimerTick = SDL_GetTicks();

    while (!quit)
    {
        // Procesar eventos del SO
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = 1;
            }
            else if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
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
            }
            else if (event.type == SDL_KEYUP)
            {
                switch (event.key.keysym.sym)
                {
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

        emulate_cycle(&myConsole);
        uint32_t currentTicks = SDL_GetTicks();

        if ((currentTicks - lastTimerTick) >= 16)
        {
            if (myConsole.delayTimer > 0)
            {
                myConsole.delayTimer -= 1;
            }
            if (myConsole.soundTimer > 0)
            {
                myConsole.soundTimer -= 1;
            }
            lastTimerTick = currentTicks;
        }

        uint32_t pixels[64 * 32];
        for (int i = 0; i < 64 * 32; i++)
        {
            if (myConsole.gfx[i] == 1)
            {
                pixels[i] = 0xFFFFFFFF;
            }
            else
            {
                pixels[i] = 0x000000FF;
            }
        }

        SDL_UpdateTexture(
            texture,
            NULL,
            pixels,
            64 * sizeof(uint32_t));

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