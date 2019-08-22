#include "stdafx.h"
#include "GraphicsDevice.h"
#include "OpenGLDefs.h"
#include "GpuProgram.h"
#include "GpuBuffer.h"
#include "GpuTexture2D.h"

#define WINDOW_TITLE "Carnage3D"

GraphicsDevice gGraphicsDevice;

GraphicsDevice::GraphicsDevice()
    : mCurrentStates()
    , mViewportRect()
    , mGraphicsWindow()
    , mGraphicsMonitor()
{
}

GraphicsDevice::~GraphicsDevice()
{
    debug_assert(!IsDeviceInited());
}

bool GraphicsDevice::Initialize(int screensizex, int screensizey, bool fullscreen, bool vsync)
{
    if (IsDeviceInited())
    {
        Deinit();
    }

    if (::glfwInit() == GL_FALSE)
    {
        gConsole.LogMessage(eLogMessage_Warning, "GLFW initialization failed");
        return false;
    }

    // dump some information
    gConsole.LogMessage(eLogMessage_Info, "GLFW Information: %s", ::glfwGetVersionString());
    gConsole.LogMessage(eLogMessage_Info, "Initialize OpenGL %d.%d (Core profile)",
        OPENGL_CONTEXT_MAJOR_VERSION, 
        OPENGL_CONTEXT_MINOR_VERSION);

    GLFWmonitor* graphicsMonitor = nullptr;
    if (fullscreen)
    {
        graphicsMonitor = ::glfwGetPrimaryMonitor();
        debug_assert(graphicsMonitor);
    }

    gConsole.LogMessage(eLogMessage_Info, "Screen resolution (%dx%d)", screensizex, screensizey);

    // opengl params
    ::glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    ::glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    ::glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_CONTEXT_MAJOR_VERSION);
    ::glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_CONTEXT_MINOR_VERSION);
    // setup window params
    ::glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    ::glfwWindowHint(GLFW_RED_BITS, 8);
    ::glfwWindowHint(GLFW_GREEN_BITS, 8);
    ::glfwWindowHint(GLFW_BLUE_BITS, 8);
    ::glfwWindowHint(GLFW_ALPHA_BITS, 8);
    ::glfwWindowHint(GLFW_DEPTH_BITS, 24);

    // create window and set current context
    GLFWwindow* graphicsWindow = ::glfwCreateWindow(screensizex, screensizey, WINDOW_TITLE, graphicsMonitor, nullptr);
    debug_assert(graphicsWindow);
    if (!graphicsWindow)
    {
        gConsole.LogMessage(eLogMessage_Warning, "glfwCreateWindow failed");
        ::glfwTerminate();
        return false;
    }

    // setup current opengl context and register callback handlers
    ::glfwMakeContextCurrent(graphicsWindow);
    ::glfwSetMouseButtonCallback(graphicsWindow, [](GLFWwindow*, int button, int action, int mods)
        {
            if (action != GLFW_REPEAT)
            {
                MouseButtonInputEvent ev { button, mods, action == GLFW_PRESS };
                gSystem.HandleEvent(ev);
            }
        });
    ::glfwSetKeyCallback(graphicsWindow, [](GLFWwindow*, int keycode, int scancode, int action, int mods)
        {
            if (action != GLFW_REPEAT)
            {
                KeyInputEvent ev { keycode, scancode, mods, action == GLFW_PRESS };
                gSystem.HandleEvent(ev);
            }
        });
    ::glfwSetCharCallback(graphicsWindow, [](GLFWwindow*, unsigned int unicodechar)
        {
            KeyCharEvent ev ( unicodechar );
            gSystem.HandleEvent(ev);
        });
    ::glfwSetScrollCallback(graphicsWindow, [](GLFWwindow*, double xscroll, double yscroll)
        {
            MouseScrollInputEvent ev 
            { 
                static_cast<int>(xscroll), 
                static_cast<int>(yscroll) 
            };
            gSystem.HandleEvent(ev);
        });
    ::glfwSetCursorPosCallback(graphicsWindow, [](GLFWwindow*, double xposition, double yposition)
        {
            MouseMovedInputEvent ev 
            { 
                static_cast<int>(xposition),
                static_cast<int>(yposition),
            };
            gSystem.HandleEvent(ev);
        });

    // setup opengl extensions
    if (!InitializeOGLExtensions())
    {
        ::glfwDestroyWindow(graphicsWindow);
        ::glfwTerminate();
        return false;
    }

    mGraphicsWindow = graphicsWindow;
    mGraphicsMonitor = graphicsMonitor;

    // clear opengl errors
    glClearError();

    // create global vertex array object
    ::glGenVertexArrays(1, &mGraphicsContext.mVaoHandle);
    glCheckError();

    ::glBindVertexArray(mGraphicsContext.mVaoHandle);
    glCheckError();

    // setup viewport
    mViewportRect.x = 0;
    mViewportRect.y = 0;
    mViewportRect.w = screensizex;
    mViewportRect.h = screensizey;

    ::glViewport(mViewportRect.x, mViewportRect.y, mViewportRect.w, mViewportRect.h);
    glCheckError();

    ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glCheckError();

    // force clear screen at stratup
    ::glfwSwapBuffers(mGraphicsWindow);
    glCheckError();

    // setup default render state
    static const RenderStates defaultRenderStates;
    InternalSetRenderStates(defaultRenderStates, true);

    EnableVSync(vsync);
    EnableFullscreen(fullscreen);
    return true;
}

void GraphicsDevice::Deinit()
{
    if (!IsDeviceInited())
        return;

    // destroy vertex array object
    ::glBindVertexArray(0);
    glCheckError();

    ::glDeleteVertexArrays(1, &mGraphicsContext.mVaoHandle);
    glCheckError();

    if (mGraphicsWindow) // shutdown glfw system
    {
        ::glfwDestroyWindow(mGraphicsWindow);
        ::glfwTerminate();

        mGraphicsMonitor = nullptr;
        mGraphicsWindow = nullptr;
    }
}

void GraphicsDevice::EnableVSync(bool vsyncEnabled)
{
    if (!IsDeviceInited())
        return;
#if 0
    // this does work for Intel HD Graphics
    if (!!::wglSwapIntervalEXT)
    {
        ::wglSwapIntervalEXT(vsyncEnabled ? 1 : 0);
        return;
    }
#endif
    ::glfwSwapInterval(vsyncEnabled ? 1 : 0);
}

void GraphicsDevice::EnableFullscreen(bool fullscreenEnabled)
{
    if (!IsDeviceInited())
        return;

    // todo
    (void) fullscreenEnabled;
}

GpuTexture2D* GraphicsDevice::CreateTexture2D()
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return nullptr;
    }

    GpuTexture2D* texture2D = new GpuTexture2D(mGraphicsContext);
    return texture2D;
}

GpuTexture2D* GraphicsDevice::CreateTexture2D(eTextureFormat textureFormat, int sizex, int sizey, const void* sourceData)
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return nullptr;
    }

    GpuTexture2D* texture2D = new GpuTexture2D(mGraphicsContext);
    if (!texture2D->Setup(textureFormat, sizex, sizey, sourceData))
    {
        DestroyTexture2D(texture2D);
        return nullptr;
    }
    return texture2D;
}

GpuProgram* GraphicsDevice::CreateRenderProgram()
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return nullptr;
    }

    GpuProgram* program = new GpuProgram(mGraphicsContext);
    return program;
}

GpuProgram* GraphicsDevice::CreateRenderProgram(const char* shaderSource)
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return nullptr;
    }

    GpuProgram* program = new GpuProgram(mGraphicsContext);
    if (!program->CompileSourceCode(shaderSource))
    {
        DestroyProgram(program);
        return nullptr;
    }
    return program;
}

GpuBuffer* GraphicsDevice::CreateBuffer()
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return nullptr;
    }

    GpuBuffer* bufferObject = new GpuBuffer(mGraphicsContext);
    return bufferObject;
}

GpuBuffer* GraphicsDevice::CreateBuffer(eBufferContent bufferContent, eBufferUsage bufferUsage, unsigned int bufferLength, const void* dataBuffer)
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return nullptr;
    }

    GpuBuffer* bufferObject = new GpuBuffer(mGraphicsContext);
    if (!bufferObject->Setup(bufferContent, bufferUsage, bufferLength, dataBuffer))
    {
        DestroyBuffer(bufferObject);
        return nullptr;
    }
    return bufferObject;
}

void GraphicsDevice::BindVertexBuffer(GpuBuffer* sourceBuffer, const VertexFormat& streamDefinition)
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return;
    }
    debug_assert(mGraphicsContext.mCurrentProgram);
    if (sourceBuffer)
    {
        debug_assert(sourceBuffer->mContent == eBufferContent_Vertices);
    }

    if (mGraphicsContext.mCurrentBuffers[eBufferContent_Vertices] != sourceBuffer)
    {
        GLenum bufferTargetGL = EnumToGL(eBufferContent_Vertices);
        mGraphicsContext.mCurrentBuffers[eBufferContent_Vertices] = sourceBuffer;
        ::glBindBuffer(bufferTargetGL, sourceBuffer ? sourceBuffer->mResourceHandle : 0);
        glCheckError();
    }

    if (sourceBuffer)
    {
        SetupVertexAttributes(streamDefinition);
    }
}

void GraphicsDevice::BindIndexBuffer(GpuBuffer* sourceBuffer)
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return;
    }

    if (sourceBuffer)
    {
        debug_assert(sourceBuffer->mContent == eBufferContent_Indices);
    }
    
    if (mGraphicsContext.mCurrentBuffers[eBufferContent_Indices] == sourceBuffer)
        return;

    mGraphicsContext.mCurrentBuffers[eBufferContent_Indices] = sourceBuffer;
    GLenum bufferTargetGL = EnumToGL(eBufferContent_Indices);
    ::glBindBuffer(bufferTargetGL, sourceBuffer ? sourceBuffer->mResourceHandle : 0);
    glCheckError();
}

void GraphicsDevice::BindTexture2D(eTextureUnit textureUnit, GpuTexture2D* texture2D)
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return;
    }

    debug_assert(textureUnit < eTextureUnit_COUNT);
    if (mGraphicsContext.mCurrentTextures2D[textureUnit] == texture2D)
        return;

    // activate texture unit
    if (mGraphicsContext.mCurrentTextureUnit != textureUnit)
    {
        mGraphicsContext.mCurrentTextureUnit = textureUnit;
        ::glActiveTexture(GL_TEXTURE0 + textureUnit);
        glCheckError();
    }

    mGraphicsContext.mCurrentTextures2D[textureUnit] = texture2D;
    ::glBindTexture(GL_TEXTURE_2D, texture2D ? texture2D->mResourceHandle : 0);
    glCheckError();
}

void GraphicsDevice::BindRenderProgram(GpuProgram* program)
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return;
    }

    if (mGraphicsContext.mCurrentProgram == program)
        return;

    ::glUseProgram(program ? program->mResourceHandle : 0);
    glCheckError();
    if (program)
    {
        bool programAttributes[eVertexAttribute_MAX] = {};
        for (int streamIndex = 0; streamIndex < eVertexAttribute_MAX; ++streamIndex)
        {
            if (program->mAttributes[streamIndex] == GpuVariableNULL)
                continue;

            programAttributes[program->mAttributes[streamIndex]] = true;
        }

        // setup attribute streams
        for (int ivattribute = 0; ivattribute < eVertexAttribute_COUNT; ++ivattribute)
        {
            if (programAttributes[ivattribute])
            {
                ::glEnableVertexAttribArray(ivattribute);
                glCheckError();
            }
            else
            {
                ::glDisableVertexAttribArray(ivattribute);
                glCheckError();
            }
        }
    }
    else
    {
        for (int ivattribute = 0; ivattribute < eVertexAttribute_MAX; ++ivattribute)
        {
            ::glDisableVertexAttribArray(ivattribute);
            glCheckError();
        }
    }
    mGraphicsContext.mCurrentProgram = program;
}

void GraphicsDevice::DestroyTexture2D(GpuTexture2D* textureResource)
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return;
    }

    SafeDelete(textureResource);
}

void GraphicsDevice::DestroyProgram(GpuProgram* programResource)
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return;
    }

    SafeDelete(programResource);
}

void GraphicsDevice::DestroyBuffer(GpuBuffer* bufferResource)
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return;
    }

    SafeDelete(bufferResource);
}

void GraphicsDevice::RenderIndexedPrimitives(ePrimitiveType primitive, eIndicesType indices, unsigned int offset, unsigned int numIndices)
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return;
    }

    GpuBuffer* indexBuffer = mGraphicsContext.mCurrentBuffers[eBufferContent_Indices];
    GpuBuffer* vertexBuffer = mGraphicsContext.mCurrentBuffers[eBufferContent_Vertices];
    debug_assert(indexBuffer && vertexBuffer && mGraphicsContext.mCurrentProgram);

    GLenum primitives = EnumToGL(primitive);
    GLenum indicesTypeGL = EnumToGL(indices);
    ::glDrawElements(primitives, numIndices, indicesTypeGL, BUFFER_OFFSET(offset));
    glCheckError();
}

void GraphicsDevice::RenderIndexedPrimitives(ePrimitiveType primitive, eIndicesType indices, unsigned int offset, unsigned int numIndices, unsigned int baseVertex)
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return;
    }

    GpuBuffer* indexBuffer = mGraphicsContext.mCurrentBuffers[eBufferContent_Indices];
    GpuBuffer* vertexBuffer = mGraphicsContext.mCurrentBuffers[eBufferContent_Vertices];
    debug_assert(indexBuffer && vertexBuffer && mGraphicsContext.mCurrentProgram);

    GLenum primitives = EnumToGL(primitive);
    GLenum indicesTypeGL = EnumToGL(indices);
    ::glDrawElementsBaseVertex(primitives, numIndices, indicesTypeGL, BUFFER_OFFSET(offset), baseVertex);
    glCheckError();
}

void GraphicsDevice::RenderPrimitives(ePrimitiveType primitiveType, unsigned int firstIndex, unsigned int numElements)
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return;
    }

    GpuBuffer* vertexBuffer = mGraphicsContext.mCurrentBuffers[eBufferContent_Vertices];
    debug_assert(vertexBuffer && mGraphicsContext.mCurrentProgram);

    GLenum primitives = EnumToGL(primitiveType);
    ::glDrawArrays(primitives, firstIndex, numElements);
    glCheckError();
}

void GraphicsDevice::Present()
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return;
    }

    ::glfwSwapBuffers(mGraphicsWindow);
    // process window messages
    ::glfwPollEvents();
    if (::glfwWindowShouldClose(mGraphicsWindow) == GL_TRUE)
    {
        gSystem.QuitRequest();
        return;
    }
}

void GraphicsDevice::SetViewportRect(const Rect2D& sourceRectangle)
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return;
    }

    if (mViewportRect == sourceRectangle)
        return;

    mViewportRect = sourceRectangle;
    ::glViewport(mViewportRect.x, mViewportRect.y, mViewportRect.w, mViewportRect.h);
    glCheckError();
}

void GraphicsDevice::SetClearColor(Color32 clearColor)
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return;
    }

    const float inv = 1.0f / 255.0f;
    ::glClearColor(clearColor.mR * inv, clearColor.mG * inv, clearColor.mB * inv, clearColor.mA * inv);
    glCheckError();
}

void GraphicsDevice::ClearScreen()
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return;
    }

    ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glCheckError();
}

bool GraphicsDevice::IsDeviceInited() const
{
    return mGraphicsWindow != nullptr;
}

bool GraphicsDevice::InitializeOGLExtensions()
{
    // initialize opengl extensions
    ::glewExperimental = GL_TRUE;

    // initialize glew
    GLenum resultCode = ::glewInit();
    if (resultCode != GLEW_OK)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Could not initialize OpenGL extensions (%s)", ::glewGetErrorString(resultCode));
        return false;
    }

    if (!GLEW_VERSION_3_2)
    {
        gConsole.LogMessage(eLogMessage_Warning, "OpenGL 3.2 API is not available");
        return false;
    }

    // dump opengl information
    gConsole.LogMessage(eLogMessage_Info, "OpenGL Vendor: %s", ::glGetString(GL_VENDOR));
    gConsole.LogMessage(eLogMessage_Info, "OpenGL Renderer: %s", ::glGetString(GL_RENDERER));
    gConsole.LogMessage(eLogMessage_Info, "OpenGL Version: %s", ::glGetString(GL_VERSION));
    gConsole.LogMessage(eLogMessage_Info, "GLSL Version: %s", ::glGetString(GL_SHADING_LANGUAGE_VERSION));
#if 0
    // query extensions
    GLint glNumExtensions = 0;

    ::glGetIntegerv(GL_NUM_EXTENSIONS, &glNumExtensions);
    if (glNumExtensions > 0)
    {
        gConsole.LogMessage(eLogMessage_Info, "Supported OpenGL Extensions:");
        // enum all extensions
        for (GLint iextension = 0; iextension < glNumExtensions; ++iextension)
        {
            gConsole.LogMessage(eLogMessage_Info, "%s", ::glGetStringi(GL_EXTENSIONS, iextension));
        }
    } // if extensions
#endif

    return true;
}

void GraphicsDevice::SetupVertexAttributes(const VertexFormat& streamDefinition)
{
    GpuProgram* currentProgram = mGraphicsContext.mCurrentProgram;
    for (int iattribute = 0; iattribute < eVertexAttribute_COUNT; ++iattribute)
    {
        if (currentProgram->mAttributes[iattribute] == GpuVariableNULL)
        {
            // current vertex attribute is unused in shader
            continue;
        }

        const auto& attribute = streamDefinition.mAttributes[iattribute];
        if (attribute.mSemantics == eVertexAttributeSemantics_Unknown)
        {
            debug_assert(false);
            continue;
        }

        GLint numComponents = GetAttributeComponentCount(attribute.mSemantics);
        if (numComponents == 0)
        {
            debug_assert(numComponents > 0);
            continue;
        }

        // set attribute location
        bool isColorAttribute = (attribute.mSemantics == eVertexAttributeSemantics_Color);
        ::glVertexAttribPointer(currentProgram->mAttributes[iattribute], numComponents, 
            isColorAttribute ? GL_UNSIGNED_BYTE : GL_FLOAT, 
            isColorAttribute ? GL_TRUE : GL_FALSE, streamDefinition.mDataStride, BUFFER_OFFSET(attribute.mDataOffset));
        glCheckError();
    }
}

void GraphicsDevice::InternalSetRenderStates(const RenderStates& renderStates, bool forceState)
{
    if (mCurrentStates == renderStates && !forceState)
        return;

    // polygon mode
    if (forceState || (mCurrentStates.mFillMode != renderStates.mFillMode))
    {
        GLenum mode = GL_FILL;
        switch (renderStates.mFillMode)
        {
            case eFillMode_WireFrame: mode = GL_LINE; break;
            case eFillMode_Solid: mode = GL_FILL; break;
            default:
                debug_assert(false);
            break;
        }
        ::glPolygonMode(GL_FRONT_AND_BACK, mode);
        glCheckError();
    }

    // depth testing
    if (forceState || !mCurrentStates.MatchFlags(renderStates, RenderStateFlags_DepthTest))
    {
        if (renderStates.IsEnabled(RenderStateFlags_DepthTest))
        {
            ::glEnable(GL_DEPTH_TEST);
        }
        else
        {
            ::glDisable(GL_DEPTH_TEST);
        }
        glCheckError();
    }

    // depth function
    if (forceState || (mCurrentStates.mDepthFunc != renderStates.mDepthFunc))
    {
        GLenum mode = GL_LEQUAL;
        switch (renderStates.mDepthFunc)
        {
            case eDepthTestFunc_NotEqual: mode = GL_NOTEQUAL; break;
            case eDepthTestFunc_Always: mode = GL_ALWAYS; break;
            case eDepthTestFunc_Equal: mode = GL_EQUAL; break;
            case eDepthTestFunc_Less: mode = GL_LESS; break;
            case eDepthTestFunc_Greater: mode = GL_GREATER; break;
            case eDepthTestFunc_LessEqual: mode = GL_LEQUAL; break;
            case eDepthTestFunc_GreaterEqual: mode = GL_GEQUAL; break;
            default:
                debug_assert(false);
            break;
        }
        ::glDepthFunc(mode);
        glCheckError();
    }

    if (forceState || !mCurrentStates.MatchFlags(renderStates, RenderStateFlags_DepthWrite))
    {
        ::glDepthMask(renderStates.IsEnabled(RenderStateFlags_DepthWrite) ? GL_TRUE : GL_FALSE);
        glCheckError();
    }

    if (forceState || !mCurrentStates.MatchFlags(renderStates, RenderStateFlags_ColorWrite))
    {
        const GLboolean isEnabled = renderStates.IsEnabled(RenderStateFlags_ColorWrite) ? GL_TRUE : GL_FALSE;
        ::glColorMask(isEnabled, isEnabled, isEnabled, isEnabled);
        glCheckError();
    }

    // blending
    if (forceState || !mCurrentStates.MatchFlags(renderStates, RenderStateFlags_AlphaBlend))
    {
        if (renderStates.IsEnabled(RenderStateFlags_AlphaBlend))
        {
            ::glEnable(GL_BLEND);
        }
        else
        {
            ::glDisable(GL_BLEND);
        }
        glCheckError();
    }

    if (forceState || (mCurrentStates.mBlendMode != renderStates.mBlendMode))
    {
        GLenum srcFactor = GL_ZERO;
        GLenum dstFactor = GL_ZERO;

        switch (renderStates.mBlendMode)
        {
            case eBlendMode_Alpha:
                srcFactor = GL_SRC_ALPHA;
                dstFactor = GL_ONE_MINUS_SRC_ALPHA;
            break;
            case eBlendMode_Additive:
                srcFactor = GL_SRC_ALPHA;
                dstFactor = GL_ONE;
            break;
            case eBlendMode_Multiply:
                srcFactor = GL_DST_COLOR;
                dstFactor = GL_ZERO;
            break;
            case eBlendMode_Premultiplied:
                srcFactor = GL_ONE;
                dstFactor = GL_ONE_MINUS_SRC_ALPHA;
            break;
            case eBlendMode_Screen:
                srcFactor = GL_ONE_MINUS_DST_COLOR;
                dstFactor = GL_ONE;
            break;
            default:
                debug_assert(false);
            break;
        }

        ::glBlendFunc(srcFactor, dstFactor);
        glCheckError();
    }

    // culling
    if (forceState || !mCurrentStates.MatchFlags(renderStates, RenderStateFlags_FaceCulling))
    {
        if (renderStates.IsEnabled(RenderStateFlags_FaceCulling))
        {
            ::glEnable(GL_CULL_FACE);
        }
        else
        {
            ::glDisable(GL_CULL_FACE);
        }
        glCheckError();
    }

    if (forceState || (mCurrentStates.mCullMode != renderStates.mCullMode))
    {
        GLenum mode = GL_BACK;
        switch (renderStates.mCullMode)
        {
            case eCullMode_Back: mode = GL_BACK; break;
            case eCullMode_Front: mode = GL_FRONT; break;
            case  eCullMode_FrontAndBack: mode = GL_FRONT_AND_BACK; break;
            default:
                debug_assert(false);
            break;
        }
        ::glCullFace(mode);
        glCheckError();
    }

    mCurrentStates = renderStates;
}