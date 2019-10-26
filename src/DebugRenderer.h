#pragma once

#include "GraphicsDefs.h"
#include "TrimeshBuffer.h"

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
    void FlushPrimitives();

    struct DebugLine
    {
        glm::vec3 mPointA;
        glm::vec3 mPointB;
        unsigned int mColor;
    };

    struct DebugTriangle
    {
        glm::vec3 mPointA; 
        glm::vec3 mPointB; 
        glm::vec3 mPointC; 
        unsigned int mColor;
    };

    // insert line points to buffer
    inline void push_line_verts(const glm::vec3& point_a, const glm::vec3& point_b, unsigned int color)
    {
        mDebugLines.emplace_back();

        DebugLine& currLine = mDebugLines.back();
        currLine.mColor = color;
        currLine.mPointA = point_a;
        currLine.mPointB = point_b;
    }
    // insert triangle points to buffer
    inline void push_tri_verts(const glm::vec3& point_a, const glm::vec3& point_b, const glm::vec3& point_c, unsigned int color)
    {
        mDebugTriangles.emplace_back();

        DebugTriangle& currTriangle = mDebugTriangles.back();
        currTriangle.mColor = color;
        currTriangle.mPointA = point_a;
        currTriangle.mPointB = point_b;
        currTriangle.mPointC = point_c;
    }
private:
    std::vector<DebugLine> mDebugLines;
    std::vector<DebugTriangle> mDebugTriangles;
    std::vector<Vertex3D_Debug> mVerticesBuffer;
    TrimeshBuffer mTrimeshBuffer;
};