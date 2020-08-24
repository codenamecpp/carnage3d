#pragma once

#include "GameObjectDefs.h"

// most of game constants is mapped to GTA files format so don't change

// side length of block cube, do not change
#define MAP_BLOCK_TEXTURE_DIMS 64
#define MAP_BLOCK_TEXTURE_AREA (MAP_BLOCK_TEXTURE_DIMS * MAP_BLOCK_TEXTURE_DIMS)

#define MAX_MAP_BLOCK_ANIM_FRAMES 32
#define MAX_CAR_DOORS 4
#define MAX_CAR_REMAPS 12
#define MAX_PED_REMAPS 64
#define NO_REMAP -1
#define MAX_SPRITE_DELTAS 32
// map width and height is same
#define MAP_DIMENSIONS 256
#define MAP_LAYERS_COUNT 6

#define PIXELS_PER_MAP_UNIT (MAP_BLOCK_TEXTURE_DIMS)
#define METERS_PER_MAP_UNIT (4.0f)

#define SPRITE_ZERO_ANGLE 90.0f // all sprites in game are rotated at 90 degrees
#define MAP_SPRITE_SCALE (METERS_PER_MAP_UNIT / PIXELS_PER_MAP_UNIT)
#define PED_SPRITE_DRAW_BOX_SIZE_PX 24 // with, height
#define CAR_WHEEL_SIZE_W_PX         6
#define CAR_WHEEL_SIZE_H_PX         12

#define GTA_CYCLES_PER_FRAME        24

// in original gta1 map height levels is counting from top to bottom - 
// 0 is highest and 5 is lowest level
#define INVERT_MAP_LAYER(index) (MAP_LAYERS_COUNT - (index) - 1)

#define GAME_MAX_PLAYERS 4

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

// define generic gameobject information
struct GameObjectStyle
{
    eGameObjectClass mClassID = eGameObjectClass_COUNT;
    eGameObjectFlags mFlags = eGameObjectFlags_None;

    int mGameObjectIndex = 0; // index within loaded styles

    // width, height and depth store the dimensions of the object with respect to collision checking
    // specified in meters
    float mWidth = 0.0f;
    float mHeight = 0.0f;
    float mDepth = 0.0f;

    int mLifeDuration = 0; // 0 for infinite, non-zero n for n animation cycles

    SpriteAnimData mAnimationData; // optional
};

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
    eSpriteType_WreckedCar,
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

using SpriteDeltaBits = unsigned int;
static_assert(sizeof(SpriteDeltaBits) * 8 >= MAX_SPRITE_DELTAS, "Delta bits underlying type is too small, see MAX_SPRITE_DELTAS");

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
    }
};

// defines map mesh data
struct MapMeshData
{
public:
    MapMeshData() = default;
    inline void Clear()
    {
        mBlocksVertices.clear();
        mBlocksIndices.clear();
    }
public:
    using TVertexType = CityVertex3D;
    std::vector<TVertexType> mBlocksVertices;
    std::vector<DrawIndex> mBlocksIndices;
};

// defines picture rectanle within sprite atlas
struct TextureRegion
{
public:
    TextureRegion() = default;
    TextureRegion(const Point& textureSize)
    {
        SetRegion(textureSize);
    }
    TextureRegion(const Rect& srcRectangle, const Point& textureSize)
    {
        SetRegion(srcRectangle, textureSize);
    }

    // init texture region to partial area
    inline void SetRegion(const Rect& srcRectangle, const Point& textureSize)
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
    inline void SetRegion(const Point& textureSize)
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
    inline void Clear()
    {
        mRectangle.SetNull();
    }
public:
    Rect mRectangle;

    float mU0, mV0; // texture coords
    float mU1, mV1; // texture coords
};

// defines sprite atlas texture with entries
class Spritesheet final
{
public:
    Spritesheet() = default;
    // clear spritesheet
    inline void Clear()
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
    int mBlock = 0; // the block number
    int mWhich = 0; // the area type ( 0 for side, 1 for lid )
    int mSpeed = 0; // the number of game cycles to display each frame
    int mFrameCount = 0; // the number of auxilliary frames
    int mFrames[MAX_MAP_BLOCK_ANIM_FRAMES]; // an array of block numbers, these refer to aux_block
};

// define car door information
struct CarDoorStyle
{
	short mRpx, mRpy;
	short mObject;
	short mDelta;
};

enum eVehicleClass
{
    eVehicleClass_Bus,
    eVehicleClass_FrontOfJuggernaut,
    eVehicleClass_BackOfJuggernaut,
    eVehicleClass_Motorcycle,
    eVehicleClass_StandardCar,
    eVehicleClass_Train,
    eVehicleClass_Tram,
    eVehicleClass_Boat,
    eVehicleClass_Tank,
    eVehicleClass_COUNT
};

decl_enum_strings(eVehicleClass);

// warning - these values are mapped to gta1, so don't change it
enum eVehicleModel
{
    eVehicle_BeastGTS_1     = 0,
    eVehicle_Bug            = 1,
    eVehicle_Counthash      = 2,
    eVehicle_Bike           = 3,
    eVehicle_Police         = 4,
    eVehicle_Ambulance      = 5,
    eVehicle_RepairVan      = 6,
    eVehicle_Juggernaut     = 7,
    eVehicle_Coach          = 9,
    eVehicle_Train          = 11,
    eVehicle_Tram           = 12,
    eVehicle_Boat           = 13,
    eVehicle_Penetrator_1   = 14,
    eVehicle_Itali          = 17,
    eVehicle_Mundano_1      = 18,
    eVehicle_4x4            = 19,
    eVehicle_Stallion       = 21,
    eVehicle_Taxi           = 22,
    eVehicle_Impaler_1      = 25,
    eVehicle_Jugular        = 26,
    eVehicle_Pickup_1       = 27,
    eVehicle_PorkaTurbo     = 28,
    eVehicle_Cossie         = 29,
    eVehicle_Bulldog        = 31,
    eVehicle_Challenger     = 34,
    eVehicle_Limousine_1    = 35,
    eVehicle_Tank           = 37,
    eVehicle_Superbike      = 41,
    eVehicle_FireTruck      = 42,
    eVehicle_Bus            = 43,
    eVehicle_Tanker         = 44,
    eVehicle_TVVan          = 45,
    eVehicle_TransitVan     = 46,
    eVehicle_ModelCar       = 47,
    eVehicle_Roadster       = 50,
    eVehicle_LoveWagon      = 51,
    eVehicle_BeastGTS_2     = 53,
    eVehicle_Mundano_2      = 54,
    eVehicle_Mamba          = 55,
    eVehicle_Portsmouth     = 58,
    eVehicle_Speeder        = 61,
    eVehicle_Porka          = 62,
    eVehicle_Flamer         = 63,
    eVehicle_Vulture        = 64,
    eVehicle_Pickup_2       = 65,
    eVehicle_ItaliGTO       = 66,
    eVehicle_Regal          = 70,
    eVehicle_MonsterBug     = 71,
    eVehicle_Thunderhead    = 72,
    eVehicle_Panther        = 73,
    eVehicle_Penetrator_2   = 74,
    eVehicle_LeBonham       = 75,
    eVehicle_Stinger        = 76,
    eVehicle_F19            = 77,
    eVehicle_Brigham        = 78,
    eVehicle_StingerZ29     = 79,
    eVehicle_Classic        = 80,
    eVehicle_29Special      = 81,
    eVehicle_ItaliGTB       = 82,
    eVehicle_Hotrod         = 83,
    eVehicle_Limousine_2    = 86,
    eVehicle_Impaler_2      = 87,
    eVehicle_Helicopter     = 88,
    eVehicle_COUNT
};

decl_enum_strings(eVehicleModel);

// warning - these values are mapped to gta1, so don't change it
enum eCarConvertible
{
    eCarConvertible_HardTop = 0,
    eCarConvertible_OpenTop = 1,
    eCarConvertible_HardTopAnimated = 2,
    eCarConvertible_OpenTopAnimated = 3,
};
decl_enum_strings(eCarConvertible);

// define car class information
struct CarStyle
{
    short mWidth, mHeight, mDepth;  // dimensions of the car with respect to collision checking, x, y, z
    short mSprNum; // first sprite number offset for this car

    struct // specs, usage is unknown
    { 
        short mWeight;	
        short mMaxSpeed, mMinSpeed;
        short mAcceleration, mBraking;
        short mGrip, mHandling;
    };

    short mRemapsBaseIndex;
    HLSRemap mRemap[MAX_CAR_REMAPS];
    eVehicleClass mClassID; // is a descriptor of the type of car / vehicle
    eVehicleModel mModelId;

    int mTurning;

    int mDamagable;
    int mValue[4];

    struct // physics props
    {
        char mCx, mCy; // pixel co-ordinates of the centre of mass of the car, relative to the graphical centre
        int mMoment; // moment of inertia
        float mMass;
        float mThrust; // 1st gear thrust
        float mTyreAdhesionX, mTyreAdhesionY;
        float mHandbrakeFriction;
        float mFootbrakeFriction;
        float mFrontBrakeBias;
        short mTurnRatio;
        short mDriveWheelOffset;
        short mSteeringWheelOffset;
        float mBackEndSlideValue;
        float mHandbrakeSlideValue;
    };

    eCarConvertible mConvertible;
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
    inline SpriteDeltaBits GetDeltaBits() const
    {
        SpriteDeltaBits deltaBits = 0;
        for (int idelta = 0; idelta < mDeltaCount; ++idelta)
        {
            deltaBits |= BIT(idelta);
        }
        return deltaBits;
    }
};

// object_pos_struct / car_pos_struct
struct StartupObjectPosStruct
{
public:
    StartupObjectPosStruct() = default;

    inline bool IsCarObject() const { return mRemap > 127; }
    
    inline bool operator < (const StartupObjectPosStruct& rhs) const
    {
        return memcmp(this, &rhs, sizeof(StartupObjectPosStruct)) < 0;
    }
    inline bool operator == (const StartupObjectPosStruct& rhs) const
    {
        return memcmp(this, &rhs, sizeof(StartupObjectPosStruct)) == 0;
    }
    inline bool operator > (const StartupObjectPosStruct& rhs) const
    {
        return memcmp(this, &rhs, sizeof(StartupObjectPosStruct)) > 0;
    }
public:
    unsigned short mX; 
    unsigned short mY;
    unsigned short mZ; // here, (x,y,z) is the position of the object in the world, stated in world co-ordinates

    unsigned short mRotation;
    unsigned short mPitch;
    unsigned short mRoll;

    unsigned short mType; // is the object type - a code between zero and the maximum number of object types which gives an index into the object info
    unsigned short mRemap; // is a remap table number (0 for none), if remap is >=128 then the item is a car
};

// pedestrian animations
enum ePedestrianAnimID
{
    ePedestrianAnim_Null, // dummy animation
    // pedestrians
    ePedestrianAnim_Walk,
    ePedestrianAnim_Run,
    ePedestrianAnim_ExitCar,
    ePedestrianAnim_EnterCar,
    ePedestrianAnim_SittingInCar,
    ePedestrianAnim_ExitBike,
    ePedestrianAnim_EnterBike,
    ePedestrianAnim_SittingOnBike,
    ePedestrianAnim_FallLong,
    ePedestrianAnim_SlideUnderTheCar,
    ePedestrianAnim_StandingStill, // no weapons
    ePedestrianAnim_Drowning,
    ePedestrianAnim_JumpOntoCar,
    ePedestrianAnim_SlideOnCar,
    ePedestrianAnim_DropOffCarSliding,
    ePedestrianAnim_FallShort,
    ePedestrianAnim_LiesOnFloor,
    ePedestrianAnim_PunchingWhileStanding,
    ePedestrianAnim_PunchingWhileRunning,
    ePedestrianAnim_ShootPistolWhileStanding,
    ePedestrianAnim_ShootPistolWhileWalking,
    ePedestrianAnim_ShootPistolWhileRunning,
    ePedestrianAnim_ShootMachinegunWhileStanding,
    ePedestrianAnim_ShootMachinegunWhileWalking,
    ePedestrianAnim_ShootMachinegunWhileRunning,
    ePedestrianAnim_ShootFlamethrowerWhileStanding,
    ePedestrianAnim_ShootFlamethrowerWhileWalking,
    ePedestrianAnim_ShootFlamethrowerWhileRunning,
    ePedestrianAnim_ShootRPGWhileStanding,
    ePedestrianAnim_ShootRPGWhileWalking,
    ePedestrianAnim_ShootRPGWhileRunning,
    ePedestrianAnim_COUNT
};

decl_enum_strings(ePedestrianAnimID);

// pedestrian weapon
enum eWeaponID
{
    eWeapon_Fists,
    eWeapon_Pistol,
    eWeapon_Machinegun,
    eWeapon_Flamethrower,
    eWeapon_RocketLauncher,
    eWeapon_COUNT
};

decl_enum_strings(eWeaponID);

// weapon fire type
enum eWeaponFireType
{
    eWeaponFireType_Melee,
    eWeaponFireType_Projectile,
    eWeaponFireType_COUNT
};

decl_enum_strings(eWeaponFireType);

// pedestrian basic action
enum ePedestrianAction
{
    ePedestrianAction_null, // not an action

    ePedestrianAction_TurnLeft,
    ePedestrianAction_TurnRight,
    ePedestrianAction_Jump,
    ePedestrianAction_WalkForward,
    ePedestrianAction_WalkBackward,
    ePedestrianAction_Run, // overrides walk_forward and walk_backward
    ePedestrianAction_Shoot,
    ePedestrianAction_EnterCar,
    ePedestrianAction_EnterCarAsPassenger,
    ePedestrianAction_LeaveCar,
    ePedestrianAction_NextWeapon,
    ePedestrianAction_PrevWeapon,

    // in car
    ePedestrianAction_HandBrake,
    ePedestrianAction_Accelerate,
    ePedestrianAction_Reverse,
    ePedestrianAction_SteerLeft,
    ePedestrianAction_SteerRight,
    ePedestrianAction_Horn,

    ePedestrianAction_COUNT
};

decl_enum_strings(ePedestrianAction);

// pedestrian basic state
enum ePedestrianState
{
    ePedestrianState_Unspecified, // special error state
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
    ePedestrianState_Dead,
    ePedestrianState_KnockedDown,
    ePedestrianState_Drowning,
    ePedestrianState_COUNT
};

decl_enum_strings(ePedestrianState);

// cat seat identifier
enum eCarSeat
{
    eCarSeat_Driver,
    eCarSeat_Passenger,
    eCarSeat_PassengerExtra,
    eCarSeat_Any // special seat type
};

decl_enum_strings(eCarSeat);

// pedestrian death cause
enum ePedestrianDeathReason
{
    ePedestrianDeathReason_null,

    ePedestrianDeathReason_FallFromHeight,
    ePedestrianDeathReason_Shot,
    ePedestrianDeathReason_BlownUp,
    ePedestrianDeathReason_Fried,
    ePedestrianDeathReason_Electrocuted,
    ePedestrianDeathReason_Drowned,
    ePedestrianDeathReason_HitByCar,
};

decl_enum_strings(ePedestrianDeathReason);

enum eProjectileType
{
    eProjectileType_Bullet, // pistol, machinegun
    eProjectileType_Flame, // flamethrower
    eProjectileType_Missile, // rocket launcher, tank rocket
    eProjectileType_COUNT
};
decl_enum_strings(eProjectileType);

// Pedestrian control actions status
struct PedestrianCtlState
{
public:
    // Reset current state
    inline void Clear()
    {
        for (int icurrent = 0; icurrent < ePedestrianAction_COUNT; ++icurrent)
        {
            mCtlActions[icurrent] = false;
        }
    }
public:
    bool mCtlActions[ePedestrianAction_COUNT]; // control actions
};