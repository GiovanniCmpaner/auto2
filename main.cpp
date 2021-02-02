#include <immintrin.h> 

#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <cstring>

#include "Simulation/Simulation.hpp"

//static int resizingEventWatcher(void* data, SDL_Event* event)
//{
//    return 0;
//}
//
//auto drawWorld(GPU_Target* target, b2World* world) -> void
//{
    //return;
    //for (auto body{ world->GetBodyList() }; body != nullptr; body = body->GetNext())
    //{
    //    const auto transform{ body->GetTransform() };
    //    for (auto fixture{ body->GetFixtureList() }; fixture != nullptr; fixture = fixture->GetNext())
    //    {
    //        switch (fixture->GetType())
    //        {
    //            case b2Shape::e_circle:
    //            {
    //                const auto circle{ reinterpret_cast<const b2CircleShape*>(fixture->GetShape()) };
    //
    //                const auto center{ b2Mul(transform, circle->m_p) };
    //                const auto axis{ b2Mul(transform.q, b2Vec2{ 1.0f, 0.0 }) };
    //                const auto line{ center + circle->m_radius * axis };
    //
    //                GPU_Circle(target, center.x, center.y, circle->m_radius, solidBorderColor);
    //                GPU_CircleFilled(target, center.x, center.y, circle->m_radius, solidFillColor);
    //                GPU_Line(target, center.x, center.y, line.x, line.y, solidBorderColor);
    //
    //                break;
    //            }
    //            case b2Shape::e_edge:
    //            {
    //                const auto edge{ reinterpret_cast<const b2EdgeShape*>(fixture->GetShape()) };
    //
    //                const auto v1{ b2Mul(transform, edge->m_vertex1) };
    //                const auto v2{ b2Mul(transform, edge->m_vertex2) };
    //
    //                GPU_Line(target, v1.x, v1.y, v2.x, v2.y, solidBorderColor);
    //
    //                break;
    //            }
    //            case b2Shape::e_chain:
    //            {
    //                const auto chain{ reinterpret_cast<const b2ChainShape*>(fixture->GetShape()) };
    //
    //                auto v1{ b2Mul(transform, chain->m_vertices[0]) };
    //
    //                for (auto i{ 1 }; i < chain->m_count; ++i)
    //                {
    //                    const auto v2{ b2Mul(transform, chain->m_vertices[i]) };
    //                    GPU_Line(target, v1.x, v1.y, v2.x, v2.y, solidBorderColor);
    //                    v1 = v2;
    //                }
    //
    //                break;
    //            }
    //            case b2Shape::e_polygon:
    //            {
    //                const auto polygon{ reinterpret_cast<const b2PolygonShape*>(fixture->GetShape()) };
    //
    //                float vertices[2 * b2_maxPolygonVertices];
    //                for (auto i{ 0 }; i < polygon->m_count; ++i)
    //                {
    //                    const auto vertice{ b2Mul(transform, polygon->m_vertices[i]) };
    //                    vertices[i * 2 + 0] = vertice.x;
    //                    vertices[i * 2 + 1] = vertice.y;
    //                }
    //
    //                GPU_Polygon(target, polygon->m_count, vertices, solidBorderColor);
    //                GPU_PolygonFilled(target, polygon->m_count, vertices, solidFillColor);
    //
    //                break;
    //            }
    //        }
    //    }
    //}
//}

Simulation simulation{};

#include <tensorflow/c/c_api.h>

int main(int argc, char* args[])
{
    //const auto status{ TF_NewStatus() };
    //const auto graph{ TF_NewGraph() };
    //const auto opts{ TF_NewImportGraphDefOptions() };
    //const auto sessionOpts{ TF_NewSessionOptions() };
    //const auto tag{ "serve" };
    //
    //const auto session{ TF_LoadSessionFromSavedModel( 
    //    sessionOpts, 
    //    nullptr, 
    //    R"(C:\Users\Giovanni\Desktop\auto2\scripts\models\model)", 
    //    &tag, 1,
    //    graph, 
    //    nullptr, 
    //    status
    //) };
    //assert(TF_GetCode(status) == TF_OK);
    //
    //const auto inputOperationName{ "serving_default_dense_input" };
    //const auto inputOperation{ TF_GraphOperationByName(graph, inputOperationName) };
    //const auto input{ TF_Output{ inputOperation, 0 } };
    //assert(inputOperation != nullptr);
    //
    //const auto outputOperationName{ "StatefulPartitionedCall" };
    //const auto outputOperation{ TF_GraphOperationByName(graph, outputOperationName) };
    //const auto output{ TF_Output{ outputOperation, 0 } };
    //assert(outputOperation != nullptr);
    //
    //const int64_t inputDims[2]{ 1, 7 };
    //auto inputTensor{ TF_AllocateTensor(TF_FLOAT, inputDims, 2, 7 * sizeof(float)) };
    //const float inputData[7]{ 1,2,3,4,5,6,7 };
    //std::memcpy(TF_TensorData(inputTensor), inputData, 7 * sizeof(float));
    //
    //TF_Tensor* outputTensor{ nullptr };
    //
    //TF_SessionRun(
    //    session,
    //    nullptr,
    //    &input, &inputTensor, 1,
    //    &output, &outputTensor, 1,
    //    &outputOperation, 1,
    //    nullptr,
    //    status
    //);
    //assert(TF_GetCode(status) == TF_OK);
    //
    //const auto outputData{ static_cast<float*>(TF_TensorData(outputTensor)) };
    //
    //for (auto i = 0; i < 7; i++)
    //{
    //    std::cout << "input[" << i << "] = " << inputData[i] << std::endl;
    //}
    //for (auto i = 0; i < 5; i++)
    //{
    //    std::cout << "output[" << i << "] = " << outputData[i] << std::endl;
    //}
    //
    //size_t pos{ 0 };
    //TF_Operation* oper{ nullptr };
    //while ((oper = TF_GraphNextOperation(graph, &pos)) != nullptr)
    //{
    //    std::cout << TF_OperationName(oper) << std::endl;
    //}
    //
    //return EXIT_SUCCESS;

    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    
    simulation.init();
    
    return EXIT_SUCCESS;
}

