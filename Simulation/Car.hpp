#pragma once

#include <SDL_gpu.h>
#include <box2d/box2d.h>

#include <map>
#include <deque>
#include <array>

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
    Car(b2World* world, b2Body* ground, const b2Vec2& position = {});
    Car(const Car& other);
    ~Car();
    auto step() -> void;
    auto render(GPU_Target* target) -> void;

    auto position() const->b2Vec2;
    auto angle() const->float;

    auto isReady() const -> bool;
    auto isStuck() const -> bool;
    auto doMove(Move move) -> void;

    auto distances() const->std::array<std::pair<int, float>, 6>;
    auto color() const ->Color;
    auto giroscope() const -> std::vector<float>;
    auto acelerometer() const ->std::vector<float>;
    auto linearVelocity() const -> float;
    auto angularVelocity() const -> float;

private:
    auto createBody(const b2Vec2& position) -> void;
    auto stepBody() -> void;
    auto stepSensor(const b2Vec2& position, float* distance, float angle) -> void;
    auto renderBody(GPU_Target* target) const -> void;
    auto renderSensor(GPU_Target* target, const b2Vec2& position, float* distance, float radians) -> void;

    b2World* world{ nullptr };
    b2Body* ground{ nullptr };
    b2Body* body{ nullptr };

    std::array<std::tuple<int, b2Vec2, float>, 6> sensors{ {
        {+33, {0.056f, 0.114f}, 0.0f},
        {+90, {0.075f, 0.000f}, 0.0f},
        {0,   {0.000f, 0.128f}, 0.0f},
        {-33, {-0.056f, 0.114f}, 0.0f},
        {-90, {-0.075f, 0.000f}, 0.0f},
        {180, {0.000f, -0.128f}, 0.0f}
    }};

    bool ready{ true };
    bool stuck{ false };
    Move move{ Move::STOP };
    std::map<int, float> requested{};

    static constexpr float width{ 0.2f };
    static constexpr float height{ 0.2f };
    static constexpr float maxDistance{ 2.0f };
    static constexpr SDL_Color backgroundColor{ 0, 0, 0, 255 };
    static constexpr SDL_Color fontColor{ 0, 255, 0, 255 };
    static constexpr SDL_Color sensorColor{ 0,0,255,255 };
    static constexpr SDL_Color solidBorderColor{ 255, 0, 255, 255 };
    static constexpr SDL_Color solidFillColor{ 255, 0, 255, 64 };
};