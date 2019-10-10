#include "stdafx.h"
#include "Pedestrian.h"
#include "PhysicsManager.h"
#include "GameMapManager.h"
#include "SpriteBatch.h"
#include "SpriteManager.h"
#include "RenderingManager.h"
#include "PedestrianStates.h"

Pedestrian::Pedestrian(GameObjectID_t id)
    : GameObject(id)
    , mPhysicsComponent()
    , mCurrentAnimID(eSpriteAnimationID_Null)
    , mController()
    , mActivePedsNode(this)
    , mDeletePedsNode(this)
{
}

Pedestrian::~Pedestrian()
{
    if (mPhysicsComponent)
    {
        gPhysics.DestroyPhysicsComponent(mPhysicsComponent);
    }
}

void Pedestrian::EnterTheGame()
{
    glm::vec3 startPosition;

    mCurrentStateTime = 0;

    // reset actions
    for (int iaction = 0; iaction < ePedestrianAction_COUNT; ++iaction)
    {
        mCtlActions[iaction] = false;
    }

    // reset weapon ammo
    for (int iweapon = 0; iweapon < eWeaponType_COUNT; ++iweapon)
    {
        mWeaponsAmmo[iweapon] = -1; // temporary
    }
    mWeaponsAmmo[eWeaponType_Fists] = -1;
    mCurrentWeapon = eWeaponType_Fists;
    
    mPhysicsComponent = gPhysics.CreatePedPhysicsComponent(this, startPosition, cxx::angle_t::from_degrees(0.0f));
    debug_assert(mPhysicsComponent);

    mMarkForDeletion = false;
    mCurrentAnimID = eSpriteAnimationID_Null;

    SetCurrentState(ePedestrianState_StandingStill, true);
}

void Pedestrian::UpdateFrame(Timespan deltaTime)
{
    // update controller logic if it specified
    if (mController)
    {
        mController->UpdateFrame(this, deltaTime);
    }

    mCurrentAnimState.AdvanceAnimation(deltaTime);

    mCurrentStateTime += deltaTime;

    // update current state logic
    PedestrianBaseState* currentState = gPedestrianBaseStatesManager.GetStateByID(mCurrentStateID);
    ePedestrianState nextState = currentState->ProcessStateFrame(this, deltaTime);
    SetCurrentState(nextState, false);
}

void Pedestrian::DrawFrame(SpriteBatch& spriteBatch)
{
    cxx::angle_t rotationAngle = mPhysicsComponent->GetRotationAngle() - cxx::angle_t::from_degrees(SPRITE_ZERO_ANGLE);
    glm::vec3 position = mPhysicsComponent->GetPosition();

    int spriteLinearIndex = gGameMap.mStyleData.GetSpriteIndex(eSpriteType_Ped, mCurrentAnimState.GetCurrentFrame());
    gSpriteManager.GetSpriteTexture(mObjectID, spriteLinearIndex, mDrawSprite);

    mDrawSprite.mPosition = glm::vec2(position.x, position.z);
    mDrawSprite.mScale = SPRITE_SCALE;
    mDrawSprite.mRotateAngle = rotationAngle;
    mDrawSprite.mHeight = ComputeDrawHeight(position, rotationAngle);
    mDrawSprite.SetOriginToCenter();
    spriteBatch.DrawSprite(mDrawSprite);
}

void Pedestrian::SetHeading(cxx::angle_t rotationAngle)
{
    debug_assert(mPhysicsComponent);
    mPhysicsComponent->SetRotationAngle(rotationAngle);
}

void Pedestrian::SetPosition(const glm::vec3& position)
{
    debug_assert(mPhysicsComponent);

    mPhysicsComponent->SetPosition(position);
}

float Pedestrian::ComputeDrawHeight(const glm::vec3& position, cxx::angle_t rotationAngle)
{
    float halfBox = PED_SPRITE_DRAW_BOX_SIZE * 0.5f;

    //glm::vec3 points[4] = {
    //    { 0.0f,     position.y + 0.01f, -halfBox },
    //    { halfBox,  position.y + 0.01f, 0.0f },
    //    { 0.0f,     position.y + 0.01f, halfBox },
    //    { -halfBox, position.y + 0.01f, 0.0f },
    //};

    glm::vec3 points[4] = {
        { -halfBox, position.y + 0.01f, -halfBox },
        { halfBox,  position.y + 0.01f, -halfBox },
        { halfBox,  position.y + 0.01f, halfBox },
        { -halfBox, position.y + 0.01f, halfBox },
    };

    float maxHeight = position.y;
    for (glm::vec3& currPoint: points)
    {
        //currPoint = glm::rotate(currPoint, angleRadians, glm::vec3(0.0f, -1.0f, 0.0f)); // dont rotate for peds
        currPoint.x += position.x;
        currPoint.z += position.z;

        // get height
        float height = gGameMap.GetHeightAtPosition(currPoint);
        if (height > maxHeight)
        {
            maxHeight = height;
        }
    }
#if 1
    // debug
    for (int i = 0; i < 4; ++i)
    {
        gRenderManager.mDebugRenderer.DrawLine(points[i], points[(i + 1) % 4], COLOR_RED);
    }
#endif

    // todo: get rid of magic numbers
    if (mCurrentStateID == ePedestrianState_SlideOnCar)
    {
        maxHeight += 0.35f;
    }

    return maxHeight + 0.01f;
}

void Pedestrian::SetAnimation(eSpriteAnimationID animation, eSpriteAnimLoop loopMode)
{
    if (mCurrentAnimID != animation)
    {
        mCurrentAnimState.SetNull();
        if (!gGameMap.mStyleData.GetSpriteAnimation(animation, mCurrentAnimState.mAnimData)) // todo
        {
            debug_assert(false);
        }
        mCurrentAnimID = animation;
    }
    mCurrentAnimState.PlayAnimation(loopMode);
}

void Pedestrian::SetCurrentState(ePedestrianState newStateID, bool isInitial)
{
    if (isInitial || newStateID != mCurrentStateID)
    {
        // exit previous state
        if (!isInitial)
        {
            PedestrianBaseState* prevState = gPedestrianBaseStatesManager.GetStateByID(mCurrentStateID);
            prevState->ProcessStateExit(this, newStateID);
        }
        ePedestrianState prevStateID = mCurrentStateID;
        mCurrentStateID = newStateID;
        mCurrentStateTime = 0;

        PedestrianBaseState* currState = gPedestrianBaseStatesManager.GetStateByID(mCurrentStateID);
        currState->ProcessStateEnter(this, prevStateID);
    }
}

void Pedestrian::ChangeWeapon(eWeaponType newWeapon)
{
    debug_assert(newWeapon < eWeaponType_COUNT);
    if (mWeaponsAmmo[newWeapon] == 0 || mCurrentWeapon == newWeapon)
        return;

    eWeaponType prevWeapon = mCurrentWeapon;
    mCurrentWeapon = newWeapon;

    // notify current state
    PedestrianBaseState* currState = gPedestrianBaseStatesManager.GetStateByID(mCurrentStateID);
    currState->ProcessStateWeaponChange(this, prevWeapon);
}
