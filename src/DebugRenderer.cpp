#include "stdafx.h"
#include "DebugRenderer.h"
#include "RenderingManager.h"
#include "RenderView.h"
#include "GpuBuffer.h"

//////////////////////////////////////////////////////////////////////////

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
    static const int NumLines = 128;
    Line lines[NumLines];

} sg_sphere;

//////////////////////////////////////////////////////////////////////////

bool DebugRenderer::Initialize()
{
    mDebugLinesCount = 0;
    mDebugLinesDepthTestCount = 0;
    mDebugVerticesCount = 0;

    int BufferSizeBytes = MaxDebugVertices * Sizeof_Vertex3D_Debug;
    mGpuVerticesBuffer = gGraphicsDevice.CreateBuffer(eBufferContent_Vertices, eBufferUsage_Stream, BufferSizeBytes, nullptr);
    debug_assert(mGpuVerticesBuffer);

    if (mGpuVerticesBuffer == nullptr)
    {
        Deinit();
        return false;
    }

    return true;
}

void DebugRenderer::Deinit()
{
    if (mGpuVerticesBuffer)
    {
        gGraphicsDevice.DestroyBuffer(mGpuVerticesBuffer);
        mGpuVerticesBuffer = nullptr;
    }
}

void DebugRenderer::RenderFrameBegin(RenderView* renderview)
{
    gRenderManager.mDebugProgram.Activate();
    mCurrentRenderView = renderview;

    debug_assert(mCurrentRenderView);
    gRenderManager.mDebugProgram.UploadCameraTransformMatrices(mCurrentRenderView->mCamera);
}

void DebugRenderer::RenderFrameEnd()
{
    debug_assert(mCurrentRenderView);        

    if (HasPendingDraws())
    {
        Flush();
    }

    gRenderManager.mDebugProgram.Deactivate();
}

void DebugRenderer::DrawLine(const glm::vec3& start_point, const glm::vec3& end_point, unsigned int color, bool depth_test)
{
    if (DebugLineStruct* linestruct = try_allocate_lines(1, depth_test))
    {
        linestruct->mLineStart = start_point;
        linestruct->mLineEnd = end_point;
        linestruct->mColor = color;
    }
}

void DebugRenderer::DrawCube(const glm::vec3& point_center, const glm::vec3& cube_dimensions, unsigned int line_color, bool depth_test)
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
    DrawLine(t_points[0], t_points[1], line_color, depth_test); // top line
    DrawLine(b_points[0], b_points[1], line_color, depth_test); // bottom line
    DrawLine(t_points[0], b_points[0], line_color, depth_test); // left line
    DrawLine(t_points[1], b_points[1], line_color, depth_test); // right line

    // far quad
    DrawLine(t_points[2], t_points[3], line_color, depth_test); // top line
    DrawLine(b_points[2], b_points[3], line_color, depth_test); // bottom line
    DrawLine(t_points[2], b_points[2], line_color, depth_test); // left line
    DrawLine(t_points[3], b_points[3], line_color, depth_test); // right line

    // edges
    DrawLine(t_points[0], t_points[2], line_color, depth_test); // top left edge
    DrawLine(b_points[0], b_points[2], line_color, depth_test); // bottom left edge
    DrawLine(t_points[1], t_points[3], line_color, depth_test); // top right edge
    DrawLine(b_points[1], b_points[3], line_color, depth_test); // bottom right edge
}

void DebugRenderer::DrawGrid(const Point& cellCount, const glm::vec2& cellSize, unsigned int color, bool depth_test)
{
    const glm::vec3 cornerTL = glm::vec3 { 
        -(cellSize.x * cellCount.x) / 2.0f, 
        0.0f,
        -(cellSize.y * cellCount.y) / 2.0f
    };

    const glm::vec3 cornerTR = glm::vec3 {
        (cellSize.x * cellCount.x) / 2.0f, 
        0.0f,
        -(cellSize.y * cellCount.y) / 2.0f
    };

    const glm::vec3 cornerBL = glm::vec3 {
        -(cellSize.x * cellCount.x) / 2.0f,
        0.0f,
        (cellSize.y * cellCount.y) / 2.0f
    };

    for (int iy = 0; iy < cellCount.y + 1; ++iy)
    {
        const glm::vec3 voffset{0.0f, 0.0f, iy * cellSize.y};
        DrawLine(cornerTL + voffset, cornerTR + voffset, color, depth_test);
    }
    for (int ix = 0; ix < cellCount.x + 1; ++ix)
    {
        const glm::vec3 voffset{ix * cellSize.x, 0.0f, 0.0f};
        DrawLine(cornerTL + voffset, cornerBL + voffset, color, depth_test);
    }
}

void DebugRenderer::DrawSphere(const cxx::bounding_sphere_t& sphere, unsigned int color, bool depth_test)
{
    if (DebugLineStruct* linestruct = try_allocate_lines(sg_sphere.NumLines, depth_test))
    {
        for (const auto& sphereLine : sg_sphere.lines)
        {
            linestruct->mLineStart.x = sphereLine.p0.x * sphere.mRadius + sphere.mOrigin.x;
            linestruct->mLineStart.y = sphereLine.p0.y * sphere.mRadius + sphere.mOrigin.y;
            linestruct->mLineStart.z = sphereLine.p0.z * sphere.mRadius + sphere.mOrigin.z;

            linestruct->mLineEnd.x = sphereLine.p1.x * sphere.mRadius + sphere.mOrigin.x;
            linestruct->mLineEnd.y = sphereLine.p1.y * sphere.mRadius + sphere.mOrigin.y;
            linestruct->mLineEnd.z = sphereLine.p1.z * sphere.mRadius + sphere.mOrigin.z;

            linestruct->mColor = color;
            ++linestruct;
        }   
    }
}

void DebugRenderer::DrawSphere(const glm::vec3& center_point, float sphere_radius, unsigned int color, bool depth_test)
{
    if (DebugLineStruct* linestruct = try_allocate_lines(sg_sphere.NumLines, depth_test))
    {
        for (const auto& sphereLine : sg_sphere.lines)
        {
            linestruct->mLineStart.x = sphereLine.p0.x * sphere_radius + center_point.x;
            linestruct->mLineStart.y = sphereLine.p0.y * sphere_radius + center_point.y;
            linestruct->mLineStart.z = sphereLine.p0.z * sphere_radius + center_point.z;

            linestruct->mLineEnd.x = sphereLine.p1.x * sphere_radius + center_point.x;
            linestruct->mLineEnd.y = sphereLine.p1.y * sphere_radius + center_point.y;
            linestruct->mLineEnd.z = sphereLine.p1.z * sphere_radius + center_point.z;

            linestruct->mColor = color;
            ++linestruct;
        }   
    }
}

void DebugRenderer::DrawAabb(const cxx::aabbox_t& aabox, unsigned int color, bool depth_test)
{
    const glm::vec3& min = aabox.mMin;
    const glm::vec3& max = aabox.mMax;

    if (DebugLineStruct* linestruct = try_allocate_lines(12, depth_test))
    {
        // corners
        glm::vec3 v1(max.x, min.y, min.z);
        glm::vec3 v2(max.x, max.y, min.z);
        glm::vec3 v3(min.x, max.y, min.z);
        glm::vec3 v4(min.x, min.y, max.z);
        glm::vec3 v5(max.x, min.y, max.z);
        glm::vec3 v6(min.x, max.y, max.z);

        // push lines
        linestruct->mLineStart = min;
        linestruct->mLineEnd = v1;
        linestruct->mColor = color;
        ++linestruct;

        linestruct->mLineStart = v1;
        linestruct->mLineEnd = v2;
        linestruct->mColor = color;
        ++linestruct;

        linestruct->mLineStart = v2;
        linestruct->mLineEnd = v3;
        linestruct->mColor = color;
        ++linestruct;

        linestruct->mLineStart = v3;
        linestruct->mLineEnd = min;
        linestruct->mColor = color;
        ++linestruct;

        linestruct->mLineStart = v4;
        linestruct->mLineEnd = v5;
        linestruct->mColor = color;
        ++linestruct;

        linestruct->mLineStart = v5;
        linestruct->mLineEnd = max;
        linestruct->mColor = color;
        ++linestruct;

        linestruct->mLineStart = max;
        linestruct->mLineEnd = v6;
        linestruct->mColor = color;
        ++linestruct;

        linestruct->mLineStart = v6;
        linestruct->mLineEnd = v4;
        linestruct->mColor = color;
        ++linestruct;

        linestruct->mLineStart = min;
        linestruct->mLineEnd = v4;
        linestruct->mColor = color;
        ++linestruct;

        linestruct->mLineStart = v1;
        linestruct->mLineEnd = v5;
        linestruct->mColor = color;
        ++linestruct;

        linestruct->mLineStart = v2;
        linestruct->mLineEnd = max;
        linestruct->mColor = color;
        ++linestruct;

        linestruct->mLineStart = v3;
        linestruct->mLineEnd = v6;
        linestruct->mColor = color;
        ++linestruct;
    }
}

void DebugRenderer::DrawAxes(const glm::mat4& transform_matrix, const glm::vec3& center_point, float axis_length, bool depth_test)
{
    glm::vec4 x_axis_vector = transform_matrix * glm::vec4(SceneAxisX, 0.0f);
    glm::vec4 y_axis_vector = transform_matrix * glm::vec4(SceneAxisY, 0.0f);
    glm::vec4 z_axis_vector = transform_matrix * glm::vec4(SceneAxisZ, 0.0f);

    DrawLine(center_point, center_point + glm::vec3(x_axis_vector) * axis_length, Color32_Red, depth_test);
    DrawLine(center_point, center_point + glm::vec3(y_axis_vector) * axis_length, Color32_Green, depth_test);
    DrawLine(center_point, center_point + glm::vec3(z_axis_vector) * axis_length, Color32_Blue, depth_test);
}

DebugRenderer::DebugLineStruct* DebugRenderer::try_allocate_lines(int numLines, bool depth_test)
{
    // try allocate lines
    if (mDebugLinesDepthTestCount + mDebugLinesCount + numLines > MaxDebugLines)
        return nullptr;
        
    DebugLineStruct* linesPtr;
    if (depth_test)
    {
        mDebugLinesDepthTestCount += numLines;
        linesPtr = &mDebugLinesArray[MaxDebugLines - mDebugLinesDepthTestCount];
    }
    else
    {
        linesPtr = &mDebugLinesArray[mDebugLinesCount];
        mDebugLinesCount += numLines;
    }
    return linesPtr;
}

Vertex3D_Debug* DebugRenderer::try_allocate_debug_vertices(int numVertices)
{
    // try allocate vertices
    if (mDebugVerticesCount + numVertices > MaxDebugVertices)
        return nullptr;

    Vertex3D_Debug* verticesPtr = &mDebugVertices[mDebugVerticesCount];
    mDebugVerticesCount += numVertices;
    return verticesPtr;
}

bool DebugRenderer::PushLineVertices(const DebugLineStruct& lineStruct)
{
    if (Vertex3D_Debug* verticesPtr = try_allocate_debug_vertices(2))
    {
        verticesPtr->mPosition = lineStruct.mLineStart;
        verticesPtr->mColor = lineStruct.mColor;
        ++verticesPtr;

        verticesPtr->mPosition = lineStruct.mLineEnd;
        verticesPtr->mColor = lineStruct.mColor;
        ++verticesPtr;

        return true;
    }
    return false;
}

void DebugRenderer::FlushDebugVertices(bool depth_test)
{
    if (mDebugVerticesCount == 0)
        return;

    // setup render states
    RenderStates renderStates;
    renderStates.SetAlphaBlend(eBlendMode_Alpha);
    if (depth_test)
    {
        renderStates.Enable(RenderStateFlags_DepthTest);
    }
    else
    {
        renderStates.Disable(RenderStateFlags_DepthTest);
    }
    gGraphicsDevice.SetRenderStates(renderStates);

    // upload data
    debug_assert(mGpuVerticesBuffer);
    gGraphicsDevice.BindIndexBuffer(nullptr);
    gGraphicsDevice.BindVertexBuffer(mGpuVerticesBuffer, Vertex3D_Debug_Format::Get());

    int vertexDataSizeBytes = mDebugVerticesCount * Sizeof_Vertex3D_Debug;
    if (void* bufferData = mGpuVerticesBuffer->Lock(BufferAccess_Write | BufferAccess_InvalidateBuffer, 0, vertexDataSizeBytes))
    {
        ::memcpy(bufferData, mDebugVertices, vertexDataSizeBytes);
        if (!mGpuVerticesBuffer->Unlock())
        {
            debug_assert(false);
        }
    }
    else
    {
        debug_assert(false);
    }

    // issue draw call
    gGraphicsDevice.RenderPrimitives(ePrimitiveType_Lines, 0, mDebugVerticesCount);

    mDebugVerticesCount = 0;
}

void DebugRenderer::Flush()
{
    // first pass, lines with depth test enabled
    for (int pushLinesCount = 0; pushLinesCount < mDebugLinesDepthTestCount; )
    {
        int offsetIndex = MaxDebugLines - mDebugLinesDepthTestCount + pushLinesCount;
        if (!PushLineVertices(mDebugLinesArray[offsetIndex]))
        {
            FlushDebugVertices(true);
            continue;
        }
        ++pushLinesCount;
    }
    FlushDebugVertices(true);
    mDebugLinesDepthTestCount = 0;

    // second pass, lines with depth test disabled
    for (int pushLinesCount = 0; pushLinesCount < mDebugLinesCount; )
    {
        if (!PushLineVertices(mDebugLinesArray[pushLinesCount]))
        {
            FlushDebugVertices(false);
            continue;
        }
        ++pushLinesCount;
    }
    FlushDebugVertices(false);
    mDebugLinesCount = 0;
}

bool DebugRenderer::HasPendingDraws() const
{
    return mDebugLinesCount > 0 || mDebugLinesDepthTestCount > 0 || mDebugVerticesCount > 0;
}
