#include "stdafx.h"
#include "RenderProgram.h"
#include "GpuProgram.h"

RenderProgram::RenderProgram(const char* srcFilename)
    : mSrcFileName(srcFilename)
{
}

RenderProgram::~RenderProgram()
{
    debug_assert(mGpuProgram == nullptr);
}

bool RenderProgram::InitProgram()
{
    debug_assert(mGpuProgram == nullptr);
    if (mGpuProgram)
        return false;

    // load shader source code
    std::string shaderSourceCode;
    if (!gFiles.ReadTextFile(mSrcFileName, shaderSourceCode))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot load shader %s", mSrcFileName);
        return false;
    }

    mGpuProgram = gGraphicsDevice.CreateRenderProgram(shaderSourceCode.c_str());
    if (mGpuProgram == nullptr)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot create shader %s", mSrcFileName);
        return false;
    }

    gConsole.LogMessage(eLogMessage_Debug, "Shader loaded %s", mSrcFileName);
    return true;
}

void RenderProgram::Deinit()
{
    if (mGpuProgram)
    {
        gGraphicsDevice.DestroyProgram(mGpuProgram);
        mGpuProgram = nullptr;
    }
}

bool RenderProgram::Reload()
{
    // load shader source code
    std::string shaderSourceCode;
    if (gFiles.ReadTextFile(mSrcFileName, shaderSourceCode))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot load shader %s", mSrcFileName);
        return false;
    }

    GpuProgram* temporaryProgram = gGraphicsDevice.CreateRenderProgram(shaderSourceCode.c_str());
    if (temporaryProgram == nullptr)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot create shader %s", mSrcFileName);
        return false;
    }

    gConsole.LogMessage(eLogMessage_Debug, "Shader loaded %s", mSrcFileName);

    // destroy old program
    if (mGpuProgram)
    {
        gGraphicsDevice.DestroyProgram(mGpuProgram);
    }
    mGpuProgram = temporaryProgram;
    return true;
}

bool RenderProgram::IsProgramLoaded() const
{
    return mGpuProgram && mGpuProgram->IsProgramCompiled();
}
