#pragma once

#include <SDL.h>
#include <map>

class Car
{
public:
    auto init(b2World* world, b2Body* ground) -> void;
    auto step() -> void;
    auto render(GPU_Target* target) const -> void;

    auto moveForward() -> void;
    auto moveBackward() -> void;
    auto rotateLeft()->void;
    auto rotateRight()->void;

    auto distances() const->std::map<int, float>;

    //auto colorBelow() const->SDL_Color;

    auto collided() const -> bool;
private:
    auto stepBody() -> void;
    auto stepSensor(float* distance, float angle) -> void;
    auto renderBody(GPU_Target* target) const -> void;
    auto renderSensor(GPU_Target* target, float distance, float angle) const -> void;

    b2World* world{ nullptr };
    b2Body* ground{ nullptr };
    b2Body* body{ nullptr };

    int move{ 0 };
    int rotate{ 0 };

    bool moved{ false };
    bool rotated{ false };

    std::map<int, float> sensors{
        {0, 0.0f},
        {90, 0.0f},
        {-90, 0.0f},
        {180, 0.0f}
    };

    bool collision{ false };

    static constexpr float width{ 0.2f };
    static constexpr float height{ 0.2f };
    static constexpr float maxDistance{ 2.0f };
    static constexpr SDL_Color backgroundColor{ 0, 0, 0, 255 };
    static constexpr SDL_Color fontColor{ 0, 255, 0, 255 };
    static constexpr SDL_Color sensorColor{ 0,0,255,255 };
    static constexpr SDL_Color solidBorderColor{ 255, 0, 255, 255 };
    static constexpr SDL_Color solidFillColor{ 255, 0, 255, 64 };
};