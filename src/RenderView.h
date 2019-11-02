#pragma once

#include "GameCamera.h"

class RenderView: public cxx::noncopyable
{
public:
    RenderView() = default;
    virtual ~RenderView()
    {
    }
    virtual void PreRender();
    virtual void PostRender();

public:
    GameCamera mRenderCamera;
};