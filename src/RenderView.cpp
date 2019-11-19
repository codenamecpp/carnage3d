#include "stdafx.h"
#include "RenderView.h"
#include "GraphicsDevice.h"
#include "SpriteManager.h"
#include "GameMapManager.h"

RenderView::~RenderView()
{
}

void RenderView::DrawFrameBegin()
{
    mCamera.ComputeMatricesAndFrustum();
   
    gGraphicsDevice.SetViewportRect(mCamera.mViewportRect);
}

void RenderView::DrawFrameEnd()
{
}

void RenderView::OnDrawUi(UiContext& uiContext)
{
}
