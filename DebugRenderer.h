#pragma once

#include "GraphicsDefs.h"
#include "StreamingVertexCache.h"

// renders debug information
class DebugRenderer final: public cxx::noncopyable
{
public:
    DebugRenderer();
    bool Initialize();
    void Deinit();
    void RenderFrame();

    void DrawLine(const glm::vec3& point_a, const glm::vec3& point_b, unsigned int line_color);
    void DrawCube(const glm::vec3& point_center, const glm::vec3& cube_dimensions, unsigned int line_color);
    void DrawSphere(const glm::vec3& point_center, float radius, unsigned int line_color);

private:
    // insert line points to buffer
    inline void push_line_verts(const glm::vec3& point_a, const glm::vec3& point_b, unsigned int color)
    {
        mDebugVertices.emplace_back();
        {
            Vertex3D_Debug& vertex = mDebugVertices.back();
            vertex.mPosition = point_a;
            vertex.mColor = color;
        }
        mDebugVertices.emplace_back();
        {
            Vertex3D_Debug& vertex = mDebugVertices.back();
            vertex.mPosition = point_b;
            vertex.mColor = color;
        }
    }
private:
    StreamingVertexCache mDebugVertexCache;

    std::vector<Vertex3D_Debug> mDebugVertices;
};