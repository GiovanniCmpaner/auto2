#pragma once

#include <gl/glew.h>
#include <box2d/box2d.h>

#include "Camera.hpp"

struct GLRenderTriangles
{
public:

	auto create(Camera* camera) -> void;

	auto destroy() -> void;

	auto triangle(const b2Vec2& v1, const b2Vec2& v2, const b2Vec2& v3, const b2Color& c) -> void;

	auto flush() -> void;

private:

	Camera* camera{ nullptr };

	enum { e_maxVertices = 3 * 512 };
	b2Vec2 vertices[e_maxVertices];
	b2Color colors[e_maxVertices];

	int32 count;

	GLuint vaoId;
	GLuint vboIds[2];
	GLuint programId;
	GLint projectionUniform;
	GLint vertexAttribute;
	GLint colorAttribute;
};