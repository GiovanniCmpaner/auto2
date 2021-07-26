#include <cmath>

#include "Follower.hpp"


Follower::Follower(Car* car, const std::vector<b2Vec2>& path)
{
	this->car = car;
	this->path = path;
}

auto Follower::normalizeAngle(float angle) -> float
{
	angle = std::fmod(angle, 2.0f * b2_pi);
	if (angle < 0.0f)
	{
		angle += 2.0f * b2_pi;
	}
	return angle;
}

auto Follower::nextPoint() -> void
{
	if (this->targetPoint == this->path.size())
	{
		return;
	}

	const auto carPosition{ this->car->position() };
	const auto currentDistance{ b2Distance(carPosition, this->path[this->targetPoint]) };
	if (currentDistance < 0.075f)
	{
		this->targetPoint++;

		if (this->targetPoint == this->path.size())
		{
			return;
		}

		const auto carDistance{ this->path[this->targetPoint] - carPosition };
		const auto carAngle{ Follower::normalizeAngle(this->car->angle() + b2_pi / 2.0f) };

		this->targetAngle = Follower::normalizeAngle(std::atan2(carDistance.y, carDistance.x));

		const auto da{ this->targetAngle - carAngle };
		if ((da > +0.075f and da < +b2_pi) or da < -b2_pi)
		{
			this->adjustingAngle = -1; // RIGHT
		}
		else if ((da < -0.075f and da > -b2_pi) or da > +b2_pi)
		{
			this->adjustingAngle = +1; // LEFT
		}
		else
		{
			this->adjustingAngle = 0; // NONE
		}
	}
}

auto Follower::step() -> void
{
	this->nextPoint();
	this->followPath();
}

auto Follower::render(GPU_Target* target) const -> void
{
	this->renderPath(target);
}

auto Follower::finished() const -> bool
{
	return (this->targetPoint == this->path.size());
}

auto Follower::followPath() -> void
{
	if (this->targetPoint == this->path.size())
	{
		this->move = Move::STOP;
	}
	else 
	{
		this->move = Move::MOVE_FORWARD;

		if (this->adjustingAngle != 0)
		{
			const auto carAngle{ Follower::normalizeAngle(this->car->angle() + b2_pi / 2.0f) };

			const auto da{ std::abs(this->targetAngle - carAngle) };
			if (this->adjustingAngle == -1)
			{
				if (da > 0.075f)
				{
					this->move = Move::ROTATE_RIGHT;
				}
				else
				{
					this->adjustingAngle = 0;
				}
			}
			else if (this->adjustingAngle == +1)
			{
				if (da > 0.075f)
				{
					this->move = Move::ROTATE_LEFT;
				}
				else
				{
					this->adjustingAngle = 0;
				}
			}
		}
	}
	this->car->doMove(this->move);
}

auto Follower::movement() const->Move
{
	return this->move;
}

auto Follower::renderPath(GPU_Target* target) const -> void
{
	for (auto n{ 1 }; n < this->path.size(); ++n)
	{
		const auto [x1, y1] { this->path[n - 1] };
		const auto [x2, y2] { this->path[n] };

		GPU_Line(target, x1, y1, x2, y2, { 0,255,0,255 });
	}
}