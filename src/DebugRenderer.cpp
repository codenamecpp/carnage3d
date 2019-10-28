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
    mDebugLines.reserve(2048);
    mDebugTriangles.reserve(2048);
    mVerticesBuffer.reserve(4096);
}

bool DebugRenderer::Initialize()
{
    return true;
}

void DebugRenderer::Deinit()
{
    mTrimeshBuffer.Deinit();
}

void DebugRenderer::RenderFrame()
{
    if (mDebugLines.empty() && mDebugTriangles.empty())
        return;

    gRenderManager.mDebugProgram.Activate();
    gRenderManager.mDebugProgram.UploadCameraTransformMatrices(gCamera);
    FlushPrimitives();
    gRenderManager.mDebugProgram.Deactivate();
}

void DebugRenderer::FlushPrimitives()
{
    int numLines = mDebugLines.size();
    // push line vertices
    {
        for (const DebugLine& currLine: mDebugLines)
        {
            mVerticesBuffer.emplace_back();
            {
                Vertex3D_Debug& vertex = mVerticesBuffer.back();
                vertex.mPosition = currLine.mPointA;
                vertex.mColor = currLine.mColor;
            }
            mVerticesBuffer.emplace_back();
            {
                Vertex3D_Debug& vertex = mVerticesBuffer.back();
                vertex.mPosition = currLine.mPointB;
                vertex.mColor = currLine.mColor;
            }
        }
    }

    int numTriangles = mDebugTriangles.size();
    // push triangle vertices
    {
        for (const DebugTriangle& currTriangle: mDebugTriangles)
        {
            mVerticesBuffer.emplace_back();
            {
                Vertex3D_Debug& vertex = mVerticesBuffer.back();
                vertex.mPosition = currTriangle.mPointA;
                vertex.mColor = currTriangle.mColor;
            }
            mVerticesBuffer.emplace_back();
            {
                Vertex3D_Debug& vertex = mVerticesBuffer.back();
                vertex.mPosition = currTriangle.mPointB;
                vertex.mColor = currTriangle.mColor;
            }
            mVerticesBuffer.emplace_back();
            {
                Vertex3D_Debug& vertex = mVerticesBuffer.back();
                vertex.mPosition = currTriangle.mPointC;
                vertex.mColor = currTriangle.mColor;
            }
        }
    }

    int numVertices = mVerticesBuffer.size();

    if (numVertices == 0)
        return;

    Vertex3D_Debug_Format vFormat;
    mTrimeshBuffer.SetVertices(numVertices * Sizeof_Vertex3D_Debug, mVerticesBuffer.data());
    mTrimeshBuffer.Bind(vFormat);

    if (numLines > 0)
    {
        gGraphicsDevice.RenderPrimitives(ePrimitiveType_Lines, 0, numLines * 2);
    }

    if (numTriangles > 0)
    {
        gGraphicsDevice.RenderPrimitives(ePrimitiveType_Triangles, numLines * 2, numTriangles * 3);
    }

    mVerticesBuffer.clear();
    mDebugLines.clear();
    mDebugTriangles.clear();
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