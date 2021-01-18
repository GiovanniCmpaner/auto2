#pragma once

#include <vector>
#include <box2d/box2d.h>
#include <SDL_gpu.h>

namespace Draw
{
    static constexpr SDL_Color backgroundColor{ 0, 0, 0, 255 };
    static constexpr SDL_Color fontColor{ 0, 255, 0, 255 };
    static constexpr SDL_Color sensorColor{ 0,0,255,255 };
    static constexpr SDL_Color solidBorderColor{ 255, 0, 255, 255 };
    static constexpr SDL_Color solidFillColor{ 255, 0, 255, 64 };

    auto draw(GPU_Target* target, const b2Body* body) -> void
    {
        for (auto fixture{ body->GetFixtureList() }; fixture != nullptr; fixture = fixture->GetNext())
        {
            const auto shape{ fixture->GetShape() };
            switch (shape->GetType())
            {
                case b2Shape::e_circle:
                {
                    const auto circle{ reinterpret_cast<const b2CircleShape*>(shape) };

                    GPU_Circle(target, circle->m_p.x, circle->m_p.y, circle->m_radius, solidBorderColor);
                    GPU_CircleFilled(target, circle->m_p.x, circle->m_p.y, circle->m_radius, solidFillColor);

                    break;
                }
                case b2Shape::e_edge:
                {
                    const auto edge{ reinterpret_cast<const b2EdgeShape*>(shape) };
                    
                    GPU_Line(target, edge->m_vertex1.x, edge->m_vertex1.y, edge->m_vertex2.x, edge->m_vertex2.y, solidBorderColor);

                    break;
                }
                case b2Shape::e_polygon:
                {
                    const auto polygon{ reinterpret_cast<const b2PolygonShape*>(shape) };

                    auto vertices{ std::vector<float>{} };
                    vertices.resize(2 * polygon->m_count);
                    for (auto n{ 0 }; n < polygon->m_count; ++n)
                    {
                        const auto vertice{ body->GetWorldPoint(polygon->m_vertices[n]) };
                        vertices[n * 2 + 0] = vertice.x;
                        vertices[n * 2 + 1] = vertice.y;
                    }

                    GPU_Polygon(target, polygon->m_count, vertices.data(), solidBorderColor);
                    GPU_PolygonFilled(target, polygon->m_count, vertices.data(), solidFillColor);

                    break;
                }
                
                {
                    const auto chain{ reinterpret_cast<const b2ChainShape*>(shape) };

                    auto vertices{ std::vector<float>{} };
                    vertices.resize(2 * chain->m_count);
                    for (auto n{ 0 }; n < chain->m_count; ++n)
                    {
                        const auto vertice{ body->GetWorldPoint(chain->m_vertices[n]) };
                        vertices[n * 2 + 0] = vertice.x;
                        vertices[n * 2 + 1] = vertice.y;
                    }
                    
                    GPU_Polyline(target, vertices.size(), vertices.data(), solidBorderColor, false);

                    break;
                }
            }
        }
    }
}