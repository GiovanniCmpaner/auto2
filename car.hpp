#pragma once

#include <SDL.h>

class Distance {
    double front, left, right;
};

class Car {
public:
    auto init() -> void;
    auto move(int horizontal, int vertical, int angle) -> void;
    auto collision(SDL_Rect* rect) const -> void;
    auto render(SDL_Renderer* renderer) const -> void;
    auto distance() const->Distance;
    auto end() -> void;
private:
    int px, py;
    int vx, vy;
    int ax, ay;
    int height, width;
    SDL_Texture* texture;
};