#pragma once

#include <SDL.h>


class Car {
public:
    Car(b2World* world, b2Body* ground);

    auto step() -> void;
    auto render(GPU_Target* target) const -> void;

    auto moveForward() -> void;
    auto moveBackward() -> void;
    auto rotateLeft()->void;
    auto rotateRight()->void;

    auto distanceFront() const -> float;
    auto distanceLeft() const -> float;
    auto distanceRight() const -> float;

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

    bool moved{ false };
    bool rotated{ false };

    float front{ 0.0f };
    float left{ 0.0f };
    float right{ 0.0f };
    bool collision{ false };

    static constexpr SDL_Color backgroundColor{ 0, 0, 0, 255 };
    static constexpr SDL_Color fontColor{ 0, 255, 0, 255 };
    static constexpr SDL_Color sensorColor{ 0,0,255,255 };
    static constexpr SDL_Color solidBorderColor{ 255, 0, 255, 255 };
    static constexpr SDL_Color solidFillColor{ 255, 0, 255, 64 };
};