#pragma once

#include <SDL.h>
#include <map>

enum class Move
{
    STOP = 0,
    MOVE_FORWARD,
    MOVE_BACKWARD,
    ROTATE_LEFT,
    ROTATE_RIGHT
};

enum class Color
{
    BLACK = 0,
    RED,
    GREEN,
    BLUE,
    YELLOW,
    MAGENTA,
    CYAN,
    WHITE
};

class Car
{
public:
    Car(b2World* world, b2Body* ground);
    Car(const Car& other);
    ~Car();
    auto step() -> void;
    auto render(GPU_Target* target) const -> void;
    auto reset() -> void;

    auto doMove(Move move) -> void;
    auto distances() const->std::map<int, float>;
    auto color() const ->Color;
    auto giroscope() const -> std::vector<float>;
    auto acelerometer() const ->std::vector<float>;
    auto collided() const -> bool;

private:
    auto stepBody() -> void;
    auto stepSensor(float* distance, float angle) -> void;
    auto renderBody(GPU_Target* target) const -> void;
    auto renderSensor(GPU_Target* target, float distance, float angle) const -> void;

    b2World* world{ nullptr };
    b2Body* ground{ nullptr };
    b2Body* body{ nullptr };

    std::map<int, float> sensors{
        {0, 0.0f},
        {+30, 0.0f},
        {-30, 0.0f},
        {+90, 0.0f},
        {-90, 0.0f},
        {180, 0.0f}
    };

    Move move{ Move::STOP };
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