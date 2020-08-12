#pragma once

// forwards
class GraphicsDevice;
class GpuBuffer;
class GpuProgram;
class GpuBufferTexture;
class GpuTexture2D;
class GpuTextureArray2D;
class GraphicsContext;

// internal types
using GpuProgramHandle = unsigned int;
using GpuBufferHandle = unsigned int;
using GpuTextureHandle = unsigned int;
using GpuVertexArrayHandle = unsigned int;
using GpuVariableLocation = int;

// predefined value for unspecified render program variable location
const GpuVariableLocation GpuVariableNULL = -1;

using DrawIndex = unsigned int;
const unsigned int Sizeof_DrawIndex = sizeof(DrawIndex);

// vertex 3d
struct Vertex3D
{
public: 
    Vertex3D() = default;

    // setup vertex
    // @param posx, posy, posz: Coordinate in 3d space
    // @param tcu, tcv: Texture coordinate normalized [0, 1]
    // @param normx, normy, normz: Normal unit vector
    // @param color: Color RGBA
    void Set(float posx, float posy, float posz, float tcu, float tcv, float normx, float normy, float normz, unsigned int color)
    {
        mPosition.x = posx;
        mPosition.y = posy;
        mPosition.z = posz;
        mNormal.x = normx;
        mNormal.y = normy;
        mNormal.z = normz;
        mTexcoord.x = tcu;
        mTexcoord.y = tcv;
        mColor = color;
    }
    // setup vertex
    void Set(float posx, float posy, float posz, float tcu, float tcv, unsigned int color)
    {
        mPosition.x = posx;
        mPosition.y = posy;
        mPosition.z = posz;
        mTexcoord.x = tcu;
        mTexcoord.y = tcv;
        mColor = color;
    }
    // setup vertex
    void Set(float posx, float posy, float posz, float tcu, float tcv)
    {
        mPosition.x = posx;
        mPosition.y = posy;
        mPosition.z = posz;
        mTexcoord.x = tcu;
        mTexcoord.y = tcv;
        mColor = Color32_White;
    }
public:
    glm::vec3 mPosition; // 12 bytes
    glm::vec3 mNormal; // 12 bytes
    glm::vec2 mTexcoord; // 8 bytes
    unsigned int mColor; // 4 bytes
};

const unsigned int Sizeof_Vertex3D = sizeof(Vertex3D);

// vertex 2d
struct Vertex2D
{
public:
    Vertex2D() = default;

    // setup vertex
    // @param posx, posy: Coordinate in 2d space
    // @param tcu, tcv: Texture coordinate normalized [0, 1]
    // @param color: Color RGBA
    void Set(float posx, float posy, float tcu, float tcv, unsigned int color)
    {
        mPosition.x = posx;
        mPosition.y = posy;
        mTexcoord.x = tcu;
        mTexcoord.y = tcv;
        mColor = color;
    }
    // setup vertex
    void Set(float posx, float posy, float tcu, float tcv)
    {
        mPosition.x = posx;
        mPosition.y = posy;
        mTexcoord.x = tcu;
        mTexcoord.y = tcv;
        mColor = Color32_White;
    }
public:
    glm::vec2 mPosition; // 8 bytes
    glm::vec2 mTexcoord; // 8 bytes
    unsigned int mColor; // 4 bytes
};

const unsigned int Sizeof_Vertex2D = sizeof(Vertex2D);

// vertex debug
struct Vertex3D_Debug
{
public:
    Vertex3D_Debug() = default;

    // setup vertex
    // @param posx, posy, posz: Coordinate in 3d space
    // @param color: Color RGBA
    void Set(float posx, float posy, float posz, unsigned int color)
    {
        mPosition.x = posx;
        mPosition.y = posy;
        mPosition.z = posz;
        mColor = color;
    }
    // setup vertex
    void Set(float posx, float posy, float posz)
    {
        mPosition.x = posx;
        mPosition.y = posy;
        mPosition.z = posz;
        mColor = Color32_White;
    }
public:
    glm::vec3 mPosition; // 12 bytes
    unsigned int mColor; // 4 bytes
};

const unsigned int Sizeof_Vertex3D_Debug = sizeof(Vertex3D_Debug);

// Build simple quad vertices
// @param textureSize: Width and Height of texture
// @param rcSrc, rcDest: Source and destination rectangles
// @param color: Quad vertices color
// @param vertices: Output 4 vertices in order TOP LEFT, BOTTOM LEFT, BOTTOM RIGHT, TOP RIGHT
inline void MakeQuad2D(const Point& textureSize, const Rect& rcSrc, const Rect& rcDest, Color32 color, Vertex2D* vertices)
{
    const float invx = 1.0f / textureSize.x;
    const float invy = 1.0f / textureSize.y;

    // TOP LEFT
    vertices[0].mColor       = color.mRGBA;
    vertices[0].mTexcoord[0] = rcSrc.x * invx;
    vertices[0].mTexcoord[1] = rcSrc.y * invy;
    vertices[0].mPosition.x  = rcDest.x * 1.0f;
    vertices[0].mPosition.y  = rcDest.y * 1.0f;

    // BOTTOM LEFT
    vertices[1].mColor       = color.mRGBA;
    vertices[1].mTexcoord[0] = vertices[0].mTexcoord[0];
    vertices[1].mTexcoord[1] = (rcSrc.y + rcSrc.h) * invy;
    vertices[1].mPosition.x  = vertices[0].mPosition.x;
    vertices[1].mPosition.y  = (rcDest.y + rcDest.h) * 1.0f;

    // BOTTOM RIGHT
    vertices[2].mColor       = color.mRGBA;
    vertices[2].mTexcoord[0] = (rcSrc.x + rcSrc.w) * invx;
    vertices[2].mTexcoord[1] = vertices[1].mTexcoord[1];
    vertices[2].mPosition.x  = (rcDest.x + rcDest.w) * 1.0f;
    vertices[2].mPosition.y  = vertices[1].mPosition.y;

    // TOP RIGHT
    vertices[3].mColor       = color.mRGBA;
    vertices[3].mTexcoord[0] = vertices[2].mTexcoord[0];
    vertices[3].mTexcoord[1] = vertices[0].mTexcoord[1];
    vertices[3].mPosition.x  = vertices[2].mPosition.x;
    vertices[3].mPosition.y  = vertices[0].mPosition.y;
}

// same as MakeQuad2D but with 3rd dimension
inline void MakeQuad3D(const Point& textureSize, const Rect& rcSrc, const Rect& rcDest, Color32 color, Vertex3D* vertices)
{
    const float invx = 1.0f / textureSize.x;
    const float invy = 1.0f / textureSize.y;

    // TOP LEFT
    vertices[0].mColor       = color.mRGBA;
    vertices[0].mTexcoord[0] = rcSrc.x * invx;
    vertices[0].mTexcoord[1] = rcSrc.y * invy;
    vertices[0].mPosition.x  = rcDest.x * 1.0f;
    vertices[0].mPosition.y  = rcDest.y * 1.0f;
    vertices[0].mPosition.z  = 0.0f;

    // BOTTOM LEFT
    vertices[1].mColor       = color.mRGBA;
    vertices[1].mTexcoord[0] = vertices[0].mTexcoord[0];
    vertices[1].mTexcoord[1] = (rcSrc.y + rcSrc.h) * invy;
    vertices[1].mPosition.x  = vertices[0].mPosition.x;
    vertices[1].mPosition.y  = (rcDest.y + rcDest.h) * 1.0f;
    vertices[1].mPosition.z  = 0.0f;

    // BOTTOM RIGHT
    vertices[2].mColor       = color.mRGBA;
    vertices[2].mTexcoord[0] = (rcSrc.x + rcSrc.w) * invx;
    vertices[2].mTexcoord[1] = vertices[1].mTexcoord[1];
    vertices[2].mPosition.x  = (rcDest.x + rcDest.w) * 1.0f;
    vertices[2].mPosition.y  = vertices[1].mPosition.y;
    vertices[2].mPosition.z  = 0.0f;

    // TOP RIGHT
    vertices[3].mColor       = color.mRGBA;
    vertices[3].mTexcoord[0] = vertices[2].mTexcoord[0];
    vertices[3].mTexcoord[1] = vertices[0].mTexcoord[1];
    vertices[3].mPosition.x  = vertices[2].mPosition.x;
    vertices[3].mPosition.y  = vertices[0].mPosition.y;
    vertices[3].mPosition.z  = 0.0f;
}

enum eTextureFilterMode
{
    eTextureFilterMode_Nearest,
    eTextureFilterMode_Bilinear, 
    eTextureFilterMode_Trilinear,
    eTextureFilterMode_COUNT
};

decl_enum_strings(eTextureFilterMode);

enum eTextureWrapMode
{
    eTextureWrapMode_Repeat, 
    eTextureWrapMode_ClampToEdge,
    eTextureWrapMode_COUNT
};

decl_enum_strings(eTextureWrapMode);

enum eTextureFormat
{
    eTextureFormat_Null,
    eTextureFormat_R8,
    eTextureFormat_R8_G8,
    eTextureFormat_RGB8,
    eTextureFormat_RGBA8,
    eTextureFormat_RGBA8UI,
    eTextureFormat_R8UI,
    eTextureFormat_R16UI,
    eTextureFormat_COUNT
};

decl_enum_strings(eTextureFormat);

// Get number of bytes per pixel for specific texture format
// @param format: Format identifier
inline int NumBytesPerPixel(eTextureFormat format) 
{
    debug_assert(format < eTextureFormat_COUNT && format > eTextureFormat_Null);
    switch (format)
    {
        case eTextureFormat_RGBA8 : return 4;
        case eTextureFormat_RGB8 : return 3;
        case eTextureFormat_R8_G8 : return 2;
        case eTextureFormat_R8 : return 1;
        case eTextureFormat_R16UI: return 2;
        case eTextureFormat_R8UI: return 1;
        case eTextureFormat_RGBA8UI: return 4;
    }
    return 0;
}

// Get number of bits per pixel for specific pixel format
// @param format: Format identifier
inline int NumBitsPerPixel(eTextureFormat format) 
{
    int numBytes = NumBytesPerPixel(format);
    return numBytes * 8;
}

enum ePrimitiveType
{
    ePrimitiveType_Points, 
    ePrimitiveType_Lines, 
    ePrimitiveType_LineLoop, 
    ePrimitiveType_Triangles,
    ePrimitiveType_TriangleStrip,
    ePrimitiveType_TriangleFan,
    ePrimitiveType_COUNT
};

decl_enum_strings(ePrimitiveType);

enum eIndicesType
{
    eIndicesType_i16, 
    eIndicesType_i32, 
    eIndicesType_COUNT
};

decl_enum_strings(eIndicesType);

enum eTextureUnit
{
    eTextureUnit_0,
    eTextureUnit_1,
    eTextureUnit_2,
    eTextureUnit_3,
    eTextureUnit_4,
    eTextureUnit_5,
    eTextureUnit_6,
    eTextureUnit_7,
    eTextureUnit_8,
    eTextureUnit_9,
    eTextureUnit_10,
    eTextureUnit_11,
    eTextureUnit_12,
    eTextureUnit_13,
    eTextureUnit_14,
    eTextureUnit_15,
    eTextureUnit_COUNT = 16, // not valid texture unit
};

decl_enum_strings(eTextureUnit);

// standard vertex attributes
enum eVertexAttributeFormat
{
    eVertexAttributeFormat_2F,      // 2 floats
    eVertexAttributeFormat_3F,      // 3 floats
    eVertexAttributeFormat_4UB,     // 4 unsigned bytes
    eVertexAttributeFormat_1US,     // 1 unsigned short
    eVertexAttributeFormat_2US,     // 2 unsigned shorts
    eVertexAttributeFormat_Unknown
};

decl_enum_strings(eVertexAttributeFormat);

enum eVertexAttribute
{
    eVertexAttribute_Position0,
    eVertexAttribute_Position1,
    eVertexAttribute_Texcoord0,
    eVertexAttribute_Texcoord1,
    eVertexAttribute_Normal0,
    eVertexAttribute_Normal1,
    eVertexAttribute_Color0,
    eVertexAttribute_Color1,
    eVertexAttribute_COUNT,
    eVertexAttribute_MAX = 16,
};

decl_enum_strings(eVertexAttribute);

// Get number of component for vertex attribute
// @param attributeFormat: Format identifier
inline unsigned int GetAttributeComponentCount(eVertexAttributeFormat attributeFormat)
{
    switch (attributeFormat)
    {
        case eVertexAttributeFormat_2F: return 2;
        case eVertexAttributeFormat_3F: return 3;
        case eVertexAttributeFormat_4UB: return 4;
        case eVertexAttributeFormat_1US: return 1;
        case eVertexAttributeFormat_2US: return 2;
    }
    debug_assert(false);
    return 0;
}

// Get vertex attribute size in bytes
// @param attributeFormat: Format identifier
inline unsigned int GetAttributeSizeBytes(eVertexAttributeFormat attributeFormat)
{
    switch (attributeFormat)
    {
        case eVertexAttributeFormat_2F: return 2 * sizeof(float);
        case eVertexAttributeFormat_3F: return 3 * sizeof(float);
        case eVertexAttributeFormat_4UB: return 4 * sizeof(unsigned char);
        case eVertexAttributeFormat_1US: return 1 * sizeof(unsigned short);
        case eVertexAttributeFormat_2US: return 2 * sizeof(unsigned short);
    }
    debug_assert(false);
    return 0;
}

// defines vertex attributes streams
struct VertexFormat
{
public:
    VertexFormat() = default;

    // Enable attribute or modify data offset for enabled attribute
    // @param attribute: Attribute identifier
    // @param attributeFormat: Attribute format format
    // @param dataOffset: Attribute data offset in bytes within buffer
    inline void SetAttribute(eVertexAttribute attribute, eVertexAttributeFormat attributeFormat, unsigned int dataOffset)
    {
        debug_assert(attribute < eVertexAttribute_COUNT);
        debug_assert(attributeFormat != eVertexAttributeFormat_Unknown);
        mAttributes[attribute].mDataOffset = dataOffset;
        mAttributes[attribute].mFormat = attributeFormat;
        mAttributes[attribute].mNormalized = false;
    }
    inline void SetAttributeNormalized(eVertexAttribute attribute, bool isNormalized = true)
    {
        debug_assert(attribute < eVertexAttribute_COUNT);
        mAttributes[attribute].mNormalized = isNormalized;
    }
public:
    struct SingleAttribute
    {
    public:
        SingleAttribute() = default;
    public:
        eVertexAttributeFormat mFormat = eVertexAttributeFormat_Unknown;
        unsigned int mDataOffset = 0;

        // attribute normalization - opengl specific
        // if set to true, it indicates that values stored in an integer format are 
        // to be mapped to the range [-1,1] (for signed values) or [0,1] (for unsigned values) when they are accessed and converted to floating point
        bool mNormalized = false;
    };
    SingleAttribute mAttributes[eVertexAttribute_COUNT];
    unsigned int mDataStride = 0; // common to all attributes
    unsigned int mBaseOffset = 0; // additional offset in bytes within source vertex buffer, affects on all attribues
};

// standard engine vertex definition
struct Vertex3D_Format: public VertexFormat
{
public:
    Vertex3D_Format()
    {
        Setup();
    }
    // Get format definition
    static const Vertex3D_Format& Get() 
    { 
        static const Vertex3D_Format sDefinition; 
        return sDefinition; 
    }
    using TVertexType = Vertex3D;
    // Initialzie definition
    inline void Setup()
    {
        this->mDataStride = Sizeof_Vertex3D;
        this->SetAttribute(eVertexAttribute_Position0, eVertexAttributeFormat_3F, offsetof(TVertexType, mPosition));
        this->SetAttribute(eVertexAttribute_Normal0, eVertexAttributeFormat_3F, offsetof(TVertexType, mNormal));
        this->SetAttribute(eVertexAttribute_Texcoord0, eVertexAttributeFormat_2F, offsetof(TVertexType, mTexcoord));
        this->SetAttribute(eVertexAttribute_Color0, eVertexAttributeFormat_4UB, offsetof(TVertexType, mColor));
        this->SetAttributeNormalized(eVertexAttribute_Color0);
    }
};

// ui vertex definition
struct Vertex2D_Format: public VertexFormat
{
public:
    Vertex2D_Format()
    {
        Setup();
    }
    // get format definition
    static const Vertex2D_Format& Get() 
    { 
        static const Vertex2D_Format sDefinition; 
        return sDefinition; 
    }
    using TVertexType = Vertex2D;
    // Initialzie definition
    inline void Setup()
    {
        this->mDataStride = Sizeof_Vertex2D;
        this->SetAttribute(eVertexAttribute_Texcoord0, eVertexAttributeFormat_2F, offsetof(TVertexType, mTexcoord));
        this->SetAttribute(eVertexAttribute_Color0, eVertexAttributeFormat_4UB, offsetof(TVertexType, mColor));
        this->SetAttributeNormalized(eVertexAttribute_Color0);
        // force semantics for pos0 attribute - expect 2 floats per vertex
        this->SetAttribute(eVertexAttribute_Position0, eVertexAttributeFormat_2F, offsetof(TVertexType, mPosition));
    }
};

// debug vertex definition
struct Vertex3D_Debug_Format: public VertexFormat
{
public:
    Vertex3D_Debug_Format()
    {
        Setup();
    }
    // Get definition instance
    static const Vertex3D_Debug_Format& Get() 
    { 
        static const Vertex3D_Debug_Format sDefinition; 
        return sDefinition; 
    }
    using TVertexType = Vertex3D_Debug;
    // Initialzie definition
    inline void Setup()
    {
        this->mDataStride = Sizeof_Vertex3D_Debug;
        this->SetAttribute(eVertexAttribute_Position0, eVertexAttributeFormat_3F, offsetof(TVertexType, mPosition));
        this->SetAttribute(eVertexAttribute_Color0, eVertexAttributeFormat_4UB, offsetof(TVertexType, mColor));
        this->SetAttributeNormalized(eVertexAttribute_Color0);
    }
};

enum eBufferContent
{
    eBufferContent_Vertices,
    eBufferContent_Indices,
    eBufferContent_COUNT
};

decl_enum_strings(eBufferContent);

enum eBufferUsage
{
    eBufferUsage_Static, // The data store contents will be modified once and used many times
    eBufferUsage_Dynamic, // The data store contents will be modified repeatedly and used many times
    eBufferUsage_Stream, // The data store contents will be modified once and used at most a few times
    eBufferUsage_COUNT
};

decl_enum_strings(eBufferUsage);

using BufferAccessBits = unsigned int;

const BufferAccessBits BufferAccess_Read  = (1 << 0);
const BufferAccessBits BufferAccess_Write = (1 << 1);
const BufferAccessBits BufferAccess_Unsynchronized = (1 << 2); // client must to guarantee that mapped buffer region is doesn't used by the GPU
const BufferAccessBits BufferAccess_InvalidateRange = (1 << 3); // have meaning only for range lock
const BufferAccessBits BufferAccess_InvalidateBuffer = (1 << 4); // orphan whole buffer
const BufferAccessBits BufferAccess_UnsynchronizedWrite = (BufferAccess_Unsynchronized | BufferAccess_Write);

enum eRenderUniform
{
    eRenderUniform_ModelMatrix,
    eRenderUniform_ViewMatrix,
    eRenderUniform_ProjectionMatrix,
    eRenderUniform_ModelViewMatrix,
    eRenderUniform_ModelViewProjectionMatrix,
    eRenderUniform_ViewProjectionMatrix,
    eRenderUniform_NormalMatrix,         
    eRenderUniform_CameraPosition, // world space camera position
    eRenderUniform_COUNT
};

decl_enum_strings(eRenderUniform);

enum eBlendMode
{
    eBlendMode_Alpha,
    eBlendMode_Additive,
    eBlendMode_Multiply,
    eBlendMode_Premultiplied,
    eBlendMode_Screen
};

decl_enum_strings(eBlendMode);

enum eDepthTestFunc : unsigned short
{
    eDepthTestFunc_Always,
    eDepthTestFunc_Equal,
    eDepthTestFunc_NotEqual,
    eDepthTestFunc_Less,
    eDepthTestFunc_Greater,
    eDepthTestFunc_LessEqual,
    eDepthTestFunc_GreaterEqual
};

decl_enum_strings(eDepthTestFunc);

enum eCullMode : unsigned short
{
    eCullMode_Front,
    eCullMode_Back,
    eCullMode_FrontAndBack,
};

decl_enum_strings(eCullMode);

enum eFillMode : unsigned short
{
    eFillMode_Solid,
    eFillMode_WireFrame,
};

decl_enum_strings(eFillMode);

using RenderStateFlags = unsigned short;

const RenderStateFlags RenderStateFlags_AlphaBlend = (1 << 0);
const RenderStateFlags RenderStateFlags_ColorWrite = (1 << 1);
const RenderStateFlags RenderStateFlags_DepthWrite = (1 << 2);
const RenderStateFlags RenderStateFlags_DepthTest = (1 << 3);
const RenderStateFlags RenderStateFlags_FaceCulling = (1 << 4);
const RenderStateFlags RenderStateFlags_Defaults = RenderStateFlags_ColorWrite | 
    RenderStateFlags_DepthWrite | 
    RenderStateFlags_DepthTest | 
    RenderStateFlags_FaceCulling;

// defines render states
struct RenderStates
{
public:
    RenderStates() = default;
    // enable render state flags
    // @param renderStateFlags: Flags to enable, bitfield
    RenderStates& Enable(RenderStateFlags renderStateFlags)
    {
        mStateFlags |= renderStateFlags;
        return *this;
    }
    // disable render state flags
    // @param renderStateFlags: Flags to disable, bitfield
    RenderStates& Disable(RenderStateFlags renderStateFlags)
    {
        mStateFlags &= ~renderStateFlags;
        return *this;
    }
    // enable alpha blending feature and set specific mode
    // @param alphaBlendMode: Mode
    RenderStates& SetAlphaBlend(eBlendMode alphaBlendMode)
    {
        Enable(RenderStateFlags_AlphaBlend);
        mBlendMode = alphaBlendMode;
        return *this;
    }
    // enable depth testing feature and set specific func
    // @param depthTestFunc: Func
    RenderStates& SetDepthTest(eDepthTestFunc depthTestFunc)
    {
        Enable(RenderStateFlags_DepthTest);
        mDepthFunc = depthTestFunc;
        return *this;
    }
    // enable face culling feature and set specific mode
    // @param faceCullMode: Mode
    RenderStates& SetFaceCulling(eCullMode faceCullMode)
    {
        Enable(RenderStateFlags_FaceCulling);
        mCullMode = faceCullMode;
        return *this;
    }
    // test whether render state flags is enabled
    inline bool IsEnabled(RenderStateFlags renderStateFlags) const
    {
        return (mStateFlags & renderStateFlags) == renderStateFlags;
    }
    // test whether specific state flags are same
    // @param otherStates: States reference
    // @param renderStateFlags: Flags to test
    inline bool MatchFlags(const RenderStates& otherStates, RenderStateFlags renderStateFlags) const
    {
        return (otherStates.mStateFlags & renderStateFlags) == (mStateFlags & renderStateFlags);
    }
public:
    RenderStateFlags mStateFlags = RenderStateFlags_Defaults;
    eBlendMode mBlendMode = eBlendMode_Alpha;
    eFillMode mFillMode = eFillMode_Solid;
    eCullMode mCullMode = eCullMode_Back;
    eDepthTestFunc mDepthFunc = eDepthTestFunc_LessEqual;
};

const unsigned int Sizeof_RenderStates = sizeof(RenderStates);

inline bool operator == (const RenderStates& a, const RenderStates& b) { return ::memcmp(&a, &b, Sizeof_RenderStates) == 0; }
inline bool operator != (const RenderStates& a, const RenderStates& b) { return ::memcmp(&a, &b, Sizeof_RenderStates) != 0; }

struct RenderProgramInputLayout
{
public:
    RenderProgramInputLayout() = default;
    // Enable vertex attribute
    // @param attributeStream: Attribute stream
    inline void IncludeAttribute(eVertexAttribute attributeStream) 
    {
        if (attributeStream < eVertexAttribute_COUNT) mEnabledAttributes |= (1 << attributeStream);
    }
    // Disable vertex attribute
    // @param attributeStream: Attribute stream
    inline void ExcludeAttribute(eVertexAttribute attributeStream) 
    {
        if (attributeStream < eVertexAttribute_COUNT) mEnabledAttributes &= ~(1 << attributeStream);
    }
    // Test whether vertex attribute enabled
    // @param attributeStream: Attribute stream
    inline bool HasAttribute(eVertexAttribute attributeStream) const 
    {
        return (attributeStream < eVertexAttribute_COUNT) && (mEnabledAttributes & (1 << attributeStream)) > 0;
    }
    unsigned int mEnabledAttributes = 0;
};

inline bool operator == (const RenderProgramInputLayout& a, const RenderProgramInputLayout& b) { return a.mEnabledAttributes == b.mEnabledAttributes; }
inline bool operator != (const RenderProgramInputLayout& a, const RenderProgramInputLayout& b) { return a.mEnabledAttributes != b.mEnabledAttributes; }

enum eGraphicsFeature
{
    eGraphicsFeature_NPOT_Textures,
    eGraphicsFeature_ABGR,
    eGraphicsFeature_COUNT
};

struct GraphicsDeviceCaps
{
public:
    GraphicsDeviceCaps() = default;

public:
    int mMaxArrayTextureLayers;
    int mMaxTextureBufferSize;
    bool mFeatures[eGraphicsFeature_COUNT];
};