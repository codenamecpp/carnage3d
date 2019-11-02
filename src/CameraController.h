#pragma once

class GameCamera;

// base camera controller interface
class CameraController
{
public:
    virtual ~CameraController()
    {
    }
    // reset camera and controller both to default state
    virtual void Setup(GameCamera* gameCamera)
    {
    }
    // handle camera controller logic
    // @param deltaTime: Time since last frame
    virtual void UpdateFrame(Timespan deltaTime)
    {
    }
    // handle input events
    // @param inputEvent: Event data
    virtual void InputEvent(KeyInputEvent& inputEvent)
    {
    }
    virtual void InputEvent(MouseButtonInputEvent& inputEvent)
    {
    }
    virtual void InputEvent(MouseMovedInputEvent& inputEvent)
    {
    }
    virtual void InputEvent(MouseScrollInputEvent& inputEvent)
    {
    }

public:
    GameCamera* mCamera = nullptr;
};