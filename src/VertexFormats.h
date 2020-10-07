#pragma once

// defines draw vertex of city mesh
struct CityVertex3D
{
public:
    CityVertex3D() = default;

    // setup vertex
    // @param posx, posy, posz: Coordinate in 3d space
    // @param tcu, tcv: Texture coordinate normalized [0, 1]
    // @param tcz: Texture layer in texture array
    void Set(float posx, float posy, float posz, float tcu, float tcv, float tcz,
        unsigned short remap, unsigned short transparency)
    {
        mPosition.x = posx;
        mPosition.y = posy;
        mPosition.z = posz;
        mTexcoord.x = tcu;
        mTexcoord.y = tcv;
        mTexcoord.z = tcz;
        mRemap = remap;
        mTransparency = transparency;
    }
    inline void SetColorData(unsigned short remap, unsigned short transparency)
    {
        mRemap = remap;
        mTransparency = transparency;
    }
public:
    glm::vec3 mPosition; // 12 bytes
    glm::vec3 mTexcoord; // 12 bytes
    unsigned short mRemap; // 2 bytes
    unsigned short mTransparency; // 2 bytes
};

const unsigned int Sizeof_CityVertex3D = sizeof(CityVertex3D);

// defines draw vertex format of city mesh
struct CityVertex3D_Format: public VertexFormat
{
public:
    CityVertex3D_Format()
    {
        Setup();
    }
    // get format definition
    static const CityVertex3D_Format& Get() 
    { 
        static const CityVertex3D_Format sDefinition; 
        return sDefinition; 
    }
    using TVertexType = CityVertex3D;
    // initialzie definition
    inline void Setup()
    {
        this->mDataStride = Sizeof_CityVertex3D;
        this->SetAttribute(eVertexAttribute_Position0, eVertexAttributeFormat_3F, offsetof(TVertexType, mPosition));
        this->SetAttribute(eVertexAttribute_Color0, eVertexAttributeFormat_1US, offsetof(TVertexType, mRemap));
        this->SetAttribute(eVertexAttribute_Color1, eVertexAttributeFormat_1US, offsetof(TVertexType, mTransparency));
        this->SetAttribute(eVertexAttribute_Texcoord0, eVertexAttributeFormat_3F, offsetof(TVertexType, mTexcoord));
    }
};

// defines draw vertex of sprite
struct SpriteVertex3D
{
public:
    SpriteVertex3D() = default;

    // setup vertex
    // @param posx, posy, posz: Coordinate in 3d space
    // @param tcu, tcv: Texture coordinate normalized [0, 1]
    void Set(float posx, float posy, float posz, float tcu, float tcv, unsigned short clutIndex)
    {
        mPosition.x = posx;
        mPosition.y = posy;
        mPosition.z = posz;
        mTexcoord.x = tcu;
        mTexcoord.y = tcv;
        mClutIndex = clutIndex;
    }
public:
    glm::vec3 mPosition; // 12 bytes
    glm::vec2 mTexcoord; // 8 bytes
    unsigned short mTextureSize[2]; // sprite texture size in pixels
    unsigned short mClutIndex; // 2 bytes
};

const unsigned int Sizeof_SpriteVertex3D = sizeof(SpriteVertex3D);

// defines draw vertex format of sprite
struct SpriteVertex3D_Format: public VertexFormat
{
public:
    SpriteVertex3D_Format()
    {
        Setup();
    }
    // get format definition
    static const SpriteVertex3D_Format& Get() 
    { 
        static const SpriteVertex3D_Format sDefinition; 
        return sDefinition; 
    }
    using TVertexType = SpriteVertex3D;
    // initialzie definition
    inline void Setup()
    {
        this->mDataStride = Sizeof_SpriteVertex3D;
        this->SetAttribute(eVertexAttribute_Position0, eVertexAttributeFormat_3F, offsetof(TVertexType, mPosition));
        this->SetAttribute(eVertexAttribute_Texcoord0, eVertexAttributeFormat_2F, offsetof(TVertexType, mTexcoord));
        this->SetAttribute(eVertexAttribute_Color0, eVertexAttributeFormat_1US, offsetof(TVertexType, mClutIndex));
        this->SetAttribute(eVertexAttribute_TextureSize, eVertexAttributeFormat_2US, offsetof(TVertexType, mTextureSize));
    }
};