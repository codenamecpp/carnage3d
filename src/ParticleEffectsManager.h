#pragma once

#include "ParticleEffect.h"

// Particle effects manager class
class ParticleEffectsManager final: public cxx::noncopyable
{
public:
    // readonly
    std::vector<ParticleEffect*> mParticleEffects;

public:
    ParticleEffectsManager();

    void EnterWorld();
    void ClearWorld();
    void UpdateFrame();
    void DebugDraw(DebugRenderer& debugRender);

    // Create new particle effect on game scene
    ParticleEffect* CreateParticleEffect(const ParticleEffectParams& effectParams, const ParticleEmitterShape& emitterShape);

    // Free particle effect object
    // @param particleEffect: Object to destroy, pointer becomes invalid
    void DestroyParticleEffect(ParticleEffect* particleEffect);
    void DestroyParticleEffects();

    // sparks effect
    void StartCarSparks(const glm::vec3& position, int numSparks);
    void StartCarSparks(const glm::vec3& position, const glm::vec3& velocity, int numSparks);

    bool IsCarSparksEffectEnabled() const;

private:
    void CreateSparksParticleEffect();

private:
    // persistent effects
    ParticleEffect* mSparksEffect = nullptr;
};

extern ParticleEffectsManager gParticleManager;