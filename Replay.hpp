#pragma once

#include <SDL_gpu.h>
#include <box2d/box2d.h>

#include <vector>
#include <string>
#include <array>
#include <iostream>
#include <fstream>

class Replay
{
public:
    Replay(b2World* world, b2Body* ground, const b2Vec2& position, const std::string& filePath);
    ~Replay();
    Replay(const Replay&) = delete;
    Replay(Replay&&) = delete;

    auto step() -> void;
    auto render(GPU_Target* target) -> void;
private:

    enum class Move
    {
        STOP = 0,
        MOVE_FORWARD,
        MOVE_BACKWARD,
        ROTATE_LEFT,
        ROTATE_RIGHT
    };

    struct Entry 
    {
        int line;
        std::array<float, 6> distances;
        Move move;
    };

    b2World* world{ nullptr };
    b2Body* ground{ nullptr };
    b2Vec2 position{};
    std::string filePath{};

    b2Body* pathBody{ nullptr };
    b2Body* carBody{ nullptr };
    std::ifstream file{};
    Entry current{};

   const std::array<std::pair<int,b2Vec2>, 6> sensors{ {
        {+33, {0.056f, 0.114f}},
        {+90, {0.075f, 0.000f}},
        {0,   {0.000f, 0.128f}},
        {-33, {-0.056f, 0.114f}},
        {-90, {-0.075f, 0.000f}},
        {180, {0.000f, -0.128f}}
    } };

    auto openFile() -> void;
    auto createPath() -> void;
    auto createCar() -> void;
    auto createSensor() -> void;

    auto renderPath(GPU_Target* target) -> void;
    auto renderCar(GPU_Target* target) -> void;
    auto renderSensor(GPU_Target* target) -> void;

    auto stepFile() -> void;
    auto stepPath() -> void;
    auto stepCar() -> void;
    auto stepSensor() -> void;
    
};