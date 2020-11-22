#pragma once

#include <vector>
#include <box2d/box2d.h>

#include "Car.hpp"
#include "Maze.hpp"

class Follower
{
public:
    Follower(Car* car, const std::vector<b2Vec2>& path);

    auto step() -> void;
    auto render(GPU_Target* target) const -> void;
    auto finished() const -> bool;
    auto movement() const->Move;
private:
    static auto normalizeAngle(float angle) -> float;
    auto nextPoint() -> void;
    auto followPath() -> void;
    auto renderPath(GPU_Target* target) const -> void;

    Car* car{ nullptr };
    std::vector<b2Vec2> path{};

    size_t currentPoint{ 0 };
    float targetAngle{ 0.0f };
    bool adjustingAngle{ false };
    Move move{ Move::STOP };
};