#include "stdafx.h"
#include "RenderProgram.h"
#include "GpuProgram.h"

RenderProgram::RenderProgram(const char* srcFileName)
    : mSourceFileName(srcFileName)
{
}

RenderProgram::~RenderProgram()
{
    debug_assert(mGpuProgram == nullptr);
}

bool RenderProgram::Initialize()
{
    if (IsProgramInited())
        return true;

    return Reinitialize();
}

bool RenderProgram::Reinitialize()
{
    if (mGpuProgram == nullptr)
    {
        mGpuProgram = gGraphicsDevice.CreateRenderProgram();
        if (mGpuProgram == nullptr)
        {
            gConsole.LogMessage(eLogMessage_Warning, "Cannot create render program object");
            return false;
        }
    }

    // load source code
    std::string shaderSourceCode;
    if (!gFiles.ReadTextFile(mSourceFileName, shaderSourceCode))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read shader source from %s", mSourceFileName);
        return false;
    }

    bool isCompiled = mGpuProgram->CompileSourceCode(shaderSourceCode.c_str());
    if (isCompiled)
    {
        InitUniformParameters();
        gConsole.LogMessage(eLogMessage_Info, "Render program loaded %s", mSourceFileName);
    }
    else
    {
        gConsole.LogMessage(eLogMessage_Info, "Cannot load render program %s", mSourceFileName);
    }
    return isCompiled;
}

void RenderProgram::Deinit()
{
    if (mGpuProgram == nullptr)
        return;

    Deactivate();
    gGraphicsDevice.DestroyProgram(mGpuProgram);
    mGpuProgram = nullptr;
}

bool RenderProgram::IsProgramInited() const
{
    return mGpuProgram && mGpuProgram->IsProgramCompiled();
}

bool RenderProgram::IsActive() const
{
    return mGpuProgram && mGpuProgram->IsProgramBound();
}

void RenderProgram::Activate()
{
    bool isInited = IsProgramInited();
    debug_assert(isInited);
    if (!isInited || mGpuProgram->IsProgramBound()) // program should be not active
        return;

    gGraphicsDevice.BindRenderProgram(mGpuProgram);
    BindUniformParameters();
}

void RenderProgram::Deactivate()
{
    bool isInited = IsProgramInited();
    debug_assert(isInited);
    if (!isInited || !mGpuProgram->IsProgramBound()) // program should be active
        return;

    gGraphicsDevice.BindRenderProgram(nullptr);
}

void RenderProgram::UploadCameraTransformMatrices()
{
    bool isInited = IsProgramInited();
    if (isInited)
    {
        #define SET_UNIFORM(uniform_id, matrix_reference) \
            if (mGpuProgram->IsUniformExists(uniform_id)) \
            { \
                mGpuProgram->SetUniform(uniform_id, matrix_reference); \
            }

        SET_UNIFORM(eRenderUniform_ViewMatrix, gCamera.mViewMatrix);
        SET_UNIFORM(eRenderUniform_ProjectionMatrix, gCamera.mProjectionMatrix);
        SET_UNIFORM(eRenderUniform_ViewProjectionMatrix, gCamera.mViewProjectionMatrix);
        SET_UNIFORM(eRenderUniform_CameraPosition, gCamera.mPosition);

        #undef SET_UNIFORM
    }
    debug_assert(isInited);
}