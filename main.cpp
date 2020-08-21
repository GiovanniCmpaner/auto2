#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <thread>
#include <chrono>

#include <box2d/box2d.h>
#include <SDL_gpu.h>

#include "maze.hpp"

static int resizingEventWatcher(void* data, SDL_Event* event)
{
    return 0;
}

int main(int argc, char* args[])
{
    GPU_SetRequiredFeatures(GPU_FEATURE_BASIC_SHADERS);
    const auto target{ GPU_InitRenderer(GPU_RENDERER_OPENGL_3, 500, 500, GPU_DEFAULT_INIT_FLAGS) };
    //SDL_AddEventWatch(resizingEventWatcher, nullptr);

    //auto framerate{ FPSmanager{} };
    //SDL_initFramerate(&framerate);
    //SDL_setFramerate(&framerate, 60);

    const auto tiles{ Maze::make(5,5) };
    Maze::print(tiles);
    const auto rectangles{ Maze::rectangles(tiles, -10, -10, 20, 20, 0.5) };

    while (1)
    {
        // Update logic here

        GPU_Clear(target);

        // Draw stuff here

        GPU_MatrixMode(target, GPU_PROJECTION);
        GPU_LoadIdentity();
        GPU_Ortho(-20, +20, -20, +20, 0, 1); // Escala do mundo
        GPU_Translate(-10, 0, 0); // Translação da câmera

        for (const auto& rect : rectangles)
        {
            GPU_RectangleFilled(target, rect.x, rect.y, rect.x + rect.width, rect.y + rect.height, { 255, 0, 255, 255 });
        }

        GPU_Flip(target);

        SDL_Delay(16);
        //SDL_framerateDelay(&framerate);
    }

    GPU_Quit();

    return EXIT_SUCCESS;
}

