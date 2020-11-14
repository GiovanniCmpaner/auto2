#include <cmath>
#include <algorithm>
#include <iostream>

#include <box2d/box2d.h>
#include <SDL_gpu.h>
#include <SDL_FontCache.h>

#include "car.hpp"

class RayCastCallback : public b2RayCastCallback
{
public:
    RayCastCallback(b2Filter* filter)
    {
        this->filter = filter;
    }

    float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override
    {
        const auto filter{ fixture->GetFilterData() };
        if ((this->filter->maskBits & filter.categoryBits) != 0 and (filter.maskBits & this->filter->categoryBits) != 0)
        {
            this->valid = true;
            this->point = point;
            return fraction;
        }

        return -1;
    }

    b2Filter* filter{ nullptr };
    bool valid{ false };
    b2Vec2 point{ 0.0, 0.0 };
};

auto Car::init(b2World* world, b2Body* ground) -> void
{
    this->world = world;
    this->ground = ground;

    { // Body
        b2BodyDef bd{};
        bd.type = b2_dynamicBody;
        bd.position = b2Vec2{ 0, 0 };
        bd.angularDamping = 6.0f;
        bd.userData = const_cast<char*>("car");

        this->body = this->world->CreateBody(&bd);
    }

    { // Chassis
        b2PolygonShape square{};
        square.SetAsBox(0.1f, 0.1f);

        b2FixtureDef fd{};
        fd.shape = &square;
        fd.density = 2.0f;
        fd.friction = 0.5f;
        fd.filter.categoryBits = 0x0002;
        fd.filter.maskBits = 0x0001;
        fd.userData = const_cast<char*>("chassis");

        this->body->CreateFixture(&fd);
    }

    { // Direction symbol (triangle)
        b2PolygonShape triangle{};
        const b2Vec2 vertices[3]{
            b2Vec2{ -0.05f, -0.05f },
            b2Vec2{ +0.05f, -0.05f },
            b2Vec2{ 0.0f, +0.05f },
        };
        triangle.Set(vertices, 3);

        b2FixtureDef fd{};
        fd.shape = &triangle;
        fd.isSensor = true;
        fd.filter.categoryBits = 0x0002;
        fd.filter.maskBits = 0x0001;
        fd.userData = const_cast<char*>("direction");

        this->body->CreateFixture(&fd);
    }

    { // Top-down friction
        const auto gravity{ 10.0f };
        const auto inertia{ this->body->GetInertia() };
        const auto mass{ this->body->GetMass() };
        const auto radius{ b2Sqrt(2.0f * inertia / mass) };

        b2FrictionJointDef jd{};
        jd.bodyA = this->ground;
        jd.bodyB = this->body;
        jd.localAnchorA = b2Vec2{ 0.0f, 0.0f };
        jd.localAnchorB = this->body->GetLocalCenter();
        jd.collideConnected = true;
        jd.maxForce = 1.3f * mass * gravity;
        jd.maxTorque = 9.7f * mass * radius * gravity;
        jd.userData = const_cast<char*>("friction");

        world->CreateJoint(&jd);
    }
}

auto Car::step() -> void
{
    stepBody();
    stepSensor(&front, 0.0f);
    stepSensor(&left, +b2_pi / 2.0f);
    stepSensor(&right, -b2_pi / 2.0f);
}

auto Car::moveForward() -> void
{
    move = +1;
}

auto Car::moveBackward() -> void
{
    move = -1;
}

auto Car::rotateLeft()->void
{
    rotate = +1;
}

auto Car::rotateRight()->void
{
    rotate = -1;
}

auto Car::distanceFront() const -> float
{
    return front;
}

auto Car::distanceLeft() const -> float
{
    return left;
}

auto Car::distanceRight() const-> float
{
    return right;
}

auto Car::collided() const -> bool
{
    return collision;
}

auto Car::render(GPU_Target* target) const -> void
{
    GPU_SetLineThickness(0.02f);

    renderBody(target);
    renderSensor(target, front, 0.0f);
    renderSensor(target, left, +b2_pi / 2.0f);
    renderSensor(target, right, -b2_pi / 2.0f);
}

auto Car::stepBody() -> void
{
    if (not collision)
    {
        if (rotated and b2Abs(this->body->GetAngularVelocity()) <= b2_linearSlop)
        {
            //collision = true;
        }
        else if (moved and this->body->GetLinearVelocity().Length() <= b2_linearSlop)
        {
            //collision = true;
        }
        rotated = false;
        moved = false;

        if (not collision)
        {
            if (rotate != 0)
            {
                this->body->ApplyTorque(1.0f * rotate, true);
                rotated = true;
            }
            else if (move != 0)
            {
                const auto force{ this->body->GetWorldVector(b2Vec2{ 0.0f, 1.1f * move }) };
                const auto point{ this->body->GetWorldPoint(b2Vec2{ 0.0f, 0.0f * move }) };
                this->body->ApplyForce(force, point, true);
                moved = true;
            }
            rotate = 0;
            move = 0;
        }
    }
}

auto Car::stepSensor(float* distance, float angle) -> void
{
    b2Filter filter{};
    filter.categoryBits = 0x0002;
    filter.maskBits = 0x0001;

    const auto maxDistance{ 2.0f };
    const auto start{ this->body->GetWorldPoint(b2Mul(b2Rot{ angle }, b2Vec2{ 0.0f, 0.1f })) };
    const auto end{ this->body->GetWorldPoint(b2Mul(b2Rot{ angle }, b2Vec2{ 0.0f, 0.1f + maxDistance })) };

    auto callback{ RayCastCallback{&filter} };
    this->world->RayCast(&callback, start, end);
    if (callback.valid)
    {
        *distance = b2Distance(start, callback.point);
    }
    else
    {
        *distance = b2Distance(start, end);
    }

}

auto Car::renderBody(GPU_Target* target) const -> void
{
    for (auto fixture{ body->GetFixtureList() }; fixture != nullptr; fixture = fixture->GetNext())
    {
        const auto polygon{ reinterpret_cast<const b2PolygonShape*>(fixture->GetShape()) };

        float vertices[2 * b2_maxPolygonVertices];
        for (auto i{ 0 }; i < polygon->m_count; ++i)
        {
            const auto vertice{ this->body->GetWorldPoint(polygon->m_vertices[i]) };
            vertices[i * 2 + 0] = vertice.x;
            vertices[i * 2 + 1] = vertice.y;
        }

        GPU_Polygon(target, polygon->m_count, vertices, solidBorderColor);
        GPU_PolygonFilled(target, polygon->m_count, vertices, solidFillColor);
    }
}

auto Car::renderSensor(GPU_Target* target, float distance, float angle) const -> void
{
    const auto start{ this->body->GetWorldPoint(b2Mul(b2Rot{ angle }, b2Vec2{ 0.0f, 0.1f })) };
    const auto end{ this->body->GetWorldPoint(b2Mul(b2Rot{ angle }, b2Vec2{ 0.0f, 0.1f + distance })) };

    // Crosshair
    GPU_Line(target, end.x - 0.075f, end.y, end.x + 0.075f, end.y, sensorColor);
    GPU_Line(target, end.x, end.y - 0.075f, end.x, end.y + 0.075f, sensorColor);
}