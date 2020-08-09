#pragma once

#include "GraphicsDefs.h"
#include "GraphicsContext.h"

// Graphics device is responsible for displaying game graphics

class GraphicsDevice final: public cxx::noncopyable
{
public:
    // public for convenience, don't change these fields directly
    RenderStates mCurrentStates;
    Rectangle mViewportRect;
    Rectangle mScissorBox;
    GraphicsDeviceCaps mCaps;

    // these params will automatically set during texture creation
    eTextureFilterMode mDefaultTextureFilter = eTextureFilterMode_Nearest;
    eTextureWrapMode mDefaultTextureWrap = eTextureWrapMode_ClampToEdge;

    // current screen params
    Point mScreenResolution;

public:
    GraphicsDevice();
    ~GraphicsDevice();

    // Initialize graphics system
    bool Initialize();

    // Shutdown graphics system, any render operations will be ignored after this
    void Deinit();

    // Turn vsync mode on or off
    // @param vsyncEnabled: True to enable or false to disable
    void EnableVSync(bool vsyncEnabled);

    // Turn fullscreen mode on or off
    // @param fullscreenEnabled: True to fullscreen or false to windowed
    void EnableFullscreen(bool fullscreenEnabled);

    // Create buffer texture, client is responsible for destroying resource
    // @param textureFormat: Format
    // @param dataLength: Data length in bytes
    // @param sourceData: Source data buffer
    GpuBufferTexture* CreateBufferTexture();
    GpuBufferTexture* CreateBufferTexture(eTextureFormat textureFormat, int dataLength, const void* sourceData);

    // Create 2D texture, client is responsible for destroying resource
    // @param textureFormat: Format
    // @param sizex, sizey: Texture dimensions, must be POT!
    // @param sourceData: Source data buffer
    GpuTexture2D* CreateTexture2D();
    GpuTexture2D* CreateTexture2D(eTextureFormat textureFormat, int sizex, int sizey, const void* sourceData);

    // Create texture array 2D, client is responsible for destroying resource
    // @param textureFormat: Format
    // @param sizex, sizey: Texture dimensions, must be POT!
    // @param layersCount: Number of textures in array
    // @param sourceData: Source data buffer, all layers must be specified if not null
    GpuTextureArray2D* CreateTextureArray2D();
    GpuTextureArray2D* CreateTextureArray2D(eTextureFormat textureFormat, int sizex, int sizey, int layersCount, const void* sourceData);

    // Create render program, client is responsible for destroying resource
    // @param shaderSource: Source code
    GpuProgram* CreateRenderProgram();
    GpuProgram* CreateRenderProgram(const char* shaderSource);

    // Create hardware buffer, client is responsible for destroying resource
    // @param bufferContent: Content type stored in buffer
    // @param bufferUsage: Usage hint of buffer
    // @param theLength: Data length
    // @param dataBuffer: Initial data, optional
    // @returns false if out of memory
    GpuBuffer* CreateBuffer(eBufferContent bufferContent);
    GpuBuffer* CreateBuffer(eBufferContent bufferContent, eBufferUsage bufferUsage, unsigned int bufferLength, const void* dataBuffer);

    // Set source buffer for geometries vertex data and setup layout for bound shader
    // @param sourceBuffer: Buffer reference or nullptr to unbind current
    // @param streamDefinition: Layout
    void BindVertexBuffer(GpuBuffer* sourceBuffer, const VertexFormat& streamDefinition);

    // Set source buffer for geometries index data
    // @param sourceBuffer: Buffer reference or nullptr to unbind current
    void BindIndexBuffer(GpuBuffer* sourceBuffer);

    // Set source texture on specified texture unit
    // @param textureUnit: Target unit
    // @param texture: Texture
    void BindTexture(eTextureUnit textureUnit, GpuBufferTexture* texture);
    void BindTexture(eTextureUnit textureUnit, GpuTexture2D* texture);
    void BindTexture(eTextureUnit textureUnit, GpuTextureArray2D* texture);

    // Set source render program to render with
    // @param program: Target program
    void BindRenderProgram(GpuProgram* program);

    // Free hardware resource
    // @param textureResource: Target texture, pointer becomes invalid
    void DestroyTexture(GpuBufferTexture* textureResource);
    void DestroyTexture(GpuTexture2D* textureResource);
    void DestroyTexture(GpuTextureArray2D* textureResource);

    // Free hardware resource
    // @param programResource: Target render program, pointer becomes invalid
    void DestroyProgram(GpuProgram* programResource);

    // Free hardware resource
    // @param bufferResource: Target buffer, pointer becomes invalid
    void DestroyBuffer(GpuBuffer* bufferResource);

    // Set current render states
    // @param renderStates: Render states
    void SetRenderStates(const RenderStates& renderStates)
    {
        InternalSetRenderStates(renderStates, false);
    }

    // Render indexed geometry
    // @param primitive: Type of primitives to render
    // @param indicesType: Type of indices data
    // @param offset: Offset within index buffer in bytes
    // @param numIndices: Number of elements
    // @param baseVertex: Specifies a constant that should be added to each element of indices when chosing elements from the vertex arrays
    void RenderIndexedPrimitives(ePrimitiveType primitive, eIndicesType indicesType, unsigned int offset, unsigned int numIndices);
    void RenderIndexedPrimitives(ePrimitiveType primitive, eIndicesType indicesType, unsigned int offset, unsigned int numIndices, unsigned int baseVertex);

    // Render geometry
    // @param primitiveType: Type of primitives to render
    // @param firstIndex: Start position in attribute buffers, index
    // @param numElements: Number of elements to render
    void RenderPrimitives(ePrimitiveType primitiveType, unsigned int firstIndex, unsigned int numElements);

    // Finish render frame, prenent on screen
    void Present();

    // Setup dimensions of graphic device viewport
    // @param sourceRectangle: Viewport rectangle
    void SetViewportRect(const Rectangle& sourceRectangle);

    // Setup clip rect, default is whole viewport area
    // @param sourceRectangle: Clipping rectangle
    void SetScissorRect(const Rectangle& sourceRectangle);

    // Set clear color for render revice
    // @param clearColor: Color components
    void SetClearColor(Color32 clearColor);

    // Clear color and depth of current framebuffer
    void ClearScreen();

    // Test whether graphics is initialized properly
    bool IsDeviceInited() const;
    
private:
    // Force render state
    // @param rstate: Render state
    void InternalSetRenderStates(const RenderStates& renderStates, bool forceState);
    bool InitializeOGLExtensions();
    void QueryGraphicsDeviceCaps();
    void ActivateTextureUnit(eTextureUnit textureUnit);

    void SetupVertexAttributes(const VertexFormat& streamDefinition);

    void ProcessGamepadsInputs();

private:
    GraphicsContext mGraphicsContext;
    GLFWwindow* mGraphicsWindow;
    GLFWmonitor* mGraphicsMonitor;
};

extern GraphicsDevice gGraphicsDevice;