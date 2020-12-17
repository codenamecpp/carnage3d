#pragma once

#include "ParticleEffect.h"

enum eGameWeather
{
    eGameWeather_Sun, // default
    eGameWeather_Snow,
    eGameWeather_Rain,
    eGameWeather_Fog,
};

// Weather effects manager
class WeatherManager final: public cxx::noncopyable
{
public:
    void EnterWorld();
    void ClearWorld();
    void UpdateFrame();
    void DebugDraw(DebugRenderer& debugRender);

    bool IsWeatherEffectsEnabled() const;

private:
    void ChangeWeather(eGameWeather weather);
    void CleanupWeather();

    void GetSnowParticleEffectParams(ParticleEffectParams& params) const;
    void GetSnowParticleEffectShape(ParticleEmitterShape& shape) const;

private:
    eGameWeather mCurrentWeather = eGameWeather_Sun;

    ParticleEffect* mParticleEffect = nullptr;
};

extern WeatherManager gWeatherManager;