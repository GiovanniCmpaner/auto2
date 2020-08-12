#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL2_gfxPrimitives.h>
#include <SDL2_framerate.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <future>

#include "simulation.hpp"

namespace Simulation
{
    static constexpr auto screenWidth{ 640 };
    static constexpr auto screenHeight{ 480 };

    static SDL_Window* window{ nullptr };
    static SDL_Renderer* renderer{ nullptr };
    static FPSmanager manager{};
    static TTF_Font* font{ nullptr };


    static std::future<void> task{};
    static std::atomic<bool> quit{ false };

    auto init() -> void
    {
        Video::init();
    }

    auto end() -> void
    {
        Video::end();
    }

    auto distance_front() -> double
    {

    }

    namespace Video
    {
        static auto init() -> bool
        {
            // Initialize SDL for video
            if (SDL_Init(SDL_INIT_VIDEO) != 0)
            {
                std::cerr << "SDL could not initialize! Error: " << SDL_GetError() << std::endl;
                return false;
            }

            // Set texture filtering to linear
            if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
            {
                std::cerr << "Warning: Linear texture filtering not enabled!" << std::endl;
                return false;
            }

            // Create window
            window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);
            if (window == nullptr)
            {
                std::cerr << "Window could not be created! Error: " << SDL_GetError() << std::endl;
                return false;
            }

            // Create renderer for window
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
            if (renderer == nullptr)
            {
                std::cerr << "Renderer could not be created! Error: " << SDL_GetError() << std::endl;
                return false;
            }

            SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
            SDL_initFramerate(&manager);
            SDL_setFramerate(&manager, 60);

            // Initialize TTF for fonts
            if (TTF_Init() != 0)
            {
                std::cerr << "TTF could not initialize! Error: " << TTF_GetError() << std::endl;
                return false;
            }

            // Open font
            font = TTF_OpenFont("C:\\Windows\\Fonts\\Arial.ttf", 16);
            if (font == nullptr)
            {
                std::cerr << "Failed to open font! Error: " << TTF_GetError() << std::endl;
                return false;
            }

            quit = false;
            task = std::async(std::launch::async, process);
        }

        static auto end() -> void
        {
            quit = true;
            task.wait();

            SDL_DestroyRenderer(renderer);
            renderer = nullptr;

            SDL_DestroyWindow(window);
            window = nullptr;

            TTF_CloseFont(font);
            font = nullptr;

            SDL_Quit();
            TTF_Quit();
        }

        static auto process() -> void
        {
            auto event{ SDL_Event{} };
            while (not quit) 
            {
                //Event handler
                while (SDL_PollEvent(&event) != 0)
                {
                    //User requests quit
                    if (event.type == SDL_QUIT)
                    {
                        quit = true;
                    }
                }

                const auto framerate{ SDL_getFramerate(&manager) };

                //Clear screen
                SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
                SDL_RenderClear(renderer);

                // Draw

                // Update screen
                SDL_RenderPresent(renderer);
                SDL_framerateDelay(&manager);
            }
        }

        static auto text(int x, int y, const std::string& texto) -> void
        {
            auto rect{ SDL_Rect{x,y,0,0} };
            TTF_SizeText(font, texto.c_str(), &rect.w, &rect.h);

            if (const auto surface{ TTF_RenderText_Solid(font, texto.c_str(), { 0, 0, 0 }) }; surface == nullptr)
            {
                std::cerr << "Failed to render text! Error: " << TTF_GetError() << std::endl;
            }
            else if (const auto texture{ SDL_CreateTextureFromSurface(renderer, surface) }; texture == nullptr)
            {
                std::cerr << "Failed to create texture! Error: " << SDL_GetError() << std::endl;
            }
            else
            {
                SDL_RenderCopy(renderer, texture, nullptr, &rect);
            }
        }
    }
}