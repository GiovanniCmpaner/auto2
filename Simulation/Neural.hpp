#pragma once

#include <vector>
#include <random>
#include <cassert>
#include <iostream>

#include "Matrix.hpp"

#include <tiny_dnn/tiny_dnn.h>


// https://matrices.io/deep-neural-network-from-scratch/

/*
When building a model, here is the list of things you have to choose:
- The number of hiddenValues layers
- The number of neurons for each hiddenValues layer
- The activation function
- The cost function
- The optimization algorithm
- The learning rate
- The type of regularization
- The regularization rate
- The number of gradient descent steps
- The way of evaluating the accuracy of the network
*/

/*
Steps:
- Data pre processing
- Forward propagation
- Backpropagation
*/

class Neural
{
public:
    Neural(int inputNeurons, std::vector<int> hiddenNeurons, int outputNeurons)
    {
        assert(inputNeurons > 0);
        assert(outputNeurons > 0);
        assert(hiddenNeurons.size() > 0);
        assert(std::all_of(hiddenNeurons.begin(), hiddenNeurons.end(), [](auto n) { return n > 0; }));

        inputValues = Matrix{ 1, inputNeurons };
        outputValues = Matrix{ 1, outputNeurons };

        hiddenValues.reserve(hiddenNeurons.size());
        for (const auto& hiddenNeuron : hiddenNeurons)
        {
            hiddenValues.emplace_back(1, hiddenNeuron);
        }

        synapseWeights.reserve(hiddenNeurons.size() + 1);
        {
            synapseWeights.emplace_back(inputNeurons, *hiddenNeurons.begin());

            for (auto it{ hiddenNeurons.begin() }; it < std::prev(hiddenNeurons.end()); it = std::next(it))
            {
                synapseWeights.emplace_back(*it, *std::next(it));
            }

            synapseWeights.emplace_back(*std::prev(hiddenNeurons.end()), outputNeurons);
        }

        for (auto& matrix : synapseWeights)
        {
            matrix = matrix.randomize();
        }

        forward({ 1,2,3 });
    }

    auto forward(std::vector<float> newInput) -> void
    {
        assert(newInput.size() == inputValues.columns());

        this->inputValues = newInput;

        auto intermediate{ inputValues };

        for (auto n{ 0 }; n < synapseWeights.size(); ++n)
        {
            const auto valuesWithBias{ intermediate.appendColumn(1.0f) };

            const auto synapseWeightWithBias{ synapseWeights[n].appendRow(0.1f) };

            const auto valuesWithWeight{ valuesWithBias.prod(synapseWeightWithBias) };
            const auto valuesWithActivation{ valuesWithWeight.apply([](auto cell)
            {
                return std::tanh(cell);
            }) };

            intermediate = valuesWithActivation;

            if (n < this->hiddenValues.size())
            {
                this->hiddenValues[n] = intermediate;
            }
        }

        this->outputValues = intermediate;

        print();
    }

    auto backward() -> void
    {
        const auto errorValues{ outputValues.apply([&](auto cell)
        {
            const auto expected{ cell * 0.5f };
            return (expected - cell) * - (1 - std::pow(cell, 2));
        }) };
    }

    auto print() -> void
    {
        std::cout << "inputValues: " << std::endl;
        std::cout << this->inputValues << std::endl;
        std::cout << std::endl;

        std::cout << "hiddenValues: " << std::endl;
        for (const auto& hiddenValue : this->hiddenValues)
        {
            std::cout << hiddenValue << std::endl;
        }
        std::cout << std::endl;

        std::cout << "outputValues: " << std::endl;
        std::cout << this->outputValues << std::endl;
        std::cout << std::endl;
    }

private:
    Matrix inputValues{};
    Matrix outputValues{};
    std::vector<Matrix> hiddenValues{};
    std::vector<Matrix> synapseWeights{};
};