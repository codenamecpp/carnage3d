#include "stdafx.h"
#include "GuiContext.h"

bool GuiContext::EnterChildClipArea(const Rect& rcLocal)
{
    Rect newCliprect = rcLocal;
    TransformClipRect(newCliprect);

    Rect currentCliprect = gGraphicsDevice.mScissorBox;

    newCliprect = newCliprect.GetIntersection(currentCliprect);
    if (newCliprect.h < 1 || newCliprect.w < 1)
        return false;

    mClipRectsStack.push_back(currentCliprect);
    if (newCliprect != currentCliprect)
    {
        mSpriteBatch.Flush();
        gGraphicsDevice.SetScissorRect(newCliprect);
    }
    return true;
}

void GuiContext::LeaveChildClipArea()
{
    if (mClipRectsStack.empty())
    {
        debug_assert(false);
        return;
    }

    Rect prevCliprect = mClipRectsStack.back();
    mClipRectsStack.pop_back();

    Rect currentCliprect = gGraphicsDevice.mScissorBox;

    if (currentCliprect != prevCliprect)
    {
        mSpriteBatch.Flush();
        gGraphicsDevice.SetScissorRect(prevCliprect);
    }
}

void GuiContext::TransformClipRect(Rect& rectangle) const
{
    rectangle.x += mCamera.mViewportRect.x;
    rectangle.y = gGraphicsDevice.mScreenResolution.y - (mCamera.mViewportRect.y + rectangle.y + rectangle.h);
   // rectangle.y = gGraphicsDevice.mScreenResolution.y - (rectangle.y + rectangle.h);

    //rectangle.y = gGraphicsDevice.mScreenResolution.y - (rectangle.y + mCamera.mViewportRect.y + rectangle.h);
    //rectangle.y -= gGraphicsDevice.mScreenResolution.y - (rectangle.y + rectangle.h);
}
