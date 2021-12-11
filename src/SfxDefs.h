#pragma once

// forwards 
class SfxEmitter;
class SfxSample;
class AudioSource;
class AudioSampleBuffer;

// constants
static const int AudioMinVolume = 0;
static const int AudioMaxVolume = 7;

// sound table index
using SfxSampleIndex = unsigned int; 

// sound effect type identifier
enum eSfxSampleType
{
    eSfxSampleType_Level,
    eSfxSampleType_Voice,
};

// sound playback flags
enum SfxFlags
{
    SfxFlags_None        = 0,
    SfxFlags_Loop        = BIT(0),
    SfxFlags_RandomPitch = BIT(1), // randomize pitch a bit
};

decl_enum_as_flags(SfxFlags);

// sound emitter flags
enum SfxEmitterFlags
{
    SfxEmitterFlags_None = 0,
    SfxEmitterFlags_Autorelease = BIT(0), // emitter will be automatically released when playback complete
};

decl_enum_as_flags(SfxEmitterFlags);

// level sound constants
enum : SfxSampleIndex
{
    SfxLevel_CarDoorOpen = 0,
    SfxLevel_CarDoorClose = 1,
    SfxLevel_Pickup = 2,
    SfxLevel_CarEngineStart = 5,
    SfxLevel_CarCrash1 = 6,
    SfxLevel_CarCrash2 = 7,
    SfxLevel_CarCrash3 = 8,
    SfxLevel_CarCrash4 = 9,
    SfxLevel_Explosion = 10,
    SfxLevel_CrateBreak = 11,
    SfxLevel_CarJacking = 15,
    SfxLevel_Punch = 16,
    SfxLevel_Squashed = 17,
    SfxLevel_WaterSplash = 18,
    SfxLevel_FootStep1 = 19,
    SfxLevel_FootStep2 = 20,
    SfxLevel_ScaredScream1 = 21,
    SfxLevel_ScaredScream2 = 22,
    SfxLevel_ScaredScream3 = 23,
    SfxLevel_DieScream1 = 24, // burn
    SfxLevel_DieScream2 = 25,
    SfxLevel_DieScream3 = 26,
    SfxLevel_DieScream4 = 27, // electrocuted
    SfxLevel_PhoneRing = 28,
    SfxLevel_PistolShot = 33,
    SfxLevel_MachineGunShot = 34,
    SfxLevel_FlamethrowerShot = 35,
    SfxLevel_RocketLauncherShot = 36,
    SfxLevel_HugeExplosion = 39,
    SfxLevel_Pager = 40,
    SfxLevel_BulletRicochet = 43,
    SfxLevel_Special1 = 76,
    SfxLevel_Special2 = 77,

    SfxLevel_FirstCarEngineSound = 45,
    // car engine sounds
    SfxLevel_FirstCarHornSound = 58,
    // car horn sounds

    SfxLevel_CarAlarm = 64,

    SfxLevel_Gang = 73, // hare krishnas
    SfxLevel_SpecialSound1 = 76, // burp
    SfxLevel_SpecialSound2 = 77, // farts

    SfxLevel_COUNT
};

// voice sound constants
enum : SfxSampleIndex
{
    SfxVoice_GrandTheftAuto = 0,
    SfxVoice_MissionComplete = 1,
    SfxVoice_MissionFailed = 2,
    SfxVoice_KillFrenzy = 3,
    SfxVoice_Excellent = 4,
    SfxVoice_PlayerDies = 12,

    SfxVoice_COUT
};

// virtual channels
enum ePedSfxChannelIndex
{
    ePedSfxChannelIndex_Voice,
    ePedSfxChannelIndex_Weapon,
    ePedSfxChannelIndex_Misc,
};
enum eCarSfxChannelIndex
{
    eCarSfxChannelIndex_Doors,
    eCarSfxChannelIndex_Engine,
    eCarSfxChannelIndex_Misc, // horn, etc
};