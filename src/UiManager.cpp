#include "stdafx.h"
#include "UiManager.h"
#include "RenderingManager.h"
#include "GpuProgram.h"
#include "SpriteManager.h"
#include "RenderView.h"
#include "UiContext.h"
#include "CarnageGame.h"
#include "ImGuiManager.h"

UiManager gUiManager;

bool UiManager::Initialize()
{
    if (!mSpriteBatch.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot initialize sprites batch");
        return false;
    }

    mCamera2D.SetIdentity();
    return true;
}

void UiManager::Deinit()
{
    mSpriteBatch.Deinit();
}

void UiManager::RenderFrame()
{
    mSpriteBatch.BeginBatch(SpriteBatch::DepthAxis_Z, eSpritesSortMode_None);

    Rect prevScreenRect = gGraphicsDevice.mViewportRect;
    Rect prevScissorsBox = gGraphicsDevice.mScissorBox;

    // draw renderviews
    {
        gGraphicsDevice.BindTexture(eTextureUnit_3, gSpriteManager.mPalettesTable);
        gGraphicsDevice.BindTexture(eTextureUnit_2, gSpriteManager.mPaletteIndicesTable);

        gRenderManager.mSpritesProgram.Activate();

        RenderStates guiRenderStates = RenderStates()
            .Disable(RenderStateFlags_FaceCulling)
            .Disable(RenderStateFlags_DepthTest);
        gGraphicsDevice.SetRenderStates(guiRenderStates);

        for (int icurr = 0; icurr < gCarnageGame.mNumPlayers; ++icurr)
        {   
            CarnageGame::HumanCharacterSlot& currPlayer = gCarnageGame.mHumanSlot[icurr];
            mCamera2D.SetIdentity();
            mCamera2D.mViewportRect = currPlayer.mCharView.mCamera.mViewportRect;
            mCamera2D.SetProjection(0.0f, mCamera2D.mViewportRect.w * 1.0f, mCamera2D.mViewportRect.h * 1.0f, 0.0f);

            gGraphicsDevice.SetViewportRect(mCamera2D.mViewportRect);
            gGraphicsDevice.SetScissorRect(mCamera2D.mViewportRect);

            gRenderManager.mSpritesProgram.UploadCameraTransformMatrices(mCamera2D);

            UiContext uiContext ( mCamera2D, mSpriteBatch );
            currPlayer.mCharView.mHUD.DrawFrame(uiContext);
            mSpriteBatch.Flush();
        }

        gRenderManager.mSpritesProgram.Deactivate();
    }

    { // draw imgui

        mCamera2D.SetIdentity();
        mCamera2D.mViewportRect = prevScreenRect;
        mCamera2D.SetProjection(0.0f, mCamera2D.mViewportRect.w * 1.0f, mCamera2D.mViewportRect.h * 1.0f, 0.0f);

        gRenderManager.mGuiTexColorProgram.Activate();
        gRenderManager.mGuiTexColorProgram.UploadCameraTransformMatrices(mCamera2D);
    
        RenderStates guiRenderStates = RenderStates()
            .Disable(RenderStateFlags_FaceCulling)
            .Disable(RenderStateFlags_DepthTest)
            .SetAlphaBlend(eBlendMode_Alpha);
        gGraphicsDevice.SetRenderStates(guiRenderStates);

        gGraphicsDevice.SetScissorRect(prevScissorsBox);
        gGraphicsDevice.SetViewportRect(prevScreenRect);

        gImGuiManager.RenderFrame();

        gRenderManager.mGuiTexColorProgram.Deactivate();
    }
}

void UiManager::UpdateFrame()
{
    // do nothing
}

void UiManager::InputEvent(MouseMovedInputEvent& inputEvent)
{
    // do nothing
}

void UiManager::InputEvent(MouseScrollInputEvent& inputEvent)
{
    // do nothing
}

void UiManager::InputEvent(MouseButtonInputEvent& inputEvent)
{
    // do nothing
}

void UiManager::InputEvent(KeyInputEvent& inputEvent)
{
    // do nothing
}

void UiManager::InputEvent(KeyCharEvent& inputEvent)
{
    // do nothing
}

void UiManager::InputEvent(GamepadInputEvent& inputEvent)
{
    // do nothing
}
