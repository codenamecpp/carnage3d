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
#include "AiCharacterController.h"
#include "cvars.h"
#include "ImGuiHelpers.h"

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
    if (!gCarnageGame.IsInGameState())
        return;

    ImGuiWindowFlags wndFlags = ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus | 
        ImGuiWindowFlags_NoNav | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar;

    ImGuiIO& io = ImGui::GetIO();
    if (!ImGui::Begin(mWindowName, &mWindowShown, wndFlags))
    {
        ImGui::End();
        return;
    }

    Pedestrian* playerCharacter = gCarnageGame.mHumanPlayers[0]->mCharacter;
    glm::ivec3 characterLogPos = Convert::MetersToMapUnits(playerCharacter->mTransform.mPosition);

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("[ Create Cars ]"))
        {
            for (int icurr = 0; icurr < (int)gGameMap.mStyleData.mVehicles.size(); ++icurr)
            {
                ImGui::PushID(icurr);
                if (ImGui::MenuItem(cxx::enum_to_string(gGameMap.mStyleData.mVehicles[icurr].mModelID))) 
                {
                    CreateCarNearby(&gGameMap.mStyleData.mVehicles[icurr], playerCharacter);
                }
                ImGui::PopID();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("[ Create Peds ]"))
        {
            if (ImGui::MenuItem("Standing"))
            {
                Pedestrian* character = gTrafficManager.GenerateRandomTrafficPedestrian(characterLogPos.x, characterLogPos.y, characterLogPos.z);
                debug_assert(character);
                if (character && character->mController)
                {
                    // disable controller
                    character->mController->StopController();
                }
            }
            if (ImGui::MenuItem("Wandering"))
            {
                Pedestrian* character = gTrafficManager.GenerateRandomTrafficPedestrian(characterLogPos.x, characterLogPos.y, characterLogPos.z);
                debug_assert(character);
            }
            if (ImGui::MenuItem("Follower"))
            {
                Pedestrian* character = gTrafficManager.GenerateRandomTrafficPedestrian(characterLogPos.x, characterLogPos.y, characterLogPos.z);
                debug_assert(character);
                AiCharacterController* controller = (AiCharacterController*) character->mController;
                debug_assert(controller);
                if (controller)
                {
                    controller->FollowPedestrian(playerCharacter);
                }
            }
            if (ImGui::MenuItem("Hare Krishnas"))
            {
                Pedestrian* character = gTrafficManager.GenerateHareKrishnas(characterLogPos.x, characterLogPos.y, characterLogPos.z);
                debug_assert(character);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::HorzSpacing();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Frame Time: %.3f ms (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    
    // pedestrian stats
    if (playerCharacter)
    {
        ImGui::HorzSpacing();

        cxx::angle_t pedHeading = playerCharacter->mTransform.mOrientation;
        ImGui::Text("heading: %.1f degs", pedHeading.to_degrees_normalize_360());

        glm::vec3 pedPosition = playerCharacter->mTransform.mPosition;
        ImGui::Text("physical pos: %.3f, %.3f, %.3f", pedPosition.x, pedPosition.y, pedPosition.z);
        ImGui::Text("logical pos: %d, %d, %d", characterLogPos.x, characterLogPos.y, characterLogPos.z);

        const MapBlockInfo* blockInfo = gGameMap.GetBlockInfo(characterLogPos.x, characterLogPos.z, characterLogPos.y);
        ImGui::Text("block: %s", cxx::enum_to_string(blockInfo->mGroundType));
        if (blockInfo->mTrafficHint)
        {
            ImGui::Text("Traffic Hint: %s", cxx::enum_to_string(blockInfo->mTrafficHint));
        }

        if (Vehicle* currCar = playerCharacter->mCurrentCar)
        {
            ImGui::Text("  N: %s", blockInfo->mUpDirection ? "OK" : "--");
            ImGui::Text("  E: %s", blockInfo->mRightDirection ? "OK" : "--");
            ImGui::Text("  S: %s", blockInfo->mDownDirection ? "OK" : "--");
            ImGui::Text("  W: %s", blockInfo->mLeftDirection ? "OK" : "--");
            ImGui::Text("Car speed : %.3f", currCar->GetCurrentSpeed());
        }

        ImGui::Text("state: %s", cxx::enum_to_string(playerCharacter->GetCurrentStateID()));
        ImGui::HorzSpacing();

        ImGui::SliderInt("ped remap", &playerCharacter->mRemapIndex, -1, MAX_PED_REMAPS - 1);

        if (playerCharacter->IsCarPassenger())
        {
            ImGui::SliderInt("car remap", &playerCharacter->mCurrentCar->mRemapIndex, -1, MAX_CAR_REMAPS - 1);
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
                if (playerCharacter->IsCarPassenger())
                {
                    playerCharacter->mCurrentCar->SetPosition(Convert::MapUnitsToMeters(setLocalPosition));
                }
                else
                {
                    playerCharacter->SetPosition(Convert::MapUnitsToMeters(setLocalPosition));
                }
            }
            ImGui::EndPopup();
        }
    }

    { // choose camera modes
        const char* modeStrings[] = { "Follow", "Free Look" };
        static const ePlayerCameraMode CameraModesList[] =
        {
            ePlayerCameraMode_Follow,
            ePlayerCameraMode_FreeLook,
        }; 
        int currentCameraMode = 0;
        for (int i = 0; i < IM_ARRAYSIZE(CameraModesList); ++i)
        {
            if (gCarnageGame.mHumanPlayers[0]->GetCurrentCameraMode() == CameraModesList[i])
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
                    gCarnageGame.mHumanPlayers[0]->SetCurrentCameraMode(CameraModesList[n]);
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

    if (ImGui::CollapsingHeader("Draw"))
    {
        ImGui::Text("Map chunks drawn: %d", gRenderManager.mMapRenderer.mRenderStats.mBlockChunksDrawnCount);
        ImGui::Text("Sprites drawn: %d", gRenderManager.mMapRenderer.mRenderStats.mSpritesDrawnCount);
        ImGui::HorzSpacing();
        ImGui::Checkbox("Debug draw", &mEnableDebugDraw);
        ImGui::Checkbox("Decorations", &mEnableDrawDecorations);
        ImGui::SameLine(); ImGui::Checkbox("Obstacles", &mEnableDrawObstacles);
        ImGui::Checkbox("Pedestrians", &mEnableDrawPedestrians);
        ImGui::SameLine(); ImGui::Checkbox("Vehicles", &mEnableDrawVehicles);
        ImGui::Checkbox("City mesh", &mEnableDrawCityMesh);
    }

    if (ImGui::CollapsingHeader("Traffic"))
    {
        ImGui::HorzSpacing();
        ImGui::TextColored(ImVec4(1.0f,1.0f,0.0f,1.0f), "Pedestrians");
        ImGui::HorzSpacing();
        ImGui::Text("Current count: %d", gTrafficManager.CountTrafficPedestrians());
        ImGui::SliderInt("Max count##ped", &gGameParams.mTrafficGenMaxPeds, 0, 100);
        ImGui::SliderInt("Generation distance max##ped", &gGameParams.mTrafficGenPedsMaxDistance, 1, 10);
        ImGui::SliderInt("Generation chance##ped", &gGameParams.mTrafficGenPedsChance, 0, 100);
        ImGui::SliderFloat("Generation cooldown##ped", &gGameParams.mTrafficGenPedsCooldownTime, 0.5f, 5.0f, "%.1f");
        ImGui::Checkbox("Generation enabled##ped", &mEnableTrafficPedsGeneration);
        ImGui::HorzSpacing();
        ImGui::TextColored(ImVec4(1.0f,1.0f,0.0f,1.0f), "Cars");
        ImGui::HorzSpacing();
        ImGui::Text("Current count: %d", gTrafficManager.CountTrafficCars());
        ImGui::SliderInt("Max count##car", &gGameParams.mTrafficGenMaxCars, 0, 100);
        ImGui::SliderInt("Generation distance max##car", &gGameParams.mTrafficGenCarsMaxDistance, 1, 10);
        ImGui::SliderInt("Generation chance##car", &gGameParams.mTrafficGenCarsChance, 0, 100);
        ImGui::SliderFloat("Generation cooldown##car", &gGameParams.mTrafficGenCarsCooldownTime, 0.5f, 5.0f, "%.1f");
        ImGui::Checkbox("Generation enabled##car", &mEnableTrafficCarsGeneration);
    }

    if (ImGui::CollapsingHeader("Graphics"))
    {
        if (ImGui::Checkbox("Enable vsync", &gCvarGraphicsVSync.mValue))
        {
            gCvarGraphicsVSync.SetModified();
        }
        if (ImGui::Checkbox("Fullscreen", &gCvarGraphicsFullscreen.mValue))
        {
            gCvarGraphicsFullscreen.SetModified();
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

    glm::vec3 currPosition = pedestrian->mTransform.mPosition;
    currPosition.x += 0.5f;
    currPosition.z += 0.5f;

    Vehicle* vehicle = gGameObjectsManager.CreateVehicle(currPosition, cxx::angle_t {}, carStyle);
    debug_assert(vehicle);

    if (vehicle)
    {
        vehicle->mObjectFlags = (vehicle->mObjectFlags | GameObjectFlags_Traffic);
    }
}