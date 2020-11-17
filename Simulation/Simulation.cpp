#include <algorithm>


#include "Simulation.hpp"

auto Simulation::constructDNN() -> void
{
    using namespace tiny_dnn;

    // 6 x sensores distancia
    // 2 x giroscopio (x,y)
    // 2 x acelerometro (x,y)
    // 1 x cor

    net << layers::fc(13, 15) << activation::tanh()
        << layers::fc(15, 15) << activation::tanh()
        << layers::fc(15, 15) << activation::tanh()
        << layers::fc(15, 4);

    assert(net.in_data_size() == 13);
    assert(net.out_data_size() == 4);

    //std::vector<vec_t> train_data{ { 1,2,3,4,5,6,7,8,9,10,11 }, { 1,2,3,4,5,6,7,8,9,10,11 } };
    //std::vector<label_t> train_labels{ 1, 2 };
    //
    //adagrad optimizer{};
    //net.train<mse, adagrad>(optimizer, train_data, train_labels, 30, 10);

    //net.save("C:/Users/Giovanni/Desktop/auto2/net.json", content_type::weights_and_model, file_format::json);
}

auto Simulation::init() -> void
{
    this->ground = createGround(&world);
    this->maze.init(&world, ground);
    //this->enviroment.init(&world, ground);

    this->cars = std::vector<Car>{ 100 };

    for (auto& car : this->cars)
    {
        car.init(&world, ground);
    }

    this->constructDNN();

    this->window.init(Simulation::realWidth, Simulation::realHeight);

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
        // Estado atual da simulação

        auto collisions{ 0 };

        for (auto& car : this->cars)
        {
            if (car.collided())
            {
                collisions++;
            }
            else
            {
                const auto distances{ car.distances() };
                const auto color{ car.color() };
                const auto giroscope{ car.giroscope() };
                const auto acelerometer{ car.acelerometer() };

                auto inputs{ std::vector<float>{} };
                inputs.insert(distances);
                inputs.insert(color);
                inputs.insert(giroscope);
                inputs.insert(acelerometer);

                auto direction{ net.predict_label(inputs) };
                if (direction == 0)
                {
                    car.moveForward();
                }

                car.step();
            }
        }

        if (collisions == this->cars.size())
        {
            auto longest{ 0 };
            for (auto n{ 1 }; n < this->cars.size(); ++n)
            {
                if (this->cars[n].timer > this->cars[longest].timer)
                {
                    longest = n;
                }
            }
            for (auto& layer : net)
            {
                for (auto& weights : layer->weights())
                {
                    for (auto& weight : *weights)
                    {
                        const auto multiplier{ tiny_dnn::uniform_rand(0.95f, 1.05f) };


                        weight = std::clamp(weight * multiplier, -1.0f, +1.0f);
                    }
                }
            }
        }
        else
        {
            maze.step();
            world.Step(Window::timeStep, 1, 10);
        }

        for (auto& car : this->cars)
        {
            car.render(target);
        }
        maze.render(target);
    });

    window.onInfos([&](std::ostringstream& oss)
    {
        const auto distances{ this->cars.front().distances() };

        oss << "front = " << distances.at(0) << " m \n"
            << "left = " << distances.at(+90) << " m \n"
            << "right = " << distances.at(-90) << " m \n"
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

