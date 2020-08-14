#include <stack>
#include <vector>
#include <random>
#include <utility>
#include <array>
#include <iostream>
#include <cstdint>
#include <cstdlib>

#include "maze.hpp"

namespace Maze
{
    auto make(size_t rows, size_t columns) -> std::vector<std::vector<Tile>>
    {
        auto stack{ std::stack<std::tuple<int,int>>{} };
        auto matrix{ std::vector(rows,std::vector(columns,Tile{true,true,true,true})) };

        auto rd{ std::random_device{} };
        auto mt{ std::mt19937{rd()} };

        auto y{ 0 }, x{ 0 };

        stack.emplace(y, x);
        while (1)
        {
            auto directions{ std::vector<std::tuple<int,int>>{} };

            const auto unvisited{ [&](int j, int i) -> bool
            {
                return matrix[y + j][x + i].up
                    && matrix[y + j][x + i].down
                    && matrix[y + j][x + i].left
                    && matrix[y + j][x + i].right;
            } };

            if (y > 0 and unvisited(-1, 0))
            {
                directions.emplace_back(-1, 0);
            }
            if (y < rows - 1 and unvisited(+1, 0))
            {
                directions.emplace_back(+1, 0);
            }
            if (x > 0 and unvisited(0, -1))
            {
                directions.emplace_back(0, -1);
            }
            if (x < columns - 1 and unvisited(0, +1))
            {
                directions.emplace_back(0, +1);
            }

            if (directions.size() > 0)
            {
                auto dist{ std::uniform_int_distribution<int>{ 0, static_cast<int>(directions.size()) - 1} };
                const auto choosen{ dist(mt) };
                const auto [j, i] { directions[choosen] };

                if (j == +1) {
                    matrix[y][x].down = false;
                    matrix[y + 1][x].up = false;
                }
                else if (j == -1) {
                    matrix[y][x].up = false;
                    matrix[y - 1][x].down = false;
                }
                else if (i == +1) {
                    matrix[y][x].right = false;
                    matrix[y][x + 1].left = false;
                }
                else if (i == -1) {
                    matrix[y][x].left = false;
                    matrix[y][x - 1].right = false;
                }

                y += j;
                x += i;

                stack.emplace(y, x);
            }
            else
            {
                stack.pop();
                if (stack.empty())
                {
                    break;
                }
                std::tie(y, x) = stack.top();
            }
        }

        return matrix;
    }

    auto print(const std::vector<std::vector<Tile>>& matrix) -> void
    {
        std::cout << std::string(matrix.front().size() * 3 - matrix.front().size() + 1, '\xDB') << std::endl;

        for (const auto& row : matrix)
        {
            std::cout << '\xDB';

            for (const auto& tile : row)
            {
                if (tile.down and tile.right)
                {
                    std::cout << " \xDB";
                }
                else if (tile.down)
                {
                    std::cout << "  ";
                }
                else if (tile.right)
                {
                    std::cout << " \xDB";
                }
                else {
                    std::cout << "  ";
                }
            }

            std::cout << std::endl;

            std::cout << '\xDB';

            for (const auto& tile : row)
            {
                if (tile.down && tile.right)
                {
                    std::cout << "\xDB\xDB";
                }
                else if (tile.down)
                {
                    std::cout << "\xDB\xDB";
                }
                else if (tile.right)
                {
                    std::cout << " \xDB";
                }
                else {
                    std::cout << " \xDB";
                }
            }

            std::cout << std::endl;
        }
    }

    auto lines(const std::vector<std::vector<Tile>>& matrix, int height, int width) -> std::vector<Line>
    {
        auto lines{ std::vector<Line>{} };

        lines.emplace_back(Line{ 0, 0, height, 0 });
        lines.emplace_back(Line{ 0, 0, 0, width });
        lines.emplace_back(Line{ 0, width, height, width });
        lines.emplace_back(Line{ height, 0, height, width });

        const auto tileHeight{ static_cast<int>(height / matrix.size()) };
        const auto tileWidth{ static_cast<int>(width / matrix.front().size()) };

        for (auto y{ 1 }; y < matrix.size(); y++)
        {
            const auto currentHeight{ y * tileHeight };

            auto start{ 0 };
            auto end{ 0 };
            for (auto x{ 0 }; x < matrix.front().size(); x++)
            {
                if (matrix[y][x].up)
                {
                    end += tileWidth;
                }
                else
                {
                    if (start != end)
                    {
                        lines.emplace_back(Line{ start, currentHeight, end,  currentHeight });
                    }
                    start = end + tileWidth;
                    end = start;
                }
            }
            if (start != end)
            {
                lines.emplace_back(Line{ start, currentHeight, end, currentHeight });
            }
        }


        for (auto x{ 1 }; x < matrix.front().size(); x++)
        {
            const auto currentWidth{ x * tileWidth };

            auto start{ 0 };
            auto end{ 0 };
            for (auto y{ 0 }; y < matrix.size(); y++)
            {
                if (matrix[y][x].left)
                {
                    end += tileHeight;
                }
                else
                {
                    if (start != end)
                    {
                        lines.emplace_back(Line{ currentWidth, start, currentWidth, end });
                    }
                    start = end + tileHeight;
                    end = start;
                }
            }
            if (start != end)
            {
                lines.emplace_back(Line{ currentWidth, start, currentWidth, end });
            }
        }

        return lines;
    }
}