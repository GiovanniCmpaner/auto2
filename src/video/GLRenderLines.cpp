#pragma once

#include <gl/glew.h>
#include <box2d/box2d.h>

#include <iostream>
#include <cstdio>
#include <cstdint>

class GLRenderLines
{
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
        	gl_Position = projectionMatrix * vec4(v_position, 0.0f, 1.0f);
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

public:

    auto create() -> void
    {
        // Load shader
        programId = createShaderProgram(vs, fs);
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

    auto destroy() -> void
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

    void drawPolygon(const b2Vec2* vertices, size_t vertexCount, const b2Color& color)
    {
        auto p1{ vertices[vertexCount - 1] };
        for (size_t i{ 0 }; i < vertexCount; ++i)
        {
            const auto p2{ vertices[i] };
            vertex(p1, color);
            vertex(p2, color);
            p1 = p2;
        }
    }

    void vertex(const b2Vec2& v, const b2Color& c)
    {
        if (count == maxVertices)
        {
            flush();
        }

        vertices[count] = v;
        colors[count] = c;
        ++count;
    }

    auto flush() -> void
    {
        if (count == 0)
        {
            return;
        }

        glUseProgram(programId);

        float proj[16]{ 0.0 };
        g_camera.BuildProjectionMatrix(proj, 0.1);

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
};