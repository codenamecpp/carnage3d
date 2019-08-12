#pragma once

#include "GraphicsDefs.h"

// defines hardware render program object
class GpuProgram final: public cxx::noncopyable
{
public:
    // public for convenience, don't change these fields directly
    GpuProgramHandle mResourceHandle;
    GpuVariableLocation mAttributes[eVertexAttribute_MAX];
    GpuVariableLocation mConstants[eRenderConstant_COUNT];
    GpuVariableLocation mSamplers[eTextureUnit_COUNT];
    
    RenderProgramInputLayout mInputLayout;

public:
    GpuProgram(GraphicsContext& graphicsContext);
    ~GpuProgram();

    // Create render program from shader source code
    // @param shaderSource: Source code
    bool CompileShader(const char* shaderSource);

    // Test whether render program is currently activated
    bool IsProgramBound() const;

    // Test whether render program is compiled and ready
    bool IsProgramCompiled() const;

    // constant setters
    void SetConstantf(eRenderConstant constant, float param0);
    void SetConstantf(eRenderConstant constant, float param0, float param1);
    void SetConstantf(eRenderConstant constant, float param0, float param1, float param2);
    void SetConstanti(eRenderConstant constant, int param0);

    // constant setters
    void SetConstant(eRenderConstant constant, const glm::vec2& floatVector2);
    void SetConstant(eRenderConstant constant, const glm::vec3& floatVector3);
    void SetConstant(eRenderConstant constant, const glm::vec4& floatVector4);
    void SetConstant(eRenderConstant constant, const glm::mat3& floatMatrix3);
    void SetConstant(eRenderConstant constant, const glm::mat4& floatMatrix4);

    // Test whether specific uniform is defined in render program
    // @param constant: Identifier
    bool ConstantExists(eRenderConstant constant) const;

private:
    bool CompileShader(GpuProgramHandle targetHandle, const char* programSrc);

private:
    GraphicsContext& mGraphicsContext;
};