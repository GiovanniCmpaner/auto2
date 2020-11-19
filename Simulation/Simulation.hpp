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

    static constexpr float realWidth{ 3.5f };
    static constexpr float realHeight{ 3.5f };

private:
    auto constructDNN()->void;

    static auto createGround(b2World* world)->b2Body*;
    static auto inputs(const Car& car) ->tiny_dnn::vec_t;
    static auto distance(const std::vector<b2Vec2>& path) -> float;

    Window window{ };

    const b2Vec2 gravity{ 0.0, 0.0 };
    b2World world{ gravity };

    b2Body* ground{ nullptr };
    Maze maze{ };
    std::vector<Car> cars{ };
    std::vector<std::chrono::system_clock::time_point> timers{};
    std::vector<tiny_dnn::network<tiny_dnn::sequential>> nets{};
    std::chrono::system_clock::time_point start{};

    
    std::vector<tiny_dnn::vec_t> data{};
    std::vector<tiny_dnn::label_t> labels{};

    Move move{ Move::STOP };
};