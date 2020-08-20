#pragma once

#include <box2d/box2d.h>
#include <cstdint>

#include "Camera.hpp"
#include "GLRenderPoints.hpp"
#include "GLRenderLines.hpp"
#include "GLRenderTriangles.hpp"

class Draw
{
	Camera* camera{ nullptr };
	GLRenderPoints* points{ nullptr };
	GLRenderLines* lines{ nullptr };
	GLRenderTriangles* triangles{ nullptr };

public:

	auto create(Camera* camera) -> void;

	auto destroy() -> void;

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