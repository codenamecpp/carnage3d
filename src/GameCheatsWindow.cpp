#include "stdafx.h"
#include "GameCheatsWindow.h"
#include "imgui.h"
#include "RenderingManager.h"
#include "PhysicsManager.h"
#include "CarnageGame.h"
#include "Pedestrian.h"
#include "TimeManager.h"
#include "AiManager.h"
#include "TrafficManager.h"

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

    Pedestrian* playerChar = gCarnageGame.mHumanSlot[0].mCharPedestrian;

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("[ Create Cars ]"))
        {
            for (int icurr = 0; icurr < (int)gGameMap.mStyleData.mVehicles.size(); ++icurr)
            {
                ImGui::PushID(icurr);
                if (ImGui::MenuItem(cxx::enum_to_string(gGameMap.mStyleData.mVehicles[icurr].mModelId))) 
                {
                    CreateCarNearby(&gGameMap.mStyleData.mVehicles[icurr], playerChar);
                }
                ImGui::PopID();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("[ Create Peds ]"))
        {
            if (ImGui::MenuItem("Basic standing"))
            {
                glm::vec3 pos = playerChar->mPhysicsBody->GetPosition();
                pos.x += Convert::MapUnitsToMeters(2.0f * gCarnageGame.mGameRand.generate_float() - 1.0f);
                pos.z += Convert::MapUnitsToMeters(2.0f * gCarnageGame.mGameRand.generate_float() - 1.0f);
                Pedestrian* character = gGameObjectsManager.CreatePedestrian(pos, cxx::angle_t(), 0);
                debug_assert(character);
                character->mFlags = character->mFlags | eGameObjectFlags_Traffic;
            }
            if (ImGui::MenuItem("Basic walking"))
            {
                glm::vec3 pos = playerChar->mPhysicsBody->GetPosition();
                pos.x += Convert::MapUnitsToMeters(2.0f * gCarnageGame.mGameRand.generate_float() - 1.0f);
                pos.z += Convert::MapUnitsToMeters(2.0f * gCarnageGame.mGameRand.generate_float() - 1.0f);
                Pedestrian* character = gGameObjectsManager.CreatePedestrian(pos, cxx::angle_t(), 1);
                debug_assert(character);
                character->mFlags = character->mFlags | eGameObjectFlags_Traffic;
                gAiManager.CreateAiController(character);
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

    ImGui::HorzSpacing();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Frame Time: %.3f ms (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::Text("Map chunks drawn: %d", gRenderManager.mMapRenderer.mRenderStats.mBlockChunksDrawnCount);
    ImGui::Text("Sprites drawn: %d", gRenderManager.mMapRenderer.mRenderStats.mSpritesDrawnCount);
    
    // pedestrian stats
    if (playerChar)
    {
        ImGui::HorzSpacing();

        glm::vec3 pedPosition = playerChar->mPhysicsBody->GetPosition();
        ImGui::Text("physical pos: %.3f, %.3f, %.3f", pedPosition.x, pedPosition.y, pedPosition.z);

        glm::ivec3 logPosition = playerChar->GetLogicalPosition();
        ImGui::Text("logical pos: %d, %d, %d", logPosition.x, logPosition.y, logPosition.z);

        MapBlockInfo* blockInfo = gGameMap.GetBlockClamp(logPosition.x, logPosition.z, logPosition.y);
        ImGui::Text("block: %s", cxx::enum_to_string(blockInfo->mGroundType));

        cxx::angle_t pedHeading = playerChar->mPhysicsBody->GetRotationAngle();
        ImGui::Text("heading: %.1f degs", pedHeading.to_degrees_normalize_360());

        ImGui::Text("weapon: %s", cxx::enum_to_string(playerChar->mCurrentWeapon));
        ImGui::Text("state: %s", cxx::enum_to_string(playerChar->GetCurrentStateID()));
        ImGui::HorzSpacing();

        ImGui::SliderInt("ped remap", &playerChar->mRemapIndex, -1, MAX_PED_REMAPS - 1);

        if (playerChar->IsCarPassenger())
        {
            ImGui::SliderInt("car remap", &playerChar->mCurrentCar->mRemapIndex, -1, MAX_CAR_REMAPS - 1);
        }

        static glm::vec3 setLocalPosition;
        if (ImGui::Button("Set position..."))
        {
            ImGui::OpenPopup("Set player position");

            setLocalPosition = Convert::MetersToMapUnits(pedPosition);
        }

        if (ImGui::BeginPopup("Set player position"))
        {
            ImGui::InputFloat3("Logical position (x, y, z)", &setLocalPosition[0]);
            if (ImGui::Button("Set Position"))
            {
                if (playerChar->IsCarPassenger())
                {
                    playerChar->mCurrentCar->mPhysicsBody->SetPosition(Convert::MapUnitsToMeters(setLocalPosition));
                }
                else
                {
                    playerChar->mPhysicsBody->SetPosition(Convert::MapUnitsToMeters(setLocalPosition));
                }
            }
            ImGui::EndPopup();
        }
    }

    { // choose camera modes
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
                bool is_selected = (currentCameraMode == n);
                if (ImGui::Selectable(modeStrings[n], is_selected))
                {
                    currentCameraMode = n;
                    gCarnageGame.mHumanSlot[0].mCharView.SetCameraController(modeControllers[n]);
                }
                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
    }

    ImGui::HorzSpacing();

    if (ImGui::CollapsingHeader("Physics"))
    {
        //ImGui::Checkbox("Enable map collisions", &mEnableMapCollisions);
        ImGui::Checkbox("Enable gravity", &mEnableGravity);
    }

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

    if (playerChar)
    {
        if (Vehicle* currCar = playerChar->mCurrentCar)
        {
            VehicleInfo* carInformation = currCar->mCarStyle;

            if (ImGui::CollapsingHeader("Vehicle Info"))
            {
                ImVec4 physicsPropsColor(0.75f, 0.75f, 0.75f, 1.0f);
                ImGui::Text("VType - %s", cxx::enum_to_string(carInformation->mClassID));
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

    if (ImGui::CollapsingHeader("Traffic"))
    {
        ImGui::HorzSpacing();
        ImGui::TextColored(ImVec4(1.0f,1.0f,0.0f,1.0f), "Pedestrians");
        ImGui::HorzSpacing();
        ImGui::Text("Current count: %d", gTrafficManager.CountTrafficPedestrians());
        ImGui::SliderInt("Max count", &gGameParams.mTrafficGenMaxPeds, 0, 100);
        ImGui::SliderInt("Generation distance max", &gGameParams.mTrafficGenPedsMaxDistance, 1, 10);
        ImGui::SliderInt("Generation chance", &gGameParams.mTrafficGenPedsChance, 0, 100);
        ImGui::SliderFloat("Generation cooldown", &gGameParams.mTrafficGenPedsCooldownTime, 0.5f, 5.0f, "%.1f");
        ImGui::Checkbox("Generation enabled", &mEnableTrafficPedsGeneration);
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

        ImGui::HorzSpacing();

        { // choose desired framerate
            static const char* framerates_str[] = {"24", "30", "60", "120", "240"};
            static int framerates[] = {24, 30, 60, 120, 240};
            static int framerate_index = 3;
            if (ImGui::BeginCombo("Framerate", framerates_str[framerate_index]))
            {
                for (int n = 0; n < IM_ARRAYSIZE(framerates_str); ++n)
                {
                    bool is_selected = (n == framerate_index);
                    if (ImGui::Selectable(framerates_str[n], is_selected))
                    {
                        framerate_index = n;
                        gTimeManager.SetMaxFramerate(framerates[framerate_index] * 1.0f);
                    }
                    if (is_selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
        }
    }

    ImGui::End();
}

void GameCheatsWindow::CreateCarNearby(VehicleInfo* carStyle, Pedestrian* pedestrian)
{
    if (carStyle == nullptr || pedestrian == nullptr)
        return;

    glm::vec3 currPosition = pedestrian->mPhysicsBody->GetPosition();
    currPosition.x += 0.5f;
    currPosition.z += 0.5f;

    gGameObjectsManager.CreateVehicle(currPosition, cxx::angle_t::from_degrees(25.0f), carStyle);
}