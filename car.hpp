#pragma once

#include <SDL.h>

struct Distance {
    double front, left, right;
};

class Car {
public:
    auto init() -> void;
    auto move(int horizontal, int vertical, int angle) -> void;
    auto measure(const SDL_Rect* rects, size_t count) -> void;
    auto render(SDL_Renderer* renderer) const -> void;
    auto distance() const->Distance;
    auto end() -> void;
private:
    SDL_Rect rect{ 100, 100, 20, 20 };
    SDL_Texture* texture{ nullptr };

    int vx{ 0 }, vy{ 0 };
    int ax{ 0 }, ay{ 0 };
    int angle{ 0 };
    double front{ 0.0 }, left{ 0.0 }, right{ 0.0 };
};