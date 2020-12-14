#include <fstream>
#include <sstream>
#include <numeric>
#include <cassert>
#include <cstdint>

#include <tensorflow/c/c_api.h>

#include "Neural.hpp"

Neural::Neural(
    const std::string& binaryGraphdefProtobufFilename,
    const std::string& inputNodeName,
    const std::string& outputNodeName)
{
    this->status = TF_NewStatus();
    this->graph = TF_NewGraph();

    // objects for session
    const auto graphDef{ Neural::readBinaryFile(binaryGraphdefProtobufFilename) };
    const auto opts{ TF_NewImportGraphDefOptions() };
    const auto sessionOpts{ TF_NewSessionOptions() };

    // import graph
    TF_GraphImportGraphDef(graph, graphDef, opts, status);
    assert(TF_GetCode(status) == TF_OK, TF_Message(status));

    // setup session
    this->session = TF_NewSession(graph, sessionOpts, status);
    assert(TF_GetCode(status) == TF_OK, TF_Message(status));

    // input
    this->inputOp = TF_GraphOperationByName(graph, inputNodeName.data());
    this->input = TF_Output{ inputOp, 0 };

    // output
    this->outputOp = TF_GraphOperationByName(graph, outputNodeName.data());
    this->output = TF_Output{ outputOp, 0 };

    // Clean Up all temporary objects
    TF_DeleteBuffer(graphDef);
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

auto Neural::inference(const std::vector<float>& inputData) const->std::vector<float>
{
    const auto inputTensor{ this->vectorToTensor(inputData, input) };
    auto outputTensor{ static_cast<TF_Tensor*>(nullptr) };

    TF_SessionRun(
        session,
        nullptr,
        &input, &inputTensor, 1,
        &output, &outputTensor, 1,
        &outputOp, 1,
        nullptr,
        status
    );
    assert(TF_GetCode(status) == TF_OK, TF_Message(status));

    const auto outputData{ this->tensorToVector(outputTensor,output) };

    TF_DeleteTensor(outputTensor);
    TF_DeleteTensor(inputTensor);

    return outputData;
}

auto Neural::readBinaryFile(const std::string& fileName)->TF_Buffer*
{
    auto ifs(std::ifstream{ fileName, std::ios::binary });

    auto oss{ std::ostringstream{} };
    oss << ifs.rdbuf();
    const auto str{ oss.str() };

    return TF_NewBufferFromString(str.data(), str.size());
}

auto Neural::tensorToVector(TF_Tensor* tensor, TF_Output output) const->std::vector<float>
{
    const auto numDims{ TF_GraphGetTensorNumDims(this->graph, output, status) };
    auto dims{ std::vector<int64_t>{} };
    dims.resize(numDims);
    TF_GraphGetTensorShape(this->graph, output, dims.data(), dims.size(), status);
    assert(TF_GetCode(status) == TF_OK, TF_Message(status));

    const auto dataSize{ std::accumulate(dims.begin(), dims.end(), 1, std::multiplies{}) };
    auto vector{ std::vector<float>{} };
    vector.resize(dataSize);
    std::memcpy(vector.data(), TF_TensorData(tensor), dataSize * sizeof(float));

    return vector;
}

auto Neural::vectorToTensor(const std::vector<float>& vector, TF_Output output) const->TF_Tensor*
{
    const auto numDims{ TF_GraphGetTensorNumDims(this->graph, output, status) };
    auto dims{ std::vector<int64_t>{} };
    dims.resize(numDims);
    TF_GraphGetTensorShape(this->graph, output, dims.data(), dims.size(), status);
    assert(TF_GetCode(status) == TF_OK, TF_Message(status));

    const auto dataSize{ std::accumulate(dims.begin(), dims.end(), 1, std::multiplies{}) };
    auto tensor(TF_AllocateTensor(TF_FLOAT, dims.data(), dims.size(), dataSize * sizeof(float)));
    assert(vector.size() == dataSize);
    std::memcpy(TF_TensorData(tensor), vector.data(), dataSize * sizeof(float));

    return tensor;
}