#include <algorithm>
#include <filesystem>

#include "Simulation.hpp"
#include "Follower.hpp"


auto randomize(tiny_dnn::network<tiny_dnn::sequential>& net) -> void
{
    for (auto& layer : net)
    {
        for (auto& weights : layer->weights())
        {
            for (auto& weight : *weights)
            {
                const auto direction{ tiny_dnn::uniform_rand(0, 1) };
                const auto multiplier{ tiny_dnn::uniform_rand(0.9f, +1.1f) };
                const auto offset{ tiny_dnn::uniform_rand(-0.1f, +0.1f) };
 
                weight = weight + offset;
            }
        }
    }
}

auto clone(const tiny_dnn::network<tiny_dnn::sequential>& other) -> tiny_dnn::network<tiny_dnn::sequential>
{
    auto net{ tiny_dnn::network<tiny_dnn::sequential>{} };

    auto ss{ std::stringstream{} };

    auto bo{ cereal::BinaryOutputArchive{ ss } };
    other.to_archive(bo, tiny_dnn::content_type::weights_and_model);

    auto bi{ cereal::BinaryInputArchive{ ss } };
    net.from_archive(bi, tiny_dnn::content_type::weights_and_model);

    return net;
}

auto Simulation::constructDNN() -> void
{
    //using namespace tiny_dnn;
    //{
    //    // 6 x sensores distancia
    //    // (2) x giroscopio (x,y)
    //    // (2) x acelerometro (x,y)
    //    // 1 x cor
    //    auto net{ tiny_dnn::network<tiny_dnn::sequential>{} };
    //
    //    if (std::filesystem::exists("net.bin"))
    //    {
    //        net.load("net.bin");
    //    }
    //    else
    //    {
    //        net << layers::fc(7, 15) << activation::tanh()
    //            << layers::fc(15, 15) << activation::tanh()
    //            << layers::fc(15, 15) << activation::tanh()
    //            << layers::fc(15, 5);
    //
    //        net.init_weight();
    //        net.save("net.bin");
    //    }
    //}

    using namespace tiny_dnn;

    this->base << layers::fc(7, 15) << activation::tanh()
            << layers::fc(15, 15) << activation::tanh()
            << layers::fc(15, 15) << activation::tanh()
            << layers::fc(15, 5);

    if (std::filesystem::exists("net_trained.bin"))
    {
        base.load("net_trained.bin");
    }
    else
    {
        base.init_weight();
    }

    assert(base.in_data_size() == 7);
    assert(base.out_data_size() == 5);

    //std::vector<vec_t> train_data{ { 1,2,3,4,5,6,7,8,9,10,11 }, { 1,2,3,4,5,6,7,8,9,10,11 } };
    //std::vector<label_t> train_labels{ 1, 2 };
    //
    //adagrad optimizer{};
    //net.train<mse, adagrad>(optimizer, train_data, train_labels, 30, 10);

    //net.save("C:/Users/Giovanni/Desktop/auto2/net.bin", content_type::weights_and_model, file_format::json);
}

auto Simulation::reset() -> void
{
    this->mazes.clear();
    this->cars.clear();
    this->solutions.clear();
    this->followers.clear();
    this->nets.clear();
    this->positions.clear();
    this->distances.clear();

    this->mazes.reserve(200);
    this->cars.reserve(200);
    this->solutions.reserve(200);
    this->followers.reserve(200);
    this->nets.reserve(200);
    this->positions.reserve(200);
    this->distances.reserve(200);

    auto& maze{ this->mazes.emplace_back(&world, ground, 5, 5, 0, 0, 3.0f, 3.0f) };

    for (auto j{ 0 }; j < 1; ++j)
    {
        for (auto i{ 0 }; i < 1; ++i)
        {
            //auto& maze{ this->mazes.emplace_back(&world, ground, 5, 5, i * 3.2f, j * 3.2f, 3.0f, 3.0f) };
            //
            //maze.randomize();

            auto& car{ this->cars.emplace_back(&world, ground, maze.start()) };

            auto& position{ this->positions.emplace_back(car.position()) };

            auto& distance{ this->distances.emplace_back(0.0f) };

            auto& solution{ this->solutions.emplace_back(maze.solve(car.position())) };

            auto& follower{ this->followers.emplace_back(&car, solution) };

            auto& net{ this->nets.emplace_back(clone(this->base)) };

            randomize(net);
        }
    }

    this->nets.front() = clone(this->base);
}


auto Simulation::init() -> void
{
    this->window.init(Simulation::realWidth, Simulation::realHeight);
    this->ground = this->createGround(&world);

    this->constructDNN();
    this->reset();
    this->start = this->window.now();
    this->generation = 0;
    
    window.onKeyboard([&](const uint8_t* state) -> void
        {
            this->move = Move::STOP;
            if (state[SDL_SCANCODE_A])
            {
                this->move = Move::ROTATE_LEFT;
            }
            else if (state[SDL_SCANCODE_D])
            {
                this->move = Move::ROTATE_RIGHT;
            }
            else if (state[SDL_SCANCODE_W])
            {
                this->move = Move::MOVE_FORWARD;
            }
            else if (state[SDL_SCANCODE_S])
            {
                this->move = Move::MOVE_BACKWARD;
            }
            //this->cars.front().doMove(this->move);
        });

    auto trained{ true };

    auto bestCar{ -1 };
    auto bestRatio{ 0.0f };

    window.onRender([&](GPU_Target* target)
        {
            const auto now{ this->window.now() };
            if (now - start > 65000)
            {
                this->reset();
                this->generation++;
                bestCar = -1;
                bestRatio = 0.0f;
                this->start = this->window.now();
            }
            else if (bestCar != -1 and now - start > 60000)
            {
                this->cars[bestCar].render(target);
                this->mazes[0].render(target);
            }
            else if (bestCar == -1 and now - start > 60000)
            {
                auto bestRatio{ 0 };
                for (auto n{ 0 }; n < this->cars.size(); ++n)
                {
                    //const auto solutionFromStart{ this->mazes[0].solve(this->mazes[0].start()) };
                    //const auto solutionFromCurrent{ this->mazes[0].solve(this->cars[n].position()) };
                    //
                    //if (not solutionFromCurrent.empty())
                    //{
                    //    const auto ratio{ Simulation::distance(solutionFromCurrent) / Simulation::distance(solutionFromStart) };
                    //
                    //    if (nearestCar == -1 or ratio < nearestRatio)
                    //    {
                    //        nearestCar = n;
                    //        nearestRatio = ratio;
                    //    }
                    //}

                    const auto solutionFromCurrent{ this->mazes[0].solve(this->cars[n].position()) };
                    if (not solutionFromCurrent.empty())
                    {
                        const auto distanceFromSolution{ Simulation::distance(solutionFromCurrent) };
                        const auto distanceTraveled{ this->distances[n] };

                        const auto ratio{ distanceFromSolution };
                        if (bestCar == -1 or ratio < bestRatio)
                        {
                            bestCar = n;
                            bestRatio = ratio;
                        }
                    }
                }

                this->base = clone(this->nets[bestCar]);
                this->base.save("net_trained.bin");
            }
            else
            {
//#pragma omp parallel for
//                for (auto n{ 0 }; n < this->cars.size(); ++n)
//                {
//                    if (not this->cars[n].collided())
//                    {
//                        this->distances[n] += b2Distance(this->positions[n], this->cars[n].position());
//                        this->positions[n] = this->cars[n].position();
//
//                        const auto inputs{ Simulation::inputs(this->cars[n]) };
//                        const auto prediction{ this->nets[n].predict_label(inputs) };
//                        this->cars[n].doMove(static_cast<Move>(prediction));
//                    }
//                }

                for (auto& follower : this->followers)
                {
                    follower.step();
                }

                for (auto& car : this->cars)
                {
                    car.step(Window::timeStep);
                }

                for (auto& maze : this->mazes)
                {
                    maze.step();
                }

                world.Step(Window::timeStep, 4, 4);

                for (auto& follower : this->followers)
                {
                    follower.render(target);
                }

                for (auto& car : this->cars)
                {
                    if (not car.stucked() and not car.collided())
                    {
                        car.render(target);
                    }
                }

                for (auto& maze : this->mazes)
                {
                    maze.render(target);
                }
            }


            //if(not trained)
            //{
            //    auto finished{ 0 };
            //
            //    for (auto n{ 0 }; n < this->followers.size(); ++n )
            //    {
            //        this->followers[n].step();
            //        if (this->followers[n].finished())
            //        {
            //            ++finished;
            //        }
            //        else
            //        {
            //            if (not this->cars[n].collided())
            //            {
            //                const auto inputs{ Simulation::inputs(this->cars[n]) };
            //                this->data.emplace_back(inputs);
            //                this->labels.emplace_back(static_cast<tiny_dnn::label_t>(this->followers[n].movement()));
            //            }
            //        }
            //    }
            //
            //    if (finished == this->followers.size() and not trained)
            //    {
            //        tiny_dnn::RMSprop optimizer{};
            //        net.train<tiny_dnn::mse>(optimizer, this->data, this->labels, 30, 20);
            //        net.save("net_trained_500.bin");
            //
            //        trained = true;
            //
            //        // RESET
            //        this->followers.clear();
            //        for (auto n{ 0 }; n < this->cars.size(); ++n)
            //        {
            //            this->cars[n] = { &world, ground, this->mazes[n].start() };
            //        }
            //    }
            //}
            //else
            //{
            //    for (auto& car : this->cars)
            //    {
            //        const auto inputs{ Simulation::inputs(car) };
            //        const auto prediction{ net.predict_label(inputs) };
            //        car.doMove(static_cast<Move>(prediction));
            //    }
            //}



            


        });

    window.onInfos([&](std::ostringstream& oss)
        {
            //const auto distances{ this->cars.front().distances() };

            //oss << "front = " << distances.at(0) << " m \n"
            //    << "left = " << distances.at(+90) << " m \n"
            //    << "right = " << distances.at(-90) << " m \n"
            //    << "move = " << (move > 0 ? "+" : move < 0 ? "-" : "o") << " \n"
            //    << "rotate = " << (rotate > 0 ? "+" : rotate < 0 ? "-" : "o") << " \n"
            //    << "collided = " << this->cars.front().collided() << " \n";
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
    fd.filter.categoryBits = 0x0000;
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

auto Simulation::distance(const std::vector<b2Vec2>& path) -> float
{
    auto distance{ 0.0f };

    for (auto n{ 1 }; n < path.size(); ++n)
    {
        distance += b2Distance(path[n - 1], path[n]);
    }

    return distance;
}

auto Simulation::inputs(const Car& car) -> tiny_dnn::vec_t
{
    auto inputs{ tiny_dnn::vec_t{} };

    const auto distances{ car.distances() };
    for (auto [angle, distance] : distances)
    {
        inputs.emplace_back(distance);
    }

    const auto color{ static_cast<int>(car.color()) };
    {
        inputs.emplace_back(color);
    }

    //const auto giroscope{ car.giroscope() };
    //for (auto& value : giroscope)
    //{
    //    inputs.emplace_back(value);
    //}
    //
    //const auto acelerometer{ car.acelerometer() };
    //for (auto& value : acelerometer)
    //{
    //    inputs.emplace_back(value);
    //}

    return inputs;
}
