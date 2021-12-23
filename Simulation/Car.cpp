#include <cmath>
#include <algorithm>
#include <iostream>
#include <map>
#include <vector>
#include <random>
#include <array>

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

Car::Car(b2World* world, b2Body* ground, const b2Vec2& position)
{
    //this->sensors.clear();

    //this->sensors.emplace(180, 0.0f);

   //for (auto n{ -8 }; n <= 8; ++n)
   //{
   //    this->sensors.emplace(180.0f / 16.0f * n, 0.0f);
   //}

   //for (auto n{ -2 }; n <= 2; ++n)
   //{
   //    this->sensors.emplace(180.0f / 4.0f * n, 0.0f);
   //}

    this->world = world;
    this->ground = ground;
    this->createBody(position);
    this->step();
}

Car::Car(const Car& other) : Car{ other.world, other.ground, other.body->GetPosition() }
{
}

Car::~Car()
{
    this->world->DestroyBody(this->body);
}

auto Car::createBody(const b2Vec2& position) -> void
{
    { // Body
        b2BodyDef bd{};
        bd.type = b2_dynamicBody;
        bd.position = position;
        bd.angle = b2_pi;
        //bd.linearDamping = 5.0f;
        //bd.angularDamping = 6.5f;
        bd.userData = const_cast<char*>("car");

        this->body = this->world->CreateBody(&bd);
    }

    { // Chassis
        b2PolygonShape square{};
        square.SetAsBox(0.1f, 0.1f);

        b2FixtureDef fd{};
        fd.shape = &square;
        fd.density = 10.0f;
        //fd.friction = 2.0f;
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
        jd.maxForce = 0.25f * mass * gravity;
        jd.maxTorque = 3.435f * mass * radius * gravity;
        jd.userData = const_cast<char*>("friction");

        //this->world->CreateJoint(&jd);
    }
}

auto Car::step() -> void
{
    /*
    for (auto&& [angle, distance] : this->sensors)
    {
        const auto radians{ (angle / 180.0f) * b2_pi };
        stepSensor(&distance, radians);
    }
    */

    stepBody();

 }

auto Car::render(GPU_Target* target) -> void
{
    GPU_SetLineThickness(0.02f);

    renderBody(target);
    for (auto&& [angle, distance] : sensors)
    {
        const auto radians{ (angle / 180.0f) * b2_pi };
        renderSensor(target, &distance, radians);
    }
}

auto Car::position() const->b2Vec2
{
    return this->body->GetPosition();
}

auto Car::angle() const->float
{
    return this->body->GetAngle();
}

auto Car::doMove(Move move) -> void
{
    if (this->ready)
    {
        this->move = move;
    }
}

auto Car::isReady() const -> bool 
{
    return this->ready;
}

auto Car::isStuck() const -> bool
{
    return this->stuck;
}

auto Car::distances() const -> std::array<std::pair<int, float>, 6>
{
    return sensors;
}

auto Car::color() const -> Color
{
    return Color::BLACK;
}

auto Car::giroscope() const->std::vector<float>
{
    return {};
}

auto Car::acelerometer() const->std::vector<float>
{
    return {};
}

auto Car::stepBody() -> void
{
    /*
    if (this->move == Move::ROTATE_LEFT)
    {
        this->body->ApplyTorque(-2.0f, true);
    }
    else if (this->move == Move::ROTATE_RIGHT)
    {
        this->body->ApplyTorque(+2.0f, true);
    }
    else if (this->move == Move::MOVE_FORWARD)
    {
        const auto force{ this->body->GetWorldVector(b2Vec2{ 0.0f, +2.0f }) };
        const auto point{ this->body->GetWorldPoint(b2Vec2{ 0.0f, 0.0f }) };
        this->body->ApplyForce(force, point, true);
    }
    else if (this->move == Move::MOVE_BACKWARD)
    {
        const auto force{ this->body->GetWorldVector(b2Vec2{ 0.0f, -2.0f }) };
        const auto point{ this->body->GetWorldPoint(b2Vec2{ 0.0f, 0.0f }) };
        this->body-a>ApplyForce(force, point, true);
    }
    */
    this->body->SetAngularVelocity(0.0f);
    this->body->SetLinearVelocity({ 0.0f,0.0f });

    if (this->move == Move::ROTATE_LEFT)
    {
        this->body->SetAngularVelocity(-1.617987633f);
    }
    else if (this->move == Move::ROTATE_RIGHT)
    {
        this->body->SetAngularVelocity(+1.617987633f);
    }
    else if (this->move == Move::MOVE_FORWARD)
    {
        const auto point{ this->body->GetWorldVector(b2Vec2{ 0.0f, +0.294117647f }) };
        this->body->SetLinearVelocity(point);
    }
    else if (this->move == Move::MOVE_BACKWARD)
    {
        const auto point{ this->body->GetWorldVector(b2Vec2{ 0.0f, -0.294117647f }) };
        this->body->SetLinearVelocity(point);
    }

    this->move = Move::STOP;
}

auto Car::stepSensor(float* distance, float angle) -> void
{
    b2Filter filter{};
    filter.categoryBits = 0x0002;
    filter.maskBits = 0x0001;

    const auto maxDistance{ 10.0f };
    const auto start{ this->body->GetWorldPoint(b2Mul(b2Rot{ -angle }, b2Vec2{ 0.0f, 0.1f })) };
    const auto end{ this->body->GetWorldPoint(b2Mul(b2Rot{ -angle }, b2Vec2{ 0.0f, 0.1f + maxDistance })) };

    auto rd{ std::random_device{} };
    auto mt{ std::mt19937{rd()} };

    auto callback{ RayCastCallback{&filter} };
    this->world->RayCast(&callback, start, end);
    if (callback.valid)
    {
        auto dist{ std::uniform_real_distribution{ -0.03f, +0.03f } };
        const auto noise{ dist(mt) };

        const auto measuredDistance{ b2Distance(start, callback.point) };
        *distance = std::clamp(measuredDistance + noise, 0.0f, 2.0f);
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

auto Car::renderSensor(GPU_Target* target, float* distance, float angle) -> void
{
    this->stepSensor(distance, angle);

    if (std::isnan(*distance) or std::isinf(*distance))
    {
        return;
    }

    const auto start{ this->body->GetWorldPoint(b2Mul(b2Rot{ -angle }, b2Vec2{ 0.0f, 0.1f })) };
    const auto end{ this->body->GetWorldPoint(b2Mul(b2Rot{ -angle }, b2Vec2{ 0.0f, 0.1f + *distance })) };

    // Crosshair
    GPU_Line(target, end.x - 0.075f, end.y, end.x + 0.075f, end.y, sensorColor);
    GPU_Line(target, end.x, end.y - 0.075f, end.x, end.y + 0.075f, sensorColor);
}

auto Car::linearVelocity() const -> float
{
    return this->body->GetLinearVelocity().Length();
}

auto Car::angularVelocity() const -> float
{
    return this->body->GetAngularVelocity();
}