#pragma once

class GuiContext;

//////////////////////////////////////////////////////////////////////////

class GuiScreen: public cxx::noncopyable
{
public:
    Rect mScreenArea;

public:
    GuiScreen() = default;
    virtual ~GuiScreen()
    {
    }
    // update frame
    virtual void UpdateScreen()
    {
    }
    // render frame
    virtual void DrawScreen(GuiContext& context)
    {
    }
};

//////////////////////////////////////////////////////////////////////////