#include <cmath>
#include <algorithm>
#include <iostream>
#include <map>
#include <vector>
#include <random>
#include <array>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <string>

#include <box2d/box2d.h>
#include <SDL_gpu.h>
#include <SDL_FontCache.h>

#include "Replay.hpp"
#include "Draw.hpp"

Replay::Replay(b2World* world, b2Body* ground, const b2Vec2& position, const std::string& filePath)
{
    this->world = world;
    this->ground = ground;
    this->position = position;
    this->filePath = filePath;
    this->openFile();
    this->createPath();
    this->createCar();
}

Replay::~Replay()
{
    this->world->DestroyBody(this->carBody);
    this->world->DestroyBody(this->pathBody);
}

auto Replay::step() -> void
{
    this->stepFile();
    this->stepPath();
    this->stepCar();
    this->stepSensor();
}

auto Replay::render(GPU_Target* target) -> void
{
    GPU_SetLineThickness(0.01f);

    this->renderPath(target);
    this->renderCar(target);
    this->renderSensor(target);
}

auto Replay::openFile() -> void
{
    auto header{ std::string{} };

    this->file.open(this->filePath);
    if (file.is_open())
    {
        ++this->current.line;
        std::getline(this->file, header);
    }

    assert(header == "+33;+90;0;-33;-90;180;stop;forward;backward;left;right;");

    this->stepFile();
}

auto Replay::createPath() -> void
{
    { // Body
        b2BodyDef bd{};
        bd.type = b2_staticBody;
        bd.position = b2Vec2{ this->position.x, this->position.y };
        bd.userData = const_cast<char*>("maze");

        this->pathBody = this->world->CreateBody(&bd);
    }

    { // Wall
        const auto thickness{ 0.015f };
        const auto height{ 1.0f };
        const auto width{ 1.5f };
        const auto segment{ 0.5f };

        b2PolygonShape shape{};
    
        b2FixtureDef fd{};
        fd.shape = &shape;
        fd.density = 0.0f;
        fd.filter.categoryBits = 0x0001;
        fd.filter.maskBits = 0x0003;
        fd.userData = const_cast<char*>("wall");
        
        { // External
            
            shape.SetAsBox(thickness / 2, height / 2, { 0, height / 2 }, 0);
            this->pathBody->CreateFixture(&fd);

            shape.SetAsBox(thickness / 2, height / 2, { width, height / 2 }, 0);
            this->pathBody->CreateFixture(&fd);

            shape.SetAsBox(width / 2, thickness / 2, { width / 2, 0 }, 0);
            this->pathBody->CreateFixture(&fd);

            shape.SetAsBox(width / 2, thickness / 2, { width / 2, height }, 0);
            this->pathBody->CreateFixture(&fd);
        }

        { // Segment
            shape.SetAsBox(thickness / 2, segment / 2, { 1.17f, height - segment / 2 }, 0);
            this->pathBody->CreateFixture(&fd);

            shape.SetAsBox(thickness / 2, segment / 2, { 0.84f, segment / 2 }, 0);
            this->pathBody->CreateFixture(&fd);

            shape.SetAsBox(segment / 2, thickness / 2, { 0.84f - segment / 2, segment }, 0);
            this->pathBody->CreateFixture(&fd);
        }
    }
}

auto Replay::createCar() -> void
{
    { // Body
        b2BodyDef bd{};
        bd.type = b2_dynamicBody;
        bd.position = this->position + b2Vec2{ 1.335, 0.015f + 0.128f + this->current.distances[2] };
        bd.angle = b2_pi;
        bd.linearDamping = 0.1f;
        bd.angularDamping = 0.1f;
        bd.userData = const_cast<char*>("car");

        this->carBody = this->world->CreateBody(&bd);
    }

    { // Chassis
        b2PolygonShape polygon{};
        polygon.SetAsBox(0.075f, 0.128f);

        b2FixtureDef fd{};
        fd.shape = &polygon;
        fd.filter.categoryBits = 0x0002;
        fd.filter.maskBits = 0x0001;
        fd.density = 10.0f;
        fd.friction = 0.1f;
        fd.userData = const_cast<char*>("chassis");

        this->carBody->CreateFixture(&fd);
    }

    { // Direction symbol (triangle)
        b2PolygonShape triangle{};
        const b2Vec2 vertices[3]{
            b2Vec2{ -0.03f, -0.03f },
            b2Vec2{ +0.03f, -0.03f },
            b2Vec2{ 0.0f, +0.03f },
        };
        triangle.Set(vertices, 3);

        b2FixtureDef fd{};
        fd.shape = &triangle;
        fd.isSensor = true;
        fd.filter.categoryBits = 0x0002;
        fd.filter.maskBits = 0x0001;
        fd.userData = const_cast<char*>("direction");

        this->carBody->CreateFixture(&fd);
    }

    { // Sensors
        for (const auto& [angle, position] : this->sensors)
        {
            b2PolygonShape polygon{};
            polygon.SetAsBox(0.0125f, 0.0125f, position, 0);

            b2FixtureDef fd{};
            fd.shape = &polygon;
            fd.isSensor = true;
            fd.userData = const_cast<char*>("sensor");

            this->carBody->CreateFixture(&fd);
        }
    }

    { // Top-down friction
        const auto gravity{ 10.0f };
        const auto inertia{ this->carBody->GetInertia() };
        const auto mass{ this->carBody->GetMass() };
        const auto radius{ b2Sqrt(2.0f * inertia / mass) };

        b2FrictionJointDef jd{};
        jd.bodyA = this->ground;
        jd.bodyB = this->carBody;
        jd.localAnchorA = b2Vec2{ 0.0f, 0.0f };
        jd.localAnchorB = this->carBody->GetLocalCenter();
        jd.collideConnected = true;
        jd.maxForce = 0.1f * mass * gravity;
        jd.maxTorque = 0.1f * mass * radius * gravity;
        jd.userData = const_cast<char*>("friction");

        this->world->CreateJoint(&jd);
    }
}

auto Replay::createSensor() -> void
{

}

auto Replay::renderPath(GPU_Target* target) -> void
{
    Draw::draw(target, this->pathBody);
}

auto Replay::renderCar(GPU_Target* target) -> void
{
    Draw::draw(target, this->carBody);
}

auto Replay::renderSensor(GPU_Target* target) -> void
{
    b2Filter filter{};
    filter.categoryBits = 0x0002;
    filter.maskBits = 0x0001;

    for(auto n{ 0 }; n < 6; ++n)
    {
        const auto radians{ static_cast<float>((this->sensors[n].first / 180.0) * M_PI) };

        const auto start{ this->carBody->GetWorldPoint(this->sensors[n].second)};
        const auto end{ this->carBody->GetWorldPoint(this->sensors[n].second + b2Mul(b2Rot{ -radians }, b2Vec2{ 0.0f, this->current.distances[n] }))};

        // Crosshair
        static constexpr SDL_Color sensorColor{ 0,0,255,255 };
        GPU_Line(target, end.x - 0.05f, end.y, end.x + 0.05f, end.y, sensorColor);
        GPU_Line(target, end.x, end.y - 0.05f, end.x, end.y + 0.05f, sensorColor);
    }
}

auto Replay::stepFile() -> void
{
    if (file.is_open())
    {
        ++this->current.line;

        auto moves{ std::array<int, 5>{} };

        auto separator{ ';' };

        file >> this->current.distances[0] >> separator
            >> this->current.distances[1] >> separator
            >> this->current.distances[2] >> separator
            >> this->current.distances[3] >> separator
            >> this->current.distances[4] >> separator
            >> this->current.distances[5] >> separator
            >> moves[0] >> separator
            >> moves[1] >> separator
            >> moves[2] >> separator
            >> moves[3] >> separator
            >> moves[4] >> separator;

        if (file.eof())
        {
            file.close();
        }

        if (moves[0])
            this->current.move = Move::STOP;
        else if (moves[1])
            this->current.move = Move::MOVE_FORWARD;
        else if (moves[2])
            this->current.move = Move::MOVE_BACKWARD;
        else if (moves[3])
            this->current.move = Move::ROTATE_LEFT;
        else if (moves[4])
            this->current.move = Move::ROTATE_RIGHT;
    }
    else
    {
        this->current.move = Move::STOP;
    }
}

auto Replay::stepPath() -> void
{

}

auto Replay::stepCar() -> void
{
    this->carBody->SetAngularVelocity(0.0f);
    this->carBody->SetLinearVelocity({ 0.0f,0.0f });

    if (this->current.move == Move::ROTATE_LEFT)
    {
        this->carBody->SetAngularVelocity(-4.0f);
    }
    else if (this->current.move == Move::ROTATE_RIGHT)
    {
        this->carBody->SetAngularVelocity(+4.0f);
    }
    else if (this->current.move == Move::MOVE_FORWARD)
    {
        const auto point{ this->carBody->GetWorldVector(b2Vec2{ 0.0f, +1.0f }) };
        this->carBody->SetLinearVelocity(point);
    }
    else if (this->current.move == Move::MOVE_BACKWARD)
    {
        const auto point{ this->carBody->GetWorldVector(b2Vec2{ 0.0f, -1.0f }) };
        this->carBody->SetLinearVelocity(point);
    }

    this->current.move = Move::STOP;
}

auto Replay::stepSensor() -> void
{
    
}