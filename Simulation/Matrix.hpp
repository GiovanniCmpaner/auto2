#pragma once

#include <vector>
#include <cstdint>
#include <random>
#include <functional>
#include <iomanip>

class Matrix
{
public:
    Matrix(const std::vector<std::vector<float>>& data = {}) 
    {
        this->data = data;
        if (this->data.size() > 0)
        {
            for (auto& row : this->data)
            {
                row.resize(this->data.front().size());
            }
        }
    }

    Matrix(int rows, int columns, float value = 0.0f)
    {
        this->data.resize(rows);
        for (auto& row : this->data)
        {
            row.resize(columns, value);
        }
    }

    Matrix(const std::vector<float>& vector)
    {
        this->data.resize(1, vector);
    }

    auto apply(const std::function<float(float, int, int)>& callback) const -> Matrix
    {
        return Matrix{ *this }.loop([&](auto& cell, auto j, auto i)
        {
            cell = callback(cell, j, i);
        });
    }

    auto apply(const std::function<float(float)>& callback) const -> Matrix
    {
        return Matrix{ *this }.loop([&](auto& cell, auto j, auto i)
        {
            cell = callback(cell);
        });
    }

    auto randomize(float min = 0.0f, float max = 1.0f) const -> Matrix
    {
        auto device{ std::random_device{} };
        auto engine{ std::mt19937{device()} };
        auto distribution{ std::uniform_real_distribution<float>{ min , max } };
        
        return this->apply([&](auto cell)
        {
            return distribution(engine);
        });
    }

    auto transpose() const -> Matrix
    {
        auto result{ Matrix{ this->columns(), this->rows() } };

        return result.apply([&](auto cell, auto j, auto i)
        {
            return this->at(i, j);
        });
    }

    auto prod(const Matrix& other) const -> Matrix
    {
        assert(this->columns() == other.rows());

        auto result{ Matrix{ this->rows(), other.columns() } };

        return result.apply([&](auto cell, auto j, auto i)
        {
            auto sum{ 0.0f };
            for (auto k{ 0 }; k < this->rows(); ++k)
            {
                sum += this->at(k, i) * other.at(j, k);
            }
            return sum;
        });
    }

    auto mul(const Matrix& other) const -> Matrix
    {
        assert(this->rows() == other.rows());
        assert(this->columns() == other.columns());

        return this->apply([&](auto cell, auto j, auto i)
        {
            return cell * other.at(j, i);
        });
    }

    auto div(const Matrix& other) const -> Matrix
    {
        assert(this->rows() == other.rows());
        assert(this->columns() == other.columns());

        return this->apply([&](auto cell, auto j, auto i)
        {
            return cell / other.at(j, i);
        });
    }

    auto add(const Matrix& other) const -> Matrix
    {
        assert(this->rows() == other.rows());
        assert(this->columns() == other.columns());

        return this->apply([&](auto cell, auto j, auto i)
        {
            return cell + other.at(j, i);
        });
    }

    auto sub(const Matrix& other) const -> Matrix
    {
        assert(this->rows() == other.rows());
        assert(this->columns() == other.columns());

        return this->apply([&](auto cell, auto j, auto i)
        {
            return cell - other.at(j, i);
        });
    }

    auto at(int j, int i) const -> float
    {
        return this->data[j][i];
    }

    auto at(int j, int i) -> float&
    {
        return this->data[j][i];
    }

    auto rows() const -> int
    {
        return this->data.size();
    }

    auto columns() const -> int
    {
        return (this->data.empty() ? 0 : this->data.front().size());
    }

    auto appendRow(float value = 0.0f) const -> Matrix
    {
        auto result{ *this };

        result.data.resize(result.data.size() + 1);
        result.data.back().resize(data.front().size(), value);

        return result;
    }

    auto chopRow() const -> Matrix
    {
        assert(this->rows() > 0);

        auto result{ *this };

        result.data.resize(result.data.size() - 1);

        return result;
    }

    auto appendColumn(float value = 0.0f) const -> Matrix
    {
        auto result{ *this };

        for (auto& row : result.data)
        {
            row.resize(row.size() + 1, value);
        }

        return result;
    }

    auto chopColumn() const -> Matrix
    {
        assert(this->columns() > 0);

        auto result{ *this };

        for (auto& row : result.data)
        {
            row.resize(row.size() - 1);
        }

        return result;
    }

private:
    auto loop(const std::function<void(float&, int, int)>& callback) -> Matrix&
    {
        for (auto j{ 0 }; j < this->rows(); ++j)
        {
            for (auto i{ 0 }; i < this->columns(); ++i)
            {
                callback(this->at(j, i), j, i);
            }
        }
        return *this;
    }

    std::vector<std::vector<float>> data{};
};

static std::ostream& operator<<(std::ostream& os, const Matrix& m)
{
    for (auto j{ 0 }; j < m.rows(); ++j)
    {
        os << '[' << ' ';
        for (auto i{ 0 }; i < m.columns(); ++i)
        {
            os << m.at(j, i) << ' ';
        }
        os << ']';
    }
    return os;
}
