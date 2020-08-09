/*This source code copyrighted by Lazy Foo' Productions (2004-2020)
and may not be redistributed without written permission.*/

//Using SDL and standard IO
#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include <cstdlib>
#include <iostream>

//Screen dimension constants
static constexpr auto screenWidth{ 640 };
static constexpr auto screenHeight{ 480 };

SDL_Window* window{ nullptr };
SDL_Renderer* renderer{ nullptr };
/*
std::unique_ptr<SDL_Window, void(*)(SDL_Window*)> window2{ nullptr, SDL_DestroyWindow };

auto createWindow(const std::string& name, int width, int height) -> std::unique_ptr<SDL_Window, void(*)(SDL_Window*)>
{
    const auto window{ SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN) };
    if (window == nullptr)
    {
        std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
    }
    return { window, SDL_DestroyWindow };
}
*/
bool init()
{
    //Initialization flag
    auto success{ true };

    //Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        success = false;
    }
    else
    {
        //Set texture filtering to linear
        if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
        {
            std::cout << "Warning: Linear texture filtering not enabled!" << std::endl;
        }

        //Create window
        window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);
        if (window == NULL)
        {
            std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
            success = false;
        }
        else
        {
            //Create renderer for window
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
            if (renderer == NULL)
            {
                std::cout << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
                success = false;
            }
            else
            {
                //Initialize renderer color
                SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
            }
        }
    }

    return success;
}

void close()
{
    //Destroy window	
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    window = NULL;
    renderer = NULL;

    //Quit SDL subsystems
    SDL_Quit();
}

int main(int argc, char* args[])
{
    //Initialize SDL
    if (not init())
    {
        std::cout << "Failed to initialize!" << std::endl;
    }
    else
    {
        auto x{ 50.0 };

        //Main loop flag
        auto quit{ false };
        auto previousTicks{ uint32_t{0} };
        while (not quit)
        {
            //Event handler
            auto e{ SDL_Event{} };
            while (SDL_PollEvent(&e) != 0)
            {
                //User requests quit
                if (e.type == SDL_QUIT)
                {
                    quit = true;
                }
            }

            //Calculate time step
            const auto currentTicks{ SDL_GetTicks() };
            const auto timeStep{ (currentTicks - previousTicks) / 1000.0 };
            previousTicks = currentTicks;

            //Clear screen
            SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
            SDL_RenderClear(renderer);

            /*
            //Render red filled quad
            const auto fillRect{ SDL_Rect{screenWidth / 4, screenHeight / 4, screenWidth / 2, screenHeight / 2} };
            SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
            SDL_RenderFillRect(renderer, &fillRect);

            //Render green outlined quad
            const auto outlineRect{ SDL_Rect{screenWidth / 6, screenHeight / 6, screenWidth * 2 / 3, screenHeight * 2 / 3 } };
            SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
            SDL_RenderDrawRect(renderer, &outlineRect);

            //Draw blue horizontal line
            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xFF, 0xFF);
            SDL_RenderDrawLine(renderer, 0, screenHeight / 2, screenWidth, screenHeight / 2);

            //Draw vertical line of yellow dots
            SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0x00, 0xFF);
            for (auto i{ 0 }; i < screenHeight; i += 4)
            {
                SDL_RenderDrawPoint(renderer, screenWidth / 2, i);
            }
            */

            circleRGBA(renderer, x, 50, 10, 0xFF, 0, 0, 0xFF);
            x += 20.0 * timeStep;

            //Update screen
            SDL_RenderPresent(renderer);
        }
    }

    close();

    return EXIT_SUCCESS;
}