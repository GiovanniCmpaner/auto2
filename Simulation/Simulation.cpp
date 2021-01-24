#include <algorithm>
#include <filesystem>
#include <fstream>

#include "Simulation.hpp"
#include "Follower.hpp"

#include "cppflow/ops.h"
#include "cppflow/model.h"

#include "..\Neural.hpp"

auto Simulation::reset() -> void
{
    this->mazes.clear();
    this->cars.clear();
    this->solutions.clear();
    this->followers.clear();
    //this->positions.clear();
    //this->distances.clear();

    this->mazes.reserve(200);
    this->cars.reserve(200);
    this->solutions.reserve(200);
    this->followers.reserve(200);
    //this->positions.reserve(500);
    //this->distances.reserve(500);

    for (auto j{ 0 }; j < 10; ++j)
    {
        for (auto i{ 0 }; i < 10; ++i)
        {
            auto& maze{ this->mazes.emplace_back(&world, ground, 5, 5, i * 3.2f, j * 3.2f, 3.0f, 3.0f) };
            
            maze.randomize();

            auto& car{ this->cars.emplace_back(&world, ground, maze.start()) };

            //auto& position{ this->positions.emplace_back(car.position()) };

            //auto& distance{ this->distances.emplace_back(0.0f) };

            auto& solution{ this->solutions.emplace_back(maze.solve(car.position(), false)) };

            auto& follower{ this->followers.emplace_back(&car, solution) };

            //randomize(net);
        }
    }

    //this->nets.front() = clone(this->base);
}


auto Simulation::init() -> void
{
    this->window.init(Simulation::realWidth, Simulation::realHeight);
    this->ground = this->createGround(&world);

    {
        auto model{ Neural{ R"(C:\Users\Giovanni\Desktop\auto2\scripts\models\model)" } };
        model.saveModel();
        const auto input{ std::vector<float>{1,1,1,1,1,1,1} };
        const auto output{ model.inference(input) };
    }

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

    auto trained{ false };

    //auto bestCar{ -1 };
    //auto bestRatio{ 0.0f };

    window.onRender([&](GPU_Target* target)
        {
            //const auto now{ this->window.now() };
            //if (now - start > 65000)
            //{
            //    this->reset();
            //    this->generation++;
            //    bestCar = -1;
            //    bestRatio = 0.0f;
            //    this->start = this->window.now();
            //}
            //else if (bestCar != -1 and now - start > 60000)
            //{
            //    this->cars[bestCar].render(target);
            //    this->mazes[0].render(target);
            //}
            //else if (bestCar == -1 and now - start > 60000)
            //{
            //    auto bestRatio{ 0 };
            //    for (auto n{ 0 }; n < this->cars.size(); ++n)
            //    {
            //        //const auto solutionFromStart{ this->mazes[0].solve(this->mazes[0].start()) };
            //        //const auto solutionFromCurrent{ this->mazes[0].solve(this->cars[n].position()) };
            //        //
            //        //if (not solutionFromCurrent.empty())
            //        //{
            //        //    const auto ratio{ Simulation::distance(solutionFromCurrent) / Simulation::distance(solutionFromStart) };
            //        //
            //        //    if (nearestCar == -1 or ratio < nearestRatio)
            //        //    {
            //        //        nearestCar = n;
            //        //        nearestRatio = ratio;
            //        //    }
            //        //}
            //
            //        const auto solutionFromCurrent{ this->mazes[0].solve(this->cars[n].position()) };
            //        if (not solutionFromCurrent.empty())
            //        {
            //            const auto distanceFromSolution{ Simulation::distance(solutionFromCurrent) };
            //            const auto distanceTraveled{ this->distances[n] };
            //
            //            const auto ratio{ distanceFromSolution };
            //            if (bestCar == -1 or ratio < bestRatio)
            //            {
            //                bestCar = n;
            //                bestRatio = ratio;
            //            }
            //        }
            //    }
            //
            //    this->base = clone(this->nets[bestCar]);
            //    this->base.save("net_trained.bin");
            //}
            //else
            //{
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

                
//            }


            if(not trained)
            {
                auto finished{ 0 };
            
                for (auto n{ 0 }; n < this->followers.size(); ++n )
                {
                    this->followers[n].step();
                    if (this->followers[n].finished())
                    {
                        ++finished;
                    }
                    else
                    {
                        if (not this->cars[n].collided())
                        {
                            const auto inputs{ Simulation::inputs(this->cars[n]) };
                            {
                                this->features.emplace_back(inputs);

                                auto label{ std::vector<int>{} };
                                label.resize(5);
                                label[static_cast<int>(this->followers[n].movement())] = 1;

                                this->labels.emplace_back(label);
                            }
                        }
                    }
                }
            
                if (finished == this->followers.size() and not trained)
                {
                    this->generateCSV();
            
                    trained = true;

                    this->reset();
                }
            }
            //else
            //{
            //    for (auto n{ 0 }; n < this->cars.size(); ++n)
            //    {
            //        const auto inputs{ Simulation::inputs(this->cars[n]) };
            //        const auto outputs{ neural->inference(inputs) };
            //        const auto max{ std::max_element(outputs.begin(), outputs.end()) - outputs.begin() };
            //        this->cars[n].doMove(static_cast<Move>(max));
            //    }
            //}

            for (auto& car : this->cars)
            {
                car.step();
            }

            for (auto& maze : this->mazes)
            {
                maze.step();
            }

            world.Step(Window::timeStep, 4, 4);

            for (const auto& follower : this->followers)
            {
                follower.render(target);
            }

            for (const auto& car : this->cars)
            {
                car.render(target);
            }

            for (const auto& maze : this->mazes)
            {
                maze.render(target);
            }
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

auto Simulation::inputs(const Car& car) -> std::vector<float>
{
    auto inputs{ std::vector<float>{} };

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

auto Simulation::generateCSV() -> void
{
    {
        auto ofs{ std::ofstream{"features.csv"} };

        for (auto j{ 0 }; j < this->features.size(); ++j)
        {
            if (j == 0)
            {
                for (auto i{ 0 }; i < this->features[j].size(); ++i)
                {
                    if (i != 0)
                    {
                        ofs << ';';
                    }
                    ofs << "f" << i;
                }
                ofs << '\n';
            }

            for (auto i{ 0 }; i < this->features[j].size(); ++i)
            {
                if (i != 0)
                {
                    ofs << ';';
                }
                ofs << this->features[j][i];
            }
            ofs << '\n';
        }
    }

    {
        auto ofs{ std::ofstream{"labels.csv"} };

        for (auto j{ 0 }; j < this->labels.size(); ++j)
        {
            if (j == 0)
            {
                for (auto i{ 0 }; i < this->labels[j].size(); ++i)
                {
                    if (i != 0)
                    {
                        ofs << ';';
                    }
                    ofs << "l" << i;
                }
                ofs << '\n';
            }

            for (auto i{ 0 }; i < this->labels[j].size(); ++i)
            {
                if (i != 0)
                {
                    ofs << ';';
                }
                ofs << this->labels[j][i];
            }
            ofs << '\n';
        }
    }

    //for(auto )
    //ofs 
}