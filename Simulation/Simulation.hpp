#pragma once

#include <vector>

#include <tiny_dnn/tiny_dnn.h>

#include "Window.hpp"
#include "Maze.hpp"
#include "Car.hpp"
#include "Follower.hpp"

class Simulation
{
public:
    auto init() -> void;

    static constexpr float realWidth{ 8 };
    static constexpr float realHeight{ 8 };
    static constexpr size_t quantity{ 500 };

private:
    auto constructDNN()->void;

    static auto createGround(b2World* world)->b2Body*;
    static auto inputs(const Car& car) ->tiny_dnn::vec_t;
    static auto distance(const std::vector<b2Vec2>& path) -> float;

    Window window{ };

    const b2Vec2 gravity{ 0.0, 0.0 };
    b2World world{ gravity };

    b2Body* ground{ nullptr };

    std::vector<Car> cars{ };
    std::vector<Maze> mazes{ };
    std::vector<std::vector<b2Vec2>> paths{ };
    std::vector<Follower> followers{ };

    std::vector<tiny_dnn::vec_t> data{};
    std::vector<tiny_dnn::label_t> labels{};
    tiny_dnn::network<tiny_dnn::sequential> net{};

    Move move{ Move::STOP };
};