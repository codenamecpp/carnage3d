#include "stdafx.h"
#include "RenderSystem.h"
#include "GpuTexture2D.h"
#include "GpuProgram.h"

RenderSystem gRenderSystem;

const char* TextureColorProgramFileName = "shaders/texture_color.glsl";

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

    int indices[6] =
    {
        0, 1, 2, 0, 2, 3
    };

    mDummyVertexBuffer = gGraphicsDevice.CreateBuffer(eBufferContent_Vertices, eBufferUsage_Static, Sizeof_Vertex3D * 4, verts);
    debug_assert(mDummyVertexBuffer);

    mDummyIndexBuffer = gGraphicsDevice.CreateBuffer(eBufferContent_Indices, eBufferUsage_Static, sizeof(int) * 6, indices);
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
    gGraphicsDevice.BindRenderProgram(mTextureColorProgram);

    gCamera.ComputeMatricesAndFrustum();

    mTextureColorProgram->SetConstant(eRenderConstant_ViewProjectionMatrix, gCamera.mViewProjectionMatrix);
    mTextureColorProgram->SetConstanti(eRenderConstant_EnableTextureMapping, 1);

    gGraphicsDevice.BindVertexBuffer(mDummyVertexBuffer, Vertex3D_Format::Get());
    gGraphicsDevice.BindIndexBuffer(mDummyIndexBuffer);
    gGraphicsDevice.RenderIndexedPrimitives(ePrimitiveType_Triangles, 0, 6);

    // todo

    gGraphicsDevice.Present();
}

void RenderSystem::FreeRenderPrograms()
{
    DeinitRenderProgram(&mTextureColorProgram);
}

bool RenderSystem::InitRenderPrograms()
{
    if (!InitRenderProgram(TextureColorProgramFileName, &mTextureColorProgram))
        return false;

    return true;
}

void RenderSystem::ReloadRenderPrograms()
{
    InitRenderProgram(TextureColorProgramFileName, &mTextureColorProgram);
}

bool RenderSystem::InitRenderProgram(const char* srcFileName, GpuProgram** program)
{
    if (program == nullptr)
    {
        debug_assert(false);
        return false;
    }

    if (*program == nullptr) // create new program object
    {
        *program = gGraphicsDevice.CreateRenderProgram();
        if (*program == nullptr)
        {
            gConsole.LogMessage(eLogMessage_Warning, "Cannot create shader object");
            return false;
        }
    }

    // load shader source code
    std::string shaderSourceCode;
    if (!gFiles.ReadTextFile(srcFileName, shaderSourceCode))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot load shader %s", srcFileName);
        return false;
    }

    if ((*program)->CompileShader(shaderSourceCode.c_str()))
    {
        gConsole.LogMessage(eLogMessage_Debug, "Shader loaded %s", srcFileName);
        return true;
    }

    gConsole.LogMessage(eLogMessage_Debug, "Cannot init shader %s", srcFileName);
    return false;
}

void RenderSystem::DeinitRenderProgram(GpuProgram** program)
{
    if (program == nullptr)
    {
        debug_assert(false);
        return;
    }

    if (*program)
    {
        gGraphicsDevice.DestroyProgram(*program);
        *program = nullptr;
    }
}
