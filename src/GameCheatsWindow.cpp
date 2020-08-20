#include "stdafx.h"
#include "GameCheatsWindow.h"
#include "imgui.h"
#include "RenderingManager.h"
#include "PhysicsManager.h"
#include "CarnageGame.h"
#include "Pedestrian.h"

namespace ImGui
{
    inline void HorzSpacing(float spacingSize = 10.0f)
    {
        Dummy(ImVec2(0.0f, spacingSize));
    }
    inline void VertSpacing(float spacingSize = 10.0f)
    {
        Dummy(ImVec2(spacingSize, 0.0f)); 
    }
}

GameCheatsWindow gGameCheatsWindow;

GameCheatsWindow::GameCheatsWindow()
    : DebugWindow("Game Cheats")
    , mEnableMapCollisions(true)
    , mEnableGravity(true)
    , mEnableBlocksAnimation(true)
    , mEnableDebugDraw(false)
{
}

void GameCheatsWindow::DoUI(ImGuiIO& imguiContext)
{
    ImGuiWindowFlags wndFlags = ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus | 
        ImGuiWindowFlags_NoNav | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar;

    ImGuiIO& io = ImGui::GetIO();
    if (!ImGui::Begin(mWindowName, &mWindowShown, wndFlags))
    {
        ImGui::End();
        return;
    }

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("[ Create Car ]"))
        {
            for (int icurr = 0; icurr < (int)gGameMap.mStyleData.mCars.size(); ++icurr)
            {
                ImGui::PushID(icurr);
                if (ImGui::MenuItem(cxx::enum_to_string(gGameMap.mStyleData.mCars[icurr].mModelId))) 
                {
                    CreateCarNearby(&gGameMap.mStyleData.mCars[icurr], gCarnageGame.mHumanSlot[0].mCharPedestrian);
                }
                ImGui::PopID();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("[ Select map ]"))
        {
            for (int icurr = 0; icurr < (int)gFiles.mGameMapsList.size(); ++icurr)
            {
                ImGui::PushID(icurr);
                if (ImGui::MenuItem(gFiles.mGameMapsList[icurr].c_str())) 
                {
                    gCarnageGame.DebugChangeMap(gFiles.mGameMapsList[icurr]);
                }
                ImGui::PopID();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Frame Time: %.3f ms (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::Text("Block chunks drawn: %d", gRenderManager.mMapRenderer.mRenderStats.mBlockChunksDrawnCount);
    
    // pedestrian stats
    if (Pedestrian* pedestrian = gCarnageGame.mHumanSlot[0].mCharPedestrian)
    {
        ImGui::HorzSpacing();
        glm::vec3 pedPosition = pedestrian->mPhysicsBody->GetPosition();
        ImGui::Text("physical pos: %.3f, %.3f, %.3f", pedPosition.x, pedPosition.y, pedPosition.z);
        glm::vec3 logicalPosition = Convert::MetersToMapUnits(pedPosition);
        ImGui::Text("logical pos: %.3f, %.3f, %.3f", logicalPosition.x, logicalPosition.y, logicalPosition.z);

        cxx::angle_t pedHeading = pedestrian->mPhysicsBody->GetRotationAngle();
        ImGui::Text("heading: %f", pedHeading.mDegrees);
        ImGui::Text("weapon: %s", cxx::enum_to_string(pedestrian->mCurrentWeapon));
        ImGui::Text("state: %s", cxx::enum_to_string(pedestrian->GetCurrentStateID()));
        ImGui::HorzSpacing();

        // get block location
        glm::ivec3 blockPosition = Convert::MetersToMapUnits(pedPosition);

        BlockStyle* currBlock = gGameMap.GetBlockClamp(blockPosition.x, blockPosition.z, blockPosition.y);

        ImGui::Text("b ground: %s", cxx::enum_to_string(currBlock->mGroundType));
        ImGui::Text("b slope: %d", currBlock->mSlopeType);
        ImGui::Text("b directions: %d, %d, %d, %d", currBlock->mUpDirection, currBlock->mRightDirection, 
            currBlock->mDownDirection, currBlock->mLeftDirection);

        ImGui::HorzSpacing();
        ImGui::SliderInt("ped remap", &pedestrian->mRemapIndex, -1, MAX_PED_REMAPS - 1);

        if (pedestrian->IsCarPassenger())
        {
            ImGui::SliderInt("car remap", &pedestrian->mCurrentCar->mRemapIndex, -1, MAX_CAR_REMAPS - 1);
        }
    }

    const char* modeStrings[] = { "Follow", "Free Look" };
    CameraController* modeControllers[] =
    {
        &gCarnageGame.mHumanSlot[0].mCharView.mFollowCameraController, 
        &gCarnageGame.mHumanSlot[0].mCharView.mFreeLookCameraController,
    }; 
    int currentCameraMode = 0;
    for (int i = 0; i < IM_ARRAYSIZE(modeControllers); ++i)
    {
        if (gCarnageGame.mHumanSlot[0].mCharView.mCameraController == modeControllers[i])
        {
            currentCameraMode = i;
            break;
        }
    }
    const char* item_current = modeStrings[currentCameraMode];
    if (ImGui::BeginCombo("Camera mode", item_current))
    {
        for (int n = 0; n < IM_ARRAYSIZE(modeStrings); n++)
        {
            bool is_selected = (item_current == modeStrings[n]);
            if (ImGui::Selectable(modeStrings[n], is_selected))
            {
                item_current = modeStrings[n];
                gCarnageGame.mHumanSlot[0].mCharView.SetCameraController(modeControllers[n]);
            }
            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    //if (ImGui::CollapsingHeader("Physics"))
    //{
    //    ImGui::Checkbox("Enable map collisions", &mEnableMapCollisions);
    //    ImGui::Checkbox("Enable gravity", &mEnableGravity);
    //}

    if (ImGui::CollapsingHeader("Map Draw"))
    {
        ImGui::Checkbox("Enable blocks animation", &mEnableBlocksAnimation);
        ImGui::Checkbox("Enable debug draw", &mEnableDebugDraw);

        ImGui::Checkbox("Draw decorations", &mEnableDrawDecorations);
        ImGui::Checkbox("Draw obstacles", &mEnableDrawObstacles);
        ImGui::Checkbox("Draw pedestrians", &mEnableDrawPedestrians);
        ImGui::Checkbox("Draw vehicles", &mEnableDrawVehicles);
        ImGui::Checkbox("Draw city mesh", &mEnableDrawCityMesh);
    }

    if (ImGui::CollapsingHeader("Ped"))
    {
        ImGui::SliderFloat("Turn speed (degs/s)", &gGameParams.mPedestrianTurnSpeed, 10.0f, 640.0f, "%.2f");
        ImGui::SliderFloat("Run speed (m/s)", &gGameParams.mPedestrianRunSpeed, 
            Convert::MapUnitsToMeters(0.1f), 
            Convert::MapUnitsToMeters(16.0f), "%.2f");
        ImGui::SliderFloat("Walk speed (m/s)", &gGameParams.mPedestrianWalkSpeed, 
            Convert::MapUnitsToMeters(0.1f), 
            Convert::MapUnitsToMeters(16.0f), "%.2f");
    }

    if (Pedestrian* pedestrian = gCarnageGame.mHumanSlot[0].mCharPedestrian)
    {
        if (Vehicle* currCar = pedestrian->mCurrentCar)
        {
            CarStyle* carInformation = currCar->mCarStyle;

            if (ImGui::CollapsingHeader("Vehicle Info"))
            {
                ImVec4 physicsPropsColor(0.75f, 0.75f, 0.75f, 1.0f);
                ImGui::Text("VType - %s", cxx::enum_to_string(carInformation->mVType));
                ImGui::HorzSpacing();
                ImGui::TextColored(physicsPropsColor, "Turning : %d", carInformation->mTurning);
                ImGui::TextColored(physicsPropsColor, "Turn Ratio : %d", carInformation->mTurnRatio);
                ImGui::TextColored(physicsPropsColor, "Moment : %d", carInformation->mMoment);
                ImGui::TextColored(physicsPropsColor, "Mass : %.3f", carInformation->mMass);
                ImGui::TextColored(physicsPropsColor, "Thurst : %.3f", carInformation->mThrust);
                ImGui::TextColored(physicsPropsColor, "Tyre Adhesion X/Y : %.3f / %.3f", carInformation->mTyreAdhesionX, carInformation->mTyreAdhesionY);
                ImGui::HorzSpacing();
                ImGui::TextColored(physicsPropsColor, "Handbrake Friction : %.3f", carInformation->mHandbrakeFriction);
                ImGui::TextColored(physicsPropsColor, "Footbrake Friction : %.3f", carInformation->mFootbrakeFriction);
                ImGui::TextColored(physicsPropsColor, "Front Brake Bias : %.3f", carInformation->mFrontBrakeBias);
                ImGui::HorzSpacing();

                ImGui::Text("Current velocity : %.3f", currCar->mPhysicsBody->GetCurrentSpeed());
                if (ImGui::Button("Clear forces"))
                {
                    currCar->mPhysicsBody->ClearForces();
                }
            }
        }
    }

    if (ImGui::CollapsingHeader("Graphics"))
    {
        if (ImGui::Checkbox("Enable vsync", &gSystem.mConfig.mEnableVSync))
        {
            gGraphicsDevice.EnableVSync(gSystem.mConfig.mEnableVSync);
        }
        if (ImGui::Checkbox("Fullscreen", &gSystem.mConfig.mFullscreen))
        {
            gGraphicsDevice.EnableFullscreen(gSystem.mConfig.mFullscreen);
            gGraphicsDevice.EnableVSync(gSystem.mConfig.mEnableVSync); // set vsync param as fullcreen mode changes
        }
    }

    ImGui::End();
}

void GameCheatsWindow::CreateCarNearby(CarStyle* carStyle, Pedestrian* pedestrian)
{
    if (carStyle == nullptr || pedestrian == nullptr)
        return;

    glm::vec3 currPosition = pedestrian->mPhysicsBody->GetPosition();
    currPosition.x += 0.5f;
    currPosition.z += 0.5f;

    gGameObjectsManager.CreateCar(currPosition, cxx::angle_t::from_degrees(25.0f), carStyle);
}