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

    void FillTriangle(const glm::vec3& point_a, const glm::vec3& point_b, const glm::vec3& point_c, unsigned int tri_color);

private:
    void FlushLines();
    void FlushTriangles();
    // insert line points to buffer
    inline void push_line_verts(const glm::vec3& point_a, const glm::vec3& point_b, unsigned int color)
    {
        mLineVertices.emplace_back();
        {
            Vertex3D_Debug& vertex = mLineVertices.back();
            vertex.mPosition = point_a;
            vertex.mColor = color;
        }
        mLineVertices.emplace_back();
        {
            Vertex3D_Debug& vertex = mLineVertices.back();
            vertex.mPosition = point_b;
            vertex.mColor = color;
        }
    }
    // insert triangle points to buffer
    inline void push_tri_verts(const glm::vec3& point_a, const glm::vec3& point_b, const glm::vec3& point_c, unsigned int color)
    {
        mTrisVertices.emplace_back();
        {
            Vertex3D_Debug& vertex = mTrisVertices.back();
            vertex.mPosition = point_a;
            vertex.mColor = color;
        }
        mTrisVertices.emplace_back();
        {
            Vertex3D_Debug& vertex = mTrisVertices.back();
            vertex.mPosition = point_b;
            vertex.mColor = color;
        }
        mTrisVertices.emplace_back();
        {
            Vertex3D_Debug& vertex = mTrisVertices.back();
            vertex.mPosition = point_c;
            vertex.mColor = color;
        }
    }
private:
    std::vector<Vertex3D_Debug> mLineVertices;
    std::vector<Vertex3D_Debug> mTrisVertices;
};