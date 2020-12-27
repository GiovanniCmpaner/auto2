#include <fstream>
#include <sstream>
#include <numeric>
#include <cassert>
#include <cstdint>

#include <tensorflow/lite/c/c_api.h>

#include "Neural.hpp"

Neural::Neural(const std::string& tfliteModelFile, bool modelQuantized)
{
	m_modelQuantized = modelQuantized;
	initDetectionModel(tfliteModelFile);
}

Neural::~Neural()
{
	if (m_model != nullptr)
		TfLiteModelDelete(m_model);
}

auto Neural::initDetectionModel(const std::string& tfliteModelFile) -> void
{
	m_model = TfLiteModelCreateFromFile(tfliteModelFile.data());
	if (m_model == nullptr)
	{
		printf("Failed to load model");
		return;
	}

	// Build the interpreter
	TfLiteInterpreterOptions* options = TfLiteInterpreterOptionsCreate();
	TfLiteInterpreterOptionsSetNumThreads(options, 1);

	// Create the interpreter.
	m_interpreter = TfLiteInterpreterCreate(m_model, options);
	if (m_interpreter == nullptr)
	{
		printf("Failed to create interpreter");
		return;
	}

	// Allocate tensor buffers.
	if (TfLiteInterpreterAllocateTensors(m_interpreter) != kTfLiteOk)
	{
		printf("Failed to allocate tensors!");
		return;
	}

	// Find input tensors.
	if (TfLiteInterpreterGetInputTensorCount(m_interpreter) != 1)
	{
		printf("Detection model graph needs to have 1 and only 1 input!");
		return;
	}

	m_input_tensor = TfLiteInterpreterGetInputTensor(m_interpreter, 0);
	m_output_tensor = TfLiteInterpreterGetOutputTensor(m_interpreter, 0);

	if (m_modelQuantized && m_input_tensor->type != kTfLiteUInt8)
	{
		printf("Detection model input should be kTfLiteUInt8!");
		return;
	}

	if (!m_modelQuantized && m_input_tensor->type != kTfLiteFloat32)
	{
		printf("Detection model input should be kTfLiteFloat32!");
		return;
	}

	
	if (TfLiteInterpreterGetInputTensorCount(m_interpreter) != 1 
		or m_input_tensor->dims->data[0] != 1 
		or m_input_tensor->dims->data[1] != 7 
	)
	{
		printf("Model must have 1 input with dims 1 x 7");
		return;
	}

	// Find output tensors.
	if (TfLiteInterpreterGetOutputTensorCount(m_interpreter) != 1
		or m_output_tensor->dims->data[0] != 1
		or m_output_tensor->dims->data[1] != 5
	)
	{
		printf("Modelodel graph must have 1 output with dims 1 x 5");
		return;
	}
}

auto Neural::inference(const std::vector<float>& inputData) const-> std::vector<float>
{
	if (m_modelQuantized)
	{
		// Copy image into input tensor
		//uchar* dst = m_input_tensor->data.uint8;
		//memcpy(dst, image.data,
			//sizeof(uchar) * DETECTION_MODEL_SIZE * DETECTION_MODEL_SIZE * DETECTION_MODEL_CNLS);
	}
	else
	{
		// Copy image into input tensor
		auto dst{ m_input_tensor->data.f };
		std::memcpy(dst, inputData.data(), sizeof(float) * inputData.size());
	}
	
	if (TfLiteInterpreterInvoke(m_interpreter) != kTfLiteOk)
	{
		printf("Error invoking detection model");
		return {};
	}
	
	auto outputData{ std::vector<float>{} };
	outputData.resize(5);
	std::memcpy(outputData.data(), m_output_tensor->data.f, sizeof(float) * outputData.size());
	return outputData;
}