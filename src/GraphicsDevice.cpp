#include "stdafx.h"
#include "GraphicsDevice.h"
#include "OpenGLDefs.h"
#include "GpuProgram.h"
#include "GpuBuffer.h"
#include "GpuBufferTexture.h"
#include "GpuTexture2D.h"
#include "GpuTextureArray2D.h"

#define WINDOW_TITLE "Carnage3D"

GraphicsDevice gGraphicsDevice;

//////////////////////////////////////////////////////////////////////////

// glfw to native input mapping
static eKeycode GlfwKeycodeToNative(int keycode)
{
    switch (keycode)
    {
        case GLFW_KEY_ESCAPE: return eKeycode_ESCAPE;
        case GLFW_KEY_SPACE: return eKeycode_SPACE;
        case GLFW_KEY_PAGE_UP: return eKeycode_PAGE_UP;
        case GLFW_KEY_PAGE_DOWN: return eKeycode_PAGE_DOWN;
        case GLFW_KEY_HOME: return eKeycode_HOME;
        case GLFW_KEY_END: return eKeycode_END;
        case GLFW_KEY_INSERT: return eKeycode_INSERT;
        case GLFW_KEY_DELETE: return eKeycode_DELETE;
        case GLFW_KEY_RIGHT_CONTROL: return eKeycode_RIGHT_CTRL;
        case GLFW_KEY_LEFT_CONTROL: return eKeycode_LEFT_CTRL;
        case GLFW_KEY_BACKSPACE: return eKeycode_BACKSPACE;
        case GLFW_KEY_ENTER: return eKeycode_ENTER;
        case GLFW_KEY_TAB: return eKeycode_TAB;
        case GLFW_KEY_GRAVE_ACCENT: return eKeycode_TILDE;
        case GLFW_KEY_F1: return eKeycode_F1;
        case GLFW_KEY_F2: return eKeycode_F2;
        case GLFW_KEY_F3: return eKeycode_F3;
        case GLFW_KEY_F4: return eKeycode_F4;
        case GLFW_KEY_F5: return eKeycode_F5;
        case GLFW_KEY_F6: return eKeycode_F6;
        case GLFW_KEY_F7: return eKeycode_F7;
        case GLFW_KEY_F8: return eKeycode_F8;
        case GLFW_KEY_F9: return eKeycode_F9;
        case GLFW_KEY_F10: return eKeycode_F10;
        case GLFW_KEY_F11: return eKeycode_F11;
        case GLFW_KEY_F12: return eKeycode_F12;
        case GLFW_KEY_A: return eKeycode_A;
        case GLFW_KEY_C: return eKeycode_C;
        case GLFW_KEY_F: return eKeycode_F;
        case GLFW_KEY_V: return eKeycode_V;
        case GLFW_KEY_X: return eKeycode_X;
        case GLFW_KEY_W: return eKeycode_W;
        case GLFW_KEY_D: return eKeycode_D;
        case GLFW_KEY_S: return eKeycode_S;
        case GLFW_KEY_Y: return eKeycode_Y;
        case GLFW_KEY_Z: return eKeycode_Z;
        case GLFW_KEY_R: return eKeycode_R;
        case GLFW_KEY_0: return eKeycode_0;
        case GLFW_KEY_1: return eKeycode_1;
        case GLFW_KEY_2: return eKeycode_2;
        case GLFW_KEY_3: return eKeycode_3;
        case GLFW_KEY_4: return eKeycode_4;
        case GLFW_KEY_5: return eKeycode_5;
        case GLFW_KEY_6: return eKeycode_6;
        case GLFW_KEY_7: return eKeycode_7;
        case GLFW_KEY_8: return eKeycode_8;
        case GLFW_KEY_9: return eKeycode_9;
        case GLFW_KEY_LEFT: return eKeycode_LEFT;
        case GLFW_KEY_RIGHT: return eKeycode_RIGHT;
        case GLFW_KEY_UP: return eKeycode_UP;
        case GLFW_KEY_DOWN: return eKeycode_DOWN;
    }
    return eKeycode_null;
}

static eMButton GlfwMouseButtonToNative(int mbutton)
{
    switch (mbutton)
    {
        case GLFW_MOUSE_BUTTON_LEFT: return eMButton_LEFT;
        case GLFW_MOUSE_BUTTON_RIGHT: return eMButton_RIGHT;
        case GLFW_MOUSE_BUTTON_MIDDLE: return eMButton_MIDDLE;
    }
    return eMButton_null;
}

static eGamepadButton GlfwGamepadButtonToNative(int gpbutton)
{
    switch(gpbutton)
    {
        case GLFW_GAMEPAD_BUTTON_A: return eGamepadButton_A;
        case GLFW_GAMEPAD_BUTTON_B: return eGamepadButton_B;
        case GLFW_GAMEPAD_BUTTON_X: return eGamepadButton_X;
        case GLFW_GAMEPAD_BUTTON_Y: return eGamepadButton_Y;
        case GLFW_GAMEPAD_BUTTON_LEFT_BUMPER: return eGamepadButton_LeftBumper;
        case GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER: return eGamepadButton_RightBumper;
        case GLFW_GAMEPAD_BUTTON_BACK: return eGamepadButton_Back;
        case GLFW_GAMEPAD_BUTTON_START: return eGamepadButton_Start;
        case GLFW_GAMEPAD_BUTTON_GUIDE: return eGamepadButton_Guide;
        case GLFW_GAMEPAD_BUTTON_LEFT_THUMB: return eGamepadButton_LeftThumb;
        case GLFW_GAMEPAD_BUTTON_RIGHT_THUMB: return eGamepadButton_RightThumb;
        case GLFW_GAMEPAD_BUTTON_DPAD_UP: return eGamepadButton_DPAD_Up;
        case GLFW_GAMEPAD_BUTTON_DPAD_RIGHT: return eGamepadButton_DPAD_Right;
        case GLFW_GAMEPAD_BUTTON_DPAD_DOWN: return eGamepadButton_DPAD_Down;
        case GLFW_GAMEPAD_BUTTON_DPAD_LEFT: return eGamepadButton_DPAD_Left;
    };
    return eGamepadButton_null;
}

//////////////////////////////////////////////////////////////////////////

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

    ::glfwSetErrorCallback([](int errorCode, const char * errorString)
        {
            gConsole.LogMessage(eLogMessage_Error, "GLFW error occurred: %s", errorString);
        });

    if (::glfwInit() == GL_FALSE)
    {
        gConsole.LogMessage(eLogMessage_Warning, "GLFW initialization failed");
        return false;
    }

    // dump some information
    gConsole.LogMessage(eLogMessage_Info, "GLFW Information: %s", ::glfwGetVersionString());
    gConsole.LogMessage(eLogMessage_Info, "Initialize OpenGL %d.%d %s",
        OPENGL_CONTEXT_MAJOR_VERSION, 
        OPENGL_CONTEXT_MINOR_VERSION,
        gSystem.mConfig.mOpenGLCoreProfile ? "(Core profile)" : "");

    GLFWmonitor* graphicsMonitor = nullptr;
    if (fullscreen)
    {
        graphicsMonitor = ::glfwGetPrimaryMonitor();
        debug_assert(graphicsMonitor);
    }

    gConsole.LogMessage(eLogMessage_Info, "Screen resolution: %dx%d, Vsync: %s, Fullscreen: %s", 
        screensizex, 
        screensizey, vsync ? "enabled" : "disabled", fullscreen ? "yes" : "no");

    // opengl params
    if (gSystem.mConfig.mOpenGLCoreProfile)
    {
        ::glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        ::glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }

    ::glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_CONTEXT_MAJOR_VERSION);
    ::glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_CONTEXT_MINOR_VERSION);
    // setup window params
    ::glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    ::glfwWindowHint(GLFW_RED_BITS, 8);
    ::glfwWindowHint(GLFW_GREEN_BITS, 8);
    ::glfwWindowHint(GLFW_BLUE_BITS, 8);
    ::glfwWindowHint(GLFW_ALPHA_BITS, 8);
    ::glfwWindowHint(GLFW_DEPTH_BITS, 16);

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
            if (action == GLFW_REPEAT)
                return;

            eMButton mbuttonNative = GlfwMouseButtonToNative(button);
            if (mbuttonNative != eMButton_null)
            {
                MouseButtonInputEvent ev { mbuttonNative, mods, action == GLFW_PRESS };
                gSystem.HandleEvent(ev);
            }
        });
    ::glfwSetKeyCallback(graphicsWindow, [](GLFWwindow*, int keycode, int scancode, int action, int mods)
        {
            if (action == GLFW_REPEAT)
                return;

            eKeycode keycodeNative = GlfwKeycodeToNative(keycode);
            if (keycodeNative != eKeycode_null)
            {
                KeyInputEvent ev { keycodeNative, scancode, mods, action == GLFW_PRESS };
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
    ::glfwSetJoystickCallback([](int gamepad, int gamepadStatus)
        {
            if (gamepad < MAX_GAMEPADS)
            {
                gInputs.SetGamepadPresent(gamepad, (gamepadStatus == GLFW_CONNECTED));
            }
        });

    // setup opengl extensions
    if (!InitializeOGLExtensions())
    {
        ::glfwDestroyWindow(graphicsWindow);
        ::glfwTerminate();
        return false;
    }

    // clear opengl errors
    glClearError();

    mGraphicsWindow = graphicsWindow;
    mGraphicsMonitor = graphicsMonitor;

    QueryGraphicsDeviceCaps();

    // create global vertex array object
    ::glGenVertexArrays(1, &mGraphicsContext.mVaoHandle);
    glCheckError();

    ::glBindVertexArray(mGraphicsContext.mVaoHandle);
    glCheckError();

    // scissor test always enabled
    ::glEnable(GL_SCISSOR_TEST);
    glCheckError();

    // setup viewport
    mViewportRect.x = 0;
    mViewportRect.y = 0;
    mViewportRect.w = screensizex;
    mViewportRect.h = screensizey;

    ::glViewport(mViewportRect.x, mViewportRect.y, mViewportRect.w, mViewportRect.h);
    glCheckError();

    // default value for scissor is whole viewport
    mScissorBox = mViewportRect;

    ::glScissor(mViewportRect.x, mViewportRect.y, mViewportRect.w, mViewportRect.h);
    glCheckError();

    ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glCheckError();

    // force clear screen at stratup
    ::glfwSwapBuffers(mGraphicsWindow);
    glCheckError();

    // setup default render state
    static const RenderStates defaultRenderStates;
    InternalSetRenderStates(defaultRenderStates, true);

    EnableFullscreen(fullscreen);
    EnableVSync(vsync);

    // init gamepads
    for (int icurr = 0; icurr < MAX_GAMEPADS; ++icurr)
    {
        bool isGamepad = ::glfwJoystickIsGamepad(icurr) == GLFW_TRUE;
        gInputs.SetGamepadPresent(icurr, isGamepad);
    }

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

    ::glfwSwapInterval(vsyncEnabled ? 1 : 0);
}

void GraphicsDevice::EnableFullscreen(bool fullscreenEnabled)
{
    if (!IsDeviceInited())
        return;

    if (mGraphicsMonitor == nullptr && fullscreenEnabled)
    {
        mGraphicsMonitor = ::glfwGetPrimaryMonitor();
        ::glfwSetWindowMonitor(mGraphicsWindow, mGraphicsMonitor, 0, 0, mViewportRect.w, mViewportRect.h, 0);
    }
    else
    {
        mGraphicsMonitor = nullptr;
        ::glfwSetWindowMonitor(mGraphicsWindow, mGraphicsMonitor, 60, 60, mViewportRect.w, mViewportRect.h, 0);
    }
}

GpuBufferTexture* GraphicsDevice::CreateBufferTexture()
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return nullptr;
    }

    GpuBufferTexture* texture = new GpuBufferTexture(mGraphicsContext);
    return texture;
}

GpuBufferTexture* GraphicsDevice::CreateBufferTexture(eTextureFormat textureFormat, int dataLength, const void* sourceData)
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return nullptr;
    }

    GpuBufferTexture* texture = new GpuBufferTexture(mGraphicsContext);
    if (!texture->Setup(textureFormat, dataLength, sourceData))
    {
        DestroyTexture(texture);
        return nullptr;
    }
    return texture;
}

GpuTexture2D* GraphicsDevice::CreateTexture2D()
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return nullptr;
    }

    GpuTexture2D* texture = new GpuTexture2D(mGraphicsContext);
    return texture;
}

GpuTexture2D* GraphicsDevice::CreateTexture2D(eTextureFormat textureFormat, int sizex, int sizey, const void* sourceData)
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return nullptr;
    }

    GpuTexture2D* texture = new GpuTexture2D(mGraphicsContext);
    if (!texture->Setup(textureFormat, sizex, sizey, sourceData))
    {
        DestroyTexture(texture);
        return nullptr;
    }
    return texture;
}

GpuTextureArray2D* GraphicsDevice::CreateTextureArray2D()
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return nullptr;
    }

    GpuTextureArray2D* texture = new GpuTextureArray2D(mGraphicsContext);
    return texture;   
}

GpuTextureArray2D* GraphicsDevice::CreateTextureArray2D(eTextureFormat textureFormat, int sizex, int sizey, int layersCount, const void* sourceData)
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return nullptr;
    }

    GpuTextureArray2D* texture = new GpuTextureArray2D(mGraphicsContext);
    if (!texture->Setup(textureFormat, sizex, sizey, layersCount, sourceData))
    {
        DestroyTexture(texture);
        return nullptr;
    }
    return texture;
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

GpuBuffer* GraphicsDevice::CreateBuffer(eBufferContent bufferContent)
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return nullptr;
    }
    debug_assert(bufferContent < eBufferContent_COUNT);
    GpuBuffer* bufferObject = new GpuBuffer(mGraphicsContext, bufferContent);
    return bufferObject;
}

GpuBuffer* GraphicsDevice::CreateBuffer(eBufferContent bufferContent, eBufferUsage bufferUsage, unsigned int bufferLength, const void* dataBuffer)
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return nullptr;
    }
    debug_assert(bufferContent < eBufferContent_COUNT);
    GpuBuffer* bufferObject = new GpuBuffer(mGraphicsContext, bufferContent);
    if (!bufferObject->Setup(bufferUsage, bufferLength, dataBuffer))
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

void GraphicsDevice::BindTexture(eTextureUnit textureUnit, GpuBufferTexture* texture)
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return;
    }

    debug_assert(textureUnit < eTextureUnit_COUNT);
    if (mGraphicsContext.mCurrentTextures[textureUnit].mBufferTexture == texture)
        return;

    ActivateTextureUnit(textureUnit);

    mGraphicsContext.mCurrentTextures[textureUnit].mBufferTexture = texture;
    ::glBindTexture(GL_TEXTURE_BUFFER, texture ? texture->mResourceHandle : 0);
    glCheckError();
}

void GraphicsDevice::BindTexture(eTextureUnit textureUnit, GpuTexture2D* texture)
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return;
    }

    debug_assert(textureUnit < eTextureUnit_COUNT);
    if (mGraphicsContext.mCurrentTextures[textureUnit].mTexture2D == texture)
        return;

    ActivateTextureUnit(textureUnit);

    mGraphicsContext.mCurrentTextures[textureUnit].mTexture2D = texture;
    ::glBindTexture(GL_TEXTURE_2D, texture ? texture->mResourceHandle : 0);
    glCheckError();
}

void GraphicsDevice::BindTexture(eTextureUnit textureUnit, GpuTextureArray2D* texture)
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return;
    }

    debug_assert(textureUnit < eTextureUnit_COUNT);
    if (mGraphicsContext.mCurrentTextures[textureUnit].mTextureArray2D == texture)
        return;

    ActivateTextureUnit(textureUnit);

    mGraphicsContext.mCurrentTextures[textureUnit].mTextureArray2D = texture;
    ::glBindTexture(GL_TEXTURE_2D_ARRAY, texture ? texture->mResourceHandle : 0);
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

void GraphicsDevice::DestroyTexture(GpuBufferTexture* textureResource)
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return;
    }

    SafeDelete(textureResource);
}

void GraphicsDevice::DestroyTexture(GpuTexture2D* textureResource)
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return;
    }

    SafeDelete(textureResource);
}

void GraphicsDevice::DestroyTexture(GpuTextureArray2D* textureResource)
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
    ProcessGamepadsInputs();
}

void GraphicsDevice::ProcessGamepadsInputs()
{
    GLFWgamepadstate gamepadstate;

    for (int icurr = 0; icurr < MAX_GAMEPADS; ++icurr)
    {
        GamepadState& currGamepad = gInputs.mGamepadsState[icurr];
        if (!currGamepad.mPresent)
            continue;

        if (::glfwGetGamepadState(icurr, &gamepadstate) != GLFW_TRUE)
            continue;

        for (int ibutton = 0; ibutton < GLFW_JOYSTICK_LAST + 1; ++ibutton)
        {
            eGamepadButton buttonNative = GlfwGamepadButtonToNative(ibutton);
            if (buttonNative == eGamepadButton_null)
                continue;

            bool newPressed = gamepadstate.buttons[ibutton] == GLFW_PRESS;
            if (currGamepad.mButtons[buttonNative] == newPressed)
                continue;

            GamepadInputEvent inputEvent { icurr, buttonNative, newPressed };
            gSystem.HandleEvent(inputEvent);
        }
        
        // triggers
        bool leftTriggerPressed = gamepadstate.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] > 0.5f;
        if (leftTriggerPressed != currGamepad.mButtons[eGamepadButton_LeftTrigger])
        {
            GamepadInputEvent inputEvent { icurr, eGamepadButton_LeftTrigger, leftTriggerPressed };
            gSystem.HandleEvent(inputEvent);
        }

        bool rightTriggerPressed = gamepadstate.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] > 0.5f;
        if (rightTriggerPressed != currGamepad.mButtons[eGamepadButton_RightTrigger])
        {
            GamepadInputEvent inputEvent { icurr, eGamepadButton_RightTrigger, rightTriggerPressed };
            gSystem.HandleEvent(inputEvent);
        }
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

void GraphicsDevice::SetScissorRect(const Rect2D& sourceRectangle)
{
    if (!IsDeviceInited())
    {
        debug_assert(false);
        return;
    }

    if (mScissorBox == sourceRectangle)
        return;

    mScissorBox = sourceRectangle;
    ::glScissor(mScissorBox.x, mScissorBox.y, mScissorBox.w, mScissorBox.h);
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

        unsigned int numComponents = GetAttributeComponentCount(attribute.mSemantics);
        if (numComponents == 0)
        {
            debug_assert(numComponents > 0);
            continue;
        }

        GLboolean normalizeData = GL_FALSE;
        if (attribute.mSemantics == eVertexAttributeSemantics_Color)
        {
            normalizeData = GL_TRUE;
        }
        GLenum dataType = GetAttributeDataTypeGL(attribute.mSemantics);
        // set attribute location
        ::glVertexAttribPointer(currentProgram->mAttributes[iattribute], numComponents, dataType, normalizeData, 
            streamDefinition.mDataStride, BUFFER_OFFSET(attribute.mDataOffset + streamDefinition.mBaseOffset));
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

void GraphicsDevice::QueryGraphicsDeviceCaps()
{
    mCaps.mFeatures[eGraphicsFeature_NPOT_Textures] = (GLEW_ARB_texture_non_power_of_two == GL_TRUE);
    mCaps.mFeatures[eGraphicsFeature_ABGR] = (GLEW_EXT_abgr == GL_TRUE);

    ::glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &mCaps.mMaxTextureBufferSize);
    glCheckError();

    ::glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &mCaps.mMaxArrayTextureLayers);
    glCheckError();

    gConsole.LogMessage(eLogMessage_Info, "Graphics Device caps:");
    gConsole.LogMessage(eLogMessage_Info, " - max array texture layers: %d", mCaps.mMaxArrayTextureLayers);
    gConsole.LogMessage(eLogMessage_Info, " - max texture buffer size: %d bytes", mCaps.mMaxTextureBufferSize);
}

void GraphicsDevice::ActivateTextureUnit(eTextureUnit textureUnit)
{
    if (mGraphicsContext.mCurrentTextureUnit == textureUnit)
        return;

    mGraphicsContext.mCurrentTextureUnit = textureUnit;

    ::glActiveTexture(GL_TEXTURE0 + textureUnit);
    glCheckError();
}
