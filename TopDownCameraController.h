#pragma once

class TopDownCameraController final
{
public:
    TopDownCameraController();

    // reset scene camera to defaults
    void SetupInitial();

    // process events and advance controller logic for single frame
    // @param deltaTime: Time since last frame
    void UpdateFrame(Timespan deltaTime);

    // process input event
    // @param inputEvent: Event data
    void InputEvent(KeyInputEvent& inputEvent);
    void InputEvent(MouseButtonInputEvent& inputEvent);
    void InputEvent(MouseMovedInputEvent& inputEvent);
    void InputEvent(MouseScrollInputEvent& inputEvent);

private:
    glm::vec3 mMoveDirection;
};