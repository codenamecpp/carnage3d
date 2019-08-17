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
    if (IsInitialized())
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
        gConsole.LogMessage(eLogMessage_Debug, "Render program loaded %s", mSourceFileName);
    }
    else
    {
        gConsole.LogMessage(eLogMessage_Debug, "Cannot load render program %s", mSourceFileName);
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

bool RenderProgram::IsInitialized() const
{
    return mGpuProgram && mGpuProgram->IsProgramCompiled();
}

bool RenderProgram::IsActive() const
{
    return mGpuProgram && mGpuProgram->IsProgramBound();
}

void RenderProgram::Activate()
{
    bool isInited = IsInitialized();
    debug_assert(isInited);
    if (!isInited || mGpuProgram->IsProgramBound()) // program should be not active
        return;

    gGraphicsDevice.BindRenderProgram(mGpuProgram);
    BindUniformParameters();
}

void RenderProgram::Deactivate()
{
    bool isInited = IsInitialized();
    debug_assert(isInited);
    if (!isInited || !mGpuProgram->IsProgramBound()) // program should be active
        return;

    gGraphicsDevice.BindRenderProgram(nullptr);
}