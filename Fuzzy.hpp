#pragma once

#include <vector>
#include <string>

#include "fl/Headers.h"

class Fuzzy
{
public:
    Fuzzy(const std::string& filePath);
    ~Fuzzy();
    Fuzzy(const Fuzzy&) = delete;
    Fuzzy(Fuzzy&&) = delete;

    auto inference(const std::vector<float>& inputData) const->std::vector<float>;
private:
    fl::Engine* engine{ nullptr };
};