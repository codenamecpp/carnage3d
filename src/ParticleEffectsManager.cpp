#include "stdafx.h"
#include "ParticleEffectsManager.h"
#include "RenderingManager.h"

ParticleEffectsManager gParticleManager;

ParticleEffectsManager::ParticleEffectsManager()
{
}

void ParticleEffectsManager::EnterWorld()
{
}

void ParticleEffectsManager::ClearWorld()
{
    DestroyParticleEffects();
}

void ParticleEffectsManager::UpdateFrame()
{
    for (ParticleEffect* currEffect: mParticleEffects)
    {
        currEffect->UpdateFrame();
    }
}

void ParticleEffectsManager::DebugDraw(DebugRenderer& debugRender)
{
    for (ParticleEffect* currEffect: mParticleEffects)
    {
        currEffect->DebugDraw(debugRender);
    }
}

ParticleEffect* ParticleEffectsManager::CreateParticleEffect(const ParticleEffectParams& effectParams, const ParticleEmitterShape& emitterShape)
{
    ParticleEffect* particleEffect = new ParticleEffect;
    debug_assert(particleEffect);
    mParticleEffects.push_back(particleEffect);

    particleEffect->SetEffectParameters(effectParams);
    particleEffect->SetEmitterShape(emitterShape);

    gRenderManager.RegisterParticleEffect(particleEffect);

    return particleEffect;
}

void ParticleEffectsManager::DestroyParticleEffect(ParticleEffect* particleEffect)
{
    debug_assert(particleEffect);

    if (particleEffect)
    {
        gRenderManager.UnregisterParticleEffect(particleEffect);
        cxx::erase_elements(mParticleEffects, particleEffect);
        delete particleEffect;
    }
}

void ParticleEffectsManager::DestroyParticleEffects()
{
    for (ParticleEffect* currEffect: mParticleEffects)
    {
        gRenderManager.UnregisterParticleEffect(currEffect);
        delete currEffect;
    }
    mParticleEffects.clear();
}