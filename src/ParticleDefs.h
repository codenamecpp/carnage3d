#pragma once

#include "GraphicsDefs.h"

// defines single particle draw vertex
struct ParticleVertex
{
public:
    ParticleVertex() = default;
    inline void SetPosition(float posx, float posy, float posz)
    {
        mPositionSize.x = posx;
        mPositionSize.y = posy;
        mPositionSize.z = posz;
    }
    inline void SetParticleSize(float particleSize)
    {
        mPositionSize.w = particleSize;
    }
public:
    glm::vec4 mPositionSize; // 16 bytes
    Color32 mColor; // 4 bytes
};

const unsigned int Sizeof_ParticleVertex = sizeof(ParticleVertex);

// defines draw vertex format of city mesh
struct ParticleVertex_Format: public VertexFormat
{
public:
    ParticleVertex_Format()
    {
        Setup();
    }
    // get format definition
    static const ParticleVertex_Format& Get() 
    { 
        static const ParticleVertex_Format sDefinition; 
        return sDefinition; 
    }
    using TVertexType = ParticleVertex;
    // initialzie definition
    inline void Setup()
    {
        this->mDataStride = Sizeof_ParticleVertex;
        this->SetAttribute(eVertexAttribute_Position0, eVertexAttributeFormat_4F, offsetof(TVertexType, mPositionSize));
        this->SetAttribute(eVertexAttribute_Color0, eVertexAttributeFormat_4UB, offsetof(TVertexType, mColor));
        this->SetAttributeNormalized(eVertexAttribute_Color0);
    }
};

enum eParticleState
{
    eParticleState_Alive,
    eParticleState_Fade,
    eParticleState_Dead,
};

// defines single particle state
struct Particle
{
public:
    Particle() = default;
    void Clear()
    {
        mStartPosition.x = 0.0f; 
        mStartPosition.y = 0.0f; 
        mStartPosition.z = 0.0f;

        mPosition.x = 0.0f; 
        mPosition.y = 0.0f; 
        mPosition.z = 0.0f;

        mVelocity.x = 0.0f; 
        mVelocity.y = 0.0f; 
        mVelocity.z = 0.0f;

        mSize = 1.0f;
        mAge = 0.0f;
        mLifeTime = 0.0f;
        mColor = Color32_White;
        mState = eParticleState_Alive;
    }
public:
    glm::vec3 mStartPosition;
    glm::vec3 mPosition;
    glm::vec3 mVelocity;
    float mSize = 1.0f;
    float mAge = 0.0f; // current age, in seconds
    float mLifeTime = 0.0f; // time duration of how long particle will live, in seconds
    eParticleState mState = eParticleState_Alive;
    Color32 mColor = Color32_White;
};

const unsigned int Sizeof_Particle = sizeof(Particle);

enum eParticleEmitterShape
{
    eParticleEmitterShape_Point,
    eParticleEmitterShape_Box,
};

struct ParticleEmitterShape
{
public:
    ParticleEmitterShape() = default;
public:
    eParticleEmitterShape mShape = eParticleEmitterShape_Point;
    glm::vec3 mPoint; // valid if spawn area is eParticleEmitterShape_Point
    cxx::aabbox_t mBox; // valid if spawn area is eParticleEmitterShape_Box
};

enum eParticleSpace
{
    eParticleSpace_Local, // particle position relative to emitter center
    eParticleSpace_Global,
};

struct ParticleEffectParams
{
public:
    ParticleEffectParams() = default;

    bool IsFadeoutOnDie() const { return mParticleFadeoutDuration > 0.0f; }

public:
    eParticleSpace mParticleSpace = eParticleSpace_Local;
    int mMaxParticlesCount = 0;
    float mParticleEmitFrequency = 1.0f; // particles per second
    float mParticleFadeoutDuration = 0.0f; // fade out on die, seconds
    // movement control
    glm::vec2 mParticleHorzVelocityRange; // x/z
    glm::vec2 mParticleVertVelocityRange; // y
    glm::vec3 mParticlesGravity;
    // initial size control
    glm::vec2 mParticleSizeRange { 1.0f, 1.0f };
    // lifetime control
    glm::vec2 mParticleLifetimeRange { 1.0f };
    Color32 mParticleColor = Color32_White; // todo: color variations

    bool mParticleDieOnTimeout = true; // whether particle lifetime is limited by mParticleLifetimeMin, mParticleLifetimeMax
    bool mParticleDieOnCollision = false; // whether particle dies when collide with solid block
};