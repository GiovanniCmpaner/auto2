#pragma once

#include <vector>
#include <cstdint>

namespace Maze
{
    struct Tile {
        bool up, down;
        bool left, right;
    };

    struct Line {
        float x0, y0;
        float x1, y1;
    };

    struct Rect {
        float x, y;
        float width, height;
    };

    auto make(size_t rows, size_t columns)->std::vector<std::vector<Tile>>;

    auto print(const std::vector<std::vector<Tile>>& matrix) -> void;

    auto lines(const std::vector<std::vector<Tile>>& matrix, float height, float width)->std::vector<Line>;

    auto rectangles(const std::vector<std::vector<Tile>>& matrix, float x, float y, float height, float width, float thickness)->std::vector<Rect>;
}