#include <fstream>
#include <sstream>
#include <numeric>
#include <cassert>
#include <cstdint>
#include <iostream>

#include <tensorflow/c/c_api.h>

#include "Neural.hpp"

Neural::Neural(const std::string& folderPath)
{
    this->status = TF_NewStatus();
    this->graph = TF_NewGraph();

    // objects for session
    const auto opts{ TF_NewImportGraphDefOptions() };
    const auto sessionOpts{ TF_NewSessionOptions() };
    const auto tag{ "serve" };

    // load session
    this->session = TF_LoadSessionFromSavedModel(
        sessionOpts, 
        nullptr, 
        folderPath.c_str(),
        &tag, 1, 
        this->graph, 
        nullptr, 
        status
    );
    this->checkStatus();

    // input
    static constexpr auto inputOperationName{ "serving_default_input_1" };
    this->inputOperation = TF_GraphOperationByName(this->graph, inputOperationName);
    this->input = TF_Output{ this->inputOperation, 0 };
    
    // output
    static constexpr auto outputOperationName{ "StatefulPartitionedCall" };
    this->outputOperation = TF_GraphOperationByName(this->graph, outputOperationName);
    this->output = TF_Output{ this->outputOperation, 0 };

    // save
    static constexpr auto saveOperationName{ "saver_filename" };
    this->saveOperation = TF_GraphOperationByName(this->graph, saveOperationName);
    this->save = TF_Output{ this->saveOperation, 0 };

    std::cout << "--- operations ---" << std::endl;
    auto pos{ static_cast<size_t>(0) };
    auto oper{ static_cast<TF_Operation*>( nullptr ) };
    while ((oper = TF_GraphNextOperation(this->graph, &pos)) != nullptr)
    {
        std::cout << TF_OperationName(oper) << std::endl;
    }

    // Clean Up all temporary objects
    TF_DeleteImportGraphDefOptions(opts);
    TF_DeleteSessionOptions(sessionOpts);
}

Neural::~Neural()
{
    TF_CloseSession(session, status);
    TF_DeleteGraph(graph);
    TF_DeleteSession(session, status);
    TF_DeleteStatus(status);
}

auto Neural::tensorToVector(TF_Tensor* tensor, TF_Output output) const->std::vector<float>
{
    const auto numDims{ TF_GraphGetTensorNumDims(this->graph, output, status) };
    auto dims{ std::vector<int64_t>{} };
    dims.resize(numDims);
    TF_GraphGetTensorShape(this->graph, output, dims.data(), dims.size(), status);
    this->checkStatus();

    dims[0] = 1;

    const auto dataSize{ std::accumulate(dims.begin(), dims.end(), 1, std::multiplies{}) };
    auto outputData{ std::vector<float>{} };
    outputData.resize(dataSize);
    std::memcpy(outputData.data(), TF_TensorData(tensor), dataSize * sizeof(float));

    return outputData;
}

auto Neural::vectorToTensor(const std::vector<float>& vector, TF_Output output) const->TF_Tensor*
{
    const auto numDims{ TF_GraphGetTensorNumDims(this->graph, output, status) };
    auto dims{ std::vector<int64_t>{} };
    dims.resize(numDims);
    TF_GraphGetTensorShape(this->graph, output, dims.data(), dims.size(), status);
    this->checkStatus();

    const auto dataSize{ std::accumulate(dims.begin(), dims.end(), 1, std::multiplies{}) };
    assert(vector.size() == dataSize && "Size mismatch");
    auto tensor(TF_AllocateTensor(TF_FLOAT, dims.data(), dims.size(), dataSize * sizeof(float)));
    std::memcpy(TF_TensorData(tensor), vector.data(), dataSize * sizeof(float));

    return tensor;
}

auto Neural::inference(const std::vector<float>& inputData) const->std::vector<float>
{
    const auto inputTensor{ this->vectorToTensor(inputData, input) };
    auto outputTensor{ static_cast<TF_Tensor*>(nullptr) };

    TF_SessionRun(
        session,
        nullptr,
        &input, &inputTensor, 1,
        &output, &outputTensor, 1,
        &outputOperation, 1,
        nullptr,
        status
    );
    this->checkStatus();

    const auto outputData{ this->tensorToVector(outputTensor,output) };

    TF_DeleteTensor(outputTensor);
    TF_DeleteTensor(inputTensor);

    return outputData;
}

auto Neural::saveModel() -> void
{
    auto input = TF_Output{ TF_GraphOperationByName(graph, "saver_filename"), 0 };
    const auto str{ R"(C:\Users\Giovanni\Desktop\auto2\scripts\models\model2)" };
    const auto str_len{ strlen(str) };
    auto nbytes = 8 + TF_StringEncodedSize(str_len); // 8 extra bytes - for start_offset.
    auto tensor = TF_AllocateTensor(TF_STRING,nullptr,0,nbytes);
    auto data = static_cast<char*>(TF_TensorData(tensor));
    std::memset(data, 0, 8);
    TF_StringEncode(str, str_len, data + 8, nbytes - 8, status);

    TF_SessionRun(
        session,
        nullptr,
        nullptr, &tensor, 1,
        nullptr, nullptr, 0,
        &saveOperation, 1,
        nullptr,
        status
    );
    this->checkStatus();
}

auto Neural::checkStatus() const -> void
{
    if (TF_GetCode(this->status) != TF_OK)
    {
        std::cerr << TF_Message(status);
        std::abort();
    }
}