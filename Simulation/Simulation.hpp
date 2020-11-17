#pragma once

#include <vector>

#include <tiny_dnn/tiny_dnn.h>

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
    auto constructDNN()->void;

    Window window{ };

    const b2Vec2 gravity{ 0.0, 0.0 };
    b2World world{ gravity };

    b2Body* ground{ nullptr };
    Maze maze{ };
    std::vector<Car> cars{ };
    std::vector<> colors{};
    std::vector<> timer{};

    tiny_dnn::network<tiny_dnn::sequential> net{};

    int move{ 0 };
    int rotate{ 0 };
};