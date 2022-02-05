#pragma once

#include <vector>
#include <future>
#include <thread>
#include <memory>
#include <filesystem>
#include <fstream>

#include "..\Neural.hpp"
#include "..\Fuzzy.hpp"
#include "..\Replay.hpp"
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
    NEURAL,
    FUZZY,
    REPLAY
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

    static constexpr float realWidth{ 5 };
    static constexpr float realHeight{ 5 };

private:
    auto reset() -> void;
    auto generateCSV()->std::future<void>;

    static auto createGround(b2World* world)->b2Body*;
    static auto inputs(const Car& car) ->std::vector<float>;

    Window window{ };

    const b2Vec2 gravity{ 0.0, 0.0 };
    b2World world{ gravity };
    b2Body* ground{ nullptr };

    std::unique_ptr<Neural> neural{};
    std::unique_ptr<Fuzzy> fuzzy{};
    std::unique_ptr<Replay> replay{};
    std::vector<Car> cars{ };
    std::vector<Maze> mazes{ };
    std::vector<Follower> followers{ };
    
    const int generations{ 0 };
    int current{ 0 };

    const int quantity{ 1 };
    int done{ 0 };
    std::future<void> generationTask{};
    std::vector<std::vector<float>> features{};
    std::vector<std::vector<int>> labels{};

    std::ofstream featuresFile{};
    std::ofstream labelsFile{};

    bool resetChanged{ false };
    bool modeChanged{ false };
    bool controlChanged{ false };
    bool dataChanged{ false };

    Mode mode{ Mode::STOPPED };
    Control control{ Control::NEURAL };
    Data data{ Data::IDLE };

    Move move{ Move::STOP };
};