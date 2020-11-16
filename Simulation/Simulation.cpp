#include <algorithm>

#include <tiny_dnn/tiny_dnn.h>

#include "Simulation.hpp"

static auto constructCNN() -> void
{
    using namespace tiny_dnn;

    network<sequential> net{};

    // add layers
    net << layers::fc(6,10) << activation::tanh()
        << layers::fc(10,10) << activation::tanh()
        << layers::fc(10,10) << activation::tanh()
        << layers::fc(10,4);

    assert(net.in_data_size() == 10);
    assert(net.out_data_size() == 4);

    {
        std::vector<vec_t> data{ { 1, 0 }, { 0, 2 } };
        std::vector<vec_t>  out{ { 2 },    { 1 } };

        // declare optimization algorithm
        gradient_descent optimizer{};

        // train (1-epoch, 30-minibatch)
        net.fit<mse, gradient_descent>(optimizer, data, out, 30, 1);
    }

    // save
    net.save("C:/Users/Giovanni/Desktop/auto2/net.json", content_type::weights_and_model, file_format::json);
}

auto Simulation::init() -> void
{
    constructCNN();

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

