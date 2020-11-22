#pragma once

#include <box2d/box2d.h>
#include <SDL_gpu.h>

#include <vector>
#include <cstdint>

class Maze
{
public:
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

    struct Coordinate
    {
        int x, y;
    };

    using Matrix = std::vector<std::vector<Tile>>;
    using Path = std::vector<Coordinate>;

    auto init(b2World* world, b2Body* ground, size_t rows, size_t columns, float x, float y, float height, float width) -> void;
    auto step() -> void;
    auto render(GPU_Target* target) const -> void;

    auto start() const->b2Vec2;
    auto end() const->b2Vec2;
    auto solve(const b2Vec2& point) const->std::vector<b2Vec2>;

    auto randomize() -> void;

private:
    auto createBody() -> void;
    auto toLocalCoordinate(const b2Vec2& point) const->Coordinate;
    auto toRealPoint(const Coordinate& coordinate) const->b2Vec2;
    auto rows() const->size_t;
    auto columns() const->size_t;

    static auto make(size_t rows, size_t columns)->Matrix;
    static auto solve(const Matrix& matrix, int y, int x)->Path;
    static auto lines(const Matrix& matrix, float height, float width)->std::vector<Line>;
    static auto rectangles(const Matrix& matrix, float x, float y, float height, float width, float thickness)->std::vector<Rect>;
    static auto print(const Matrix& matrix, const Path& path = {}) -> void;

    b2World* world{ nullptr };
    b2Body* ground{ nullptr };
    b2Body* body{ nullptr };
   
    float x{ 0.0f };
    float y{ 0.0f };
    float width{ 0.0f };
    float height{ 0.0f };
    Matrix matrix{};
    
    static constexpr SDL_Color backgroundColor{ 0, 0, 0, 255 };
    static constexpr SDL_Color fontColor{ 0, 255, 0, 255 };
    static constexpr SDL_Color sensorColor{ 0,0,255,255 };
    static constexpr SDL_Color solidBorderColor{ 255, 0, 255, 255 };
    static constexpr SDL_Color solidFillColor{ 255, 0, 255, 64 };
};