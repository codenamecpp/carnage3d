#include "stdafx.h"
#include "WeatherManager.h"
#include "ParticleEffectsManager.h"
#include "CarnageGame.h"
#include "cvars.h"

WeatherManager gWeatherManager;

//////////////////////////////////////////////////////////////////////////
// cvars
//////////////////////////////////////////////////////////////////////////

CvarBoolean gCvarWeatherActive ("g_weather", 0, "Enable weather effects", CvarFlags_Init);
CvarEnum<eWeatherEffect> gCvarWeatherEffect ("g_weatherEffect", eWeatherEffect_Sun, "Currently active weather effect", CvarFlags_None);

//////////////////////////////////////////////////////////////////////////

void WeatherManager::EnterWorld()
{
    ChangeWeather(gCvarWeatherEffect.mValue);
    gCvarWeatherEffect.ClearModified();
}

void WeatherManager::ClearWorld()
{
    CleanupWeather();
}

void WeatherManager::UpdateFrame()
{
    if (!IsWeatherEffectsEnabled())
        return;

    if (gCvarWeatherEffect.IsModified())
    {
        gCvarWeatherEffect.ClearModified();
        ChangeWeather(gCvarWeatherEffect.mValue);
    }

    if (mParticleEffect)
    {
        ParticleEmitterShape effectShape;
        GetParticleEffectShape(gCvarWeatherEffect.mValue, effectShape);
        mParticleEffect->SetEmitterShape(effectShape);
    }
}

void WeatherManager::DebugDraw(DebugRenderer& debugRender)
{
}

void WeatherManager::ChangeWeather(eWeatherEffect weather)
{
    if (!IsWeatherEffectsEnabled())
        return;

    // todo: implement effect ot all active players

    CleanupWeather();

    if ((weather == eWeatherEffect_Snow) || (weather == eWeatherEffect_Rain))
    {
        ParticleEffectParams effectParams;
        GetParticleEffectParams(weather, effectParams);

        ParticleEmitterShape effectShape;
        GetParticleEffectShape(weather, effectShape);

        mParticleEffect = gParticleManager.CreateParticleEffect(effectParams, effectShape);
        debug_assert(mParticleEffect);
        mParticleEffect->StartEffect();
    }

    gCvarWeatherEffect.mValue = weather;
    gCvarWeatherEffect.ClearModified();
}

void WeatherManager::CleanupWeather()
{
    if (mParticleEffect)
    {
        gParticleManager.DestroyParticleEffect(mParticleEffect);
        mParticleEffect = nullptr;
    }

    gCvarWeatherEffect.mValue = eWeatherEffect_Sun;
    gCvarWeatherEffect.ClearModified();
}

void WeatherManager::GetParticleEffectParams(eWeatherEffect weather, ParticleEffectParams& params) const
{
    if (weather == eWeatherEffect_Snow)
    {
        params.mParticleSpace = eParticleSpace_Global;
        params.mMaxParticlesCount = 2000;
        params.mParticlesPerSecond = 520.0f;
        params.mParticleHorzVelocityRange.x = -0.2f;
        params.mParticleHorzVelocityRange.y = 0.2f;
        params.mParticleVertVelocityRange.x = -7.0f;
        params.mParticleVertVelocityRange.y = 7.2f;
        params.mParticlesGravity.y = -8.0f;
        params.mParticleSizeRange.x = 6.0f;
        params.mParticleSizeRange.y = 7.0f;
        params.mParticleLifetimeRange.x = 5.0f;
        params.mParticleLifetimeRange.y = 8.0f;
        params.mParticleDieOnTimeout = true;
        params.mParticleDieOnCollision = true;
        Color32 snowflakesColor = Color32_White;
        snowflakesColor.mA = 212;
        params.mParticleColors = { snowflakesColor };
        params.mParticleFadeoutDuration = 0.5f;
    }

    if (weather == eWeatherEffect_Rain)
    {
        params.mParticleSpace = eParticleSpace_Global;
        params.mMaxParticlesCount = 2200;
        params.mParticlesPerSecond = 1080.0f;
        params.mParticleHorzVelocityRange.x = -1.0f;
        params.mParticleHorzVelocityRange.y = 1.0f;
        params.mParticleVertVelocityRange.x = 0.0f;
        params.mParticleVertVelocityRange.y = 5.0f;
        params.mParticlesGravity.x = -2.0f;
        params.mParticlesGravity.y = -30.0f;
        params.mParticlesGravity.z = 1.0f;
        params.mParticleSizeRange.x = 5.0f;
        params.mParticleSizeRange.y = 6.0f;
        params.mParticleLifetimeRange.x = 5.0f;
        params.mParticleLifetimeRange.y = 5.0f;
        params.mParticleDieOnTimeout = true;
        params.mParticleDieOnCollision = true;
        Color32 raindropsColor = Color32_SkyBlue;
        raindropsColor.mA = 162;
        params.mParticleColors = { raindropsColor };
        params.mParticleFadeoutDuration = 0.2f;
    }
}

void WeatherManager::GetParticleEffectShape(eWeatherEffect weather, ParticleEmitterShape& shape) const
{
    if ((weather == eWeatherEffect_Snow) || (weather == eWeatherEffect_Rain))
    {
        const float EffectCellSize = Convert::MapUnitsToMeters(10.0f);
        const float EffectHeight = Convert::MapUnitsToMeters(6.0f);

        GameCamera& currentCamera = gCarnageGame.mHumanPlayers[0]->mViewCamera;

        glm::vec3 cameraPosition = currentCamera.mPosition;
        glm::vec3 minPos {cameraPosition.x - EffectCellSize * 0.5f, EffectHeight, cameraPosition.z - EffectCellSize * 0.5f};
        glm::vec3 maxPos {cameraPosition.x + EffectCellSize * 0.5f, EffectHeight, cameraPosition.z + EffectCellSize * 0.5f};

        shape.mShape = eParticleEmitterShape_Box;
        shape.mBox.extend(minPos);
        shape.mBox.extend(maxPos);
    }
}

bool WeatherManager::IsWeatherEffectsEnabled() const
{
    return gCvarWeatherActive.mValue;
}
