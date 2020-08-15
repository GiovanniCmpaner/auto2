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
        int x0, y0;
        int x1, y1;
    };

    struct Rect {
        int x, y;
        int width, height;
    };

    auto make(size_t rows, size_t columns)->std::vector<std::vector<Tile>>;

    auto print(const std::vector<std::vector<Tile>>& matrix) -> void;

    auto lines(const std::vector<std::vector<Tile>>& matrix, int height, int width)->std::vector<Line>;

    auto rectangles(const std::vector<std::vector<Tile>>& matrix, int x, int y, int height, int width, int thickness)->std::vector<Rect>;
}