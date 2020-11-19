#include <algorithm>
#include <filesystem>

#include "Simulation.hpp"


auto randomize(tiny_dnn::network<tiny_dnn::sequential>& net) -> void
{
    for (auto& layer : net)
    {
        for (auto& weights : layer->weights())
        {
            for (auto& weight : *weights)
            {
                const auto direction{ tiny_dnn::uniform_rand(0, 1) };
                const auto intensity{ tiny_dnn::uniform_rand(0, 1) };
                const auto multiplier{ tiny_dnn::uniform_rand(1, 10) };

                weight = std::clamp(weight * multiplier * (direction ? +1 : -1 ), -1.0f, +1.0f);
            }
        }
    }
}

auto Simulation::constructDNN() -> void
{
    using namespace tiny_dnn;
    {
        // 6 x sensores distancia
        // (2) x giroscopio (x,y)
        // (2) x acelerometro (x,y)
        // 1 x cor
        auto net{ tiny_dnn::network<tiny_dnn::sequential>{} };

        if (std::filesystem::exists("net.bin"))
        {
            net.load("net.bin");
        }
        else
        {
            net << layers::fc(7, 15) << activation::tanh()
                << layers::fc(15, 15) << activation::tanh()
                << layers::fc(15, 15) << activation::tanh()
                << layers::fc(15, 5);

            net.init_weight();
            net.save("net.bin");
        }
    }
    {
        this->nets.clear();
        for (auto n{ 0 }; n < this->cars.size(); ++n)
        {
            auto net{ tiny_dnn::network<tiny_dnn::sequential>{} };
            net.load("net.bin");

            assert(net.in_data_size() == 7);
            assert(net.out_data_size() == 5);

            randomize(net);
            this->nets.emplace_back(net);
        }
    }

    //std::vector<vec_t> train_data{ { 1,2,3,4,5,6,7,8,9,10,11 }, { 1,2,3,4,5,6,7,8,9,10,11 } };
    //std::vector<label_t> train_labels{ 1, 2 };
    //
    //adagrad optimizer{};
    //net.train<mse, adagrad>(optimizer, train_data, train_labels, 30, 10);

    //net.save("C:/Users/Giovanni/Desktop/auto2/net.bin", content_type::weights_and_model, file_format::json);
}


auto Simulation::init() -> void
{
    this->ground = this->createGround(&world);
    this->maze.init(&world, ground, 5, 5, 0.0f, 0.0f, 3.0f, 3.0f);
    //this->enviroment.init(&world, ground);

    this->cars = { 100, {&world, ground} };
    this->timers = { 100, std::chrono::system_clock::time_point::min() };
    this->start = std::chrono::system_clock::now();
    this->constructDNN();

    this->window.init(Simulation::realWidth, Simulation::realHeight);

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

    window.onRender([&](GPU_Target* target)
        {
            // Estado atual da simulação




            //{
            //    const auto now{ std::chrono::system_clock::now() };
            //
            //    auto collisions{ 0 };
            //
            //    for (auto n{ 0 }; n < this->cars.size(); ++n)
            //    {
            //        if (this->cars[n].collided())
            //        {
            //            if (timers[n] == std::chrono::system_clock::time_point::min())
            //            {
            //                timers[n] = now;
            //            }
            //            collisions++;
            //        }
            //        else
            //        {
            //            this->cars[n].step();
            //
            //            auto inputs{ this->inputs(this->cars[n]) };
            //            const auto prediction{ this->nets[n].predict_label({inputs}) };
            //            this->cars[n].doMove(static_cast<Move>(prediction + 1));
            //        }
            //    }
            //
            //    if (collisions == this->cars.size() || now - start > std::chrono::seconds(10))
            //    {
            //        auto longest{ 0 };
            //        for (auto n{ 1 }; n < this->cars.size(); ++n)
            //        {
            //            if (this->timers[n] > this->timers[longest])
            //            {
            //                longest = n;
            //            }
            //        }
            //        this->cars = { 100, {&world, ground} };
            //        this->timers = { 100, std::chrono::system_clock::time_point::min() };
            //        this->start = now;
            //
            //        this->nets[longest].save("net.bin");
            //        this->nets.clear();
            //        for (auto n{ 0 }; n < this->cars.size(); ++n)
            //        {
            //            auto net{ tiny_dnn::network<tiny_dnn::sequential>{} };
            //            net.load("net.bin");
            //            randomize(net);
            //            this->nets.emplace_back(net);
            //        }
            //    }
            //}

            cars.front().step();
            maze.step();
            world.Step(Window::timeStep, 5, 5);

            if (this->cars.front().collided())
            {
                //tiny_dnn::gradient_descent optimizer{};
                //this->nets.front().train<tiny_dnn::absolute>(optimizer, this->data, this->labels, 30, 2);
                //
                //this->nets.front().save("net.bin");
            }
            else
            {

                //auto inputs{ this->inputs(this->cars.front()) };
                //
                //const auto prediction{ this->nets.front().predict_label({inputs}) };
                //this->cars.front().doMove(static_cast<Move>(prediction));

                //this->data.emplace_back(inputs);
                //this->labels.emplace_back(static_cast<tiny_dnn::label_t>(this->move));
            }

            static const auto path{ this->maze.solve({ 0.0,0.0 }) };
            static const auto distance{ Simulation::distance(path) };

            for (auto n{ 1 }; n < path.size(); ++n)
            {
                const auto [x1, y1] { path[n - 1] };
                const auto [x2, y2] { path[n] };

                GPU_Line(target, x1, y1, x2, y2, { 0,0,255,255 });
            }

            cars.front().render(target);
            maze.render(target);
        });

    window.onInfos([&](std::ostringstream& oss)
        {
            const auto distances{ this->cars.front().distances() };

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

auto Simulation::distance(const std::vector<b2Vec2>& path) -> float
{
    auto distance{ 0.0f };

    for (auto n{ 1 }; n < path.size(); ++n)
    {
        distance += b2Distance(path[n - 1], path[n]);
    }

    return distance;
}

//auto Simulation::inputs(const Car& car) const -> tiny_dnn::vec_t
//{
//    auto inputs{ tiny_dnn::vec_t{} };
//
//    const auto distances{ car.distances() };
//    for (auto [angle, distance] : distances)
//    {
//        inputs.emplace_back(distance);
//    }
//
//    const auto color{ static_cast<int>(car.color()) };
//    {
//        inputs.emplace_back(color);
//    }
//
//    //const auto giroscope{ car.giroscope() };
//    //for (auto& value : giroscope)
//    //{
//    //    inputs.emplace_back(value);
//    //}
//    //
//    //const auto acelerometer{ car.acelerometer() };
//    //for (auto& value : acelerometer)
//    //{
//    //    inputs.emplace_back(value);
//    //}
//
//    return inputs;
//}
