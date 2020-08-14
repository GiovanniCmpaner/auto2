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
#include "maze.hpp"

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

    static std::vector<Maze::Line> lines{};

    namespace Video
    {
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
                SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
                for (const auto& line : lines)
                {
                    //thickLineRGBA(renderer, line.x0, line.y0, line.x1, line.y1, 3, 0xFF, 0x00, 0x00, 0xFF);
                    const auto rect{ SDL_Rect{line.x0, line.y0, line.x1 - line.x0 + 6, line.y1 - line.y0 + 6} };
                    SDL_RenderFillRect(renderer, &rect);
                }

                // Update screen
                SDL_RenderPresent(renderer);
                SDL_framerateDelay(&manager);
            }
        }

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
            if (window == nullptr)
            {
                window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);
                if (window == nullptr)
                {
                    std::cerr << "Window could not be created! Error: " << SDL_GetError() << std::endl;
                    return false;
                }
            }

            // Create renderer for window
            if (renderer == nullptr)
            {
                renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
                if (renderer == nullptr)
                {
                    std::cerr << "Renderer could not be created! Error: " << SDL_GetError() << std::endl;
                    return false;
                }
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
            if (font == nullptr)
            {
                font = TTF_OpenFont("C:\\Windows\\Fonts\\Arial.ttf", 16);
                if (font == nullptr)
                {
                    std::cerr << "Failed to open font! Error: " << TTF_GetError() << std::endl;
                    return false;
                }
            }

            if(not task.valid())
            {
                quit = false;
                task = std::async(std::launch::async, process);
            }
        }

        static auto end() -> void
        {
            if (task.valid())
            {
                quit = true;
                task.wait();
                task = {};
            }

            if (renderer != nullptr)
            {
                SDL_DestroyRenderer(renderer);
                renderer = nullptr;
            }

            if (window != nullptr)
            {
                SDL_DestroyWindow(window);
                window = nullptr;
            }

            if (font != nullptr)
            {
                TTF_CloseFont(font);
                font = nullptr;
            }

            SDL_Quit();
            TTF_Quit();
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

    auto init() -> void
    {
        const auto maze{ Maze::make(5,5) };
        Maze::print(maze);
        lines = Maze::lines(maze, 200, 200);

        Video::init();
    }

    auto end() -> void
    {
        Video::end();
    }

    auto distance_front() -> double
    {
        return 0.0;
    }
}