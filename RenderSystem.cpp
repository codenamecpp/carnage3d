#include "stdafx.h"
#include "RenderSystem.h"
#include "GpuTexture2D.h"
#include "GpuProgram.h"

RenderSystem gRenderSystem;

RenderSystem::RenderSystem()
    : mDefaultTexColorProgram("shaders/texture_color.glsl")
    , mGuiTexColorProgram("shaders/gui.glsl")
{
}

bool RenderSystem::Initialize()
{
    if (!InitRenderPrograms())
    {
        Deinit();
        return false;
    }

    PixelsArray texPixels;
    if (!texPixels.LoadFromFile("textures/flowey.png", eTextureFormat_RGB8))
    {
        Deinit();
        return false;
    }

    mDummyTexture = gGraphicsDevice.CreateTexture2D(texPixels.mFormat, texPixels.mSizex, texPixels.mSizey, texPixels.mData);
    debug_assert(mDummyTexture);

    Rect2D rcSource { 0, 0, texPixels.mSizex, texPixels.mSizey };
    Rect2D rcDest { 
        gGraphicsDevice.mViewportRect.x + gGraphicsDevice.mViewportRect.w / 2 - texPixels.mSizex / 2,
        gGraphicsDevice.mViewportRect.y + gGraphicsDevice.mViewportRect.h / 2 - texPixels.mSizey / 2,
        texPixels.mSizex,
        texPixels.mSizey
    };

    Vertex3D verts[4];
    MakeQuad3D(mDummyTexture->mSize, rcSource, rcDest, COLOR_WHITE, verts);

    short indices[6] =
    {
        0, 1, 2, 0, 2, 3
    };

    mDummyVertexBuffer = gGraphicsDevice.CreateBuffer(eBufferContent_Vertices, eBufferUsage_Static, Sizeof_Vertex3D * 4, verts);
    debug_assert(mDummyVertexBuffer);

    mDummyIndexBuffer = gGraphicsDevice.CreateBuffer(eBufferContent_Indices, eBufferUsage_Static, sizeof(short) * 6, indices);
    debug_assert(mDummyIndexBuffer);

    return true;
}

void RenderSystem::Deinit()
{
    if (mDummyTexture)
    {
        gGraphicsDevice.DestroyTexture2D(mDummyTexture);
        mDummyTexture = nullptr;
    }
    if (mDummyVertexBuffer)
    {
        gGraphicsDevice.DestroyBuffer(mDummyVertexBuffer);
        mDummyVertexBuffer = nullptr;
    }
    if (mDummyIndexBuffer)
    {
        gGraphicsDevice.DestroyBuffer(mDummyIndexBuffer);
        mDummyIndexBuffer = nullptr;
    }
    FreeRenderPrograms();
}

void RenderSystem::RenderFrame()
{
    gGraphicsDevice.ClearScreen();

    // todo

    gGraphicsDevice.BindTexture2D(eTextureUnit_0, mDummyTexture);
    mDefaultTexColorProgram.Activate();

    gCamera.ComputeMatricesAndFrustum();

    mDefaultTexColorProgram.UploadCameraTransformMatrices();

    gGraphicsDevice.BindVertexBuffer(mDummyVertexBuffer, Vertex3D_Format::Get());
    gGraphicsDevice.BindIndexBuffer(mDummyIndexBuffer);
    gGraphicsDevice.RenderIndexedPrimitives(ePrimitiveType_Triangles, eIndicesType_i16, 0, 6);

    // todo

    gGuiSystem.RenderFrame();

    gGraphicsDevice.Present();
}

void RenderSystem::FreeRenderPrograms()
{
    mDefaultTexColorProgram.Deinit();
    mGuiTexColorProgram.Deinit();
}

bool RenderSystem::InitRenderPrograms()
{
    mDefaultTexColorProgram.Initialize();
    mGuiTexColorProgram.Initialize();

    return true;
}

void RenderSystem::ReloadRenderPrograms()
{
    mDefaultTexColorProgram.Reinitialize();
    mGuiTexColorProgram.Reinitialize();
}