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

define_enum_strings(eLidRotation)
{
    eLidRotation_0, "0",
    eLidRotation_90, "90",
    eLidRotation_180, "180",
    eLidRotation_270, "270",
};

// map block type
enum eBlockType : unsigned short
{
    eBlockType_Side,
    eBlockType_Lid,
    eBlockType_Aux,
    eBlockType_COUNT
};

define_enum_strings(eBlockType)
{
    eBlockType_Side, "side",
    eBlockType_Lid, "lid",
    eBlockType_Aux, "aux",
};

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

define_enum_strings(eSpriteType)
{
    eSpriteType_Arrow, "arrow",
    eSpriteType_Digit, "digit",
    eSpriteType_Boat, "boat",
    eSpriteType_Box, "box",
    eSpriteType_Bus, "bus",
    eSpriteType_Car, "car",
    eSpriteType_Object, "object",
    eSpriteType_Ped, "ped",
    eSpriteType_Speedo, "speedo",
    eSpriteType_Tank, "tank",
    eSpriteType_TrafficLight, "traffic_light",
    eSpriteType_Train, "train",
    eSpriteType_TrDoor, "trdoor",
    eSpriteType_Bike, "bike",
    eSpriteType_Tram, "tram",
    eSpriteType_WCar, "wcar",
    eSpriteType_WBus, "wbus",
    eSpriteType_Ex, "ex",
    eSpriteType_TumCar, "tumcar",
    eSpriteType_TumTruck, "tumtruck",
    eSpriteType_Ferry, "ferry",
};

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

define_enum_strings(eGroundType)
{
    eGroundType_Air, "air",
    eGroundType_Water, "water",
    eGroundType_Road, "road",
    eGroundType_Pawement, "pawement",
    eGroundType_Field, "field",
    eGroundType_Building, "building",
};

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

define_enum_strings(eBlockFace)
{
    eBlockFace_W, "w",
    eBlockFace_E, "e",
    eBlockFace_N, "n",
    eBlockFace_S, "s",
    eBlockFace_Lid, "lid",
};

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
        mEtries.clear();
    }
public:
    GpuTexture2D* mSpritesheetTexture = nullptr;
    std::vector<TextureRegion> mEtries;
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
	int mRpx, mRpy;
	int mObject;
	int mDelta;
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

define_enum_strings(eCarVType)
{
    eCarVType_Bus, "bus",
    eCarVType_FrontOfJuggernaut, "front_of_juggernaut",
    eCarVType_BackOfJuggernaut, "back_of_juggernaut",
    eCarVType_Motorcycle, "motorcycle",
    eCarVType_StandardCar, "standard_car",
    eCarVType_Train, "standard_train",
    eCarVType_Tram, "tram",
    eCarVType_Boat, "boat",
    eCarVType_Tank, "tank",
};

// define car class information
struct CarStyle
{
public:
    int mWidth, mHeight, mDepth;  // dimensions of the car with respect to collision checking, x, y, z
    int mSprNum; // first sprite number offset for this car
    int mWeight;	
    int mMaxSpeed, mMinSpeed;
    int mAcceleration, mBraking;
    int mGrip, mHandling;
    HLSRemap mRemap[MAX_CAR_REMAPS];
    eCarVType mVType; // is a descriptor of the type of car / vehicle
    int mModel; // sub-type within vtype for cars which holds an identifier for the model of car
    int mTurning;
    int mDamagable;
    int mValue[4];
    int mCx, mCy; // pixel co-ordinates of the centre of mass of the car, relative to the graphical centre
    int mMoment;
    float mRbpMass;
    float mG1Thrust;
    float mTyreAdhesionX, mTyreAdhesionY;
    float mHandbrakeFriction;
    float mFootbrakeFriction;
    float mFrontBrakeBias;
    int mTurnRatio;
    int mDriveWheelOffset;
    int mSteeringWheelOffset;
    float mBackEndSlideValue;
    float mHandbrakeSlideValue;
    bool mConvertible;
    int mEngine;
    int mRadio;
    int mHorn;
    int mSoundFunction;
    int mFastChangeFlag;
    int mDoorsCount;
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

define_enum_strings(eSpriteAnimationID)
{
    eSpriteAnimationID_Null, "null",
    eSpriteAnimationID_Ped_Walk, "ped_walk",
    eSpriteAnimationID_Ped_Run, "ped_run",
    eSpriteAnimationID_Ped_ExitCar, "ped_exit_car",
    eSpriteAnimationID_Ped_EnterCar, "ped_enter_car",
    eSpriteAnimationID_Ped_FallLong, "ped_fall_long",
    eSpriteAnimationID_Ped_SlideUnderTheCar, "ped_slide_under_the_car",
    eSpriteAnimationID_Ped_StandingStill, "ped_standing_still",
    eSpriteAnimationID_Ped_JumpOntoCar, "ped_jump_onto_car",
    eSpriteAnimationID_Ped_SlideOnCar, "ped_slide_on_car",
    eSpriteAnimationID_Ped_DropOffCarSliding, "ped_drop_off_car_sliding",
    eSpriteAnimationID_Ped_FallShort, "ped_fall_short",
    eSpriteAnimationID_Ped_LiesOnFloor, "ped_lies_on_floor",
    eSpriteAnimationID_Ped_PunchingWhileStanding, "ped_punching_while_standing",
    eSpriteAnimationID_Ped_PunchingWhileRunning, "ped_punching_while_running",
    eSpriteAnimationID_Ped_ShootPistolWhileStanding, "ped_shoot_pistol_while_standing",
    eSpriteAnimationID_Ped_ShootPistolWhileWalking, "ped_shoot_pistol_while_walking",
    eSpriteAnimationID_Ped_ShootPistolWhileRunning, "ped_shoot_pistol_while_running",
    eSpriteAnimationID_Ped_ShootMachinegunWhileStanding, "ped_shoot_machinegun_while_standing",
    eSpriteAnimationID_Ped_ShootMachinegunWhileWalking, "ped_shoot_machinegun_while_walking",
    eSpriteAnimationID_Ped_ShootMachinegunWhileRunning, "ped_shoot_machinegun_while_running",
    eSpriteAnimationID_Ped_ShootFlamethrowerWhileStanding, "ped_shoot_flamethrower_while_standing",
    eSpriteAnimationID_Ped_ShootFlamethrowerWhileWalking, "ped_shoot_flamethrower_while_walking",
    eSpriteAnimationID_Ped_ShootFlamethrowerWhileRunning, "ped_shoot_flamethrower_while_running",
    eSpriteAnimationID_Ped_ShootRPGWhileStanding, "ped_shoot_rpg_while_standing",
    eSpriteAnimationID_Ped_ShootRPGWhileWalking, "ped_shoot_rpg_while_walking",
    eSpriteAnimationID_Ped_ShootRPGWhileRunning, "ped_shoot_rpg_while_running",
};

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

define_enum_strings(eWeaponType)
{
    eWeaponType_Fists, "fists",
    eWeaponType_Pistol, "pistol",
    eWeaponType_Machinegun, "machinegun",
    eWeaponType_Flamethrower, "flamethrower",
    eWeaponType_RocketLauncher, "rocket_launcher",
};

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

define_enum_strings(ePedestrianAction)
{
    ePedestrianAction_TurnLeft, "turn_left",
    ePedestrianAction_TurnRight, "turn_right",
    ePedestrianAction_Jump, "jump",
    ePedestrianAction_WalkForward, "walk_forward",
    ePedestrianAction_WalkBackward, "walk_backward",
    ePedestrianAction_Run, "run",
    ePedestrianAction_Shoot, "shoot",
    ePedestrianAction_EnterCar, "enter_car",
    ePedestrianAction_LeaveCar, "leave_car",
};

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

define_enum_strings(ePedestrianState)
{
    ePedestrianState_StandingStill, "standing_still",
    ePedestrianState_StandsAndShoots, "stands_and_shoots",
    ePedestrianState_Walks, "walks",
    ePedestrianState_Runs, "runs",
    ePedestrianState_WalksAndShoots, "walks_and_shoots",
    ePedestrianState_RunsAndShoots, "runs_and_shoots",
    ePedestrianState_Falling, "falling",
    ePedestrianState_EnteringCar, "entering_car",
    ePedestrianState_ExitingCar, "exiting_car",
    ePedestrianState_DrivingCar, "driving_car",
    ePedestrianState_SlideOnCar, "slide_on_car",
    ePedestrianState_Dying, "dying",
    ePedestrianState_Dead, "dead",
    ePedestrianState_KnockedDown, "knocked_down",
};