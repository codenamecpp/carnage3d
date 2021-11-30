#pragma once

// forwards
class AiCharacterController;

//////////////////////////////////////////////////////////////////////////

// pedestrian behavior id
enum eAiPedestrianBehavior
{
    eAiPedestrianBehavior_Civilian,
    eAiPedestrianBehavior_HareKrishnasGang,
};

//////////////////////////////////////////////////////////////////////////

enum AiBehaviorMemoryBits: std::uint64_t
{
    AiBehaviorMemoryBits_None = 0,
    AiBehaviorMemoryBits_InPanic = BIT(0), // character was scared to death
    AiBehaviorMemoryBits_HearGunShots = BIT(1), // gunshots was detected nearby
    AiBehaviorMemoryBits_HearExplosion = BIT(2), // explosion was detected nearby
};
decl_enum_as_flags(AiBehaviorMemoryBits);

//////////////////////////////////////////////////////////////////////////

enum AiBehaviorBits: std::uint64_t
{
    AiBehaviorBits_None = 0,

    AiBehaviorBits_PlayerFollower = BIT(0), // start automatically follow human character nearby
    AiBehaviorBits_CanJump = BIT(1), // ability to jump over cars
    // fears
    AiBehaviorBits_Fear_Player = BIT(2),
    AiBehaviorBits_Fear_Police = BIT(3),
    AiBehaviorBits_Fear_GunShots = BIT(4),
    AiBehaviorBits_Fear_Explosions = BIT(5),
    AiBehaviorBits_Fear_DeadPeds = BIT(6),

};
decl_enum_as_flags(AiBehaviorBits);