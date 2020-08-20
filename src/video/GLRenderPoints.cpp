#include <gl/glew.h>
#include <box2d/box2d.h>

#include <iostream>
#include <cstdio>
#include <cstdint>

#include "Camera.hpp"
#include "Utilities.hpp"
#include "GLRenderPoints.hpp"

static constexpr auto vs{ R"(
    #version 330
    uniform mat4 projectionMatrix;
    layout(location = 0) in vec2 v_position;
    layout(location = 1) in vec4 v_color;
	layout(location = 2) in float v_size;
    out vec4 f_color;
    void main(void)
    {
        f_color = v_color;
        gl_Position = projectionMatrix * vec4(v_position, 0.0, 1.0);
		gl_PointSize = v_size;
    }
)" };

static constexpr auto fs{ R"(
    #version 330
    in vec4 f_color;
    out vec4 color;
    void main(void)
    {
        color = f_color;
    }
)" };

auto GLRenderPoints::create(Camera* camera) -> void
{
    this->camera = camera;

    // Load shader
    programId = Utilities::createShaderProgram(vs, fs);
    projectionUniform = glGetUniformLocation(programId, "projectionMatrix");
    vertexAttribute = 0;
    colorAttribute = 1;
    sizeAttribute = 2;

    // Generate
    glGenVertexArrays(1, &vaoId);
    glGenBuffers(3, vboIds);

    // Bind Objects
    glBindVertexArray(vaoId);
    glEnableVertexAttribArray(vertexAttribute);
    glEnableVertexAttribArray(colorAttribute);
    glEnableVertexAttribArray(sizeAttribute);

    // Bind Vertices
    glBindBuffer(GL_ARRAY_BUFFER, vboIds[0]);
    glVertexAttribPointer(vertexAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    // Bind colors
    glBindBuffer(GL_ARRAY_BUFFER, vboIds[1]);
    glVertexAttribPointer(colorAttribute, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_DYNAMIC_DRAW);

    // Bind size
    glBindBuffer(GL_ARRAY_BUFFER, vboIds[2]);
    glVertexAttribPointer(sizeAttribute, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sizes), sizes, GL_DYNAMIC_DRAW);

    // Cleanup
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    count = 0;
}

auto GLRenderPoints::destroy() -> void
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

auto GLRenderPoints::point(const b2Vec2& vector, const b2Color& color, float size) -> void
{
    auto vertex{ [this](const b2Vec2& v, const b2Color& c, float size)
    {
        if (count == e_maxVertices)
        {
            flush();
        }

        vertices[count] = v;
        colors[count] = c;
        sizes[count] = size;

        ++count;
    }};

    vertex(vector, color, size);
}

auto GLRenderPoints::flush() -> void
{
    if (count == 0)
    {
        return;
    }

    glUseProgram(programId);

    float proj[16]{ 0.0 };
    camera->buildProjectionMatrix(proj, 0.0);

    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, proj);

    glBindVertexArray(vaoId);

    glBindBuffer(GL_ARRAY_BUFFER, vboIds[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(b2Vec2), vertices);

    glBindBuffer(GL_ARRAY_BUFFER, vboIds[1]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(b2Color), colors);

    glBindBuffer(GL_ARRAY_BUFFER, vboIds[2]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(float), sizes);

    glEnable(GL_PROGRAM_POINT_SIZE);
    glDrawArrays(GL_POINTS, 0, count);
    glDisable(GL_PROGRAM_POINT_SIZE);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    count = 0;
}