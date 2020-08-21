#pragma once

#include <gl/glew.h>
#include <box2d/box2d.h>

#include "Camera.hpp"

class GLRenderPoints
{
public:

	GLRenderPoints(std::shared_ptr<Camera> camera);

	~GLRenderPoints();

	auto point(const b2Vec2& vector, const b2Color& color, float size) -> void;

	auto flush() -> void;

private:

	std::shared_ptr<Camera> camera{ nullptr };

	enum { e_maxVertices = 512 };
	b2Vec2 vertices[e_maxVertices];
	b2Color colors[e_maxVertices];
	float sizes[e_maxVertices];

	int32 count;

	GLuint vaoId;
	GLuint vboIds[3];
	GLuint programId;
	GLint projectionUniform;
	GLint vertexAttribute;
	GLint colorAttribute;
	GLint sizeAttribute;
};