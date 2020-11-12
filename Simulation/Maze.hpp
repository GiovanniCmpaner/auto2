#pragma once

#include <vector>
#include <cstdint>

class Maze
{
    struct Tile
    {
        bool up, down;
        bool left, right;
    };

    struct Line
    {
        float x0, y0;
        float x1, y1;
    };

    struct Rect
    {
        float x, y;
        float width, height;
    };


    static auto make(size_t rows, size_t columns)->std::vector<std::vector<Tile>>;

    static auto print(const std::vector<std::vector<Tile>>& matrix) -> void;

    static auto lines(const std::vector<std::vector<Tile>>& matrix, float height, float width)->std::vector<Line>;

    static auto rectangles(const std::vector<std::vector<Tile>>& matrix, float x, float y, float height, float width, float thickness)->std::vector<Rect>;
public:
    auto init(b2World* world, b2Body* ground) -> void;

    auto step() -> void;
    auto render(GPU_Target* target) const -> void;

private:
    b2World* world{ nullptr };
    b2Body* ground{ nullptr };
    b2Body* body{ nullptr };

    static constexpr float realWidth{ 4.0f };
    static constexpr float realHeight{ 4.0f };
    static constexpr SDL_Color backgroundColor{ 0, 0, 0, 255 };
    static constexpr SDL_Color fontColor{ 0, 255, 0, 255 };
    static constexpr SDL_Color sensorColor{ 0,0,255,255 };
    static constexpr SDL_Color solidBorderColor{ 255, 0, 255, 255 };
    static constexpr SDL_Color solidFillColor{ 255, 0, 255, 64 };
};