#include "stdafx.h"
#include "UiManager.h"
#include "ImGuiManager.h"
#include "RenderingManager.h"
#include "GpuProgram.h"
#include "SpriteManager.h"
#include "RenderView.h"
#include "UiContext.h"

UiManager gUiManager;

bool UiManager::Initialize()
{
    if (!mSpriteBatch.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot initialize sprites batch");
        return false;
    }

    gImGuiManager.Initialize();
    mCamera2D.SetIdentity();
    return true;
}

void UiManager::Deinit()
{
    mSpriteBatch.Deinit();
    gImGuiManager.Deinit();
}

void UiManager::RenderFrame()
{
    mSpriteBatch.BeginBatch(SpriteBatch::DepthAxis_Z);

    Rect2D prevScreenRect = gGraphicsDevice.mViewportRect;
    Rect2D prevScissorsBox = gGraphicsDevice.mScissorBox;

    // draw renderviews
    {
        gGraphicsDevice.BindTexture(eTextureUnit_3, gSpriteManager.mPalettesTable);
        gGraphicsDevice.BindTexture(eTextureUnit_2, gSpriteManager.mPaletteIndicesTable);

        gRenderManager.mSpritesProgram.Activate();

        RenderStates guiRenderStates = RenderStates()
            .Disable(RenderStateFlags_FaceCulling)
            .Disable(RenderStateFlags_DepthTest);
        gGraphicsDevice.SetRenderStates(guiRenderStates);

        for (RenderView* currRenderview: gRenderManager.mActiveRenderViews)
        {
            mCamera2D.SetIdentity();
            mCamera2D.mViewportRect = currRenderview->mCamera.mViewportRect;
            mCamera2D.SetProjection(0.0f, mCamera2D.mViewportRect.w * 1.0f, mCamera2D.mViewportRect.h * 1.0f, 0.0f);

            gGraphicsDevice.SetViewportRect(mCamera2D.mViewportRect);
            gGraphicsDevice.SetScissorRect(mCamera2D.mViewportRect);

            gRenderManager.mSpritesProgram.UploadCameraTransformMatrices(mCamera2D);

            UiContext uiContext ( mCamera2D, mSpriteBatch );
            currRenderview->OnDrawUi(uiContext);
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

void UiManager::UpdateFrame(Timespan deltaTime)
{
    gImGuiManager.UpdateFrame(deltaTime);
}

void UiManager::HandleEvent(MouseMovedInputEvent& inputEvent)
{
    gImGuiManager.HandleEvent(inputEvent);
}

void UiManager::HandleEvent(MouseScrollInputEvent& inputEvent)
{
    gImGuiManager.HandleEvent(inputEvent);
}

void UiManager::HandleEvent(MouseButtonInputEvent& inputEvent)
{
    gImGuiManager.HandleEvent(inputEvent);
}

void UiManager::HandleEvent(KeyInputEvent& inputEvent)
{
    gImGuiManager.HandleEvent(inputEvent);
}

void UiManager::HandleEvent(KeyCharEvent& inputEvent)
{
    gImGuiManager.HandleEvent(inputEvent);
}