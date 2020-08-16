#pragma once

class Simulation
{
public:
    auto init() -> void;
    auto end() -> void;
private:
    const int screenWidth{ 500 };
    const int screenHeight{ 500 };

    SDL_Window* window{ nullptr };
    SDL_Renderer* renderer{ nullptr };
    FPSmanager manager{};
    TTF_Font* font{ nullptr };

    std::future<void> task{};
    std::atomic<bool> quit{ false };
};