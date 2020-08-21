#pragma once

#include <box2d/box2d.h>
#include <cstdint>
#include <memory>

#include "Camera.hpp"
#include "GLRenderPoints.hpp"
#include "GLRenderLines.hpp"
#include "GLRenderTriangles.hpp"

class Draw
{
	std::shared_ptr<Camera> camera{ nullptr };
	std::unique_ptr<GLRenderPoints> points{ nullptr };
	std::unique_ptr<GLRenderLines> lines{ nullptr };
	std::unique_ptr<GLRenderTriangles> triangles{ nullptr };

	static auto debugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) -> void;

public:

	Draw(std::shared_ptr<Camera> camera);

	auto drawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) -> void;

	auto drawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) -> void;

	auto drawCircle(const b2Vec2& center, float radius, const b2Color& color) -> void;

	auto drawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color) -> void;

	auto drawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) -> void;

	auto drawPoint(const b2Vec2& p, float size, const b2Color& color) -> void;

	auto drawString(int x, int y, const char* string, ...) -> void;

	auto drawString(const b2Vec2& pw, const char* string, ...) -> void;

	auto flush() -> void;
};