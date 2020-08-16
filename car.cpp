#include <cmath>
#include <algorithm>

#include "car.hpp"

auto Car::init() -> void
{

}

auto Car::move(int horizontal, int vertical, int angle) -> void
{
    front = 9999.0;
    left = 9999.0;
    right = 9999.0;

    this->angle = ( this->angle + 1 % 360 );
}

auto Car::measure(const SDL_Rect* rect) -> void
{
    {
        auto x1{ px }, y1{ py };
        auto x2{ px + static_cast<int>(front * std::cos((angle + 0) * M_PI / 180.0)) }, y2{ py + static_cast<int>(front * std::sin((angle + 0) * M_PI / 180.0)) };
        if (SDL_IntersectRectAndLine(rect, &x1, &y1, &x2, &y2))
        {
            front = std::sqrt(std::pow(x1 - px, 2) + std::pow(y1 - py, 2));
        }
        
    }
    {
        auto x1{ px }, y1{ py };
        auto x2{ px + static_cast<int>(left * std::cos((angle + 90) * M_PI / 180.0)) }, y2{ py + static_cast<int>(left * std::sin((angle + 90) * M_PI / 180.0)) };
        if (SDL_IntersectRectAndLine(rect, &x1, &y1, &x2, &y2))
        {
            left = std::sqrt(std::pow(x1 - px, 2) + std::pow(y1 - py, 2));
        }
    }
    {
        auto x1{ px }, y1{ py };
        auto x2{ px + static_cast<int>(right * std::cos((angle - 90) * M_PI / 180.0)) }, y2{ py + static_cast<int>(right * std::sin((angle - 90) * M_PI / 180.0)) };
        if (SDL_IntersectRectAndLine(rect, &x1, &y1, &x2, &y2))
        {
            right = std::sqrt(std::pow(x1 - px, 2) + std::pow(y1 - py, 2));
        }
    }
}

auto Car::render(SDL_Renderer* renderer) const -> void
{
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xFF, 0xFF);
    SDL_RenderDrawLine(renderer, px, py, px + front * std::cos((angle + 0) * M_PI / 180.0), py + front * std::sin((angle + 0) * M_PI / 180.0));
    SDL_RenderDrawLine(renderer, px, py, px + left * std::cos((angle + 90) * M_PI / 180.0), py + left * std::sin((angle + 90) * M_PI / 180.0));
    SDL_RenderDrawLine(renderer, px, py, px + right * std::cos((angle - 90) * M_PI / 180.0), py + right * std::sin((angle - 90) * M_PI / 180.0));
}

auto Car::distance() const->Distance
{
    return {front,left,right};
}

auto Car::end() -> void
{

}