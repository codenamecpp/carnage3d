#include "stdafx.h"
#include "GuiContext.h"

GuiContext::GuiContext(const Rect& virtualScreenRect, const Rect& viewportRect, SpriteBatch& spriteBatch)
    : mVirtualScreenRect(virtualScreenRect)
    , mViewportRect(viewportRect)
    , mSpriteBatch(spriteBatch)
{
    debug_assert(mVirtualScreenRect.w > 0.0f);
    debug_assert(mVirtualScreenRect.h > 0.0f);

    // precompute ui scale
    mUiScale.x = (mVirtualScreenRect.w > 0.0f) ? (static_cast<float>(mViewportRect.w) / mVirtualScreenRect.w) : 1.0f;
    mUiScale.y = (mVirtualScreenRect.h > 0.0f) ? (static_cast<float>(mViewportRect.h) / mVirtualScreenRect.h) : 1.0f;
}

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
    // assume logical rectangle, (0..mVirtualScreenRect)

    rectangle.x = mViewportRect.x + (int)std::lround(rectangle.x * mUiScale.x);
    rectangle.y = mViewportRect.y + (int)std::lround(rectangle.y * mUiScale.y);
    rectangle.w = static_cast<int>(std::lround(rectangle.w * mUiScale.x));
    rectangle.h = static_cast<int>(std::lround(rectangle.h * mUiScale.y));

    rectangle.y = gGraphicsDevice.mScreenResolution.y - (rectangle.y + rectangle.h);
}
