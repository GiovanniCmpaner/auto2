#pragma once

#include <vector>

#include "Window.hpp"
#include "Maze.hpp"
#include "Car.hpp"
#include "Follower.hpp"
#include "../Neural.hpp"

class Simulation
{
public:
    auto init() -> void;

    static constexpr float realWidth{ 16 };
    static constexpr float realHeight{ 16 };
    static constexpr size_t quantity{ 500 };

private:
    auto reset() -> void;
    auto generateCSV() -> void;

    static auto createGround(b2World* world)->b2Body*;
    static auto inputs(const Car& car) ->std::vector<float>;
    static auto distance(const std::vector<b2Vec2>& path) -> float;

    Window window{ };

    const b2Vec2 gravity{ 0.0, 0.0 };
    b2World world{ gravity };

    b2Body* ground{ nullptr };

    std::vector<Car> cars{ };
    std::vector<Maze> mazes{ };
    std::vector<std::vector<b2Vec2>> solutions{ };
    std::vector<Follower> followers{ };
    std::vector<b2Vec2> positions{ };
    std::vector<float> distances{ };

    unsigned long long start{};
    int generation{ 0 };
    std::vector<std::vector<float>> features{};
    std::vector<std::vector<int>> labels{};

    Move move{ Move::STOP };

    std::unique_ptr<Neural> neural{};
};