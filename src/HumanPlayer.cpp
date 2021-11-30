#include "stdafx.h"
#include "HumanPlayer.h"
#include "Pedestrian.h"
#include "PhysicsManager.h"
#include "PhysicsBody.h"
#include "PhysicsDefs.h"
#include "Vehicle.h"
#include "GameMapManager.h"
#include "CarnageGame.h"
#include "TimeManager.h"
#include "AudioDevice.h"
#include "DebugRenderer.h"
#include "RenderingManager.h"

HumanPlayer::HumanPlayer(): CharacterController(eCharacterControllerType_Human)
{
    mAudioListener = gAudioDevice.CreateAudioListener();
    debug_assert(mAudioListener);

    gRenderManager.AttachRenderView(&mViewCamera);
    mHUD.InitHUD(this);
}

HumanPlayer::~HumanPlayer()
{
    if (mAudioListener)
    {
        gAudioDevice.DestroyAudioListener(mAudioListener);
        mAudioListener = nullptr;
    }
    gRenderManager.DetachRenderView(&mViewCamera);
    mHUD.DeinitHUD();
}

void HumanPlayer::OnControllerStart()
{
    debug_assert(mCharacter);

    mSpawnPosition = mCharacter->mTransform.mPosition;
    mFollowCameraController.SetFollowTarget(mCharacter);

    // temporary
    Cheat_GiveAllAmmunitions();
}

void HumanPlayer::OnControllerStop()
{
    mFollowCameraController.SetFollowTarget(nullptr);
}

void HumanPlayer::SetCurrentCameraMode(ePlayerCameraMode cameraMode)
{
    CameraController* newController = (cameraMode == ePlayerCameraMode_Follow) ? 
        (CameraController*) &mFollowCameraController : 
        (CameraController*) &mFreeLookCameraController;

    if (mCameraController == newController)
        return;

    mCameraController = newController;
    if (mCameraController)
    {
        mCameraController->Setup(&mViewCamera);
    }
}

ePlayerCameraMode HumanPlayer::GetCurrentCameraMode() const
{
    if (mCameraController == &mFollowCameraController)
        return ePlayerCameraMode_Follow;

    debug_assert(mCameraController == &mFreeLookCameraController);
    return ePlayerCameraMode_FreeLook;
}

void HumanPlayer::UpdateFrame()
{
    if (mCameraController)
    {
        mCameraController->UpdateFrame();
    }

    mViewCamera.ComputeViewBounds2();

    if (mCharacter->GetWeapon().IsOutOfAmmunition())
    {
        SwitchPrevWeapon();
    }

    if (mCharacter->IsDead())
    {
        UpdateRespawnTimer();
    }

    ProcessRepetitiveActions();

    UpdateDistrictLocation();

    // update audio listener location
    if (mAudioListener)
    {
        mAudioListener->SetPosition(mViewCamera.mPosition);
    }
}

void HumanPlayer::InputEvent(MouseButtonInputEvent& inputEvent)
{
    if (mCameraController)
    {
        mCameraController->InputEvent(inputEvent);
    }
}

void HumanPlayer::InputEvent(MouseMovedInputEvent& inputEvent)
{
    if (mCameraController)
    {
        mCameraController->InputEvent(inputEvent);
    }
}

void HumanPlayer::InputEvent(MouseScrollInputEvent& inputEvent)
{
    if (mCameraController)
    {
        mCameraController->InputEvent(inputEvent);
    }
}

void HumanPlayer::InputEvent(KeyInputEvent& inputEvent)
{
    debug_assert(mCharacter);

    if (mCameraController)
    {
        mCameraController->InputEvent(inputEvent);
    }

    eInputActionsGroup actionGroup = mCharacter->IsCarPassenger() ? eInputActionsGroup_InCar : eInputActionsGroup_OnFoot;
    eInputAction action = mActionsMapping.GetAction(actionGroup, inputEvent.mKeycode);
    if (action == eInputAction_null)
    {
        // try commons
        action = mActionsMapping.GetAction(eInputActionsGroup_Common, inputEvent.mKeycode);
    }

    if (action != eInputAction_null)
    {
        ProcessInputAction(action, inputEvent.mPressed);
        inputEvent.SetConsumed();
    }

    // handle special keys
    if (inputEvent.HasPressed(eKeycode_F9))
    {
        ShowLastDistrictLocation();
        inputEvent.SetConsumed();
    }
}

void HumanPlayer::InputEvent(GamepadInputEvent& inputEvent)
{
    debug_assert(mCharacter);

    if (inputEvent.mGamepad != mActionsMapping.mGamepadID)
        return;

    eInputActionsGroup actionGroup = mCharacter->IsCarPassenger() ? eInputActionsGroup_InCar : eInputActionsGroup_OnFoot;
    eInputAction action = mActionsMapping.GetAction(actionGroup, inputEvent.mButton);
    if (action == eInputAction_null)
    {
        // try commons
        action = mActionsMapping.GetAction(eInputActionsGroup_Common, inputEvent.mButton);
    }

    if (action != eInputAction_null)
    {
        ProcessInputAction(action, inputEvent.mPressed);
        inputEvent.SetConsumed();
    }
}

void HumanPlayer::InputEventLost()
{
    if (mCameraController)
    {
        mCameraController->InputEventLost();
    }

    // reset actions
    mCtlState.Clear();
    mUpdateInputs = false;
}

void HumanPlayer::ProcessInputAction(eInputAction action, bool isActivated)
{    
    switch (action)
    {
        case eInputAction_SteerLeft:
        case eInputAction_TurnLeft:
        case eInputAction_SteerRight:
        case eInputAction_TurnRight:
        case eInputAction_Run:
        case eInputAction_Accelerate:
        case eInputAction_Reverse:
        case eInputAction_WalkBackward:
        case eInputAction_WalkForward:
        case eInputAction_Jump:
        case eInputAction_HandBrake:
        case eInputAction_Shoot:
            // those are updating in ProcessRepetitiveActions
        break;

        case eInputAction_Horn:
            if (mCharacter->IsCarPassenger())
            {
                mCtlState.mHorn = isActivated;
                if (mCharacter->mCurrentCar->HasEmergencyLightsAnimation())
                {
                    mCharacter->mCurrentCar->EnableEmergencyLights(isActivated);
                }
            }
        break;

        case eInputAction_NextWeapon:
            if (isActivated)
            {
                SwitchNextWeapon();
            }
        break;

        case eInputAction_PrevWeapon:
            if (isActivated)
            {
                SwitchPrevWeapon();
            }
        break;

        case eInputAction_Special:
            mCtlState.mSpecial = isActivated;
        break;

        case eInputAction_EnterCar:
        case eInputAction_LeaveCar:
        case eInputAction_EnterCarAsPassenger:
            if (isActivated)
            {
                EnterOrExitCar(action == eInputAction_EnterCarAsPassenger);
            }
        break;

        default:
            debug_assert(false);
        break;
    }

    mUpdateInputs = true;
}

int HumanPlayer::GetWantedLevel() const
{
    return mWantedLevel;
}

void HumanPlayer::SetWantedLevel(int wantedLevel)
{
    debug_assert(wantedLevel <= GAME_MAX_WANTED_LEVEL);
    mWantedLevel = wantedLevel;
}

void HumanPlayer::SwitchNextWeapon()
{
    int nextWeaponIndex = (mCharacter->mCurrentWeapon + 1) % eWeapon_COUNT;
    for (; nextWeaponIndex != mCharacter->mCurrentWeapon; )
    {
        eWeaponID weaponID = (eWeaponID) nextWeaponIndex;
        if (!mCharacter->mWeapons[weaponID].IsOutOfAmmunition())
        {
            mCharacter->ChangeWeapon(weaponID);
            return;
        }
        nextWeaponIndex = (nextWeaponIndex + 1) % eWeapon_COUNT;
    }
}

void HumanPlayer::SwitchPrevWeapon()
{
    int nextWeaponIndex = mCharacter->mCurrentWeapon == 0 ? (eWeapon_COUNT - 1) : (mCharacter->mCurrentWeapon - 1);
    for (; nextWeaponIndex != mCharacter->mCurrentWeapon; )
    {
        eWeaponID weaponID = (eWeaponID) nextWeaponIndex;
        if (!mCharacter->mWeapons[weaponID].IsOutOfAmmunition())
        {
            mCharacter->ChangeWeapon(weaponID);
            return;
        }
        nextWeaponIndex = nextWeaponIndex == 0 ? (eWeapon_COUNT - 1) : (nextWeaponIndex - 1);
    }
}

void HumanPlayer::EnterOrExitCar(bool alternative)
{
    if (mCharacter->IsCarPassenger())
    {
        mCharacter->LeaveCar();
        return;
    }

    PhysicsQueryResult queryResults;

    glm::vec3 pos = mCharacter->mPhysicsBody->GetPosition();
    glm::vec2 posA { pos.x, pos.z };
    glm::vec2 posB = posA + (mCharacter->mPhysicsBody->GetSignVector() * gGameParams.mPedestrianSpotTheCarDistance);

    gPhysics.QueryObjectsLinecast(posA, posB, queryResults, CollisionGroup_Car);

    // process all cars
    for (int iElement = 0; iElement < queryResults.mElementsCount; ++iElement)
    {
        PhysicsBody* physicsBody = queryResults.mElements[iElement].mPhysicsObject;
        debug_assert(physicsBody);
        debug_assert(physicsBody->mGameObject);
        debug_assert(physicsBody->mGameObject->IsVehicleClass());

        Vehicle* carObject = (Vehicle*) physicsBody->mGameObject;

        eCarSeat carSeat = alternative ? eCarSeat_Passenger : eCarSeat_Driver;
        if (carObject->IsSeatPresent(carSeat))
        {
            mCharacter->EnterCar(carObject, carSeat);
        }
        return;
    }
}

void HumanPlayer::Respawn()
{
    SetWantedLevel(GAME_MAX_WANTED_LEVEL);//todo
    mLastDistrictIndex = 0;
    mRespawnTime = 0.0f;

    // todo : exit from car

    mCharacter->HandleDespawn();
    mCharacter->SetPosition(mSpawnPosition);
    mCharacter->HandleSpawn();

    Cheat_GiveAllAmmunitions();
}

void HumanPlayer::ProcessRepetitiveActions()
{
    if (!mUpdateInputs)
        return;

    debug_assert(mCharacter);
    mCtlState.Clear();

    // update in car
    if (mCharacter->IsCarPassenger())
    {
        mCtlState.mSteerDirection = 0.0f;
        if (GetActionState(eInputAction_SteerRight))
        {
            mCtlState.mSteerDirection += 1.0f;
        }
        if (GetActionState(eInputAction_SteerLeft))
        {
            mCtlState.mSteerDirection -= 1.0f;
        }

        mCtlState.mAcceleration = 0.0f;
        if (GetActionState(eInputAction_Accelerate))
        {
            mCtlState.mAcceleration += 1.0f;
        }
        if (GetActionState(eInputAction_Reverse))
        {
            mCtlState.mAcceleration -= 1.0f;
        }

        mCtlState.mHandBrake = GetActionState(eInputAction_HandBrake);
    }
    // update on foot
    else
    {
        mCtlState.mTurnLeft = GetActionState(eInputAction_TurnLeft);
        mCtlState.mTurnRight = GetActionState(eInputAction_TurnRight);
        mCtlState.mRun = GetActionState(eInputAction_Run);
        mCtlState.mWalkBackward = GetActionState(eInputAction_WalkBackward);
        mCtlState.mWalkForward = GetActionState(eInputAction_WalkForward);
        mCtlState.mJump = GetActionState(eInputAction_Jump);
        mCtlState.mShoot = GetActionState(eInputAction_Shoot);

        if ((mCtlState.mTurnLeft == false) && (mCtlState.mTurnRight == false))
        {
            UpdateMouseAiming();
        }
    }
}

bool HumanPlayer::GetActionState(eInputAction action) const
{
    const auto& mapping = mActionsMapping.mActionToKeys[action];
    if (mapping.mKeycode != eKeycode_null)
    {
        if (gInputs.GetKeyState(mapping.mKeycode))
            return true;
    }
    if (mapping.mGpButton != eGamepadButton_null)
    {
        if (gInputs.GetGamepadButtonState(mActionsMapping.mGamepadID, mapping.mGpButton))
            return true;
    }
    return false;
}

void HumanPlayer::UpdateDistrictLocation()
{
    const DistrictInfo* currentDistrict = gGameMap.GetDistrictAtPosition2(mCharacter->mTransform.GetPosition2());
    if (currentDistrict == nullptr)
        return;

    if (currentDistrict->mSampleIndex != mLastDistrictIndex)
    {
        mLastDistrictIndex = currentDistrict->mSampleIndex;
        mHUD.ShowDistrictNameMessage(mLastDistrictIndex);
    }
}

void HumanPlayer::Cheat_GiveAllAmmunitions()
{
    debug_assert(mCharacter);
    for (int icurr = 0; icurr < eWeapon_COUNT; ++icurr)
    {
        mCharacter->mWeapons[icurr].AddAmmunition(99);
    }
    mCharacter->IncArmorMax();
}

void HumanPlayer::SetMouseAiming(bool isEnabled)
{
    mMouseAimingEnabled = isEnabled;
}

bool HumanPlayer::IsMouseAmingEnabled() const
{
    return mMouseAimingEnabled;
}

void HumanPlayer::UpdateMouseAiming()
{
    if (!mMouseAimingEnabled)
        return;

    debug_assert(mCharacter);

    // get current mouse position in world space
    glm::ivec2 screenPosition(gInputs.mCursorPositionX, gInputs.mCursorPositionY);
    cxx::ray3d_t raycastResult;
    if (!mViewCamera.CastRayFromScreenPoint(screenPosition, raycastResult))
        return;

    float distanceFromCameraToCharacter = mViewCamera.mPosition.y - mCharacter->mTransform.mPosition.y;
    glm::vec2 worldPosition
    (
        raycastResult.mOrigin.x + (raycastResult.mDirection.x * distanceFromCameraToCharacter), 
        raycastResult.mOrigin.z + (raycastResult.mDirection.z * distanceFromCameraToCharacter)
    );
    glm::vec2 toTarget = worldPosition - mCharacter->mTransform.GetPosition2();
    mCtlState.mRotateToDesiredAngle = true;
    mCtlState.mDesiredRotationAngle = cxx::angle_t::from_radians(::atan2f(toTarget.y, toTarget.x));
}

void HumanPlayer::UpdateRespawnTimer()
{
    if (mRespawnTime <= 0.0f)
        return;

    float deltaTime = gTimeManager.mGameFrameDelta;
    mRespawnTime -= deltaTime;
    if (mRespawnTime > 0.0f)
        return;

    Respawn();
}

void HumanPlayer::SetRespawnTimer()
{
    if (mCharacter && mCharacter->IsDead())
    {
        mRespawnTime = gGameParams.mGamePlayerRespawnTime;
    }
}

void HumanPlayer::ShowLastDistrictLocation()
{
    mHUD.ShowDistrictNameMessage(mLastDistrictIndex);
}

void HumanPlayer::SetScreenViewArea(const Rect& screenViewArea)
{
    mViewCamera.mViewportRect = screenViewArea;
    mHUD.mScreenArea = screenViewArea;
}
