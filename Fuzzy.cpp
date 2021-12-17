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
    auto entradaFrente{ engine->getInputVariable("Sfrente") };
    auto entradaEsquerda{ engine->getInputVariable("Sesquerdo") };
    auto entradaDireita{ engine->getInputVariable("Sdireito") };

    entradaFrente->setValue(inputData[2]);
    entradaEsquerda->setValue(inputData[0]);
    entradaDireita->setValue(inputData[4]);

    engine->process();

    auto saidaCarro{ engine->getOutputVariable("Carro")->getValue() };
    /*
    std::cout << entradaFrente->getValue() << ", " << entradaEsquerda->getValue() << ", " << entradaDireita->getValue() << '\n';
    std::cout << saidaCarro << '\n';
    std::cout << std::endl;
    */
    auto outputs{ std::vector<float>{
        0.0f,
        static_cast<float>(saidaCarro > 1.0 and saidaCarro < 2.0),
        0.0f,
        static_cast<float>(saidaCarro > 0.0 and saidaCarro < 1.0),
        static_cast<float>(saidaCarro > 2.0 and saidaCarro < 3.0)
    }};

    return outputs;
}