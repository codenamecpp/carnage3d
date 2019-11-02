#include "stdafx.h"
#include "RenderView.h"
#include "GraphicsDevice.h"

void RenderView::PreRender()
{
    mRenderCamera.ComputeMatricesAndFrustum();
   
    gGraphicsDevice.SetViewportRect(mRenderCamera.mViewportRect);
}

void RenderView::PostRender()
{
}