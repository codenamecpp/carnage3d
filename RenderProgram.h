#pragma once

#include "GraphicsDefs.h"

// defines base class for render program
class RenderProgram: public cxx::noncopyable
{
public:
    const char* const mSourceFileName; // immutable

    // public for convenience, should not be modified directly
    GpuProgram* mGpuProgram = nullptr;

public:
    // @param srcFileName: File name of shader source, should be static string
    RenderProgram(const char* srcFileName);
    virtual ~RenderProgram();

    // loading and uloading routines, returns false on error
    bool Initialize();
    bool Reinitialize();
    void Deinit();

    // test whether program is initialized and currently active
    bool IsInitialized() const;
    bool IsActive() const;

    // activate or deactivate render program
    void Activate();
    void Deactivate();

    // will update projection and view parameters of render program
    // the matrices stored in game camera class, make sure compute them first
    void UploadCameraTransformMatrices();

    // enable or disable texture mapping for render program
    // @param isEnabled: State
    void SetTextureMappingEnabled(bool isEnabled);

protected:
    // overridable
    virtual void InitUniformParameters()
    {
    }

    virtual void BindUniformParameters()
    {
    }
};