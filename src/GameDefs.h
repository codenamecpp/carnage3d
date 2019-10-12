#pragma once

// most of game constants is mapped to GTA files format so don't change

// side length of block cube, do not change
#define MAP_BLOCK_LENGTH 1.0f

#define MAP_BLOCK_TEXTURE_DIMS 64
#define MAP_BLOCK_TEXTURE_AREA (MAP_BLOCK_TEXTURE_DIMS * MAP_BLOCK_TEXTURE_DIMS)

#define MAX_MAP_BLOCK_ANIM_FRAMES 32
#define MAX_CAR_DOORS 4
#define MAX_CAR_REMAPS 12
#define MAX_SPRITE_DELTAS 32

// map width and height is same
#define MAP_DIMENSIONS 256
#define MAP_LAYERS_COUNT 6

#define MAP_PIXELS_PER_TILE MAP_BLOCK_TEXTURE_DIMS

#define SPRITE_ZERO_ANGLE 90.0f // all sprites in game are rotated at 90 degrees
#define SPRITE_SCALE (1.0f / MAP_PIXELS_PER_TILE)

#define PED_SPRITE_DRAW_BOX_SIZE_PX 24 // with, height
#define PED_SPRITE_DRAW_BOX_SIZE ((1.0f * PED_SPRITE_DRAW_BOX_SIZE_PX) / MAP_BLOCK_TEXTURE_DIMS)

// car sprite deltas - damage
#define CAR_DAMAGE_SPRITE_DELTA_FR 0 // front right
#define CAR_DAMAGE_SPRITE_DELTA_BL 1 // back left
#define CAR_DAMAGE_SPRITE_DELTA_ML 2 // middle left
#define CAR_DAMAGE_SPRITE_DELTA_FL 3 // front left
#define CAR_DAMAGE_SPRITE_DELTA_BR 4 // back right
#define CAR_DAMAGE_SPRITE_DELTA_MR 5 // middle right
#define CAR_DAMAGE_SPRITE_DELTA_WS 10 // windshield broken

// car sprite deltas - door 1
#define CAR_DOOR1_SPRITE_DELTA_0 6 // first frame
#define CAR_DOOR1_SPRITE_DELTA_1 7
#define CAR_DOOR1_SPRITE_DELTA_2 8
#define CAR_DOOR1_SPRITE_DELTA_3 9 // last frame

// car sprite deltas - door 2
#define CAR_DOOR2_SPRITE_DELTA_0 11 // first frame
#define CAR_DOOR2_SPRITE_DELTA_1 12 
#define CAR_DOOR2_SPRITE_DELTA_2 13
#define CAR_DOOR2_SPRITE_DELTA_3 14 // last frame

// car sprite deltas - lighting (police car and ambulance have those)
#define CAR_LIGHTING_SPRITE_DELTA_0 15
#define CAR_LIGHTING_SPRITE_DELTA_1 16

#define CAR_DELTA_ANIMS_SPEED 10.0f

// forwards
class Pedestrian;
class Vehicle;

// map block lid rotation
enum eLidRotation : unsigned char
{
    eLidRotation_0,
    eLidRotation_90,
    eLidRotation_180,
    eLidRotation_270
};

decl_enum_strings(eLidRotation);

// map block type
enum eBlockType : unsigned short
{
    eBlockType_Side,
    eBlockType_Lid,
    eBlockType_Aux,
    eBlockType_COUNT
};

decl_enum_strings(eBlockType);

enum eSpriteType : unsigned short
{
    eSpriteType_Arrow,
    eSpriteType_Digit,
    eSpriteType_Boat,
    eSpriteType_Box,
    eSpriteType_Bus,
    eSpriteType_Car,
    eSpriteType_Object,
    eSpriteType_Ped,
    eSpriteType_Speedo,
    eSpriteType_Tank,
    eSpriteType_TrafficLight,
    eSpriteType_Train,
    eSpriteType_TrDoor,
    eSpriteType_Bike,
    eSpriteType_Tram,
    eSpriteType_WCar,
    eSpriteType_WBus,
    eSpriteType_Ex,
    eSpriteType_TumCar,
    eSpriteType_TumTruck,
    eSpriteType_Ferry,
    eSpriteType_COUNT
};

decl_enum_strings(eSpriteType);

// tile ground type
enum eGroundType : unsigned char
{
    eGroundType_Air,
    eGroundType_Water,
    eGroundType_Road,
    eGroundType_Pawement,
    eGroundType_Field,
    eGroundType_Building,
    eGroundType_COUNT
};

decl_enum_strings(eGroundType);

// map block face direction
enum eBlockFace : unsigned short
{
    eBlockFace_W, // left
    eBlockFace_E, // right
    eBlockFace_N, // top
    eBlockFace_S, // botom
    eBlockFace_Lid,
    eBlockFace_COUNT
};

decl_enum_strings(eBlockFace);

using SpriteDeltaBits_t = unsigned int;
static_assert(sizeof(SpriteDeltaBits_t) * 8 >= MAX_SPRITE_DELTAS, "Delta bits underlying type is too small, see MAX_SPRITE_DELTAS");

using GameObjectID_t = unsigned int;

// defines draw vertex of city mesh
struct CityVertex3D
{
public:
    CityVertex3D() = default;

    // setup vertex
    // @param posx, posy, posz: Coordinate in 3d space
    // @param tcu, tcv: Texture coordinate normalized [0, 1]
    // @param tcz: Texture layer in texture array
    // @param normx, normy, normz: Normal unit vector
    // @param color: Color RGBA
    void Set(float posx, float posy, float posz, float tcu, float tcv, float tcz, float normx, float normy, float normz, unsigned int color)
    {
        mPosition.x = posx;
        mPosition.y = posy;
        mPosition.z = posz;
        mNormal.x = normx;
        mNormal.y = normy;
        mNormal.z = normz;
        mTexcoord.x = tcu;
        mTexcoord.y = tcv;
        mTexcoord.z = tcz;
        mColor = color;
    }
    // setup vertex
    void Set(float posx, float posy, float posz, float tcu, float tcv, float tcz, unsigned int color)
    {
        mPosition.x = posx;
        mPosition.y = posy;
        mPosition.z = posz;
        mTexcoord.x = tcu;
        mTexcoord.y = tcv;
        mTexcoord.z = tcz;
        mColor = color;
    }
    // setup vertex
    void Set(float posx, float posy, float posz, float tcu, float tcv, float tcz)
    {
        mPosition.x = posx;
        mPosition.y = posy;
        mPosition.z = posz;
        mTexcoord.x = tcu;
        mTexcoord.y = tcv;
        mTexcoord.z = tcz;
        mColor = COLOR_WHITE;
    }
public:
    glm::vec3 mPosition; // 12 bytes
    glm::vec3 mNormal; // 12 bytes
    glm::vec3 mTexcoord; // 12 bytes
    unsigned int mColor; // 4 bytes
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
        this->SetAttribute(eVertexAttribute_Position0, offsetof(TVertexType, mPosition));
        this->SetAttribute(eVertexAttribute_Normal0, offsetof(TVertexType, mNormal));
        this->SetAttribute(eVertexAttribute_Color0, offsetof(TVertexType, mColor));
        // force semantics for texcoord0 attribute - expect 3 floats per vertex
        this->SetAttribute(eVertexAttribute_Texcoord0, eVertexAttributeSemantics_Texcoord3d, offsetof(TVertexType, mTexcoord));
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
    // @param color: Color RGBA
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
        mColor = COLOR_WHITE;
    }
public:
    glm::vec3 mPosition; // 12 bytes
    glm::vec2 mTexcoord; // 8 bytes
    unsigned int mColor; // 4 bytes
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
        this->SetAttribute(eVertexAttribute_Position0, offsetof(TVertexType, mPosition));
        this->SetAttribute(eVertexAttribute_Texcoord0, offsetof(TVertexType, mTexcoord));
        this->SetAttribute(eVertexAttribute_Color0, offsetof(TVertexType, mColor));
    }
};

// defines map mesh data
struct MapMeshData
{
public:
    using TVertexType = CityVertex3D;
    MapMeshData() = default;

    inline void SetNull()
    {
        mBlocksVertices.clear();
        mBlocksIndices.clear();
    }

public:
    std::vector<TVertexType> mBlocksVertices;
    std::vector<DrawIndex_t> mBlocksIndices;
};

// defines picture rectanle within sprite atlas
struct TextureRegion
{
public:
    TextureRegion() = default;
    TextureRegion(const Size2D& textureSize)
    {
        SetRegion(textureSize);
    }
    TextureRegion(const Rect2D& srcRectangle, const Size2D& textureSize)
    {
        SetRegion(srcRectangle, textureSize);
    }

    // init texture region to partial area
    inline void SetRegion(const Rect2D& srcRectangle, const Size2D& textureSize)
    {
        debug_assert(textureSize.x > 0);
        debug_assert(textureSize.y > 0);
        float tcx = (1.0f / textureSize.x);
        float tcy = (1.0f / textureSize.y);
        // compute texture region coordinates
        mRectangle = srcRectangle;
        mU0 = mRectangle.x * tcx;
        mV0 = mRectangle.y * tcy;
        mU1 = (mRectangle.x + mRectangle.w) * tcx;
        mV1 = (mRectangle.y + mRectangle.h) * tcy;
    }
    // init texture region to whole texture area
    inline void SetRegion(const Size2D& textureSize)
    {
        mRectangle.x = 0;
        mRectangle.y = 0;
        mRectangle.w = textureSize.x;
        mRectangle.h = textureSize.y;
        mU0 = 0.0f;
        mV0 = 0.0f;
        mU1 = 1.0f;
        mV1 = 1.0f;
    }
    // clear texture region
    inline void SetNull()
    {
        mRectangle.SetNull();
    }
public:
    Rect2D mRectangle;

    float mU0, mV0; // texture coords
    float mU1, mV1; // texture coords
};

// defines simple 2d sprite
struct Sprite
{
public:
    Sprite() = default;

    // set origin to center of sprite, texture region and scale must be specified
    inline void SetOriginToCenter()
    {
        mOrigin.x = (-mTextureRegion.mRectangle.w * mScale) * 0.5f;
        mOrigin.y = (-mTextureRegion.mRectangle.h * mScale) * 0.5f;
    }
    // clear sprite data
    inline void SetNull()
    {
        mTexture = nullptr;
        mTextureRegion.SetNull();

        mOrigin.x = 0.0f;
        mOrigin.y = 0.0f;
        mPosition.x = 0.0f;
        mPosition.y = 0.0f;

        mHeight = 0.0f;
        mScale = 1.0f;
        mRotateAngle = cxx::angle_t::from_degrees(0.0f);
    }
public:
    GpuTexture2D* mTexture = nullptr;
    TextureRegion mTextureRegion; 

    // origin is relative to sprite position and must be set each time texture region or scale is changes
    glm::vec2 mOrigin;
    glm::vec2 mPosition;

    float mHeight = 0.0f; // z order
    float mScale = 1.0f;

    cxx::angle_t mRotateAngle;
};

// defines sprite atlas texture with entries
class Spritesheet final
{
public:
    Spritesheet() = default;
    // clear spritesheet
    inline void SetNull()
    {
        mSpritesheetTexture = nullptr;
        mEntries.clear();
    }
public:
    GpuTexture2D* mSpritesheetTexture = nullptr;
    std::vector<TextureRegion> mEntries;
};

// define sprite HLS remap information
struct HLSRemap
{
public:
    HLSRemap() = default;
    HLSRemap(short h, short l, short s)
        : mH(h)
        , mL(l)
        , mS(s)
    {
    }
public:
    short mH, mL, mS;
};

// define map block information
struct BlockStyle
{
public:
    unsigned char mRemap;

    eGroundType mGroundType;

    eLidRotation mLidRotation;

    unsigned char mTrafficLight;
    unsigned char mFaces[eBlockFace_COUNT]; // stores a value which indicates the correct graphic square to use for that face
        // A value of zero indicates no face - hidden faces must be set to zero

    unsigned char mSlopeType; // 0 = none
        // 1 - 2 = up 26 low, high
        // 3 - 4 = down 26 low, high
        // 5 - 6 = left 26 low, high
        // 7 - 8 = right 26 low, high
        // 9 - 16 = up 7 low - high
        // 17 - 24 = down 7 low - high
        // 25 - 32 = left 7 low - high
        // 33 - 40 = right 7 low - high
        // 41 - 44 = 45 up,down,left,right 

    bool mUpDirection : 1;
    bool mDownDirection : 1;
    bool mLeftDirection : 1;
    bool mRightDirection : 1;

    bool mIsFlat : 1;
    bool mFlipTopBottomFaces : 1;
    bool mFlipLeftRightFaces : 1;
    bool mIsRailway : 1;
};

const unsigned int Sizeof_BlockStyle = sizeof(BlockStyle);

// define map block anim information
struct BlockAnimationStyle
{
public:
    int mBlock = 0; // the block number
    int mWhich = 0; // the area type ( 0 for side, 1 for lid )
    int mSpeed = 0; // the number of game cycles to display each frame
    int mFrameCount = 0; // the number of auxilliary frames
    int mFrames[MAX_MAP_BLOCK_ANIM_FRAMES]; // an array of block numbers, these refer to aux_block
};

// define map object information
struct MapObjectStyle
{
public:
    int mWidth;
    int mHeight; 
    int mDepth;
    int mSpriteIndex;
    int mWeight;
    int mAux;
    int mStatus;
};

// define car door information
struct CarDoorStyle
{
public:
	short mRpx, mRpy;
	short mObject;
	short mDelta;
};

enum eCarVType
{
    eCarVType_Bus,
    eCarVType_FrontOfJuggernaut,
    eCarVType_BackOfJuggernaut,
    eCarVType_Motorcycle,
    eCarVType_StandardCar,
    eCarVType_Train,
    eCarVType_Tram,
    eCarVType_Boat,
    eCarVType_Tank,
    eCarVType_COUNT
};

decl_enum_strings(eCarVType);

// warning - these values are mapped to gta1, so don't change it
enum eCarModel
{
    eCarModel_BeastGTS_1    = 0,
    eCarModel_Bug           = 1,
    eCarModel_Counthash     = 2,
    eCarModel_Bike          = 3,
    eCarModel_Police        = 4,
    eCarModel_Ambulance     = 5,
    eCarModel_RepairVan     = 6,
    eCarModel_Juggernaut    = 7,
    eCarModel_Coach         = 9,
    eCarModel_Train         = 11,
    eCarModel_Tram          = 12,
    eCarModel_Boat          = 13,
    eCarModel_Penetrator_1  = 14,
    eCarModel_Itali         = 17,
    eCarModel_Mundano_1     = 18,
    eCarModel_4x4           = 19,
    eCarModel_Stallion      = 21,
    eCarModel_Taxi          = 22,
    eCarModel_Impaler_1     = 25,
    eCarModel_Jugular       = 26,
    eCarModel_Pickup_1      = 27,
    eCarModel_PorkaTurbo    = 28,
    eCarModel_Cossie        = 29,
    eCarModel_Bulldog       = 31,
    eCarModel_Challenger    = 34,
    eCarModel_Limousine_1   = 35,
    eCarModel_Tank          = 37,
    eCarModel_Superbike     = 41,
    eCarModel_FireTruck     = 42,
    eCarModel_Bus           = 43,
    eCarModel_Tanker        = 44,
    eCarModel_TVVan         = 45,
    eCarModel_TransitVan    = 46,
    eCarModel_ModelCar      = 47,
    eCarModel_Roadster      = 50,
    eCarModel_LoveWagon     = 51,
    eCarModel_BeastGTS_2    = 53,
    eCarModel_Mundano_2     = 54,
    eCarModel_Mamba         = 55,
    eCarModel_Portsmouth    = 58,
    eCarModel_Speeder       = 61,
    eCarModel_Porka         = 62,
    eCarModel_Flamer        = 63,
    eCarModel_Vulture       = 64,
    eCarModel_Pickup_2      = 65,
    eCarModel_ItaliGTO      = 66,
    eCarModel_Regal         = 70,
    eCarModel_MonsterBug    = 71,
    eCarModel_Thunderhead   = 72,
    eCarModel_Panther       = 73,
    eCarModel_Penetrator_2  = 74,
    eCarModel_LeBonham      = 75,
    eCarModel_Stinger       = 76,
    eCarModel_F19           = 77,
    eCarModel_Brigham       = 78,
    eCarModel_StingerZ29    = 79,
    eCarModel_Classic       = 80,
    eCarModel_29Special     = 81,
    eCarModel_ItaliGTB      = 82,
    eCarModel_Hotrod        = 83,
    eCarModel_Limousine_2   = 86,
    eCarModel_Impaler_2     = 87,
    eCarModel_Helicopter    = 88,
};

decl_enum_strings(eCarModel);

// define car class information
struct CarStyle
{
public:
    short mWidth, mHeight, mDepth;  // dimensions of the car with respect to collision checking, x, y, z
    short mSprNum; // first sprite number offset for this car
    short mWeight;	
    short mMaxSpeed, mMinSpeed;
    short mAcceleration, mBraking;
    short mGrip, mHandling;
    HLSRemap mRemap[MAX_CAR_REMAPS];
    eCarVType mVType; // is a descriptor of the type of car / vehicle
    eCarModel mModelId;
    int mTurning;
    int mDamagable;
    int mValue[4];
    char mCx, mCy; // pixel co-ordinates of the centre of mass of the car, relative to the graphical centre
    int mMoment;
    float mRbpMass;
    float mG1Thrust;
    float mTyreAdhesionX, mTyreAdhesionY;
    float mHandbrakeFriction;
    float mFootbrakeFriction;
    float mFrontBrakeBias;
    short mTurnRatio;
    short mDriveWheelOffset;
    short mSteeringWheelOffset;
    float mBackEndSlideValue;
    float mHandbrakeSlideValue;
    bool mConvertible;
    int mEngine;
    int mRadio;
    int mHorn;
    int mSoundFunction;
    int mFastChangeFlag;
    short mDoorsCount;
    CarDoorStyle mDoors[MAX_CAR_DOORS];
};

// define sprite information
struct SpriteStyle
{
public:
    int mWidth;
    int mHeight;
    int mSize; // bytes per frame w x h
    int mClut;
    int mPageOffsetX, mPageOffsetY;
    int mPageNumber;
    int mDeltaCount;

    // delta information for sprite
    struct DeltaInfo
    {
        int mSize; // bytes for this delta
        int mOffset;
    };
    DeltaInfo mDeltas[MAX_SPRITE_DELTAS];

    // get sprite delta bits
    inline SpriteDeltaBits_t GetDeltaBits() const
    {
        SpriteDeltaBits_t deltaBits = 0;
        for (int idelta = 0; idelta < mDeltaCount; ++idelta)
        {
            deltaBits |= BIT(idelta);
        }
        return deltaBits;
    }
};

// various sprites animations
enum eSpriteAnimationID
{
    eSpriteAnimationID_Null, // dummy animation
    // pedestrians
    eSpriteAnimationID_Ped_Walk,
    eSpriteAnimationID_Ped_Run,
    eSpriteAnimationID_Ped_ExitCar,
    eSpriteAnimationID_Ped_EnterCar,
    eSpriteAnimationID_Ped_FallLong,
    eSpriteAnimationID_Ped_SlideUnderTheCar,
    eSpriteAnimationID_Ped_StandingStill, // no weapons
    eSpriteAnimationID_Ped_JumpOntoCar,
    eSpriteAnimationID_Ped_SlideOnCar,
    eSpriteAnimationID_Ped_DropOffCarSliding,
    eSpriteAnimationID_Ped_FallShort,
    eSpriteAnimationID_Ped_LiesOnFloor,
    eSpriteAnimationID_Ped_PunchingWhileStanding,
    eSpriteAnimationID_Ped_PunchingWhileRunning,
    eSpriteAnimationID_Ped_ShootPistolWhileStanding,
    eSpriteAnimationID_Ped_ShootPistolWhileWalking,
    eSpriteAnimationID_Ped_ShootPistolWhileRunning,
    eSpriteAnimationID_Ped_ShootMachinegunWhileStanding,
    eSpriteAnimationID_Ped_ShootMachinegunWhileWalking,
    eSpriteAnimationID_Ped_ShootMachinegunWhileRunning,
    eSpriteAnimationID_Ped_ShootFlamethrowerWhileStanding,
    eSpriteAnimationID_Ped_ShootFlamethrowerWhileWalking,
    eSpriteAnimationID_Ped_ShootFlamethrowerWhileRunning,
    eSpriteAnimationID_Ped_ShootRPGWhileStanding,
    eSpriteAnimationID_Ped_ShootRPGWhileWalking,
    eSpriteAnimationID_Ped_ShootRPGWhileRunning,
    // cops

    eSpriteAnimation_COUNT
};

decl_enum_strings(eSpriteAnimationID);

// pedestrian weapon
enum eWeaponType
{
    eWeaponType_Fists,
    eWeaponType_Pistol,
    eWeaponType_Machinegun,
    eWeaponType_Flamethrower,
    eWeaponType_RocketLauncher,
    eWeaponType_COUNT
};

decl_enum_strings(eWeaponType);

// pedestrian basic action
enum ePedestrianAction
{
    ePedestrianAction_TurnLeft,
    ePedestrianAction_TurnRight,
    ePedestrianAction_Jump,
    ePedestrianAction_WalkForward,
    ePedestrianAction_WalkBackward,
    ePedestrianAction_Run, // overrides walk_forward and walk_backward
    ePedestrianAction_Shoot,
    ePedestrianAction_EnterCar,
    ePedestrianAction_LeaveCar,
    ePedestrianAction_COUNT
};

decl_enum_strings(ePedestrianAction);

// pedestrian basic state
enum ePedestrianState
{
    ePedestrianState_StandingStill,
    ePedestrianState_StandsAndShoots,
    ePedestrianState_Walks,
    ePedestrianState_Runs,
    ePedestrianState_WalksAndShoots,
    ePedestrianState_RunsAndShoots,
    ePedestrianState_Falling,
    ePedestrianState_EnteringCar,
    ePedestrianState_ExitingCar,
    ePedestrianState_DrivingCar,
    ePedestrianState_SlideOnCar,
    ePedestrianState_Dying,
    ePedestrianState_Dead,
    ePedestrianState_KnockedDown,
    ePedestrianState_COUNT
};

decl_enum_strings(ePedestrianState);