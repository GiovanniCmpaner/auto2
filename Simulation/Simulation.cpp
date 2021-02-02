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
	this->mode = Mode::STOPPED;
	this->control = Control::MANUAL;
	this->data = Data::IDLE;

	this->mazes.clear();
	this->cars.clear();
	this->followers.clear();

	this->features.clear();
	this->labels.clear();

	const auto quantity{ 500 };
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
				auto& maze{ this->mazes.emplace_back(&world, ground, 5, 5, i * 3.2f, j * 3.2f, 3.0f, 3.0f) };

				maze.randomize();

				auto& car{ this->cars.emplace_back(&world, ground, maze.start()) };

				const auto solution{ maze.solve(car.position(), false) };

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
	this->neural = std::make_unique<Neural>(R"(C:\Users\Giovanni\Desktop\auto2\scripts\models\model)");
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
						// Nothing
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

	//auto bestCar{ -1 };
	//auto bestRatio{ 0.0f };

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
						{
							for (auto& input : inputs)
							{
								if (std::isnan(input) or std::isinf(input))
								{
									input = 9999.9;
								}
							}

							this->features.emplace_back(inputs);

							auto label{ std::vector<int>{} };
							label.resize(5);
							label[static_cast<int>(this->move)] = 1;

							this->labels.emplace_back(label);
						}

						const auto currentDistance{ b2Distance(this->cars[0].position(), this->mazes[0].end()) };
						if (currentDistance < 0.05f)
						{
							this->generationTask = this->generateCSV();
							this->data = Data::SAVING;
						}
					}
				}
				else if (control == Control::AUTO)
				{
					auto finished{ 0 };
					for (auto n{ 0 }; n < this->followers.size(); ++n)
					{
						this->followers[n].step();
						if (this->followers[n].finished())
						{
							++finished;
						}
					}

					if (data == Data::GENERATING)
					{
						for (auto n{ 0 }; n < this->followers.size(); ++n)
						{
							if (not this->followers[n].finished())
							{
								auto inputs{ Simulation::inputs(this->cars[n]) };
								{
									for (auto& input : inputs)
									{
										if (std::isnan(input) or std::isinf(input))
										{
											input = 9999.9;
										}
									}

									this->features.emplace_back(inputs);

									auto label{ std::vector<int>{} };
									label.resize(5);
									label[static_cast<int>(this->followers[n].movement())] = 1;

									this->labels.emplace_back(label);
								}
							}
						}

						if (finished == this->followers.size())
						{
							this->generationTask = this->generateCSV();
							this->data = Data::SAVING;
						}
					}
				}
				else if (control == Control::NEURAL)
				{
					for (auto n{ 0 }; n < this->cars.size(); ++n)
					{
						const auto inputs{ Simulation::inputs(this->cars[n]) };
						const auto outputs{ neural->inference(inputs) };
						const auto max{ std::max_element(outputs.begin(), outputs.end()) - outputs.begin() };
						this->cars[n].doMove(static_cast<Move>(max));
					}
				}

				if (data == Data::SAVING)
				{
					if (this->generationTask.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
					{
						this->data = Data::DONE;
					}
				}

				for (auto& car : this->cars)
				{
					car.step();
				}

				for (auto& maze : this->mazes)
				{
					maze.step();
				}

				world.Step(Window::timeStep, 4, 4);
			}

			if (control == Control::AUTO)
			{
				for (const auto& follower : this->followers)
				{
					follower.render(target);
				}
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

auto Simulation::generateCSV() -> std::future<void>
{
	return std::async(std::launch::async, [this]
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
		});
}