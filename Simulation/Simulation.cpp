#include <algorithm>

#include <tiny_dnn/tiny_dnn.h>

#include "Simulation.hpp"

auto Simulation::init() -> void
{
    this->ground = createGround(&world);
    this->maze.init(&world, ground);
    this->cars = std::vector<Car>{ 100 };

    for (auto& car : this->cars)
    {
        car.init(&world, ground);
    }

    window.init(Simulation::realWidth, Simulation::realHeight);

    window.onKeyboard([&](const uint8_t* state) -> void
    {
        rotate = 0;
        if (state[SDL_SCANCODE_A])
        {
            this->cars.front().rotateLeft();
            rotate += 1;
        }
        if (state[SDL_SCANCODE_D])
        {
            this->cars.front().rotateRight();
            rotate -= 1;
        }

        move = 0;
        if (state[SDL_SCANCODE_W])
        {
            this->cars.front().moveForward();
            move += 1;
        }
        if (state[SDL_SCANCODE_S])
        {
            this->cars.front().moveBackward();
            move -= 1;
        }
    });

    window.onRender([&](GPU_Target* target)
        {
            for (auto& car : this->cars)
            {
                car.step();
            }
            maze.step();
            world.Step(Window::timeStep, 6, 6);

            for (auto& car : this->cars)
            {
                car.render(target);
            }
            maze.render(target);
        });

    window.onInfos([&](std::ostringstream& oss)
        {
            oss << "front = " << this->cars.front().distanceFront() << " m \n"
                << "left = " << this->cars.front().distanceLeft() << " m \n"
                << "right = " << this->cars.front().distanceRight() << " m \n"
                << "move = " << (move > 0 ? "+" : move < 0 ? "-" : "o") << " \n"
                << "rotate = " << (rotate > 0 ? "+" : rotate < 0 ? "-" : "o") << " \n"
                << "collided = " << this->cars.front().collided() << " \n";
        });

    window.process();
}

auto Simulation::createGround(b2World* world) -> b2Body*
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

