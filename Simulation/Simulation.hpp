#pragma once

#include <vector>
#include <future>
#include <thread>
#include <memory>

#include "..\Neural.hpp"
#include "Window.hpp"
#include "Maze.hpp"
#include "Car.hpp"
#include "Follower.hpp"

enum class Mode 
{
    STOPPED = 0,
    PLAYING
};

enum class Control
{
    MANUAL = 0,
    AUTO,
    NEURAL
};

enum class Data 
{
    IDLE = 0,
    GENERATING,
    SAVING,
    DONE
};

class Simulation
{
public:
    auto init() -> void;

    static constexpr float realWidth{ 16 };
    static constexpr float realHeight{ 16 };
    static constexpr size_t quantity{ 500 };

private:
    auto reset() -> void;
    auto generateCSV()->std::future<void>;

    static auto createGround(b2World* world)->b2Body*;
    static auto inputs(const Car& car) ->std::vector<float>;
    static auto distance(const std::vector<b2Vec2>& path) -> float;

    Window window{ };

    const b2Vec2 gravity{ 0.0, 0.0 };
    b2World world{ gravity };
    b2Body* ground{ nullptr };

    std::unique_ptr<Neural> neural{};
    std::vector<Car> cars{ };
    std::vector<Maze> mazes{ };
    std::vector<Follower> followers{ };

    std::future<void> generationTask{};
    std::vector<std::vector<float>> features{};
    std::vector<std::vector<int>> labels{};

    bool resetChanged{ false };
    bool modeChanged{ false };
    bool controlChanged{ false };
    bool dataChanged{ false };

    Mode mode{ Mode::STOPPED };
    Control control{ Control::MANUAL };
    Data data{ Data::IDLE };

    Move move{ Move::STOP };
};