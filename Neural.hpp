#pragma once

#include <vector>
#include <string>

#include <tensorflow/c/c_api.h>

class Neural
{
public:
    Neural(const std::string& folderPath);
    ~Neural();
    Neural(const Neural&) = delete;
    Neural(Neural&&) = delete;

    auto inference(const std::vector<float>& inputData) const -> std::vector<float>;
    auto saveModel() -> void;

private:
    auto vectorToTensor(const std::vector<float>& vector, TF_Output output) const->TF_Tensor*;
    auto tensorToVector(TF_Tensor* tensor, TF_Output output) const->std::vector<float>;

    TF_Status* status{ nullptr };
    TF_Graph* graph{ nullptr };
    TF_Session* session{ nullptr };
    TF_Operation* inputOperation{ nullptr };
    TF_Output input{ };
    TF_Operation* outputOperation{ nullptr };
    TF_Output output{ };
    TF_Operation* saveOperation{ nullptr };
    TF_Output save{ };
};