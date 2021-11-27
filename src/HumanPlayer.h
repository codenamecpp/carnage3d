#pragma once

#include "GameDefs.h"
#include "CharacterController.h"
#include "InputActionsMapping.h"
#include "AudioListener.h"
#include "FollowCameraController.h"
#include "FreeLookCameraController.h"
#include "HUD.h"

//////////////////////////////////////////////////////////////////////////

enum ePlayerCameraMode
{
    ePlayerCameraMode_Follow, // default
    ePlayerCameraMode_FreeLook, // debug
};

//////////////////////////////////////////////////////////////////////////

// Contains human player related information but also processes character controller logic
class HumanPlayer final: public CharacterController
{
public:
    // readonly
    InputActionsMapping mActionsMapping;
    glm::vec3 mSpawnPosition;
    AudioListener* mAudioListener = nullptr;
    GameCamera mViewCamera;
    HUD mHUD;

public:
    HumanPlayer(Pedestrian* character);
    ~HumanPlayer();

    void EnterGameScenario();
    void LeaveGameScenario();

    // override CharacterController
    void OnCharacterUpdateFrame() override;

    // Get or set current police attention level
    int GetWantedLevel() const;
    void SetWantedLevel(int wantedLevel);
   
    // Mouse aiming control
    void SetMouseAiming(bool isEnabled);
    bool IsMouseAmingEnabled() const;

    void SetRespawnTimer();
    void SetScreenViewArea(const Rect& screenViewArea);

    // process players inputs
    // @param inputEvent: Event data
    void InputEvent(KeyInputEvent& inputEvent);
    void InputEvent(GamepadInputEvent& inputEvent);
    void InputEvent(MouseButtonInputEvent& inputEvent);
    void InputEvent(MouseMovedInputEvent& inputEvent);
    void InputEvent(MouseScrollInputEvent& inputEvent);
    void InputEventLost();

    // public for debug purposes
    void SetCurrentCameraMode(ePlayerCameraMode cameraMode);
    ePlayerCameraMode GetCurrentCameraMode() const;

    // cheats
    void Cheat_GiveAllAmmunitions();

private:
    void SwitchNextWeapon();
    void SwitchPrevWeapon();
    void EnterOrExitCar(bool alternative);

    void Respawn();

    void ProcessRepetitiveActions();
    void ProcessInputAction(eInputAction action, bool isActivated);
    bool GetActionState(eInputAction action) const;

    void ShowLastDistrictLocation();

    void UpdateDistrictLocation();
    void UpdateMouseAiming();
    void UpdateRespawnTimer();

private:
    int mLastDistrictIndex = 0; // district index where character was last time
    int mWantedLevel = 0; // current police attention level
    
    float mRespawnTime = 0.0f;
    bool mUpdateInputs = false;
    bool mMouseAimingEnabled = false;

    FollowCameraController mFollowCameraController;
    FreeLookCameraController mFreeLookCameraController;
    CameraController* mCameraController = nullptr;
};