#pragma once

#include "ParticleDefs.h"
#include "RenderView.h"

// forwards
class ParticleRenderdata;

enum eParticleEffectState
{
    eParticleEffectState_Initial, // effect is waiting to be configured and launched
    eParticleEffectState_Active, // effect is emitting particles
    eParticleEffectState_Stopping, // no longer emitting new particles, but some is still alive
    eParticleEffectState_Done, // effect is inactive
};

// particles generator class
class ParticleEffect final: public cxx::noncopyable
{
    friend class RenderingManager;

public:
    ParticleEffect() = default;
    ~ParticleEffect();

    void UpdateFrame();
    void DebugDraw(DebugRenderer& debugRender);

    // Effect control
    void StartEffect();
    void StopEffect();
    void ClearEffect();

    // Whether or not effect is in active state
    bool IsEffectInactive() const;
    bool IsEffectActive() const;

    // Setup main particle effect parameters
    void GetEffectParameters(ParticleEffectParams& effectParams) const;
    void SetEffectParameters(const ParticleEffectParams& effectParams);

    // Setup emitter shape parameters
    void GetEmitterShape(ParticleEmitterShape& emitterShape) const;
    void SetEmitterShape(const ParticleEmitterShape& emitterShape);

private:
    void ResetParticles();
    // returns false if particle dead
    bool UpdateParticle(Particle& particle, float deltaTime);
    void SpawnParticle(Particle& particle);

    void UpdateAliveParticles(float deltaTime);
    void GenerateNewParticles();

    void SetRenderdata(ParticleRenderdata* renderdata);

private:
    ParticleEffectParams mEffectParams;
    ParticleEmitterShape mEmitterShapeParams;
    eParticleEffectState mEffectState = eParticleEffectState_Initial;
    std::vector<Particle> mParticles;
    float mSpawnTimer = 0.0f;
    int mAliveParticlesCount = 0;
    ParticleRenderdata* mRenderdata = nullptr; // renderdata is owned by particle renderer
};