#include <fstream>
#include <sstream>
#include <numeric>
#include <cassert>
#include <cstdint>
#include <iostream>

#include "fl/Headers.h"

#include "Fuzzy.hpp"

Fuzzy::Fuzzy(const std::string& filePath)
{
    fl::fuzzylite::setDebugging(false);
    fl::fuzzylite::setLogging(false);

    this->engine = fl::FllImporter().fromFile(filePath);
    assert(this->engine->isReady());
}

Fuzzy::~Fuzzy()
{

}

auto Fuzzy::inference(const std::vector<float>& inputData) const->std::vector<float>
{
    auto sensorFrente{ engine->getInputVariable("Sfrente") };
    auto sensorEsquerdo{ engine->getInputVariable("Sesquerdo") };
    auto sensorDireito{ engine->getInputVariable("Sdireito") };

    sensorFrente->setValue(inputData[2]);
    sensorEsquerdo->setValue(inputData[0]);
    sensorDireito->setValue(inputData[4]);

    engine->process();

    auto moveFrente{ engine->getOutputVariable("Frente") };
    auto moveEsquerda{ engine->getOutputVariable("Esquerda") };
    auto moveDireita{ engine->getOutputVariable("Direita") };

    std::cout << sensorFrente->getValue() << ", " << sensorEsquerdo->getValue() << ", " << sensorDireito->getValue() << '\n';
    std::cout << moveFrente->getValue() << ", " << moveEsquerda->getValue() << ", " << moveDireita->getValue() << '\n';
    std::cout << std::endl;

    auto outputs{ std::vector<float>{
        0.0f,
        static_cast<float>(moveFrente->getValue()),
        0.0f,
        static_cast<float>(moveEsquerda->getValue()),
        static_cast<float>(moveDireita->getValue())
    }};

    return outputs;
}