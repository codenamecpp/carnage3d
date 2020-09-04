#pragma once

#include "GameCamera.h"

class GuiContext;

// base renderview class
class RenderView: public cxx::noncopyable
{
public:
    GameCamera mCamera;
    cxx::aabbox2d_t mOnScreenArea; // current visible map rectangle

public:
    RenderView() = default;
    virtual ~RenderView();

    virtual void DrawFrameBegin();
    virtual void DrawFrameEnd();
};