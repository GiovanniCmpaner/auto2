#include "Camera.hpp"

auto Camera::convertScreenToWorld(const b2Vec2& ps) -> b2Vec2
{
    const auto ratio{ static_cast<float>(width) / static_cast<float>(height) };
    const auto extents{ zoom * b2Vec2{ ratio * 25.0f, 25.0f } };

    const auto lower{ b2Vec2{center - extents} };
    const auto upper{ b2Vec2{center + extents} };

    const auto u{ ps.x / width };
    const auto v{ (height - ps.y) / height };

    return b2Vec2{
        (1.0f - u) * lower.x + u * upper.x,
        (1.0f - v) * lower.y + v * upper.y
    };
}

auto Camera::convertWorldToScreen(const b2Vec2& pw) -> b2Vec2
{
    const auto ratio{ static_cast<float>(width) / static_cast<float>(height) };
    const auto extents{ zoom * b2Vec2{ ratio * 25.0f, 25.0f } };

    const auto lower{ b2Vec2{center - extents} };
    const auto upper{ b2Vec2{center + extents} };

    const auto u{ (pw.x - lower.x) / (upper.x - lower.x) };
    const auto v{ (pw.y - lower.y) / (upper.y - lower.y) };

    return b2Vec2{
        u * width,
        (1.0f - v) * height
    };
}

auto Camera::buildProjectionMatrix(float* matrix, float zBias) -> void
{
    const auto ratio{ static_cast<float>(width) / static_cast<float>(height) };
    const auto extents{ zoom * b2Vec2{ ratio * 25.0f, 25.0f } };

    const auto lower{ b2Vec2{center - extents} };
    const auto upper{ b2Vec2{center + extents} };

    matrix[0] = 2.0f / (upper.x - lower.x);
    matrix[1] = 0.0f;
    matrix[2] = 0.0f;
    matrix[3] = 0.0f;

    matrix[4] = 0.0f;
    matrix[5] = 2.0f / (upper.y - lower.y);
    matrix[6] = 0.0f;
    matrix[7] = 0.0f;

    matrix[8] = 0.0f;
    matrix[9] = 0.0f;
    matrix[10] = 1.0f;
    matrix[11] = 0.0f;

    matrix[12] = -(upper.x + lower.x) / (upper.x - lower.x);
    matrix[13] = -(upper.y + lower.y) / (upper.y - lower.y);
    matrix[14] = zBias;
    matrix[15] = 1.0f;
}

