#include <stack>
#include <vector>
#include <random>
#include <utility>
#include <array>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <functional>

#include <box2d/box2d.h>
#include <SDL_gpu.h>
#include <SDL_FontCache.h>

#include "maze.hpp"
#include "..\Draw.hpp"

auto Maze::make(size_t rows, size_t columns) -> Matrix
{
    auto matrix{ std::vector(rows,std::vector(columns,Tile{true,true,true,true})) };

    auto tracking{ std::deque<std::tuple<int,int>>{} };
    
    auto rd{ std::random_device{} };
    auto mt{ std::mt19937{rd()} };
    //auto mt{ std::mt19937{999} };

    auto j{ 0 }, i{ 0 };

    tracking.emplace_back(j, i);
    while (1)
    {
        auto directions{ std::vector<std::tuple<int,int>>{} };

        const auto unvisited{ [&](int dj, int di) -> bool
        {
            return matrix[j + dj][i + di].up
                && matrix[j + dj][i + di].down
                && matrix[j + dj][i + di].left
                && matrix[j + dj][i + di].right;
        } };

        if (j > 0 and unvisited(-1, 0))
        {
            directions.emplace_back(-1, 0);
        }
        if (j < rows - 1 and unvisited(+1, 0))
        {
            directions.emplace_back(+1, 0);
        }
        if (i > 0 and unvisited(0, -1))
        {
            directions.emplace_back(0, -1);
        }
        if (i < columns - 1 and unvisited(0, +1))
        {
            directions.emplace_back(0, +1);
        }

        if (directions.size() > 0)
        {
            auto dist{ std::uniform_int_distribution<int>{ 0, static_cast<int>(directions.size()) - 1} };
            const auto choosen{ dist(mt) };
            const auto [dj, di] { directions[choosen] };

            if (dj == +1)
            {
                matrix[j][i].down = false;
                matrix[j + 1][i].up = false;
            }
            else if (dj == -1)
            {
                matrix[j][i].up = false;
                matrix[j - 1][i].down = false;
            }
            else if (di == +1)
            {
                matrix[j][i].right = false;
                matrix[j][i + 1].left = false;
            }
            else if (di == -1)
            {
                matrix[j][i].left = false;
                matrix[j][i - 1].right = false;
            }

            j += dj;
            i += di;

            tracking.emplace_back(j, i);
        }
        else
        {
            tracking.pop_back();
            if (tracking.empty())
            {
                break;
            }
            std::tie(j, i) = tracking.back();
        }
    }

    return matrix;
}

auto Maze::solve(const Matrix& matrix, int y, int x, bool bestSolution)->Path
{
    auto solution{ Path{} };

    auto visited{ std::vector(matrix.size(), std::vector(matrix.front().size(), false)) };
    auto tracking{ std::deque<std::tuple<int,int,int>>{} };

    tracking.emplace_back(0, y, x);

    while (not tracking.empty())
    {
        auto& [dir, j, i] { tracking.back() };

        if (bestSolution)
        {
            if (j == 0 and i == 0)
            {
                while (1)
                {
                    auto& [dir, j, i] { tracking.back() };
                    solution.emplace(solution.begin(), Coordinate{ i, j });
                    if (j == y and i == x)
                    {
                        break;
                    }
                    tracking.pop_back();
                }
                break;
            }
        }
        else
        {
            solution.emplace_back(Coordinate{ i, j });

            if (j == 0 and i == 0)
            {
                break;
            }
        }

        if (dir == 0) // UP
        {
            if (j - 1 >= 0 and not matrix[j][i].up and not visited[j - 1][i])
            {
                visited[j - 1][i] = true;
                tracking.emplace_back(0, j - 1, i);
            }
        }
        else if (dir == 1) // LEFT
        {
            if (i - 1 >= 0 and not matrix[j][i].left and not visited[j][i - 1])
            {
                visited[j][i - 1] = true;
                tracking.emplace_back(0, j, i - 1);
            }
        }
        else if (dir == 2) // DOWN
        {
            if (j + 1 < matrix.size() and not matrix[j][i].down and not visited[j + 1][i])
            {
                visited[j + 1][i] = true;
                tracking.emplace_back(0, j + 1, i);
            }
        }
        else if (dir == 3) // RIGHT
        {
            if (i + 1 < matrix.front().size() and not matrix[j][i].right and not visited[j][i + 1])
            {
                visited[j][i + 1] = true;
                tracking.emplace_back(0, j, i + 1);
            }
        }
        else
        {
            visited[j][i] = false;
            tracking.pop_back();
        }

        dir++;
    }

    return solution;
}

auto Maze::print(const Matrix& matrix, const Path& path) -> void
{
    static constexpr auto square{ '\xDB' };
    static constexpr auto free{ ' ' };
    static constexpr auto occupied{ 'X' };

    std::cout << square;

    {
        auto y{ 0 };

        for (auto x{ 0 }; x < matrix[y].size(); ++x)
        {
            if (matrix[y][x].up)
            {
                std::cout << square;
            }
            else
            {
                std::cout << free;
            }

            std::cout << square;
        }

        std::cout << std::endl;
    }

    for (auto y{ 0 }; y < matrix.size(); ++y)
    {
        if (matrix[y].front().left)
        {
            std::cout << square;
        }
        else
        {
            std::cout << free;
        }

        for (auto x{ 0 }; x < matrix[y].size(); ++x)
        {
            auto found{ false };
            for (auto&& coordinate : path)
            {
                if (coordinate.y == y == coordinate.x == x)
                {
                    found = true;
                    break;
                }
            }
            if (found)
            {
                std::cout << occupied;
            }
            else
            {
                std::cout << free;
            }

            if (matrix[y][x].down and matrix[y][x].right)
            {
                std::cout << square;
            }
            else if (matrix[y][x].down)
            {
                std::cout << free;
            }
            else if (matrix[y][x].right)
            {
                std::cout << square;
            }
            else
            {
                std::cout << free;
            }
        }

        std::cout << std::endl;

        std::cout << square;

        for (auto x{ 0 }; x < matrix[y].size(); ++x)
        {
            if (matrix[y][x].down && matrix[y][x].right)
            {
                std::cout << square;
            }
            else if (matrix[y][x].down)
            {
                std::cout << square;
            }
            else if (matrix[y][x].right)
            {
                std::cout << free;
            }
            else
            {
                std::cout << free;
            }

            std::cout << square;
        }

        std::cout << std::endl;
    }
}

auto Maze::lines(const Matrix& matrix, float height, float width) -> std::vector<Line>
{
    auto lines{ std::vector<Line>{} };

    const auto tileHeight{ height / matrix.size() };
    const auto tileWidth{ width / matrix.front().size() };

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
        } };

        make(0.0f, 0.0f, &Tile::up);

        for (auto y{ 0 }; y < matrix.size(); y++)
        {
            make(y, (y + 1.0f) * tileHeight, &Tile::down);
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
        } };

        make(0.0f, 0.0f, &Tile::left);

        for (auto x{ 0 }; x < matrix.front().size(); x++)
        {
            make(x, (x + 1.0f) * tileWidth, &Tile::right);
        }
    }

    return lines;
}

auto Maze::polygons(const Matrix& matrix, float x, float y, float width, float height, float thickness) -> std::vector<Polygon>
{
    const auto lines{ Maze::lines(matrix,height,width) };
    auto polygons{ std::vector<Polygon>{} };
    polygons.reserve(lines.size());

    for (const auto& line : lines)
    {
        const auto x0{ x + line.x0 };
        const auto y0{ y + line.y0 };
        const auto width{ line.x1 - line.x0 + thickness };
        const auto height{ line.y1 - line.y0 + thickness };

        polygons.emplace_back(Polygon{{
            {x0, y0},
            {x0 + width, y0},
            {x0 + width, y0 + height},
            {x0, y0 + height}
        }});
    }


    return polygons;
}

Maze::Maze(b2World* world, b2Body* ground, size_t columns, size_t rows, float x, float y, float width, float height)
{
    this->world = world;
    this->ground = ground;
    this->x = x;
    this->y = y;
    this->height = height;
    this->width = width;
    this->tileHeight = height / rows;
    this->tileWidth = width / columns;
    this->matrix = Maze::make(rows, columns);
    this->createBody();
}

Maze::Maze(const Maze& other)
{
    this->world = other.world;
    this->ground = other.ground;
    this->x = other.x;
    this->y = other.y;
    this->height = other.height;
    this->width = other.width;
    this->tileHeight = other.tileHeight;
    this->tileWidth = other.tileWidth;
    this->matrix = other.matrix;
    this->createBody();
}

Maze::~Maze()
{
    this->world->DestroyBody(this->body);
}

auto Maze::randomize() -> void
{
    this->matrix = Maze::make(this->rows(), this->columns());
    this->createBody();
}

auto Maze::createBody() -> void
{
    if (this->body != nullptr)
    {
        this->world->DestroyBody(this->body);
    }

    b2BodyDef bd{};
    bd.type = b2_staticBody;
    bd.position = b2Vec2{ this->x, this->y };
    bd.userData = const_cast<char*>("maze");

    this->body = this->world->CreateBody(&bd);

    const auto polygons{ Maze::polygons(this->matrix, 0, 0, this->width, this->height, 0.05f) };
    for (const auto& poly : polygons)
    {
        b2PolygonShape shape{};
    
        b2FixtureDef fd{};
        fd.shape = &shape;
        fd.density = 0.0f;
        fd.restitution = 0.1f;
        fd.filter.categoryBits = 0x0001;
        fd.filter.maskBits = 0x0003;
        fd.userData = const_cast<char*>("wall");
    
        shape.Set(reinterpret_cast<const b2Vec2*>(poly.vertices.data()), poly.vertices.size());
        this->body->CreateFixture(&fd);
    }

    //{
    //    b2PolygonShape shape{};
    //
    //    b2FixtureDef fd{};
    //    fd.shape = &shape;
    //    fd.density = 0.0f;
    //    fd.restitution = 0.4f;
    //    fd.filter.categoryBits = 0x0001;
    //    fd.filter.maskBits = 0x0003;
    //    fd.userData = const_cast<char*>("wall");
    //    
    //    shape.SetAsBox(0.05f / 2.0f, this->height / 2.0f, { -this->width, this->height / 2.0f }, 0.0f);
    //    this->body->CreateFixture(&fd);
    //
    //    shape.SetAsBox(0.05f / 2.0f, this->height / 2.0f, { +this->width, this->height / 2.0f }, 0.0f);
    //    this->body->CreateFixture(&fd);
    //
    //    shape.SetAsBox(this->width / 2.0f, 0.05f / 2.0f, { this->width / 2.0f, -this->height }, 0.0f);
    //    this->body->CreateFixture(&fd);
    //    
    //    shape.SetAsBox(this->width / 2.0f, 0.05f / 2.0f, { this->width / 2.0f, +this->height }, 0.0f);
    //    this->body->CreateFixture(&fd);
    //}

    {
        //b2BodyDef bd{};
        //bd.type = b2_staticBody;
        //bd.position = b2Vec2{ this->width / 2.0f, this->height / 2.0f };
        //bd.userData = const_cast<char*>("obstacle");
        //
        //auto body{ this->world->CreateBody(&bd) };

        b2CircleShape shape{};
        
        b2FixtureDef fd{};
        fd.shape = &shape;
        fd.density = 0.0f;
        fd.restitution = 0.1f;
        fd.filter.categoryBits = 0x0001;
        fd.filter.maskBits = 0x0003;
        fd.userData = const_cast<char*>("circle");
        
        shape.m_radius = 0.1f;
        shape.m_p = { this->width / 2.0f, this->height / 2.0f };
        this->body->CreateFixture(&fd);
    }

    {
        b2CircleShape shape{};

        b2FixtureDef fd{};
        fd.shape = &shape;
        fd.isSensor = true;
        fd.filter.categoryBits = 0x0002;
        fd.filter.maskBits = 0x0001;

        shape.m_radius = std::min(this->tileHeight, this->tileWidth) / 4;

        shape.m_p = this->start();
        shape.m_p -= { this->x, this->y };
        fd.userData = const_cast<char*>("start");
        this->body->CreateFixture(&fd);

        shape.m_p = this->end();
        shape.m_p -= { this->x, this->y };
        fd.userData = const_cast<char*>("end");
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
    
    Draw::draw(target, this->body);
}

auto Maze::start() const->b2Vec2
{
    return this->toRealPoint({ static_cast<int>(this->columns() - 1), static_cast<int>(this->rows() - 1) });
}

auto Maze::end() const->b2Vec2
{
    return this->toRealPoint({ 0,0 });
}

auto Maze::solve(const b2Vec2& point, bool bestSolution) const->std::vector<b2Vec2>
{
    auto path{ std::vector<b2Vec2>{} };

    const auto coordinate{ this->toLocalCoordinate(point) };
    if (coordinate.x < 0 or coordinate.y < 0 or coordinate.y >= matrix.size() or coordinate.x >= matrix.front().size())
    {
        return {};
    }

    path.emplace_back(point.x, point.y);

    const auto solution{ Maze::solve(this->matrix, coordinate.y, coordinate.x, bestSolution) };
    for (auto&& coordinate : solution)
    {
        const auto point{ this->toRealPoint(coordinate) };
        path.emplace_back(point.x, point.y);
    }
    return path;
}

auto Maze::toLocalCoordinate(const b2Vec2& point) const->Coordinate
{
    const auto y{ static_cast<int>( ( point.y - this->y ) / this->tileHeight ) };
    const auto x{ static_cast<int>( ( point.x - this->x ) / this->tileWidth ) };

    return Coordinate{ x, y };
}

auto Maze::toRealPoint(const Coordinate& coordinate) const->b2Vec2
{
    const auto y{ static_cast<float>( coordinate.y * this->tileHeight + this->tileHeight / 2.0f + this->y ) };
    const auto x{ static_cast<float>( coordinate.x * this->tileWidth + this->tileWidth / 2.0f + this->x ) };

    return b2Vec2{ x, y };
}

auto Maze::rows() const->size_t
{
    return this->matrix.size();
}

auto Maze::columns() const->size_t
{
    return ( this->matrix.empty() ? 0 : this->matrix.front().size() );
}