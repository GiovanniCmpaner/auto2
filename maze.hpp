#pragma once

#include <vector>
#include <cstdint>

namespace Maze
{
    struct Tile {
        bool up;
        bool down;
        bool left;
        bool right;
    };

    struct Line {
        int x0;
        int y0;
        int x1;
        int y1;
    };

    auto make(size_t rows, size_t columns)->std::vector<std::vector<Tile>>;

    auto print(const std::vector<std::vector<Tile>>& matrix) -> void;

    auto lines(const std::vector<std::vector<Tile>>& matrix, int height, int width)->std::vector<Line>;
}