#pragma once

#include <box2d/box2d.h>

class Camera
{
    b2Vec2 center{ 0.0, 0.0 };
    float zoom{ 1.0 };
    int32 width{ 1280 };
    int32 height{ 800 };
public:
    auto zoomIn() -> void;
    auto zoomOut() -> void;
    auto home() -> void;
    auto convertScreenToWorld(const b2Vec2& screenPoint)->b2Vec2;
    auto convertWorldToScreen(const b2Vec2& worldPoint)->b2Vec2;
    auto buildProjectionMatrix(float* matrix, float zBias) -> void;
};