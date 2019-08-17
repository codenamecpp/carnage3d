#pragma once

#include "GraphicsDefs.h"

// defines hardware render program object
class GpuProgram final: public cxx::noncopyable
{
public:
    // public for convenience, don't change these fields directly
    GpuProgramHandle mResourceHandle;
    GpuVariableLocation mAttributes[eVertexAttribute_MAX];
    GpuVariableLocation mConstants[eRenderUniform_COUNT];
    GpuVariableLocation mSamplers[eTextureUnit_COUNT];
    
    RenderProgramInputLayout mInputLayout;

public:
    GpuProgram(GraphicsContext& graphicsContext);
    ~GpuProgram();

    // Create render program from shader source code
    // @param shaderSource: Source code
    bool CompileSourceCode(const char* shaderSource);

    // Test whether render program is currently activated
    bool IsProgramBound() const;

    // Test whether render program is compiled and ready
    bool IsProgramCompiled() const;

    // standard constant setters
    void SetUniform(eRenderUniform constant, float param0);
    void SetUniform(eRenderUniform constant, float param0, float param1);
    void SetUniform(eRenderUniform constant, float param0, float param1, float param2);
    void SetUniform(eRenderUniform constant, int param0);
    void SetUniform(eRenderUniform constant, const glm::vec2& floatVector2);
    void SetUniform(eRenderUniform constant, const glm::vec3& floatVector3);
    void SetUniform(eRenderUniform constant, const glm::vec4& floatVector4);
    void SetUniform(eRenderUniform constant, const glm::mat3& floatMatrix3);
    void SetUniform(eRenderUniform constant, const glm::mat4& floatMatrix4);

    // custom constant setters
    // @param constantLocation: Constant location
    void SetCustomUniform(GpuVariableLocation constantLocation, float param0);
    void SetCustomUniform(GpuVariableLocation constantLocation, float param0, float param1);
    void SetCustomUniform(GpuVariableLocation constantLocation, float param0, float param1, float param2);
    void SetCustomUniform(GpuVariableLocation constantLocation, int param0);
    void SetCustomUniform(GpuVariableLocation constantLocation, const glm::vec2& floatVector2);
    void SetCustomUniform(GpuVariableLocation constantLocation, const glm::vec3& floatVector3);
    void SetCustomUniform(GpuVariableLocation constantLocation, const glm::vec4& floatVector4);
    void SetCustomUniform(GpuVariableLocation constantLocation, const glm::mat3& floatMatrix3);
    void SetCustomUniform(GpuVariableLocation constantLocation, const glm::mat4& floatMatrix4);

    // Test whether specific uniform is defined in render program
    // @param constant: Identifier
    bool IsUniformExists(eRenderUniform constant) const;

    // custom constants support
    // @param constantName: Uniform name
    // @param outLocation: Out location index
    bool QueryUniformLocation(const char* constantName, GpuVariableLocation& outLocation) const;

private:
    // implementation details
    bool CompileSourceCode(GpuProgramHandle targetHandle, const char* programSrc);

private:
    GraphicsContext& mGraphicsContext;
};