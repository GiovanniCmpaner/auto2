#include <stack>
#include <vector>
#include <random>
#include <utility>
#include <array>
#include <iostream>
#include <cstdint>
#include <cstdlib>


#include <box2d/box2d.h>
#include <SDL_gpu.h>
#include <SDL_FontCache.h>

#include "maze.hpp"

auto Maze::make(size_t rows, size_t columns) -> std::vector<std::vector<Tile>>
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

    matrix.front().front().up = false;
    matrix.back().back().down = false;

    return matrix;
}

auto Maze::print(const std::vector<std::vector<Tile>>& matrix) -> void
{
    std::cout << '\xDB';

    for (const auto& tile : matrix.front())
    {
        if (tile.up)
        {
            std::cout << '\xDB';
        }
        else
        {
            std::cout << ' ';
        }

        std::cout << '\xDB';
    }

    std::cout << std::endl;

    for (const auto& row : matrix)
    {
        if (row.front().left)
        {
            std::cout << '\xDB';
        }
        else {
            std::cout << ' ';
        }

        for (const auto& tile : row)
        {
            std::cout << ' ';

            if (tile.down and tile.right)
            {
                std::cout << '\xDB';
            }
            else if (tile.down)
            {
                std::cout << ' ';
            }
            else if (tile.right)
            {
                std::cout << '\xDB';
            }
            else {
                std::cout << ' ';
            }
        }

        std::cout << std::endl;

        std::cout << '\xDB';

        for (const auto& tile : row)
        {
            if (tile.down && tile.right)
            {
                std::cout << '\xDB';
            }
            else if (tile.down)
            {
                std::cout << '\xDB';
            }
            else if (tile.right)
            {
                std::cout << ' ';
            }
            else {
                std::cout << ' ';
            }

            std::cout << '\xDB';
        }

        std::cout << std::endl;
    }
}

auto Maze::lines(const std::vector<std::vector<Tile>>& matrix, float height, float width) -> std::vector<Line>
{
    auto lines{ std::vector<Line>{} };

    const auto tileHeight{ static_cast<float>(height / matrix.size()) };
    const auto tileWidth{ static_cast<float>(width / matrix.front().size()) };

    {
        auto make{ [&](float y, float currentHeight, bool Tile::* direction)
        {
            auto start{ 0.0f };
            auto end{ 0.0f };
            for (auto x{ 0 }; x < matrix.front().size(); x++)
            {
                if (matrix[y][x].*direction)
                {
                    end += tileWidth;
                }
                else
                {
                    if (end - start > 0.001f)
                    {
                        lines.emplace_back(Line{ start, currentHeight, end,  currentHeight });
                    }
                    start = end + tileWidth;
                    end = start;
                }
            }
            if (end - start > 0.001f)
            {
                lines.emplace_back(Line{ start, currentHeight, end, currentHeight });
            }
        }};

        make(0.0f, 0.0f, &Tile::up);

        for (auto y{ 0 }; y < matrix.size(); y++)
        {
            make(y, ( y + 1.0f) * tileHeight, &Tile::down);
        }
    }

    {
        auto make{ [&](float x, float currentWidth, bool Tile::* direction)
        {
            auto start{ 0.0f };
            auto end{ 0.0f };
            for (auto y{ 0 }; y < matrix.size(); y++)
            {
                if (matrix[y][x].*direction)
                {
                    end += tileHeight;
                }
                else
                {
                    if (end - start > 0.001f)
                    {
                        lines.emplace_back(Line{ currentWidth, start, currentWidth, end });
                    }
                    start = end + tileHeight;
                    end = start;
                }
            }
            if (end - start > 0.001f)
            {
                lines.emplace_back(Line{ currentWidth, start, currentWidth, end });
            }
        }};

        make(0.0f, 0.0f, &Tile::left);

        for (auto x{ 0 }; x < matrix.front().size(); x++)
        {
            make(x, ( x + 1.0f) * tileWidth, &Tile::right);
        }
    }

    return lines;
}

auto Maze::rectangles(const std::vector<std::vector<Tile>>& matrix, float x, float y, float height, float width, float thickness) -> std::vector<Rect>
{
    const auto lines{ Maze::lines(matrix,height,width) };
    auto rectangles{ std::vector<Rect>{} };
    rectangles.reserve(lines.size());

    for (const auto& line : lines)
    {
        rectangles.emplace_back(Rect{ x + line.x0, y + line.y0, line.x1 - line.x0 + thickness, line.y1 - line.y0 + thickness });
    }

    return rectangles;
}

auto Maze::init(b2World* world, b2Body* ground) -> void
{
    this->world = world;
    this->ground = ground;

    const auto tiles{ Maze::make(7,7) };
    const auto rectangles{ Maze::rectangles(tiles, -realWidth / 2.0f, -realHeight / 2.0f, realWidth, realHeight, 0.05f) };

    b2BodyDef bd{};
    bd.type = b2_staticBody;
    bd.position = b2Vec2{ 0.0f, 0.0f };
    bd.userData = const_cast<char*>("maze");

    this->body = world->CreateBody(&bd);

    for (const auto& rect : rectangles)
    {
        b2PolygonShape shape{};

        b2FixtureDef fd{};
        fd.shape = &shape;
        fd.density = 0.0f;
        fd.restitution = 0.4f;
        fd.filter.categoryBits = 0x0001;
        fd.filter.maskBits = 0x0003;
        fd.userData = const_cast<char*>("wall");

        shape.SetAsBox(rect.width / 2.0f, rect.height / 2.0f, b2Vec2{ rect.x + rect.width / 2.0f, rect.y + rect.height / 2.0f }, 0.0f);
        this->body->CreateFixture(&fd);
    }
}

auto Maze::step() -> void
{
    // Nada
}

auto Maze::render(GPU_Target* target) const -> void
{
    GPU_SetLineThickness(0.02f);

    for (auto fixture{ body->GetFixtureList() }; fixture != nullptr; fixture = fixture->GetNext())
    {
        const auto polygon{ reinterpret_cast<const b2PolygonShape*>(fixture->GetShape()) };

        float vertices[2 * b2_maxPolygonVertices];
        for (auto i{ 0 }; i < polygon->m_count; ++i)
        {
            const auto vertice{ this->body->GetWorldPoint(polygon->m_vertices[i]) };
            vertices[i * 2 + 0] = vertice.x;
            vertices[i * 2 + 1] = vertice.y;
        }

        GPU_Polygon(target, polygon->m_count, vertices, solidBorderColor);
        GPU_PolygonFilled(target, polygon->m_count, vertices, solidFillColor);
    }
}
