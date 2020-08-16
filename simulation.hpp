#pragma once

#include <SDL.h>
#include <SDL2_framerate.h>
#include <SDL_ttf.h>
#include <future>
#include <atomic>
#include <cstdint>

class Simulation
{
public:
    auto init() -> bool;
    auto end() -> void;
private:
    auto text(int x, int y, const std::string& texto) -> void;
    auto move() -> void;
    auto collisions() -> void;
    auto draw() -> void;
    auto input() -> void;
    auto clear() -> void;
    auto update() -> void;
    auto process() -> void;

    int screenWidth{ 500 };
    int screenHeight{ 500 };

    SDL_Window* window{ nullptr };
    SDL_Renderer* renderer{ nullptr };
    FPSmanager manager{};
    TTF_Font* font{ nullptr };

    std::future<void> task{};
    std::atomic<bool> quit{ false };
};