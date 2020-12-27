#pragma once

#include <vector>
#include <string>

#include <tensorflow/lite/c/c_api.h>

class Neural
{
public:
    Neural(const std::string& tfliteModelFile, bool quantized);
    ~Neural();
    Neural(const Neural&) = delete;
    Neural(Neural&&) = delete;

    auto inference(const std::vector<float>& inputData) const -> std::vector<float>;

private:
	auto initDetectionModel(const std::string& tfliteModelFile) -> void;

	bool m_modelQuantized = false;
	TfLiteModel* m_model;
	TfLiteInterpreter* m_interpreter;
	TfLiteTensor* m_input_tensor = nullptr;
	const TfLiteTensor* m_output_tensor = nullptr;

};