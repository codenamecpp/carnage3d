#pragma once

// Represents audio listener in 3d space
class AudioListener final: public cxx::noncopyable
{
    friend class AudioDevice;

public:
    // readonly
    glm::vec3 mPosition;

public:
    AudioListener() 
        : mPosition(0.0f, 0.0f, 1.0f)
    {
    }
    // Set audio listener position in 3d space
    void SetPosition(const glm::vec3& position)
    {
        mPosition = position;
    }
    void SetPosition2(const glm::vec2& position)
    {
        mPosition.x = position.x;
        mPosition.y = position.y;
    }
};