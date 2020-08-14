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

    static std::vector<Maze::Rect> rectangles{};

    namespace Video
    {
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

        static auto process() -> void
        {
            static auto ang{ 90 };
            static auto px{ 100 };
            static auto py{ 100 };

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
                for (const auto& rect : rectangles)
                {
                    SDL_RenderFillRect(renderer, reinterpret_cast<const SDL_Rect*>(&rect));
                };

                //-------------------------------------------------------------------------------------------------------------------------------
                // Colision 
                SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xFF, 0xFF);
                int x1{ px }, y1{ py }, x2{ static_cast<int>(999.0 * cos(ang * M_PI / 180.0)) }, y2{ static_cast<int>(999.0 * sin(ang * M_PI / 180.0)) };
                for (const auto& rect : rectangles)
                {
                    if (SDL_IntersectRectAndLine(reinterpret_cast<const SDL_Rect*>(&rect), &x1, &y1, &x2, &y2))
                    {
                        x2 = x1;
                        y2 = y1;
                        x1 = px;
                        y1 = py;
                    }
                }
                SDL_RenderDrawLine(renderer, x1, y1, x2, y2);

                ang = (ang + 5) % 360;
                const auto distance{ std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1,2)) };
                text(0, 0, "d = " + std::to_string(distance));
                text(200, 0, "fps = " + std::to_string(SDL_getFramerate(&manager)));
                //-------------------------------------------------------------------------------------------------------------------------------

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

    }

    auto init() -> void
    {
        const auto maze{ Maze::make(30,30) };
        Maze::print(maze);
        rectangles = Maze::rectangles(maze, 10, 50, 400, 400, 3);

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