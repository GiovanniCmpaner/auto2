#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <cstring>

#include <box2d/box2d.h>
#include <SDL_gpu.h>
#include "SDL_FontCache.h"

#include "maze.hpp"

static int screenWidth{ 800 };
static int screenHeight{ 800 };
static GPU_Target* target{ nullptr };

static float realWidth{ 4.0f };
static float realHeight{ 4.0f };
static const b2Vec2 gravity{ 0.0, 0.0 };
static b2World world{ gravity };

static bool quit{ false };
static float front{ 0.0f };
static float left{ 0.0f };
static float right{ 0.0f };
static int move{ 0 };
static int rotate{ 0 };

static constexpr SDL_Color backgroundColor{ 0, 0, 0, 255 };
static constexpr SDL_Color fontColor{ 0, 255, 0, 255 };
static constexpr SDL_Color sensorColor{ 0,0,255,255 };
static constexpr SDL_Color solidBorderColor{ 255, 0, 255, 255 };
static constexpr SDL_Color solidFillColor{ 255, 0, 255, 64 };

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


static int resizingEventWatcher(void* data, SDL_Event* event)
{
    return 0;
}

auto drawSensor(GPU_Target* target, b2World* world, b2Body* body)
{
    const auto text{ reinterpret_cast<const char*>(body->GetUserData()) };
    if (text == nullptr or std::strcmp(text, "car") != 0)
    {
        return;
    }

    b2Filter filter{};
    filter.categoryBits = 0x0002;
    filter.maskBits = 0x0001;

    { // Front
        auto start{ body->GetWorldPoint(b2Vec2{ 0.0f, 0.2f }) };
        auto end{ body->GetWorldPoint(b2Vec2{ 0.0f, 2.2f }) };

        auto callback{ RayCastCallback{&filter} };
        world->RayCast(&callback, start, end);
        if (callback.valid)
        {
            end = callback.point;
            GPU_Line(target, end.x - 0.075f, end.y, end.x + 0.075f, end.y, sensorColor);
            GPU_Line(target, end.x, end.y - 0.075f, end.x, end.y + 0.075f, sensorColor);
        }
        front = b2Distance(start, end);
    }
    { // Left
        auto start{ body->GetWorldPoint(b2Vec2{ -0.2f, 0.0f }) };
        auto end{ body->GetWorldPoint(b2Vec2{ -2.2f, 0.0f }) };

        auto callback{ RayCastCallback{&filter} };
        world->RayCast(&callback, start, end);
        if (callback.valid)
        {
            end = callback.point;
            GPU_Line(target, end.x - 0.075f, end.y, end.x + 0.075f, end.y, sensorColor);
            GPU_Line(target, end.x, end.y - 0.075f, end.x, end.y + 0.075f, sensorColor);
        }
        right = b2Distance(start, end);
    }
    { // Right
        auto start{ body->GetWorldPoint(b2Vec2{ +0.2f, 0.0f }) };
        auto end{ body->GetWorldPoint(b2Vec2{ +2.2f, 0.0f }) };

        auto callback{ RayCastCallback{&filter} };
        world->RayCast(&callback, start, end);
        if (callback.valid)
        {
            end = callback.point;
            GPU_Line(target, end.x - 0.075f, end.y, end.x + 0.075f, end.y, sensorColor);
            GPU_Line(target, end.x, end.y - 0.075f, end.x, end.y + 0.075f, sensorColor);
        }
        right = b2Distance(start, end);
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

                    GPU_Circle(target, center.x, center.y, circle->m_radius, solidBorderColor);
                    GPU_CircleFilled(target, center.x, center.y, circle->m_radius, solidFillColor);
                    GPU_Line(target, center.x, center.y, line.x, line.y, solidBorderColor);

                    break;
                }
                case b2Shape::e_edge:
                {
                    const auto edge{ reinterpret_cast<const b2EdgeShape*>(fixture->GetShape()) };

                    const auto v1{ b2Mul(transform, edge->m_vertex1) };
                    const auto v2{ b2Mul(transform, edge->m_vertex2) };

                    GPU_Line(target, v1.x, v1.y, v2.x, v2.y, solidBorderColor);

                    break;
                }
                case b2Shape::e_chain:
                {
                    const auto chain{ reinterpret_cast<const b2ChainShape*>(fixture->GetShape()) };

                    auto v1{ b2Mul(transform, chain->m_vertices[0]) };

                    for (auto i{ 1 }; i < chain->m_count; ++i)
                    {
                        const auto v2{ b2Mul(transform, chain->m_vertices[i]) };
                        GPU_Line(target, v1.x, v1.y, v2.x, v2.y, solidBorderColor);
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

                    GPU_Polygon(target, polygon->m_count, vertices, solidBorderColor);
                    GPU_PolygonFilled(target, polygon->m_count, vertices, solidFillColor);

                    break;
                }
            }
        }
    }
}

auto createMaze(b2World* world) -> b2Body*
{
    const auto tiles{ Maze::make(7,7) };
    const auto rectangles{ Maze::rectangles(tiles, -realWidth / 2.0f, -realHeight / 2.0f, realWidth, realHeight, 0.05f) };

    b2BodyDef bd{};
    bd.type = b2_staticBody;
    bd.position = b2Vec2{ 0.0f, 0.0f };
    bd.userData = const_cast<char*>("maze");

    auto maze{ world->CreateBody(&bd) };

    for (const auto& rect : rectangles)
    {
        b2PolygonShape shape{};

        b2FixtureDef fd{};
        fd.shape = &shape;
        fd.density = 0.0f;
        fd.restitution = 0.4f;
        fd.filter.categoryBits = 0x0001;
        fd.filter.maskBits = 0x0003;
        fd.userData = const_cast<char*>("wall");

        shape.SetAsBox(rect.width / 2.0f, rect.height / 2.0f, b2Vec2{ rect.x + rect.width / 2.0f, rect.y + rect.height / 2.0f }, 0.0f);
        maze->CreateFixture(&fd);
    }

    return maze;
}

auto createGround(b2World* world) -> b2Body*
{
    b2BodyDef bd{};
    bd.position = b2Vec2{ 0.0f, 0.0f };
    bd.userData = const_cast<char*>("ground");

    auto ground{ world->CreateBody(&bd) };

    b2EdgeShape shape{};

    b2FixtureDef fd{};
    fd.shape = &shape;
    fd.density = 0.0f;
    fd.restitution = 0.4f;
    fd.filter.categoryBits = 0x0001;
    fd.filter.maskBits = 0x0003;
    fd.userData = const_cast<char*>("wall");

    // Left vertical
    shape.SetTwoSided(b2Vec2{ -realWidth, -realHeight }, b2Vec2{ -realWidth, realHeight });
    ground->CreateFixture(&fd);

    // Right vertical
    shape.SetTwoSided(b2Vec2{ realWidth, -realHeight }, b2Vec2{ realWidth, realHeight });
    ground->CreateFixture(&fd);

    // Top horizontal
    shape.SetTwoSided(b2Vec2{ -realWidth, realHeight }, b2Vec2{ realWidth, realHeight });
    ground->CreateFixture(&fd);

    // Bottom horizontal
    shape.SetTwoSided(b2Vec2{ -realWidth, -realHeight }, b2Vec2{ realWidth, -realHeight });
    ground->CreateFixture(&fd);

    return ground;
}

auto createCar(b2World* world, b2Body* ground) -> b2Body*
{
    b2BodyDef bd{};
    bd.type = b2_dynamicBody;
    bd.position = b2Vec2{ 0, 0 };
    bd.angularDamping = 6.0f;
    bd.userData = const_cast<char*>("car");

    auto car{ world->CreateBody(&bd) };

    { // Chassis

        b2PolygonShape border{};
        border.SetAsBox(0.1, 0.1);

        b2FixtureDef fd{};
        fd.shape = &border;
        fd.density = 2.0f;
        fd.friction = 0.5f;
        fd.filter.categoryBits = 0x0002;
        fd.filter.maskBits = 0x0001;
        fd.userData = const_cast<char*>("chassis");

        car->CreateFixture(&fd);

        { // Triangle
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

            car->CreateFixture(&fd);
        }
    }

    { // Top-down friction
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
        jd.maxForce = 1.2f * mass * gravity;
        jd.maxTorque = 9.7f * mass * radius * gravity;

        world->CreateJoint(&jd);
    }

    return car;
}

int main(int argc, char* args[])
{
    // TODO:
    // Separar Simulation, Car, Rede Neural, Fuzzy
    // Objetivo Circulo Vermelho

    GPU_SetDebugLevel(GPU_DEBUG_LEVEL_MAX);
    GPU_SetRequiredFeatures(GPU_FEATURE_BASIC_SHADERS);
    target = GPU_InitRenderer(GPU_RENDERER_OPENGL_3, 800, 800, GPU_DEFAULT_INIT_FLAGS);
    //SDL_AddEventWatch(resizingEventWatcher, nullptr);
    GPU_SetLineThickness(0.02f);

    auto font{ FC_CreateFont() };
    FC_LoadFont(font, "C:/Windows/Fonts/Arial.ttf", screenHeight / 40, fontColor, TTF_STYLE_NORMAL);

    createMaze(&world);
    auto ground{ createGround(&world) };
    auto car{ createCar(&world, ground) };
    createCar(&world, ground);

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

        rotate = 0;
        if (state[SDL_SCANCODE_A])
        {
            car->ApplyTorque(+1.0f, true);
            rotate += 1;
        }
        if (state[SDL_SCANCODE_D])
        {
            car->ApplyTorque(-1.0f, true);
            rotate -= 1;
        }

        move = 0;
        if (state[SDL_SCANCODE_W])
        {
            const auto force{ car->GetWorldVector(b2Vec2{ 0.0f, +1.0f }) };
            const auto point{ car->GetWorldPoint(b2Vec2{ 0.0f, 0.2f }) };
            car->ApplyForce(force, point, true);
            move += 1;
        }
        if (state[SDL_SCANCODE_S])
        {
            const auto force{ car->GetWorldVector(b2Vec2{ 0.0f, -1.0f }) };
            const auto point{ car->GetWorldPoint(b2Vec2{ 0.0f, 0.2f }) };
            car->ApplyForce(force, point, true);
            move -= 1;
        }

        world.Step(timeStep, 6, 6);

        // Update logic here

        GPU_ClearColor(target, backgroundColor);

        // Draw stuff here
        {
            GPU_MatrixMode(target, GPU_PROJECTION);
            GPU_LoadIdentity();
            GPU_Ortho(-realHeight, +realHeight, -realWidth, +realWidth, 0, 1);

            GPU_MatrixMode(target, GPU_MODEL);
            GPU_LoadIdentity();
            drawWorld(target, &world);
        }

        {
            GPU_MatrixMode(target, GPU_PROJECTION);
            GPU_LoadIdentity();
            GPU_Ortho(0, screenHeight, screenWidth, 0, 0, 1);

            GPU_MatrixMode(target, GPU_MODEL);
            GPU_LoadIdentity();
            FC_Draw(font, target, 5, 5,
                "front = %.2f m \n"
                "left = %.2f m \n"
                "right = %.2f m \n"
                "move = %s \n"
                "rotate = %s \n",
                front,
                left,
                right,
                ( move > 0 ? "+" : move < 0 ? "-" : "o" ),
                ( rotate > 0 ? "+" : rotate < 0 ? "-" : "o" )
            );
        }
        //FC_Draw(font, target, -screenWidth / 2, -230, "left = %.2f m", left);
        //FC_Draw(font, target, -screenWidth / 2, -210, "right = %.2f m", right);
        //FC_Draw(font, target, -screenWidth / 2, -190, "move = %s%d", move > 0 ? "+" : move < 0 ? "-" : "", abs(move));
        //FC_Draw(font, target, -screenWidth / 2, -170, "rotate = %s%d", rotate > 0 ? "+" : rotate < 0 ? "-" : "", abs(rotate));

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

