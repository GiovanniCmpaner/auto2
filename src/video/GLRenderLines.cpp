#include <gl/glew.h>
#include <box2d/box2d.h>

#include <iostream>
#include <cstdio>
#include <cstdint>

#include "Camera.hpp"
#include "Utilities.hpp"
#include "GLRenderLines.hpp"

static constexpr auto vertexShader{ R"(
    #version 330
    uniform mat4 projectionMatrix;
    layout(location = 0) in vec2 v_position;
    layout(location = 1) in vec4 v_color;
    layout(location = 2) in float v_size;
    out vec4 f_color;
    void main(void)
    {
        f_color = v_color;
        gl_Position = projectionMatrix * vec4(v_position, 0.0f, 1.0f);
        gl_PointSize = v_size;
    }
)" };

static constexpr auto fragmentShader{ R"(
    #version 330
    in vec4 f_color;
    out vec4 color;
    void main(void)
    {
        color = f_color;
    }
)" };

auto GLRenderLines::create(Camera* camera) -> void
{
    this->camera = camera;

    // Load shader
    programId = Utilities::createShaderProgram(vertexShader, fragmentShader);
    projectionUniform = glGetUniformLocation(programId, "projectionMatrix");
    vertexAttribute = 0;
    colorAttribute = 1;

    // Generate
    glGenVertexArrays(1, &vaoId);
    glGenBuffers(2, vboIds);

    // Bind Objects
    glBindVertexArray(vaoId);
    glEnableVertexAttribArray(vertexAttribute);
    glEnableVertexAttribArray(colorAttribute);

    // Bind Vertices
    glBindBuffer(GL_ARRAY_BUFFER, vboIds[0]);
    glVertexAttribPointer(vertexAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    // Bind Colors
    glBindBuffer(GL_ARRAY_BUFFER, vboIds[1]);
    glVertexAttribPointer(colorAttribute, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_DYNAMIC_DRAW);

    // Cleanup
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

auto GLRenderLines::destroy() -> void
{
    if (vaoId != 0)
    {
        glDeleteVertexArrays(1, &vaoId);
        glDeleteBuffers(2, vboIds);
        vaoId = 0;
    }

    if (programId != 0)
    {
        glDeleteProgram(programId);
        programId = 0;
    }
}

auto GLRenderLines::line(const b2Vec2& v1, const b2Vec2& v2, const b2Color& c) -> void
{
    auto vertex{ [this](const b2Vec2& v, const b2Color& c)
    {
        if (count == maxVertices)
        {
            flush();
        }

        vertices[count] = v;
        colors[count] = c;

        ++count;
    } };

    vertex(v1, c);
    vertex(v2, c);
}

auto GLRenderLines::flush() -> void
{
    if (count == 0)
    {
        return;
    }

    glUseProgram(programId);

    float proj[16]{ 0.0 };
    camera->buildProjectionMatrix(proj, 0.1);

    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, proj);

    glBindVertexArray(vaoId);

    glBindBuffer(GL_ARRAY_BUFFER, vboIds[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(b2Vec2), vertices);

    glBindBuffer(GL_ARRAY_BUFFER, vboIds[1]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(b2Color), colors);

    glDrawArrays(GL_LINES, 0, count);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    count = 0;
}