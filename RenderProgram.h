#pragma once

#include "GraphicsDefs.h"

// render program wrapper
class RenderProgram final: public cxx::noncopyable
{
public:
    // @param srcFilename: Render program source code file name, string should be static
    RenderProgram(const char* srcFilename);
    ~RenderProgram();

    // initialize render program, will load source code and compile gpu program
    // graphics device and file system should be ready
    bool InitProgram();
    // destroy gpu program
    void Deinit();
    // try to reload gpu program
    bool Reload();
    // Test whether render program is loaded
    bool IsProgramLoaded() const;

public:
    const char* mSrcFileName;
    GpuProgram* mGpuProgram = nullptr;
};