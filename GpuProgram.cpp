#include "stdafx.h"
#include "GpuProgram.h"
#include "GraphicsContext.h"
#include "OpenGLDefs.h"

static const char* gGLSL_version_string = "#version 330 core\n";
static const char* gGLSL_vertex_shader_string = "#define VERTEX_SHADER\n";
static const char* gGLSL_fragment_shader_string = "#define FRAGMENT_SHADER\n";
const int MaxShaderInfoLength = 2048;
static char ShaderMessageBuffer[MaxShaderInfoLength + 1];

//////////////////////////////////////////////////////////////////////////

// Internal helper class
struct GpuShader
{
public:
    GpuShader(GLenum aShaderType)
        : mTarget(aShaderType)
        , mHandle()
    {
        mHandle = ::glCreateShader(aShaderType);
        glCheckError();
    }
    ~GpuShader() 
    {
        ::glDeleteShader(mHandle);
        glCheckError();
    }
    // compile shader
    bool Compile(const char** szSource, int numStrings) 
    {
        GLint resultGL;
        GLint length;

        ::glShaderSource(mHandle, numStrings, szSource, nullptr);
        glCheckError();

        ::glCompileShader(mHandle);
        glCheckError();

        ::glGetShaderiv(mHandle, GL_COMPILE_STATUS, &resultGL);
        glCheckError();

        ::glGetShaderiv(mHandle, GL_INFO_LOG_LENGTH, &length);
        glCheckError();

        if (length > 1)
        {
            ::glGetShaderInfoLog(mHandle, MaxShaderInfoLength, nullptr, ShaderMessageBuffer);
            glCheckError();

            gConsole.LogMessage(eLogMessage_Info, "Shader compilation message: '%s'", ShaderMessageBuffer);
        }
        return resultGL != GL_FALSE;
    };
public:
    GLenum mTarget;
    GLuint mHandle;
};

//////////////////////////////////////////////////////////////////////////

GpuProgram::GpuProgram(GraphicsContext& graphicsContext)
    : mResourceHandle()
    , mInputLayout()
    , mGraphicsContext(graphicsContext)
{
    mResourceHandle = ::glCreateProgram();
    glCheckError();

    // clear all locations
    for (GpuVariableLocation& location: mAttributes) { location = GpuVariableNULL; }
    for (GpuVariableLocation& location: mConstants) { location = GpuVariableNULL; }
    for (GpuVariableLocation& location: mSamplers) { location = GpuVariableNULL; }
}

GpuProgram::~GpuProgram()
{
    SetUnbound();

    ::glDeleteProgram(mResourceHandle);
    glCheckError();
}

bool GpuProgram::IsProgramBound() const
{
    return this == mGraphicsContext.mCurrentProgram;
}

bool GpuProgram::IsProgramCompiled() const
{
    return mInputLayout.mEnabledAttributes > 0;
}

void GpuProgram::SetUniform(eRenderUniform constant, float param0)
{
    debug_assert(constant < eRenderUniform_COUNT);
    SetCustomUniform(mConstants[constant], param0);
}

void GpuProgram::SetUniform(eRenderUniform constant, float param0, float param1)
{
    debug_assert(constant < eRenderUniform_COUNT);
    SetCustomUniform(mConstants[constant], param0, param1);
}

void GpuProgram::SetUniform(eRenderUniform constant, float param0, float param1, float param2)
{
    debug_assert(constant < eRenderUniform_COUNT);
    SetCustomUniform(mConstants[constant], param0, param1, param2);
}

void GpuProgram::SetUniform(eRenderUniform constant, int param0)
{
    debug_assert(constant < eRenderUniform_COUNT);
    SetCustomUniform(mConstants[constant], param0);
}

void GpuProgram::SetUniform(eRenderUniform constant, const glm::vec2& floatVector2)
{
    debug_assert(constant < eRenderUniform_COUNT);
    SetCustomUniform(mConstants[constant], floatVector2);
}

void GpuProgram::SetUniform(eRenderUniform constant, const glm::vec3& floatVector3)
{
    debug_assert(constant < eRenderUniform_COUNT);
    SetCustomUniform(mConstants[constant], floatVector3);
}

void GpuProgram::SetUniform(eRenderUniform constant, const glm::vec4& floatVector4)
{
    debug_assert(constant < eRenderUniform_COUNT);
    SetCustomUniform(mConstants[constant], floatVector4);
}

void GpuProgram::SetUniform(eRenderUniform constant, const glm::mat3& floatMatrix3)
{
    debug_assert(constant < eRenderUniform_COUNT);
    SetCustomUniform(mConstants[constant], floatMatrix3);
}

void GpuProgram::SetUniform(eRenderUniform constant, const glm::mat4& floatMatrix4)
{
    debug_assert(constant < eRenderUniform_COUNT);
    SetCustomUniform(mConstants[constant], floatMatrix4);
}

void GpuProgram::SetCustomUniform(GpuVariableLocation constantLocation, float param0)
{
    debug_assert(constantLocation != GpuVariableNULL);
    ::glProgramUniform1f(mResourceHandle, constantLocation, param0);
    glCheckError();
}

void GpuProgram::SetCustomUniform(GpuVariableLocation constantLocation, float param0, float param1)
{
    debug_assert(constantLocation != GpuVariableNULL);
    ::glProgramUniform2f(mResourceHandle, constantLocation, param0, param1);
    glCheckError();
}

void GpuProgram::SetCustomUniform(GpuVariableLocation constantLocation, float param0, float param1, float param2)
{
    debug_assert(constantLocation != GpuVariableNULL);
    ::glProgramUniform3f(mResourceHandle, constantLocation, param0, param1, param2);
    glCheckError();
}

void GpuProgram::SetCustomUniform(GpuVariableLocation constantLocation, int param0)
{
    debug_assert(constantLocation != GpuVariableNULL);
    ::glProgramUniform1i(mResourceHandle, constantLocation, param0);
    glCheckError();
}

void GpuProgram::SetCustomUniform(GpuVariableLocation constantLocation, const glm::vec2& floatVector2)
{
    debug_assert(constantLocation != GpuVariableNULL);
    ::glProgramUniform2fv(mResourceHandle, constantLocation, 1, &floatVector2.x);
    glCheckError();
}

void GpuProgram::SetCustomUniform(GpuVariableLocation constantLocation, const glm::vec3& floatVector3)
{
    debug_assert(constantLocation != GpuVariableNULL);
    ::glProgramUniform3fv(mResourceHandle, constantLocation, 1, &floatVector3.x);
    glCheckError();
}

void GpuProgram::SetCustomUniform(GpuVariableLocation constantLocation, const glm::vec4& floatVector4)
{
    debug_assert(constantLocation != GpuVariableNULL);
    ::glProgramUniform4fv(mResourceHandle, constantLocation, 1, &floatVector4.x);
    glCheckError();
}

void GpuProgram::SetCustomUniform(GpuVariableLocation constantLocation, const glm::mat3& floatMatrix3)
{
    debug_assert(constantLocation != GpuVariableNULL);
    ::glProgramUniformMatrix3fv(mResourceHandle, constantLocation, 1, GL_FALSE, &floatMatrix3[0][0]);
    glCheckError();
}

void GpuProgram::SetCustomUniform(GpuVariableLocation constantLocation, const glm::mat4& floatMatrix4)
{
    debug_assert(constantLocation != GpuVariableNULL);
    ::glProgramUniformMatrix4fv(mResourceHandle, constantLocation, 1, GL_FALSE, &floatMatrix4[0][0]);
    glCheckError();
}

bool GpuProgram::CompileSourceCode(const char* shaderSource)
{
    // set unbound
    if (this == mGraphicsContext.mCurrentProgram)
    {
        mGraphicsContext.mCurrentProgram = nullptr;
    }

    bool isSuccessed = false;
    if (IsProgramCompiled())
    {
        // create temporary program
        GLuint programHandleGL = 0;
        programHandleGL = ::glCreateProgram();
        glCheckError();

        isSuccessed = CompileSourceCode(programHandleGL, shaderSource);
        if (!isSuccessed)
        {
            // destroy temporary program
            ::glDeleteProgram(programHandleGL);
            glCheckError();
        }
        else
        {
            // destroy old program object
            ::glDeleteProgram(mResourceHandle);
            glCheckError();
            mResourceHandle = programHandleGL;
        }
    }
    else
    {
        isSuccessed = CompileSourceCode(mResourceHandle, shaderSource);
    }

    if (!isSuccessed)
        return false;

    // clear old program data
    mInputLayout.mEnabledAttributes = 0;

    for (GpuVariableLocation& location: mAttributes) { location = GpuVariableNULL; }
    for (GpuVariableLocation& location: mConstants) { location = GpuVariableNULL; }
    for (GpuVariableLocation& location: mSamplers) { location = GpuVariableNULL; }

    // query attributes
    for (int iattribute = 0; iattribute < eVertexAttribute_COUNT; ++iattribute)
    {
        eVertexAttribute vertexAttribute = (eVertexAttribute) iattribute;
        mAttributes[iattribute] = ::glGetAttribLocation(mResourceHandle, ToString(vertexAttribute));
        glCheckError();
        if (mAttributes[iattribute] != GpuVariableNULL)
        {
            mInputLayout.IncludeAttribute(vertexAttribute);
        }        
    }

    // query standard constants
    for (int iconst = 0; iconst < eRenderUniform_COUNT; ++iconst)
    { 
        mConstants[iconst] = ::glGetUniformLocation(mResourceHandle, ToString((eRenderUniform) iconst));
        glCheckError();
    }

    // query samplers
    for (int isampler = 0; isampler < eTextureUnit_COUNT; ++isampler)
    {
        GLint ilocation = ::glGetUniformLocation(mResourceHandle, ToString((eTextureUnit) isampler));
        glCheckError();
        if (ilocation != GpuVariableNULL)
        {
            mSamplers[isampler] = ilocation;
            // bind sampler to default slot
            ::glProgramUniform1i(mResourceHandle, ilocation, isampler);
            glCheckError();
        }
    }
    return true;
}

bool GpuProgram::CompileSourceCode(GpuProgramHandle targetHandle, const char* programSrc)
{
    // source strings
    const char* vertSource[] = { gGLSL_version_string, gGLSL_vertex_shader_string, programSrc };
    const char* fragSource[] = { gGLSL_version_string, gGLSL_fragment_shader_string, programSrc };

    // compile shaders
    GpuShader vertexShader (GL_VERTEX_SHADER);
    if (!vertexShader.Compile(vertSource, 3))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Vertex shader compilation failed");
        return false;
    }

    GpuShader fragmentShader (GL_FRAGMENT_SHADER);
    if (!fragmentShader.Compile(fragSource, 3))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Fragment shader compilation failed");
        return false;
    }

    // linking program
    GLint length;
    GLint linkResultGL;
    
    ::glAttachShader(targetHandle, vertexShader.mHandle);
    glCheckError();

    ::glAttachShader(targetHandle, fragmentShader.mHandle);
    glCheckError();

    ::glLinkProgram(targetHandle);
    glCheckError();

    ::glGetProgramiv(targetHandle, GL_LINK_STATUS, &linkResultGL);
    glCheckError();

    ::glGetProgramiv(targetHandle, GL_INFO_LOG_LENGTH, &length);
    glCheckError();

    if (length > 1) 
    {
        ::glGetProgramInfoLog(targetHandle, MaxShaderInfoLength, nullptr, ShaderMessageBuffer);
        glCheckError();

        gConsole.LogMessage(eLogMessage_Warning, "Program linkage message: '%s'", ShaderMessageBuffer);
    }

    if (linkResultGL == GL_FALSE)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Render program linkage error");
        return false;
    }
    return true;
}

bool GpuProgram::IsUniformExists(eRenderUniform constant) const
{
    debug_assert(constant < eRenderUniform_COUNT);
    return mConstants[constant] > GpuVariableNULL;
}

bool GpuProgram::QueryUniformLocation(const char* constantName, GpuVariableLocation& outLocation) const
{
    outLocation = ::glGetUniformLocation(mResourceHandle, constantName);
    glCheckError();

    bool constantExists = (outLocation != GpuVariableNULL);
    return constantExists;
}

void GpuProgram::SetUnbound()
{
    if (this == mGraphicsContext.mCurrentProgram)
    {
        mGraphicsContext.mCurrentProgram = nullptr;
    }
}
