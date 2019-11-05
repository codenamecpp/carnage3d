#include "stdafx.h"
#include "DebugRenderer.h"
#include "RenderingManager.h"
#include "RenderView.h"
#include "GpuBuffer.h"

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
}

bool DebugRenderer::Initialize()
{
    mVerticesBuffer = gGraphicsDevice.CreateBuffer(eBufferContent_Vertices, eBufferUsage_Stream, mMaxVerticesCount * Sizeof_Vertex3D_Debug, nullptr);
    debug_assert(mVerticesBuffer);

    mVerticesPointer = static_cast<Vertex3D_Debug*>(mVerticesBuffer->Lock(BufferAccess_UnsynchronizedWrite));
    debug_assert(mVerticesPointer);

    mCurrVerticesCount = 0;

    return true;
}

void DebugRenderer::Deinit()
{
    if (mVerticesBuffer)
    {
        gGraphicsDevice.DestroyBuffer(mVerticesBuffer);
        mVerticesBuffer = nullptr;
        mVerticesPointer = nullptr;
    }
}

void DebugRenderer::RenderFrameBegin(RenderView* renderview)
{
    gRenderManager.mDebugProgram.Activate();
    mCurrentRenderView = renderview;

    debug_assert(mCurrentRenderView);
    gRenderManager.mDebugProgram.UploadCameraTransformMatrices(mCurrentRenderView->mRenderCamera);
}

void DebugRenderer::RenderFrameEnd()
{
    debug_assert(mCurrentRenderView);        

    FlushPrimitives();

    gRenderManager.mDebugProgram.Deactivate();
}

void DebugRenderer::FlushPrimitives()
{
    if (mCurrVerticesCount == 0)
        return;

    if (!mVerticesBuffer->Unlock())
    {
        debug_assert(false);
    }

    Vertex3D_Debug_Format vFormat;
    gGraphicsDevice.BindVertexBuffer(mVerticesBuffer, vFormat);
    gGraphicsDevice.RenderPrimitives(ePrimitiveType_Lines, 0, mCurrVerticesCount);

    mVerticesBuffer->Invalidate();

    mVerticesPointer = static_cast<Vertex3D_Debug*>(mVerticesBuffer->Lock(BufferAccess_UnsynchronizedWrite));
    debug_assert(mVerticesPointer);

    mCurrVerticesCount = 0;
}

void DebugRenderer::DrawLine(const glm::vec3& point_a, const glm::vec3& point_b, unsigned int line_color)
{
    if ((mCurrVerticesCount + 2) >= mMaxVerticesCount)
    {
        FlushPrimitives();
    }

    mVerticesPointer[mCurrVerticesCount + 0].mPosition = point_a;
    mVerticesPointer[mCurrVerticesCount + 0].mColor = line_color;

    mVerticesPointer[mCurrVerticesCount + 1].mPosition = point_b;
    mVerticesPointer[mCurrVerticesCount + 1].mColor = line_color;
    mCurrVerticesCount += 2;
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
    DrawLine(t_points[0], t_points[1], line_color); // top line
    DrawLine(b_points[0], b_points[1], line_color); // bottom line
    DrawLine(t_points[0], b_points[0], line_color); // left line
    DrawLine(t_points[1], b_points[1], line_color); // right line

    // far quad
    DrawLine(t_points[2], t_points[3], line_color); // top line
    DrawLine(b_points[2], b_points[3], line_color); // bottom line
    DrawLine(t_points[2], b_points[2], line_color); // left line
    DrawLine(t_points[3], b_points[3], line_color); // right line

    // edges
    DrawLine(t_points[0], t_points[2], line_color); // top left edge
    DrawLine(b_points[0], b_points[2], line_color); // bottom left edge
    DrawLine(t_points[1], t_points[3], line_color); // top right edge
    DrawLine(b_points[1], b_points[3], line_color); // bottom right edge
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
        DrawLine(startp, endp, line_color);
    }
}
