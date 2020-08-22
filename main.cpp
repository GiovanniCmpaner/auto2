#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <cstring>

#include <box2d/box2d.h>
#include <SDL_gpu.h>

#include "maze.hpp"

static const auto borderColor{ SDL_Color{ 255, 0, 255, 255 } };
static const auto fillColor{ SDL_Color{ 255, 0, 255, 64 } };

static const auto gravity{ b2Vec2{ 0.0, 0.0 } };
static auto world{ b2World{ gravity } };

static auto quit{ false };

class RayCastCallback : public b2RayCastCallback
{
public:
    float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override
    {
        this->fixture = fixture;
        this->point = point;
        this->normal = normal;

        return fraction;
    }
    b2Fixture* fixture{ nullptr };
    b2Vec2 point{ 0.0, 0.0 };
    b2Vec2 normal{ 0.0, 0.0 };
};


static int resizingEventWatcher(void* data, SDL_Event* event)
{
    return 0;
}

auto drawSensor(GPU_Target* target, b2World* world, b2Body* body)
{
    const auto text{ reinterpret_cast<const char*>(body->GetUserData()) };
    if (text == nullptr || std::strcmp(text, "car") != 0)
    {
        return;
    }

    { // Front
        auto start{ body->GetWorldPoint(b2Vec2{ 0.0f, 0.2f }) };
        auto end{ body->GetWorldPoint(b2Vec2{ 0.0f, 5.0f }) };

        auto callback{ RayCastCallback{} };
        world->RayCast(&callback, start, end);
        if (callback.fixture)
        {
            GPU_Line(target, start.x, start.y, callback.point.x, callback.point.y, { 0,0,255,255 });
        }
        else
        {
            GPU_Line(target, start.x, start.y, end.x, end.y, { 0,0,255,255 });
        }
    }
    { // Left
        auto start{ body->GetWorldPoint(b2Vec2{ -0.2f, 0.0f }) };
        auto end{ body->GetWorldPoint(b2Vec2{ -5.0f, 0.0f }) };

        auto callback{ RayCastCallback{} };
        world->RayCast(&callback, start, end);
        if (callback.fixture)
        {
            GPU_Line(target, start.x, start.y, callback.point.x, callback.point.y, { 0,0,255,255 });
        }
        else
        {
            GPU_Line(target, start.x, start.y, end.x, end.y, { 0,0,255,255 });
        }
    }
    { // Right
        auto start{ body->GetWorldPoint(b2Vec2{ +0.2f, 0.0f }) };
        auto end{ body->GetWorldPoint(b2Vec2{ +5.0f, 0.0f }) };

        auto callback{ RayCastCallback{} };
        world->RayCast(&callback, start, end);
        if (callback.fixture)
        {
            GPU_Line(target, start.x, start.y, callback.point.x, callback.point.y, { 0,0,255,255 });
        }
        else
        {
            GPU_Line(target, start.x, start.y, end.x, end.y, { 0,0,255,255 });
        }
    }
}

auto drawWorld(GPU_Target* target, b2World* world) -> void
{
    for (auto body{ world->GetBodyList() }; body != nullptr; body = body->GetNext())
    {
        drawSensor(target, world, body);

        const auto transform{ body->GetTransform() };
        for (auto fixture{ body->GetFixtureList() }; fixture != nullptr; fixture = fixture->GetNext())
        {
            switch (fixture->GetType())
            {
                case b2Shape::e_circle:
                {
                    const auto circle{ reinterpret_cast<const b2CircleShape*>(fixture->GetShape()) };

                    const auto center{ b2Mul(transform, circle->m_p) };
                    const auto axis{ b2Mul(transform.q, b2Vec2{ 1.0f, 0.0 }) };
                    const auto line{ center + circle->m_radius * axis };

                    GPU_Circle(target, center.x, center.y, circle->m_radius, borderColor);
                    GPU_CircleFilled(target, center.x, center.y, circle->m_radius, fillColor);
                    GPU_Line(target, center.x, center.y, line.x, line.y, borderColor);

                    break;
                }
                case b2Shape::e_edge:
                {
                    const auto edge{ reinterpret_cast<const b2EdgeShape*>(fixture->GetShape()) };

                    const auto v1{ b2Mul(transform, edge->m_vertex1) };
                    const auto v2{ b2Mul(transform, edge->m_vertex2) };

                    GPU_Line(target, v1.x, v1.y, v2.x, v2.y, borderColor);
                    if (edge->m_oneSided == false)
                    {
                        GPU_CircleFilled(target, v1.x, v1.y, 0.2f, borderColor);
                        GPU_CircleFilled(target, v2.x, v2.y, 0.2f, borderColor);
                    }

                    break;
                }
                case b2Shape::e_chain:
                {
                    const auto chain{ reinterpret_cast<const b2ChainShape*>(fixture->GetShape()) };

                    auto v1{ b2Mul(transform, chain->m_vertices[0]) };

                    for (auto i{ 1 }; i < chain->m_count; ++i)
                    {
                        const auto v2{ b2Mul(transform, chain->m_vertices[i]) };
                        GPU_Line(target, v1.x, v1.y, v2.x, v2.y, borderColor);
                        v1 = v2;
                    }

                    break;
                }
                case b2Shape::e_polygon:
                {
                    const auto polygon{ reinterpret_cast<const b2PolygonShape*>(fixture->GetShape()) };

                    float vertices[2 * b2_maxPolygonVertices];
                    for (auto i{ 0 }; i < polygon->m_count; ++i)
                    {
                        const auto vertice{ b2Mul(transform, polygon->m_vertices[i]) };
                        vertices[i * 2 + 0] = vertice.x;
                        vertices[i * 2 + 1] = vertice.y;
                    }

                    GPU_Polygon(target, polygon->m_count, vertices, borderColor);
                    GPU_PolygonFilled(target, polygon->m_count, vertices, fillColor);

                    break;
                }
            }
        }
    }
}

int main(int argc, char* args[])
{
    GPU_SetRequiredFeatures(GPU_FEATURE_BASIC_SHADERS);
    const auto target{ GPU_InitRenderer(GPU_RENDERER_OPENGL_3, 500, 500, GPU_DEFAULT_INIT_FLAGS) };
    //SDL_AddEventWatch(resizingEventWatcher, nullptr);
    GPU_SetLineThickness(0.02f);

    { // Maze

        const auto tiles{ Maze::make(5,5) };
        Maze::print(tiles);
        const auto rectangles{ Maze::rectangles(tiles, -2.5f, -2.5f, 5.0f, 5.0f, 0.02f) };

        for (const auto& rect : rectangles)
        {
            b2PolygonShape polygon{};
            polygon.SetAsBox(rect.width / 2.0, rect.height / 2.0);

            b2BodyDef bd{};
            bd.type = b2_staticBody;
            bd.position = b2Vec2{ rect.x + rect.width / 2.0f, rect.y + rect.height / 2.0f };

            const auto wall{ world.CreateBody(&bd) };
            wall->SetUserData((void*)"wall");
            wall->CreateFixture(&polygon, 0.0);
        }
    }

    b2Body* ground{ nullptr };
    {
        b2BodyDef bd{};
        bd.position = b2Vec2{ 0.0f, 0.0f };
        ground = world.CreateBody(&bd);

        b2EdgeShape shape{};

        b2FixtureDef sd{};
        sd.shape = &shape;
        sd.density = 0.0f;
        sd.restitution = 0.4f;

        // Left vertical
        shape.SetTwoSided(b2Vec2(-5.0f, -5.0f), b2Vec2(-5.0f, 5.0f));
        ground->CreateFixture(&sd);

        // Right vertical
        shape.SetTwoSided(b2Vec2(5.0f, -5.0f), b2Vec2(5.0f, 5.0f));
        ground->CreateFixture(&sd);

        // Top horizontal
        shape.SetTwoSided(b2Vec2(-5.0f, 5.0f), b2Vec2(5.0f, 5.0f));
        ground->CreateFixture(&sd);

        // Bottom horizontal
        shape.SetTwoSided(b2Vec2(-5.0f, -5.0f), b2Vec2(5.0f, -5.0f));
        ground->CreateFixture(&sd);
    }

    b2Body* car{ nullptr };
    {
        b2PolygonShape polygon{};
        polygon.SetAsBox(0.2, 0.2);

        b2BodyDef bd{};
        bd.type = b2_dynamicBody;
        bd.position = b2Vec2{ 0, 0 };

        car = world.CreateBody(&bd);
        car->SetUserData((void*)"car");

        b2FixtureDef fd{};
        fd.shape = &polygon;
        fd.density = 2.0f;
        fd.friction = 0.5f;

        car->CreateFixture(&fd);
        car->SetAngularDamping(5.0f);

        const auto gravity{ 10.0f };
        const auto inertia{ car->GetInertia() };
        const auto mass{ car->GetMass() };
        const auto radius{ b2Sqrt(2.0f * inertia / mass) };

        b2FrictionJointDef jd{};
        jd.bodyA = ground;
        jd.bodyB = car;
        jd.localAnchorA = b2Vec2{ 0.0f, 0.0f };
        jd.localAnchorB = car->GetLocalCenter();
        jd.collideConnected = true;
        jd.maxForce = 0.5f * mass * gravity;
        jd.maxTorque = 3.5f * mass * radius * gravity;

        world.CreateJoint(&jd);
    }

    static constexpr auto fps{ 60 };
    static constexpr auto timeStep{ 1.0 / fps };
    auto targetTicks{ SDL_GetTicks() };
    while (not quit)
    {
        SDL_Event e{};
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                quit = true;
            }
        }
        SDL_PumpEvents();
        const auto state{ SDL_GetKeyboardState(nullptr) };
        if (state[SDL_SCANCODE_A])
        {
            car->ApplyTorque(+3.0f, true);
        }
        if (state[SDL_SCANCODE_D])
        {
            car->ApplyTorque(-3.0f, true);
        }
        if (state[SDL_SCANCODE_W])
        {
            const auto force{ car->GetWorldVector(b2Vec2{ 0.0f, +2.0f }) };
            const auto point{ car->GetWorldPoint(b2Vec2{ 0.0f, 0.2f }) };
            car->ApplyForce(force, point, true);
        }
        if (state[SDL_SCANCODE_S])
        {
            const auto force{ car->GetWorldVector(b2Vec2{ 0.0f, -2.0f }) };
            const auto point{ car->GetWorldPoint(b2Vec2{ 0.0f, 0.2f }) };
            car->ApplyForce(force, point, true);
        }

        world.Step(timeStep, 4, 4);

        // Update logic here

        GPU_ClearColor(target, { 52, 52, 52, 255 });
        GPU_Clear(target);

        // Draw stuff here

        GPU_MatrixMode(target, GPU_PROJECTION);
        GPU_LoadIdentity();
        GPU_Ortho(-5, +5, -5, +5, 0, 1); // Escala do mundo
        GPU_Translate(0, 0, 0); // Translação da câmera

        drawWorld(target, &world);

        GPU_Flip(target);

        const auto currentTicks{ SDL_GetTicks() };
        targetTicks += 1000 / fps;
        if (targetTicks > currentTicks)
        {
            SDL_Delay(targetTicks - currentTicks);
        }
    }

    GPU_Quit();

    return EXIT_SUCCESS;
}

