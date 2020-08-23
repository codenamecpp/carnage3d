#include "stdafx.h"
#include "CommonTypes.h"
#include "GameDefs.h"
#include "GraphicsDefs.h"
#include "GameObject.h"

impl_enum_strings(eKeycode)
{
    {eKeycode_ESCAPE, "Escape"},
    {eKeycode_SPACE, "Space"},
    {eKeycode_PAGE_UP, "PageUp"},
    {eKeycode_PAGE_DOWN, "PageDown"},
    {eKeycode_HOME, "Home"},
    {eKeycode_END, "End"},
    {eKeycode_INSERT, "Insert"},
    {eKeycode_DELETE, "Delete"},
    {eKeycode_RIGHT_CTRL, "RCtrl"},
    {eKeycode_LEFT_CTRL, "LCtrl"},
    {eKeycode_BACKSPACE, "Backspace"},
    {eKeycode_ENTER, "Enter"},
    {eKeycode_TAB, "Tab"},
    {eKeycode_TILDE, "~"},
    {eKeycode_F1, "F1"},
    {eKeycode_F2, "F2"},
    {eKeycode_F3, "F3"},
    {eKeycode_F4, "F4"},
    {eKeycode_F5, "F5"},
    {eKeycode_F6, "F6"},
    {eKeycode_F7, "F7"},
    {eKeycode_F8, "F8"},
    {eKeycode_F9, "F9"},
    {eKeycode_F10, "F10"},
    {eKeycode_F11, "F11"},
    {eKeycode_F12, "F12"},
    {eKeycode_A, "a"},
    {eKeycode_C, "c"},
    {eKeycode_F, "f"},
    {eKeycode_V, "v"},
    {eKeycode_X, "x"},
    {eKeycode_W, "w"},
    {eKeycode_D, "d"},
    {eKeycode_S, "s"},
    {eKeycode_Y, "y"},
    {eKeycode_Z, "z"},
    {eKeycode_R, "r"},
    {eKeycode_0, "0"},
    {eKeycode_1, "1"},
    {eKeycode_2, "2"},
    {eKeycode_3, "3"},
    {eKeycode_4, "4"},
    {eKeycode_5, "5"},
    {eKeycode_6, "6"},
    {eKeycode_7, "7"},
    {eKeycode_8, "8"},
    {eKeycode_9, "9"},
    {eKeycode_LEFT, "Left"},
    {eKeycode_RIGHT, "Right"},
    {eKeycode_UP, "Up"},
    {eKeycode_DOWN, "Down"},
};

impl_enum_strings(eMButton)
{
    {eMButton_LEFT, "Left"},
    {eMButton_RIGHT, "Right"},
    {eMButton_MIDDLE, "Middle"},
};

impl_enum_strings(eGamepadButton)
{
    {eGamepadButton_A, "A"},
    {eGamepadButton_B, "B"},
    {eGamepadButton_X, "X"},
    {eGamepadButton_Y, "Y"},
    {eGamepadButton_LeftBumper, "LeftBumper"},
    {eGamepadButton_RightBumper, "RightBumper"},
    {eGamepadButton_Back, "Back"},
    {eGamepadButton_Start, "Start"},
    {eGamepadButton_Guide, "Guide"},
    {eGamepadButton_LeftThumb, "LeftThumb"},
    {eGamepadButton_RightThumb, "RightThumb"},
    {eGamepadButton_DPAD_Up, "Up"},
    {eGamepadButton_DPAD_Right, "Right"},
    {eGamepadButton_DPAD_Down, "Down"},
    {eGamepadButton_DPAD_Left, "Left"},
    {eGamepadButton_LeftTrigger, "LeftTrigger"},
    {eGamepadButton_RightTrigger, "RightTrigger"},
};

impl_enum_strings(eInputControllerType)
{
    {eInputControllerType_None, "None"},
    {eInputControllerType_Keyboard, "Keyboard"},
    {eInputControllerType_Gamepad1, "Gamepad1"},
    {eInputControllerType_Gamepad2, "Gamepad2"},
    {eInputControllerType_Gamepad3, "Gamepad3"},
    {eInputControllerType_Gamepad4, "Gamepad4"},
};

impl_enum_strings(eLogMessage)
{
    {eLogMessage_Debug, "debug"},
    {eLogMessage_Info, "info"},
    {eLogMessage_Warning, "warning"},
    {eLogMessage_Error, "error"},
};

impl_enum_strings(eLidRotation)
{
    {eLidRotation_0, "0"},
    {eLidRotation_90, "90"},
    {eLidRotation_180, "180"},
    {eLidRotation_270, "270"},
};

impl_enum_strings(eTextureWrapMode)
{
    {eTextureWrapMode_Repeat, "repeat"},
    {eTextureWrapMode_ClampToEdge, "clamp_to_edge"},
};

impl_enum_strings(eTextureFilterMode)
{
    {eTextureFilterMode_Nearest, "nearest"},
    {eTextureFilterMode_Bilinear, "bilinear"},
    {eTextureFilterMode_Trilinear, "trilinear"},
};

impl_enum_strings(eTextureFormat)
{
    {eTextureFormat_Null, "null"},
    {eTextureFormat_R8, "r8"},
    {eTextureFormat_R8_G8, "r8_g8"},
    {eTextureFormat_RGB8, "rgb8"},
    {eTextureFormat_RGBA8, "rgba8"},
    {eTextureFormat_R8UI, "r8ui"},
    {eTextureFormat_RGBA8UI, "rgba8ui"},
    {eTextureFormat_R16UI, "r16ui"},
};

impl_enum_strings(ePrimitiveType)
{
    {ePrimitiveType_Points, "points"},
    {ePrimitiveType_Lines, "lines"},
    {ePrimitiveType_LineLoop, "line_loop"},
    {ePrimitiveType_Triangles, "triangles"},
    {ePrimitiveType_TriangleStrip, "triangle_strip"},
    {ePrimitiveType_TriangleFan, "triangle_fan"},
};

impl_enum_strings(eIndicesType)
{
    {eIndicesType_i16, "i16"},
    {eIndicesType_i32, "i32"},
};

impl_enum_strings(eTextureUnit)
{
    {eTextureUnit_0, "tex_0"},
    {eTextureUnit_1, "tex_1"},
    {eTextureUnit_2, "tex_2"},
    {eTextureUnit_3, "tex_3"},
    {eTextureUnit_4, "tex_4"},
    {eTextureUnit_5, "tex_5"},
    {eTextureUnit_6, "tex_6"},
    {eTextureUnit_7, "tex_7"},
    {eTextureUnit_8, "tex_8"},
    {eTextureUnit_9, "tex_9"},
    {eTextureUnit_10, "tex_10"},
    {eTextureUnit_11, "tex_11"},
    {eTextureUnit_12, "tex_12"},
    {eTextureUnit_13, "tex_13"},
    {eTextureUnit_14, "tex_14"},
    {eTextureUnit_15, "tex_15"},
};

impl_enum_strings(eVertexAttributeFormat)
{
    {eVertexAttributeFormat_2F, "2f"},
    {eVertexAttributeFormat_3F, "3f"},
    {eVertexAttributeFormat_4UB, "4ub"},
    {eVertexAttributeFormat_1US, "1us"},
    {eVertexAttributeFormat_2US, "2us"},
    {eVertexAttributeFormat_Unknown, "unknown"},
};

impl_enum_strings(eVertexAttribute)
{
    {eVertexAttribute_Position0, "in_pos0"},
    {eVertexAttribute_Position1, "in_pos1"},
    {eVertexAttribute_Texcoord0, "in_texcoord0"},
    {eVertexAttribute_Texcoord1, "in_texcoord1"},
    {eVertexAttribute_Normal0, "in_normal0"},
    {eVertexAttribute_Normal1, "in_normal1"},
    {eVertexAttribute_Color0, "in_color0"},
    {eVertexAttribute_Color1, "in_color1"},
};

impl_enum_strings(eBufferContent)
{
    {eBufferContent_Vertices, "vertices"},
    {eBufferContent_Indices, "indices"},
};

impl_enum_strings(eBufferUsage)
{
    {eBufferUsage_Static, "static"},
    {eBufferUsage_Dynamic, "dynamic"},
    {eBufferUsage_Stream, "stream"},
};

impl_enum_strings(eRenderUniform)
{
    {eRenderUniform_ModelMatrix, "model_matrix"},
    {eRenderUniform_ViewMatrix, "view_matrix"},
    {eRenderUniform_ProjectionMatrix, "projection_matrix"},
    {eRenderUniform_ModelViewMatrix, "model_view_matrix"},
    {eRenderUniform_ModelViewProjectionMatrix, "model_view_projection_matrix"},
    {eRenderUniform_ViewProjectionMatrix, "view_projection_matrix"},
    {eRenderUniform_NormalMatrix, "normal_matrix"},
    {eRenderUniform_CameraPosition, "camera_position"},
};

impl_enum_strings(eBlendMode)
{
    {eBlendMode_Alpha, "alpha"},
    {eBlendMode_Additive, "additive"},
    {eBlendMode_Multiply, "multiply"},
    {eBlendMode_Premultiplied, "premultiplied"},
    {eBlendMode_Screen, "screen"},
};

impl_enum_strings(eDepthTestFunc)
{
    {eDepthTestFunc_Always, "always"},
    {eDepthTestFunc_Equal, "equal"},
    {eDepthTestFunc_NotEqual, "not_equal"},
    {eDepthTestFunc_Less, "less"},
    {eDepthTestFunc_Greater, "greater"},
    {eDepthTestFunc_LessEqual, "less_equal"},
    {eDepthTestFunc_GreaterEqual, "greater_equal"},
};

impl_enum_strings(eCullMode)
{
    {eCullMode_Front, "front"},
    {eCullMode_Back, "back"},
    {eCullMode_FrontAndBack, "front_and_back"},
};

impl_enum_strings(eFillMode)
{
    {eFillMode_Solid, "solid"},
    {eFillMode_WireFrame, "wireframe"},
};

impl_enum_strings(eBlockType)
{
    {eBlockType_Side, "side"},
    {eBlockType_Lid, "lid"},
    {eBlockType_Aux, "aux"},
};

impl_enum_strings(eSpriteType)
{
    {eSpriteType_Arrow, "arrow"},
    {eSpriteType_Digit, "digit"},
    {eSpriteType_Boat, "boat"},
    {eSpriteType_Box, "box"},
    {eSpriteType_Bus, "bus"},
    {eSpriteType_Car, "car"},
    {eSpriteType_Object, "object"},
    {eSpriteType_Ped, "ped"},
    {eSpriteType_Speedo, "speedo"},
    {eSpriteType_Tank, "tank"},
    {eSpriteType_TrafficLight, "traffic_light"},
    {eSpriteType_Train, "train"},
    {eSpriteType_TrDoor, "trdoor"},
    {eSpriteType_Bike, "bike"},
    {eSpriteType_Tram, "tram"},
    {eSpriteType_WreckedCar, "wcar"},
    {eSpriteType_WBus, "wbus"},
    {eSpriteType_Ex, "ex"},
    {eSpriteType_TumCar, "tumcar"},
    {eSpriteType_TumTruck, "tumtruck"},
    {eSpriteType_Ferry, "ferry"},
};

impl_enum_strings(eGroundType)
{
    {eGroundType_Air, "air"},
    {eGroundType_Water, "water"},
    {eGroundType_Road, "road"},
    {eGroundType_Pawement, "pawement"},
    {eGroundType_Field, "field"},
    {eGroundType_Building, "building"},
};

impl_enum_strings(eBlockFace)
{
    {eBlockFace_W, "w"},
    {eBlockFace_E, "e"},
    {eBlockFace_N, "n"},
    {eBlockFace_S, "s"},
    {eBlockFace_Lid, "lid"},
};

impl_enum_strings(eVehicleClass)
{
    {eVehicleClass_Bus, "bus"},
    {eVehicleClass_FrontOfJuggernaut, "front_of_juggernaut"},
    {eVehicleClass_BackOfJuggernaut, "back_of_juggernaut"},
    {eVehicleClass_Motorcycle, "motorcycle"},
    {eVehicleClass_StandardCar, "standard_car"},
    {eVehicleClass_Train, "train"},
    {eVehicleClass_Tram, "tram"},
    {eVehicleClass_Boat, "boat"},
    {eVehicleClass_Tank, "tank"},
};

impl_enum_strings(ePedestrianAnimID)
{
    {ePedestrianAnim_Null, "null"},
    {ePedestrianAnim_Walk, "walk"},
    {ePedestrianAnim_Run, "run"},
    {ePedestrianAnim_ExitCar, "exit_car"},
    {ePedestrianAnim_EnterCar, "enter_car"},
    {ePedestrianAnim_SittingInCar, "sitting_in_car"},
    {ePedestrianAnim_ExitBike, "exit_bike"},
    {ePedestrianAnim_Drowning, "drowning"},
    {ePedestrianAnim_EnterBike, "enter_bike"},
    {ePedestrianAnim_SittingOnBike, "sitting_on_bike"},
    {ePedestrianAnim_FallLong, "fall_long"},
    {ePedestrianAnim_SlideUnderTheCar, "slide_under_the_car"},
    {ePedestrianAnim_StandingStill, "standing_still"},
    {ePedestrianAnim_JumpOntoCar, "jump_onto_car"},
    {ePedestrianAnim_SlideOnCar, "slide_on_car"},
    {ePedestrianAnim_DropOffCarSliding, "drop_off_car_sliding"},
    {ePedestrianAnim_FallShort, "fall_short"},
    {ePedestrianAnim_LiesOnFloor, "lies_on_floor"},
    {ePedestrianAnim_PunchingWhileStanding, "punching_while_standing"},
    {ePedestrianAnim_PunchingWhileRunning, "punching_while_running"},
    {ePedestrianAnim_ShootPistolWhileStanding, "shoot_pistol_while_standing"},
    {ePedestrianAnim_ShootPistolWhileWalking, "shoot_pistol_while_walking"},
    {ePedestrianAnim_ShootPistolWhileRunning, "shoot_pistol_while_running"},
    {ePedestrianAnim_ShootMachinegunWhileStanding, "shoot_machinegun_while_standing"},
    {ePedestrianAnim_ShootMachinegunWhileWalking, "shoot_machinegun_while_walking"},
    {ePedestrianAnim_ShootMachinegunWhileRunning, "shoot_machinegun_while_running"},
    {ePedestrianAnim_ShootFlamethrowerWhileStanding, "shoot_flamethrower_while_standing"},
    {ePedestrianAnim_ShootFlamethrowerWhileWalking, "shoot_flamethrower_while_walking"},
    {ePedestrianAnim_ShootFlamethrowerWhileRunning, "shoot_flamethrower_while_running"},
    {ePedestrianAnim_ShootRPGWhileStanding, "shoot_rpg_while_standing"},
    {ePedestrianAnim_ShootRPGWhileWalking, "shoot_rpg_while_walking"},
    {ePedestrianAnim_ShootRPGWhileRunning, "shoot_rpg_while_running"},
};

impl_enum_strings(eWeaponID)
{
    {eWeapon_Fists, "fists"},
    {eWeapon_Pistol, "pistol"},
    {eWeapon_Machinegun, "machinegun"},
    {eWeapon_Flamethrower, "flamethrower"},
    {eWeapon_RocketLauncher, "rocket_launcher"},
};

impl_enum_strings(ePedestrianAction)
{
    {ePedestrianAction_null, "None"},

    {ePedestrianAction_TurnLeft, "TurnLeft"},
    {ePedestrianAction_TurnRight, "TurnRight"},
    {ePedestrianAction_Jump, "Jump"},
    {ePedestrianAction_WalkForward, "WalkForward"},
    {ePedestrianAction_WalkBackward, "WalkBackward"},
    {ePedestrianAction_Run, "Run"},
    {ePedestrianAction_Shoot, "Shoot"},
    {ePedestrianAction_NextWeapon, "NextWeapon"},
    {ePedestrianAction_PrevWeapon, "PrevWeapon"},
    {ePedestrianAction_EnterCar, "EnterCar"},
    {ePedestrianAction_EnterCarAsPassenger, "EnterCarAsPassenger"},

    {ePedestrianAction_LeaveCar, "LeaveCar"},
    {ePedestrianAction_HandBrake, "HandBrake"},
    {ePedestrianAction_Accelerate, "Accelerate"},
    {ePedestrianAction_Reverse, "Reverse"},
    {ePedestrianAction_SteerLeft, "SteerLeft"},
    {ePedestrianAction_SteerRight, "SteerRight"},
    {ePedestrianAction_Horn, "Horn"},
};

impl_enum_strings(ePedestrianState)
{   
    {ePedestrianState_Unspecified, "unspecified"},
    {ePedestrianState_StandingStill, "standing_still"},
    {ePedestrianState_StandsAndShoots, "stands_and_shoots"},
    {ePedestrianState_Walks, "walks"},
    {ePedestrianState_Runs, "runs"},
    {ePedestrianState_WalksAndShoots, "walks_and_shoots"},
    {ePedestrianState_RunsAndShoots, "runs_and_shoots"},
    {ePedestrianState_Falling, "falling"},
    {ePedestrianState_EnteringCar, "entering_car"},
    {ePedestrianState_ExitingCar, "exiting_car"},
    {ePedestrianState_DrivingCar, "driving_car"},
    {ePedestrianState_SlideOnCar, "slide_on_car"},
    {ePedestrianState_Dead, "dead"},
    {ePedestrianState_KnockedDown, "knocked_down"},
    {ePedestrianState_Drowning, "drowning"},
};

impl_enum_strings(eConsoleLineType)
{
    {eConsoleLineType_Message, "message"},
    {eConsoleLineType_Command, "command"},
};

impl_enum_strings(eSceneCameraMode)
{
    {eSceneCameraMode_Perspective, "perspective"},
    {eSceneCameraMode_Orthographic, "orthographic"},
};

impl_enum_strings(eSpriteAnimStatus)
{
    {eSpriteAnimStatus_Stop, "stop"},
    {eSpriteAnimStatus_PlayForward, "play_forward"},
    {eSpriteAnimStatus_PlayBackward, "play_backward"},
};

impl_enum_strings(eSpriteAnimLoop)
{
    {eSpriteAnimLoop_None, "none"},
    {eSpriteAnimLoop_PingPong, "ping_pong"},
    {eSpriteAnimLoop_FromStart, "from_start"},
};

impl_enum_strings(eVehicleModel)
{
    {eVehicle_BeastGTS_1, "beast_gts_1"},
    {eVehicle_Bug, "bug"},
    {eVehicle_Counthash, "counthash"},
    {eVehicle_Bike, "bike"},
    {eVehicle_Police, "police"},
    {eVehicle_Ambulance, "ambulance"},
    {eVehicle_RepairVan, "repair_van"},
    {eVehicle_Juggernaut, "juggernaut"},
    {eVehicle_Coach, "coach"},
    {eVehicle_Train, "train"},
    {eVehicle_Tram, "tram"},
    {eVehicle_Boat, "boat"},
    {eVehicle_Penetrator_1, "penetrator_1"},
    {eVehicle_Itali, "itali"},
    {eVehicle_Mundano_1, "mundano_1"},
    {eVehicle_4x4, "4x4"},
    {eVehicle_Stallion, "stallion"},
    {eVehicle_Taxi, "taxi"},
    {eVehicle_Impaler_1, "impaler_1"},
    {eVehicle_Jugular, "jugular"},
    {eVehicle_Pickup_1, "pickup_1"},
    {eVehicle_PorkaTurbo, "porka_turbo"},
    {eVehicle_Cossie, "cossie"},
    {eVehicle_Bulldog, "bulldog"},
    {eVehicle_Challenger, "challenger"},
    {eVehicle_Limousine_1, "limousine_1"},
    {eVehicle_Tank, "tank"},
    {eVehicle_Superbike, "superbike"},
    {eVehicle_FireTruck, "fire_truck"},
    {eVehicle_Bus, "bus"},
    {eVehicle_Tanker, "tanker"},
    {eVehicle_TVVan, "tv_van"},
    {eVehicle_TransitVan, "transit_van"},
    {eVehicle_ModelCar, "model_car"},
    {eVehicle_Roadster, "roadster"},
    {eVehicle_LoveWagon, "love_wagon"},
    {eVehicle_BeastGTS_2, "beast_gts_2"},
    {eVehicle_Mundano_2, "mundano_2"},
    {eVehicle_Mamba, "mamba"},
    {eVehicle_Portsmouth, "portsmouth"},
    {eVehicle_Speeder, "speeder"},
    {eVehicle_Porka, "porka"},
    {eVehicle_Flamer, "flamer"},
    {eVehicle_Vulture, "vulture"},
    {eVehicle_Pickup_2, "pickup_2"},
    {eVehicle_ItaliGTO, "itali_tgo"},
    {eVehicle_Regal, "regal"},
    {eVehicle_MonsterBug, "monster_bug"},
    {eVehicle_Thunderhead, "thunderhead"},
    {eVehicle_Panther, "panther"},
    {eVehicle_Penetrator_2, "penetrator_2"},
    {eVehicle_LeBonham, "lebonham"},
    {eVehicle_Stinger, "stinger"},
    {eVehicle_F19, "f19"},
    {eVehicle_Brigham, "brigham"},
    {eVehicle_StingerZ29, "stinger_z29"},
    {eVehicle_Classic, "classic"},
    {eVehicle_29Special, "29_special"},
    {eVehicle_ItaliGTB, "itali_gtb"},
    {eVehicle_Hotrod, "hotrod"},
    {eVehicle_Limousine_2, "limousine_2"},
    {eVehicle_Impaler_2, "impaler_2"},
    {eVehicle_Helicopter, "helicopter"},
};

impl_enum_strings(eCarSeat)
{
    {eCarSeat_Driver, "driver"},
    {eCarSeat_Passenger, "passenger"},
    {eCarSeat_PassengerExtra, "passenger_extra"},
    {eCarSeat_Any, "any"},
};

impl_enum_strings(eCarConvertible)
{
    {eCarConvertible_HardTop, "hard_top"},
    {eCarConvertible_OpenTop, "open_top"},
    {eCarConvertible_HardTopAnimated, "hard_top_animated"},
    {eCarConvertible_OpenTopAnimated, "open_top_animated"},
};

impl_enum_strings(eGameObjectClass)
{
    {eGameObjectClass_Pedestrian, "pedestrian"},
    {eGameObjectClass_Car, "car"},
    {eGameObjectClass_Projectile, "projectile"},
    {eGameObjectClass_Powerup, "powerup"},
    {eGameObjectClass_Decoration, "decoration"},
    {eGameObjectClass_Obstacle, "obstacle"},
    {eGameObjectClass_Explosion, "explosion"},
};

impl_enum_strings(ePedestrianDeathReason)
{
    {ePedestrianDeathReason_null, "None"},
    {ePedestrianDeathReason_FallFromHeight, "FallFromHeight"},
    {ePedestrianDeathReason_Shot, "Shot"},
    {ePedestrianDeathReason_BlownUp, "BlownUp"},
    {ePedestrianDeathReason_Fried, "Fried"},
    {ePedestrianDeathReason_Electrocuted, "Electrocuted"},
    {ePedestrianDeathReason_Drowned, "Drowned"},
    {ePedestrianDeathReason_HitByCar, "HitByCar"},
};

impl_enum_strings(eProjectileType)
{
    {eProjectileType_Bullet, "bullet"},
    {eProjectileType_Flame, "flame"},
    {eProjectileType_Missile, "missile"},
};

impl_enum_strings(eWeaponFireType)
{
    {eWeaponFireType_Melee, "melee"},
    {eWeaponFireType_Projectile, "projectile"},
};