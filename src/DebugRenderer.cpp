#include "stdafx.h"
#include "DebugRenderer.h"
#include "RenderingManager.h"

static struct DebugSpherePrecomp
{
    DebugSpherePrecomp()
    {
        auto PointOnSphere = [](float theta, float phi) -> glm::vec3
        {
            glm::vec3 point;
            theta = glm::radians(theta);
            phi = glm::radians(phi);
            point.x = sinf(theta) * sinf(phi);
            point.y = cosf(phi);
            point.z = cosf(theta) * sinf(phi);
            return point;
        };

        int iline = 0;
        for (float j = 0.0f; j < 180.0f; j += 45.0f)
        {
            for (float i = 0.0f; i < 360.0f; i += 45.0f)
            {
                glm::vec3 p1 = PointOnSphere(i, j);
                glm::vec3 p2 = PointOnSphere(i + 45.0f, j);
                glm::vec3 p3 = PointOnSphere(i, j + 45.0f);
                glm::vec3 p4 = PointOnSphere(i + 45.0f, j + 45.0f);
            
                lines[iline++] = { p1, p2 };
                lines[iline++] = { p3, p4 };
                lines[iline++] = { p1, p3 };
                lines[iline++] = { p2, p4 };
            }
        }
    }

    struct Line
    {
        glm::vec3 p0;
        glm::vec3 p1;
    };

    Line lines[128];

} sg_sphere;

DebugRenderer::DebugRenderer()
{
    mLineVertices.reserve(2048);
    mTrisVertices.reserve(2048);
}

bool DebugRenderer::Initialize()
{
    return true;
}

void DebugRenderer::Deinit()
{
}

void DebugRenderer::RenderFrame()
{
    if (mLineVertices.empty() && mTrisVertices.empty())
        return;

    gRenderManager.mDebugProgram.Activate();
    gRenderManager.mDebugProgram.UploadCameraTransformMatrices();
    FlushLines();
    FlushTriangles();
    gRenderManager.mDebugProgram.Deactivate();
}

void DebugRenderer::FlushLines()
{
    int numVertices = mLineVertices.size();

    if (numVertices == 0)
        return;

    StreamingVertexCache& vertscache = gRenderManager.mStreamingVertexCache;
    TransientBuffer vBuffer;
    if (!vertscache.AllocVertex(numVertices * Sizeof_Vertex3D_Debug, mLineVertices.data(), vBuffer))
    {
        debug_assert(false);
        return;
    }

    Vertex3D_Debug_Format vFormat;
    vFormat.mBaseOffset = vBuffer.mBufferDataOffset;

    gGraphicsDevice.BindIndexBuffer(nullptr);
    gGraphicsDevice.BindVertexBuffer(vBuffer.mGraphicsBuffer, vFormat);
    gGraphicsDevice.RenderPrimitives(ePrimitiveType_Lines, 0, numVertices);

    mLineVertices.clear();
}

void DebugRenderer::FlushTriangles()
{
    int numVertices = mTrisVertices.size();

    if (numVertices == 0)
        return;

    StreamingVertexCache& vertscache = gRenderManager.mStreamingVertexCache;
    TransientBuffer vBuffer;
    if (!vertscache.AllocVertex(numVertices * Sizeof_Vertex3D_Debug, mTrisVertices.data(), vBuffer))
    {
        debug_assert(false);
        return;
    }

    Vertex3D_Debug_Format vFormat;
    vFormat.mBaseOffset = vBuffer.mBufferDataOffset;

    gGraphicsDevice.BindIndexBuffer(nullptr);
    gGraphicsDevice.BindVertexBuffer(vBuffer.mGraphicsBuffer, vFormat);
    gGraphicsDevice.RenderPrimitives(ePrimitiveType_Triangles, 0, numVertices);

    mTrisVertices.clear();
}

void DebugRenderer::DrawLine(const glm::vec3& point_a, const glm::vec3& point_b, unsigned int line_color)
{
    push_line_verts(point_a, point_b, line_color);
}

void DebugRenderer::FillTriangle(const glm::vec3& point_a, const glm::vec3& point_b, const glm::vec3& point_c, unsigned int tri_color)
{
    push_tri_verts(point_a, point_b, point_c, tri_color);
}

void DebugRenderer::DrawCube(const glm::vec3& point_center, const glm::vec3& cube_dimensions, unsigned int line_color)
{
    glm::vec3 t_points[4] = // top quad points
    {
        // near left
        { point_center.x - cube_dimensions.x * 0.5f, point_center.y + cube_dimensions.y * 0.5f, point_center.z + cube_dimensions.z * 0.5f },
        // near right
        { point_center.x + cube_dimensions.x * 0.5f, point_center.y + cube_dimensions.y * 0.5f, point_center.z + cube_dimensions.z * 0.5f },
        // far left
        { point_center.x - cube_dimensions.x * 0.5f, point_center.y + cube_dimensions.y * 0.5f, point_center.z - cube_dimensions.z * 0.5f },
        // far right
        { point_center.x + cube_dimensions.x * 0.5f, point_center.y + cube_dimensions.y * 0.5f, point_center.z - cube_dimensions.z * 0.5f },
    };

    glm::vec3 b_points[4] = // bottom quad points
    {
        // near left
        { point_center.x - cube_dimensions.x * 0.5f, point_center.y - cube_dimensions.y * 0.5f, point_center.z + cube_dimensions.z * 0.5f },
        // near right
        { point_center.x + cube_dimensions.x * 0.5f, point_center.y - cube_dimensions.y * 0.5f, point_center.z + cube_dimensions.z * 0.5f },
        // far left
        { point_center.x - cube_dimensions.x * 0.5f, point_center.y - cube_dimensions.y * 0.5f, point_center.z - cube_dimensions.z * 0.5f },
        // far right
        { point_center.x + cube_dimensions.x * 0.5f, point_center.y - cube_dimensions.y * 0.5f, point_center.z - cube_dimensions.z * 0.5f },
    };

    // near quad
    push_line_verts(t_points[0], t_points[1], line_color); // top line
    push_line_verts(b_points[0], b_points[1], line_color); // bottom line
    push_line_verts(t_points[0], b_points[0], line_color); // left line
    push_line_verts(t_points[1], b_points[1], line_color); // right line

    // far quad
    push_line_verts(t_points[2], t_points[3], line_color); // top line
    push_line_verts(b_points[2], b_points[3], line_color); // bottom line
    push_line_verts(t_points[2], b_points[2], line_color); // left line
    push_line_verts(t_points[3], b_points[3], line_color); // right line

    // edges
    push_line_verts(t_points[0], t_points[2], line_color); // top left edge
    push_line_verts(b_points[0], b_points[2], line_color); // bottom left edge
    push_line_verts(t_points[1], t_points[3], line_color); // top right edge
    push_line_verts(b_points[1], b_points[3], line_color); // bottom right edge
}

void DebugRenderer::DrawSphere(const glm::vec3& point_center, float radius, unsigned int line_color)
{
    for (const auto& sphereLine : sg_sphere.lines)
    {
        const glm::vec3 startp (
            sphereLine.p0.x * radius + point_center.x,
            sphereLine.p0.y * radius + point_center.y,
            sphereLine.p0.z * radius + point_center.z
        );
        const glm::vec3 endp (
            sphereLine.p1.x * radius + point_center.x,
            sphereLine.p1.y * radius + point_center.y,
            sphereLine.p1.z * radius + point_center.z
        );
        push_line_verts(startp, endp, line_color);
    }
}