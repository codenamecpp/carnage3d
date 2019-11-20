#pragma once

#include "GameCamera.h"

class UiContext;

// base renderview class
class RenderView: public cxx::noncopyable
{
public:
    GameCamera mCamera;

public:
    RenderView() = default;
    virtual ~RenderView();

    virtual void DrawFrameBegin();
    virtual void DrawFrameEnd();
};