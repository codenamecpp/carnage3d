#pragma once

#include "ParticleEffect.h"
#include "GameDefs.h"

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
    void ChangeWeather(eWeatherEffect weather);
    void CleanupWeather();

    void GetParticleEffectParams(eWeatherEffect weather, ParticleEffectParams& params) const;
    void GetParticleEffectShape(eWeatherEffect weather, ParticleEmitterShape& shape) const;

private:
    ParticleEffect* mParticleEffect = nullptr;
};

extern WeatherManager gWeatherManager;