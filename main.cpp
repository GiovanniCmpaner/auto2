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

int main(int argc, char* args[])
{
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);

    simulation.init();

    return EXIT_SUCCESS;
}

