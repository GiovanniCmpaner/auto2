#include <algorithm>
#include <filesystem>
#include <fstream>
#include <thread>
#include <future>
#include <chrono>

#include "Simulation.hpp"
#include "Follower.hpp"


#include "..\Neural.hpp"

auto Simulation::reset() -> void
{
	this->neural = std::make_unique<Neural>(R"(C:\Users\Giovanni\Desktop\auto2\scripts\models\model)");
	this->fuzzy = std::make_unique<Fuzzy>(R"(C:\Users\Giovanni\Desktop\auto2\fuzzy.fll)");
	this->replay = std::make_unique<Replay>(&world, ground, b2Vec2{ 3, 1 }, R"(D:\Google Drive\TCC SENAI\Capturas\capture_1x_1,5x1m.csv)");

	this->mazes.clear();
	this->cars.clear();
	this->followers.clear();

	this->done = 0;

	constexpr auto rows{ 3 };
	constexpr auto columns{ 3 };
	constexpr auto width{ 1.5f };
	constexpr auto height{ 1.5f };
	if (quantity > 0)
	{
		this->mazes.reserve(quantity);
		this->cars.reserve(quantity);
		this->followers.reserve(quantity);

		auto squareHeight{ static_cast<int>(std::sqrt(quantity)) };
		auto squareWidth{ quantity / squareHeight };
		while (squareHeight * squareWidth != quantity)
		{
			--squareHeight;
			squareWidth = quantity / squareHeight;
		}

		for (auto j{ 0 }; j < squareHeight; ++j)
		{
			for (auto i{ 0 }; i < squareWidth; ++i)
			{
				auto& maze{ this->mazes.emplace_back(&world, ground, columns, rows, i * (width + 0.2f), 3 + j * (height + 0.2f), width, height) };

				maze.randomize();

				auto& car{ this->cars.emplace_back(&world, ground, maze.startPoint()) };

				const auto solution{ maze.solve(car.position(), true) };

				auto& follower{ this->followers.emplace_back(&car, solution) };
			}
		}
	}

	//this->nets.front() = clone(this->base);
}

auto Simulation::init() -> void
{
	this->window.init(Simulation::realWidth, Simulation::realHeight);
	this->ground = this->createGround(&world);
	
	this->reset();

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

			if (state[SDL_SCANCODE_F1])
			{
				if (not resetChanged)
				{
					this->resetChanged = true;

					this->mode = Mode::STOPPED;
					this->control = Control::REPLAY;
					this->data = Data::IDLE;
					this->current = 0;

					this->reset();
				}
			}
			else
			{
				this->resetChanged = false;
			}

			if (state[SDL_SCANCODE_F2])
			{
				if (not controlChanged)
				{
					this->controlChanged = true;

					if (this->control == Control::MANUAL)
					{
						this->control = Control::AUTO;

						for (auto n{ 0 }; n < this->cars.size(); ++n)
						{
							const auto solution{ this->mazes[n].solve(this->cars[n].position(), false) };
							this->followers[n] = { &this->cars[n], solution };
						}
					}
					else if (this->control == Control::AUTO)
					{
						this->control = Control::NEURAL;
					}
					else if (this->control == Control::NEURAL)
					{
						this->control = Control::FUZZY;
					}
					else if (this->control == Control::FUZZY)
					{
						this->control = Control::REPLAY;
					}
					else if (this->control == Control::REPLAY)
					{
						this->control = Control::MANUAL;
					}
				}
			}
			else
			{
				this->controlChanged = false;
			}

			if (state[SDL_SCANCODE_F3])
			{
				if (not dataChanged)
				{
					this->dataChanged = true;

					if (this->data == Data::IDLE)
					{
						this->data = Data::GENERATING;
					}
					else if (this->data == Data::GENERATING)
					{
						this->generationTask = this->generateCSV();
						this->data = Data::SAVING;
					}
					else if (this->data == Data::SAVING)
					{
						// Nothing
					}
					else if (this->data == Data::DONE)
					{
						// Nothing
					}
				}
			}
			else
			{
				this->dataChanged = false;
			}

			if (state[SDL_SCANCODE_F4])
			{
				if (not modeChanged)
				{
					this->modeChanged = true;

					if (this->mode == Mode::STOPPED)
					{
						this->mode = Mode::PLAYING;
					}
					else if (this->mode == Mode::PLAYING)
					{
						this->mode = Mode::STOPPED;
					}
				}
			}
			else
			{
				this->modeChanged = false;
			}

			//this->cars.front().doMove(this->move);
		});

	auto trained{ false };


	window.onRender([&](GPU_Target* target)
		{
			if (mode == Mode::STOPPED)
			{
				// Nothing
			}
			else if (mode == Mode::PLAYING)
			{
				if (control == Control::MANUAL)
				{
					this->cars[0].doMove(this->move);

					if (data == Data::GENERATING)
					{
						auto inputs{ Simulation::inputs(this->cars[0]) };
						this->features.emplace_back(inputs);

						auto label{ std::vector<int>{} };
						label.resize(5);
						label[static_cast<int>(this->move)] = 1;

						this->labels.emplace_back(label);
					}
				}
				else if (control == Control::AUTO)
				{
#pragma omp parallel for
					for (auto n{ 0 }; n < this->followers.size(); ++n)
					{
						this->followers[n].step();
					}

					if (data == Data::GENERATING)
					{

						for (auto n{ 0 }; n < this->followers.size(); ++n)
						{
							if (not this->followers[n].isDone())
							{
								auto inputs{ Simulation::inputs(this->cars[n]) };
								this->features.emplace_back(inputs);

								auto label{ std::vector<int>{} };
								label.resize(5);
								label[static_cast<int>(this->followers[n].movement())] = 1;

								this->labels.emplace_back(label);
							}
						}
					}
				}
				else if (control == Control::NEURAL)
				{

#pragma omp parallel for
					for (auto n{ 0 }; n < this->cars.size(); ++n)
					{
						const auto inputs{ Simulation::inputs(this->cars[n]) };
						const auto outputs{ this->neural->inference(inputs) };

						auto max{ 0 };
						for (auto n{ 1 }; n < outputs.size(); ++n)
						{
							if (std::abs(outputs[n]) > std::abs(outputs[max]))
							{
								max = n;
							}
						}

						this->cars[n].doMove(static_cast<Move>(max));
					}
				}
				else if (control == Control::FUZZY)
				{
//#pragma omp parallel for
					for (auto n{ 0 }; n < this->cars.size(); ++n)
					{
						const auto inputs{ Simulation::inputs(this->cars[n]) };
						const auto outputs{ this->fuzzy->inference(inputs) };

						auto max{ 0 };
						for (auto n{ 1 }; n < outputs.size(); ++n)
						{
							if (std::abs(outputs[n]) > std::abs(outputs[max]))
							{
								max = n;
							}
						}

						this->cars[n].doMove(static_cast<Move>(max));
					}
				}
				else if (control == Control::REPLAY)
				{
					this->replay->step();
				}

				if (data == Data::SAVING)
				{
					if (this->generationTask.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
					{
						this->data = Data::DONE;
						this->features.clear();
						this->labels.clear();
					}
				}

				if (control == Control::MANUAL or control == Control::AUTO or control == Control::NEURAL or control == Control::FUZZY)
				{
					this->done = 0;

#pragma omp parallel for
					for (auto n{ 0 }; n < this->cars.size(); n++)
					{
						if (this->mazes[n].isOnEnd(this->cars[n].position()))
						{
#pragma omp atomic
							++this->done;
						}
						else
						{
							this->cars[n].step();
						}
					}

#pragma omp parallel for
					for (auto n{ 0 }; n < this->mazes.size(); n++)
					{
						this->mazes[n].step();
					}
				}

				world.Step(Window::timeStep, 4, 4);

				if (this->done == this->cars.size())
				{
					if (this->current < this->generations)
					{
						++this->current;
						this->reset();
					}
					else
					{
						this->control = Control::MANUAL;

						if (this->data == Data::GENERATING)
						{
							this->generationTask = this->generateCSV();
							this->data = Data::SAVING;
						}
					}
				}
			}

			if (control == Control::AUTO)
			{
				for (auto n{ 0 }; n < this->followers.size(); ++n)
				{
					this->followers[n].render(target);
				}
			}
			else if (control == Control::MANUAL or control == Control::NEURAL or control == Control::FUZZY)
			{
				for (auto n{ 0 }; n < this->cars.size(); n++)
				{
					this->cars[n].render(target);
				}

				for (auto n{ 0 }; n < this->mazes.size(); n++)
				{
					this->mazes[n].render(target);
				}
			}
			else if (control == Control::REPLAY)
			{
				this->replay->render(target);
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

			oss << "[F1] Reset" << '\n';

			oss << "[F2] Control = ";
			if (this->control == Control::MANUAL)
			{
				oss << "MANUAL";
			}
			else if (this->control == Control::AUTO)
			{
				oss << "AUTO";
			}
			else if (this->control == Control::NEURAL)
			{
				oss << "NEURAL";
			}
			else if (this->control == Control::FUZZY)
			{
				oss << "FUZZY";
			}
			else if (this->control == Control::REPLAY)
			{
				oss << "REPLAY";
			}
			oss << '\n';

			oss << "[F3] Data = ";
			if (this->data == Data::IDLE)
			{
				oss << "IDLE";
			}
			else if (this->data == Data::GENERATING)
			{
				oss << "GENERATING";
			}
			else if (this->data == Data::SAVING)
			{
				oss << "SAVING";
			}
			else if (this->data == Data::DONE)
			{
				oss << "DONE";
			}
			oss << '\n';

			oss << "[F4] Mode = ";
			if (this->mode == Mode::STOPPED)
			{
				oss << "STOPPED";
			}
			else if (this->mode == Mode::PLAYING)
			{
				oss << "PLAYING";
			}
			oss << '\n';

			static auto count{ 0 };
			static auto linearVelocity{ 0.0f };
			static auto angularVelocity{ 0.0f };
			if (count == 10)
			{
				count = 0;

				if (this->cars.size() > 0)
				{
					linearVelocity = this->cars[0].linearVelocity();
					angularVelocity = this->cars[0].angularVelocity();
				}
			}
			count++;

			oss << "generation = " << this->current << " / " << this->generations << '\n';
			oss << "done = " << this->done << " / " << this->quantity << '\n';
			oss << "linear velocity = " << linearVelocity << '\n';
			oss << "angular velocity = " << angularVelocity << '\n';
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

auto Simulation::inputs(const Car& car) -> std::vector<float>
{
	auto inputs{ std::vector<float>{} };

	const auto distances{ car.distances() };
	for (auto [angle, distance] : distances)
	{
		inputs.emplace_back(distance);
	}

	//const auto color{ static_cast<int>(car.color()) };
	//{
	//	inputs.emplace_back(color);
	//}

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

auto Simulation::generateCSV() -> std::future<void>
{
	return std::async(std::launch::async, [this]
		{
			{
				auto ofs{ std::ofstream{R"(scripts\features.csv)"} };

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
				auto ofs{ std::ofstream{R"(scripts\labels.csv)"} };

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
		});
}