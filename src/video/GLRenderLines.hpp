#pragma once

#include <gl/glew.h>
#include <box2d/box2d.h>

#include "Camera.hpp"

class GLRenderLines
{
public:

    auto create(Camera* camera) -> void;

    auto destroy() -> void;

    auto line(const b2Vec2& v1, const b2Vec2& v2, const b2Color& c) -> void;

    auto flush() -> void;

private:

    Camera* camera{ nullptr };

    GLint programId{ 0 };
    GLint projectionUniform{ 0 };
    GLint vertexAttribute{ 0 };
    GLint colorAttribute{ 0 };

    GLuint vaoId{ 0 };
    GLuint vboIds[2]{ 0, 0 };

    static constexpr size_t maxVertices{ 1024 };
    b2Vec2 vertices[maxVertices]{};
    b2Color colors[maxVertices]{};
    size_t count{ 0 };
};