#pragma once

#include "GuiDefs.h"

class GuiContext
{
public:
    Rect mVirtualScreenRect {};
    Rect mViewportRect {};

    glm::vec2 mUiScale {1.0f, 1.0f};

    SpriteBatch& mSpriteBatch;

public:
    GuiContext(const Rect& virtualScreenRect, const Rect& viewportRect, SpriteBatch& spriteBatch);

    // helpers
    inline int GetScreenSizex() const { return mVirtualScreenRect.w; }
    inline int GetScreenSizey() const { return mVirtualScreenRect.h; }

    inline glm::vec2 ScreenToVirtualPoint(const glm::vec2& screenPoint) const
    {
        return glm::vec2 {
            (screenPoint.x - static_cast<float>(mViewportRect.x)) / mUiScale.x,
            (screenPoint.y - static_cast<float>(mViewportRect.y)) / mUiScale.y
        };
    }

    inline glm::vec2 VirtualToScreenPoint(const glm::vec2& virtualPoint) const
    {
        return glm::vec2 {
            static_cast<float>(mViewportRect.x) + virtualPoint.x * mUiScale.x,
            static_cast<float>(mViewportRect.y) + virtualPoint.y * mUiScale.y
        };
    }

    inline Point VirtualSizeToScreen(const glm::vec2& virtualSize) const
    {
        return Point {
            static_cast<int>(std::lround(virtualSize.x * mUiScale.x)),
            static_cast<int>(std::lround(virtualSize.y * mUiScale.y))
        };
    }

    inline glm::vec2 ScreenSizeToVirtual(const Point& screenSize) const
    {
        return glm::vec2 {
            static_cast<float>(screenSize.x) / mUiScale.x,
            static_cast<float>(screenSize.y) / mUiScale.y
        };
    }

    bool EnterChildClipArea(const Rect& rcLocal);
    void LeaveChildClipArea();

private:
    void TransformClipRect(Rect& rectangle) const;

private:
    std::vector<Rect> mClipRectsStack;
};