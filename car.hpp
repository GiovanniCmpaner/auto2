#pragma once

#include <SDL.h>

struct Distance {
    double front, left, right;
};

class Car {
public:
    auto init() -> void;
    auto move(int horizontal, int vertical, int angle) -> void;
    auto measure(const SDL_Rect* rect) -> void;
    auto render(SDL_Renderer* renderer) const -> void;
    auto distance() const->Distance;
    auto end() -> void;
private:
    int px{ 0 }, py{ 0 };
    int vx{ 0 }, vy{ 0 };
    int ax{ 0 }, ay{ 0 };
    int angle{ 0 };
    int height{ 0 }, width{ 0 };
    SDL_Texture* texture{ nullptr };
    double front{ 0.0 }, left{ 0.0 }, right{ 0.0 };
};