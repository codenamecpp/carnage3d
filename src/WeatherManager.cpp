#include "stdafx.h"
#include "WeatherManager.h"
#include "ParticleEffectsManager.h"
#include "CarnageGame.h"
#include "Cvars.h"

WeatherManager gWeatherManager;

//////////////////////////////////////////////////////////////////////////
// cvars
//////////////////////////////////////////////////////////////////////////

CvarBoolean gCvarWeatherActive ("g_weather", 0, "Enable weather effects", CvarFlags_Init);

//////////////////////////////////////////////////////////////////////////

void WeatherManager::EnterWorld()
{
    ChangeWeather(eGameWeather_Snow);
}

void WeatherManager::ClearWorld()
{
    CleanupWeather();
}

void WeatherManager::UpdateFrame()
{
    if (!IsWeatherEffectsEnabled())
        return;

    if (mCurrentWeather == eGameWeather_Snow)
    {
        if (mParticleEffect)
        {
            ParticleEmitterShape effectShape;
            GetSnowParticleEffectShape(effectShape);
            mParticleEffect->SetEmitterShape(effectShape);
        }
    }
}

void WeatherManager::DebugDraw(DebugRenderer& debugRender)
{
}

void WeatherManager::ChangeWeather(eGameWeather weather)
{
    if (!IsWeatherEffectsEnabled())
        return;

    // todo: implement effect ot all active players

    if (mCurrentWeather == weather)
        return;

    CleanupWeather();

    if (weather == eGameWeather_Snow)
    {
        ParticleEffectParams effectParams;
        GetSnowParticleEffectParams(effectParams);

        ParticleEmitterShape effectShape;
        GetSnowParticleEffectShape(effectShape);

        mParticleEffect = gParticleManager.CreateParticleEffect(effectParams, effectShape);
        debug_assert(mParticleEffect);
        mParticleEffect->StartEffect();
    }

    mCurrentWeather = weather;
}

void WeatherManager::CleanupWeather()
{
    if (mParticleEffect)
    {
        gParticleManager.DestroyParticleEffect(mParticleEffect);
        mParticleEffect = nullptr;
    }

    mCurrentWeather = eGameWeather_Sun;
}

void WeatherManager::GetSnowParticleEffectParams(ParticleEffectParams& params) const
{
    params.mParticleSpace = eParticleSpace_Global;
    params.mMaxParticlesCount = 2200;
    params.mParticleEmitFrequency = 350.0f;
    params.mParticleHorzVelocityRange.x = -0.2f;
    params.mParticleHorzVelocityRange.y = 0.2f;
    params.mParticleVertVelocityRange.x = 0.0f;
    params.mParticleVertVelocityRange.y = 6.2f;
    params.mParticlesGravity.y = -8.0f;
    params.mParticleSizeRange.x = 8.0f;
    params.mParticleSizeRange.y = 10.0f;
    params.mParticleLifetimeRange.x = 5.0f;
    params.mParticleLifetimeRange.y = 7.0f;
    params.mParticleDieOnTimeout = true;
    params.mParticleDieOnCollision = true;
}

void WeatherManager::GetSnowParticleEffectShape(ParticleEmitterShape& shape) const
{
    const float EffectCellSize = Convert::MapUnitsToMeters(10.0f);
    const float EffectHeight = Convert::MapUnitsToMeters(6.0f);

    GameCamera& currentCamera = gCarnageGame.mHumanPlayers[0]->mPlayerView.mCamera;

    glm::vec3 cameraPosition = currentCamera.mPosition;
    glm::vec3 minPos {cameraPosition.x - EffectCellSize * 0.5f, EffectHeight, cameraPosition.z - EffectCellSize * 0.5f};
    glm::vec3 maxPos {cameraPosition.x + EffectCellSize * 0.5f, EffectHeight, cameraPosition.z + EffectCellSize * 0.5f};

    shape.mShape = eParticleEmitterShape_Box;
    shape.mBox.extend(minPos);
    shape.mBox.extend(maxPos);
}

bool WeatherManager::IsWeatherEffectsEnabled() const
{
    return gCvarWeatherActive.mValue;
}
