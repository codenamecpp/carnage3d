#include "stdafx.h"
#include "ParticleEffectsManager.h"
#include "RenderingManager.h"
#include "cvars.h"

//////////////////////////////////////////////////////////////////////////
// cvars
//////////////////////////////////////////////////////////////////////////

CvarBoolean gCvarCarSparksActive("g_carSparks", true, "Enable or disable car sparks effect", CvarFlags_Archive);

//////////////////////////////////////////////////////////////////////////

ParticleEffectsManager gParticleManager;

//////////////////////////////////////////////////////////////////////////

ParticleEffectsManager::ParticleEffectsManager()
{
}

void ParticleEffectsManager::EnterWorld()
{
    CreateSparksParticleEffect();
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
    mSparksEffect = nullptr;
    for (ParticleEffect* currEffect: mParticleEffects)
    {
        gRenderManager.UnregisterParticleEffect(currEffect);
        delete currEffect;
    }
    mParticleEffects.clear();
}

void ParticleEffectsManager::CreateSparksParticleEffect()
{
    debug_assert(mSparksEffect == nullptr);

    ParticleEffectParams effectParams;
    effectParams.mParticleSpace = eParticleSpace_Global;
    effectParams.mMaxParticlesCount = 1200;
    effectParams.mEffectDuration = 0.0f;
    effectParams.mParticlesPerSecond = 0.0f;
    effectParams.mParticleFadeoutDuration = 0.1f;
    effectParams.mParticleSizeRange.x = 5.0f;
    effectParams.mParticleSizeRange.y = 6.0f;
    effectParams.mParticleLifetimeRange.x = 0.2f;
    effectParams.mParticleLifetimeRange.y = 0.3f;
    effectParams.mParticleHorzVelocityRange.x = -1.5f;
    effectParams.mParticleHorzVelocityRange.y = 1.5f;
    effectParams.mParticleColors = {
        Color32::MakeRGBA(255, 216, 130, 180),
        Color32::MakeRGBA(255, 216, 0, 180),
        Color32::MakeRGBA(255,  106, 0, 180),
        Color32::MakeRGBA(80,  80, 80, 180),
        Color32::MakeRGBA(60,  60, 60, 180)
    };
    effectParams.mParticleDieOnTimeout = true;
    effectParams.mParticleChangesColorOverTime = true;

    ParticleEmitterShape effectShape;
    effectShape.mShape = eParticleEmitterShape_Point;

    mSparksEffect = CreateParticleEffect(effectParams, effectShape);
    debug_assert(mSparksEffect);

    if (mSparksEffect)
    {
        mSparksEffect->StartEffect(); // make it active
    }
}

void ParticleEffectsManager::StartCarSparks(const glm::vec3& position, int numSparks)
{
    debug_assert(mSparksEffect);

    if (mSparksEffect == nullptr)
        return;

    for (int icurr = 0; icurr < numSparks; ++icurr)
    {
        mSparksEffect->PutParticle(position);
    }
}

void ParticleEffectsManager::StartCarSparks(const glm::vec3& position, const glm::vec3& velocity, int numSparks)
{
    debug_assert(mSparksEffect);

    if (mSparksEffect == nullptr)
        return;

    for (int icurr = 0; icurr < numSparks; ++icurr)
    {
        mSparksEffect->PutParticle(position, velocity);
    }
}

bool ParticleEffectsManager::IsCarSparksEffectEnabled() const
{
    return gCvarCarSparksActive.mValue;
}
