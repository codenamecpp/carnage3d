#pragma once

// forwards
class AiCharacterController;

//////////////////////////////////////////////////////////////////////////

// pedestrian behavior id
enum eAiPedestrianBehavior
{
    eAiPedestrianBehavior_Civilian,
    eAiPedestrianBehavior_Gang, // Hare Krishnas gang member / leader
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
    AiBehaviorBits_GangLeader = BIT(2),

    // fears
    AiBehaviorBits_Fear_Player = BIT(8),
    AiBehaviorBits_Fear_Police = BIT(9),
    AiBehaviorBits_Fear_GunShots = BIT(10),
    AiBehaviorBits_Fear_Explosions = BIT(11),
    AiBehaviorBits_Fear_DeadPeds = BIT(12),

};
decl_enum_as_flags(AiBehaviorBits);