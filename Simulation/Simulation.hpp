#pragma once

#include <vector>

#include "Window.hpp"
#include "Maze.hpp"
#include "Car.hpp"

class Simulation
{
public:
    auto init() -> void;

    static constexpr float realWidth{ 4.0f };
    static constexpr float realHeight{ 4.0f };

private:
    auto createGround(b2World* world)->b2Body*;

    Window window{ };

    const b2Vec2 gravity{ 0.0, 0.0 };
    b2World world{ gravity };

    b2Body* ground{ nullptr };
    Maze maze{ };
    std::vector<Car> cars{ };

    int move{ 0 };
    int rotate{ 0 };
};