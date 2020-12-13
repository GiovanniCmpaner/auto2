#pragma once

#include <vector>
#include <string>

#include <tensorflow/c/c_api.h>

class Neural
{
public:
    Neural(
        const std::string& binaryGraphdefProtobufFilename,
        const std::string& inputNodeName,
        const std::string& outputNodeName
    );
    ~Neural();
    Neural(const Neural&) = delete;
    Neural(Neural&&) = delete;

    auto inference(const std::vector<float>& inputData) const -> std::vector<float>;

private:
    static auto readBinaryFile(const std::string& fileName)->TF_Buffer*;
    auto tensorToVector(TF_Tensor* tensor, TF_Output output) const->std::vector<float>;
    auto vectorToTensor(const std::vector<float>& vector, TF_Output output) const->TF_Tensor*;

    TF_Status* status{ nullptr };
    TF_Graph* graph{ nullptr };
    TF_Session* session{ nullptr };
    TF_Operation* inputOp{ nullptr };
    TF_Output input{ };
    TF_Operation* outputOp{ nullptr };
    TF_Output output{ };

};