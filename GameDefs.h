#pragma once

// most of game constants is mapped to GTA files format so don't change

// side length of block cube
#define MAP_BLOCK_DIMS 1.0f

#define MAP_BLOCK_TEXTURE_DIMS 64
#define MAP_BLOCK_TEXTURE_AREA (MAP_BLOCK_TEXTURE_DIMS * MAP_BLOCK_TEXTURE_DIMS)

#define MAX_MAP_BLOCK_ANIM_FRAMES 32
#define MAX_CAR_DOORS 4
#define MAX_CAR_REMAPS 12
#define MAX_SPRITE_DELTAS 32

// map width and height is same
#define MAP_DIMENSIONS 256
#define MAP_LAYERS_COUNT 6

enum eLidRotation : unsigned short
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
enum eGroundType : unsigned short
{
    eGroundType_Air,
    eGroundType_Water,
    eGroundType_Road,
    eGroundType_Pawement,
    eGroundType_Field,
    eGroundType_Building,
    eGroundType_COUNT,
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
    eBlockFace_COUNT,
};

define_enum_strings(eBlockFace)
{
    eBlockFace_W, "w",
    eBlockFace_E, "e",
    eBlockFace_N, "n",
    eBlockFace_S, "s",
    eBlockFace_Lid, "lid",
};

// defines part of city mesh
struct CityMeshData
{
public:
    CityMeshData() = default;

    inline void SetNull()
    {
        mMeshVertices.clear();
        mMeshIndices.clear();
    }
public:
    std::vector<Vertex3D> mMeshVertices;
    std::vector<DrawIndex_t> mMeshIndices;
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
struct BlockStyleData
{
public:
    eLidRotation mLidRotation;
    eGroundType mGroundType;

    short mRemap;
    short mTrafficLight;

    short mSlopeType; // 0 = none
        // 1 - 2 = up 26 low, high
        // 3 - 4 = down 26 low, high
        // 5 - 6 = left 26 low, high
        // 7 - 8 = right 26 low, high
        // 9 - 16 = up 7 low - high
        // 17 - 24 = down 7 low - high
        // 25 - 32 = left 7 low - high
        // 33 - 40 = right 7 low - high
        // 41 - 44 = 45 up,down,left,right 

    short mFaces[eBlockFace_COUNT]; // stores a value which indicates the correct graphic square to use for that face
        // A value of zero indicates no face - hidden faces must be set to zero

    bool mUpDirection : 1;
    bool mDownDirection : 1;
    bool mLeftDirection : 1;
    bool mRightDirection : 1;

    bool mIsFlat : 1;
    bool mFlipTopBottomFaces : 1;
    bool mFlipLeftRightFaces : 1;
    bool mIsRailway : 1;
};

// define map block anim information
struct BlockAnimationStyleData
{
public:
    int mBlock = 0; // the block number
    int mWhich = 0; // the area type ( 0 for side, 1 for lid )
    int mSpeed = 0; // the number of game cycles to display each frame
    int mFrameCount = 0; // the number of auxilliary frames
    int mFrames[MAX_MAP_BLOCK_ANIM_FRAMES]; // an array of block numbers, these refer to aux_block
};

// define map object information
struct MapObjectStyleData
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
struct CarDoorStyleData
{
public:
	int mRpx, mRpy;
	int mObject;
	int mDelta;
};

// define car class information
struct CarStyleData
{
public:
    // dimensions of the car with respect to collision checking
    int mWidth, mHeight, mDepth; // x, y, z
    int mSprNum; // first sprite number offset for this car
    int mWeight;	
    int mMaxSpeed, mMinSpeed;
    int mAcceleration, mBraking;
    int mGrip, mHandling;
    HLSRemap mRemap[MAX_CAR_REMAPS];
    int mVType; // is a descriptor of the type of car / vehicle
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
    CarDoorStyleData mDoors[MAX_CAR_DOORS];
};

// define sprite information
struct SpriteStyleData
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