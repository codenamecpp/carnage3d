#include "stdafx.h"
#include "ParticleEffect.h"
#include "TimeManager.h"
#include "DebugRenderer.h"
#include "CarnageGame.h"
#include "ParticleRenderdata.h"

ParticleEffect::~ParticleEffect()
{
    debug_assert(mRenderdata == nullptr);
}

void ParticleEffect::UpdateFrame()
{
    if (IsEffectInactive())
        return;

    if (mEffectParams.mMaxParticlesCount == 0)
    {
        debug_assert(false);
        return;
    }

    float deltaTime = gTimeManager.mGameFrameDelta;
    mParticleTimer += deltaTime;
    mActivityTimer += deltaTime;

    UpdateAliveParticles(deltaTime);

    if (mEffectState == eParticleEffectState_Active)
    {
        bool effectTimeDone = mEffectParams.IsEffectFinite() && (mActivityTimer > mEffectParams.mEffectDuration);
        if (effectTimeDone)
        {
            StopEffect();
        }
        else
        {
            GenerateNewParticles();
        }
    }

    if ((mAliveParticlesCount > 0) && mRenderdata)
    {
        mRenderdata->Invalidate();
    }

    if ((mAliveParticlesCount == 0) && (mEffectState == eParticleEffectState_Stopping))
    {
        mEffectState = eParticleEffectState_Done;
    }
}

void ParticleEffect::DebugDraw(DebugRenderer& debugRender)
{
    if (IsEffectInactive())
        return;

    Color32 shapeColor = IsEffectActive() ? Color32_Green : Color32_Red;
    if (mEmitterShapeParams.mShape == eParticleEmitterShape_Box)
    {
        debugRender.DrawAabb(mEmitterShapeParams.mBox, shapeColor, false);
    }
    else
    {
        debug_assert(mEmitterShapeParams.mShape == eParticleEmitterShape_Point);

        float dims = Convert::MapUnitsToMeters(0.1f);
        glm::vec3 dims3 { dims, dims, dims };
        debugRender.DrawCube(mEmitterShapeParams.mPoint, dims3, shapeColor, false);
    }
}

void ParticleEffect::GetEffectParameters(ParticleEffectParams& effectParams) const
{
    effectParams = mEffectParams;
}

void ParticleEffect::SetEffectParameters(const ParticleEffectParams& effectParams)
{
    mEffectParams = effectParams;
    ResetParticles();
}

void ParticleEffect::GetEmitterShape(ParticleEmitterShape& emitterShape) const
{
    emitterShape = mEmitterShapeParams;
}

void ParticleEffect::SetEmitterShape(const ParticleEmitterShape& emitterShape)
{
    mEmitterShapeParams = emitterShape;
}

void ParticleEffect::StartEffect()
{
    if ((mEffectState == eParticleEffectState_Done) || (mEffectState == eParticleEffectState_Initial))
    {
        ClearEffect();

        mEffectState = eParticleEffectState_Active;
    }
}

void ParticleEffect::StopEffect()
{
    if (mEffectState == eParticleEffectState_Active)
    {
        mEffectState = (mAliveParticlesCount == 0) ? eParticleEffectState_Done : eParticleEffectState_Stopping;
    }
}

void ParticleEffect::ClearEffect()
{
    ResetParticles();

    mEffectState = eParticleEffectState_Initial;
    mParticleTimer = 0.0f;
    mActivityTimer = 0.0f;
}

bool ParticleEffect::PutParticle(const glm::vec3& position)
{
    if (mAliveParticlesCount < mEffectParams.mMaxParticlesCount)
    {
        Particle& particle = mParticles[mAliveParticlesCount++];
        SpawnParticle(particle);
        // fix start params
        particle.mStartPosition = position;
        particle.mPosition = position;
        return true;
    }
    return false;
}

bool ParticleEffect::PutParticle(const glm::vec3& position, const glm::vec3& velocity)
{
    if (mAliveParticlesCount < mEffectParams.mMaxParticlesCount)
    {
        Particle& particle = mParticles[mAliveParticlesCount++];
        SpawnParticle(particle);
        // fix start params
        particle.mStartPosition = position;
        particle.mPosition = position;
        particle.mVelocity += velocity; // accumulate
        return true;
    }
    return false;
}

bool ParticleEffect::IsEffectInactive() const
{
    return !IsEffectActive();
}

bool ParticleEffect::IsEffectActive() const
{
    return (mEffectState == eParticleEffectState_Active) || (mEffectState == eParticleEffectState_Stopping);
}

void ParticleEffect::ResetParticles()
{
    mParticles.clear();
    if (mEffectParams.mMaxParticlesCount > 0)
    {
        mParticles.resize(mEffectParams.mMaxParticlesCount);
    }
    mAliveParticlesCount = 0;
}

bool ParticleEffect::UpdateParticle(Particle& particle, float deltaTime)
{
    particle.mAge += deltaTime;

    if (particle.mState == eParticleState_Alive)
    {
        // check timeout
        if (mEffectParams.mParticleDieOnTimeout && (particle.mAge > particle.mLifeTime))
        {
            if (mEffectParams.IsParticleFadeoutOnDie())
            {
                particle.mState = eParticleState_Fade;
            }
            else return false; // particle dead
        }

        // update current position based on velocity and time
        particle.mPosition += (particle.mVelocity + mEffectParams.mParticlesGravity) * deltaTime;

        // update color
        if (mEffectParams.mParticleChangesColorOverTime)
        {
            int colorCount = (int) mEffectParams.mParticleColors.size();
            if (colorCount > 1)
            {
                float progression = (particle.mAge / particle.mLifeTime); // [0,1]
                int colorIndex = glm::min((int) ((colorCount - 1) * progression + 0.5f), (colorCount - 1));
                particle.mColor = mEffectParams.mParticleColors[colorIndex];
            }
        }

        // check collision
        if (mEffectParams.mParticleDieOnCollision)
        {
            float height = gGameMap.GetHeightAtPosition(particle.mPosition, false);
            if (height > particle.mPosition.y)
            {
                particle.mPosition.y = height; // fix height
                if (mEffectParams.IsParticleFadeoutOnDie())
                {
                    particle.mState = eParticleState_Fade;
                }
                else return false; // particle dead
            }
        }
    }

    // update fadeout
    if (particle.mState == eParticleState_Fade)
    {
        debug_assert(mEffectParams.mParticleFadeoutDuration > 0.0f);
        int currAlpha = (int) (particle.mColor.mA - (255.0f * (deltaTime / mEffectParams.mParticleFadeoutDuration)));
        if (currAlpha < 0)
        {
            currAlpha = 0;
        }
        particle.mColor.mA = (unsigned char) currAlpha;
        if (currAlpha == 0)
            return false; // particle dead
    }

    return true; // particle alive
}

void ParticleEffect::SpawnParticle(Particle& particle)
{
    cxx::randomizer& random = gCarnageGame.mGameRand;

    particle.mAge = 0.0f;
    particle.mState = eParticleState_Alive;

    // choose position
    if (mEmitterShapeParams.mShape == eParticleEmitterShape_Box)
    {
        particle.mPosition.x = random.generate_float(mEmitterShapeParams.mBox.mMin.x, mEmitterShapeParams.mBox.mMax.x);
        particle.mPosition.y = random.generate_float(mEmitterShapeParams.mBox.mMin.y, mEmitterShapeParams.mBox.mMax.y);
        particle.mPosition.z = random.generate_float(mEmitterShapeParams.mBox.mMin.z, mEmitterShapeParams.mBox.mMax.z);
    }
    else
    {
        debug_assert(mEmitterShapeParams.mShape == eParticleEmitterShape_Point);
        particle.mPosition = mEmitterShapeParams.mPoint;
    }

    particle.mStartPosition = particle.mPosition;

    // choose velocity
    particle.mVelocity.x = random.generate_float(mEffectParams.mParticleHorzVelocityRange.x, mEffectParams.mParticleHorzVelocityRange.y);
    particle.mVelocity.z = random.generate_float(mEffectParams.mParticleHorzVelocityRange.x, mEffectParams.mParticleHorzVelocityRange.y);
    particle.mVelocity.y = random.generate_float(mEffectParams.mParticleVertVelocityRange.x, mEffectParams.mParticleVertVelocityRange.y);

    // choose size
    particle.mSize = random.generate_float(mEffectParams.mParticleSizeRange.x, mEffectParams.mParticleSizeRange.y);

    // choose lifetime
    particle.mLifeTime = random.generate_float(mEffectParams.mParticleLifetimeRange.x, mEffectParams.mParticleLifetimeRange.y);

    // choose color
    Color32 color = Color32_White;
    if (!mEffectParams.mParticleColors.empty())
    {
        int colorIndex = 0;
        int colorCount = (int) mEffectParams.mParticleColors.size();
        if ((colorCount > 1) && !mEffectParams.mParticleChangesColorOverTime)
        {
            colorIndex = random.generate_int() % colorCount;
        }
        color = mEffectParams.mParticleColors[colorIndex];
    }
    particle.mColor = color;
}

void ParticleEffect::GenerateNewParticles()
{
    debug_assert(mEffectState == eParticleEffectState_Active);
    if ((mEffectParams.mMaxParticlesCount == mAliveParticlesCount) ||
        (mEffectParams.mParticlesPerSecond == 0.0f))
    {
        return;
    }

    const float timePerParticle = 1.0f / mEffectParams.mParticlesPerSecond;
    int particlesToGenerate = (int) (mParticleTimer / timePerParticle);
    if (particlesToGenerate == 0)
        return;

    if (particlesToGenerate > (mEffectParams.mMaxParticlesCount - mAliveParticlesCount))
    {
        particlesToGenerate = (mEffectParams.mMaxParticlesCount - mAliveParticlesCount);
    }
    mParticleTimer -= (particlesToGenerate * timePerParticle);

    for (int icurr = 0; icurr < particlesToGenerate; ++icurr)
    {
        int iNextParticle = mAliveParticlesCount + icurr;
        debug_assert(iNextParticle < mEffectParams.mMaxParticlesCount);
        SpawnParticle(mParticles[iNextParticle]);
    }
    mAliveParticlesCount += particlesToGenerate;
}

void ParticleEffect::UpdateAliveParticles(float deltaTime)
{
    for (int icurr = 0; icurr < mAliveParticlesCount; )
    {
        Particle& currParticle = mParticles[icurr];
        if (UpdateParticle(currParticle, deltaTime))
        {
            ++icurr;
            continue;
        }

        currParticle.mState = eParticleState_Dead;
        // kill particle
        if (icurr < (mAliveParticlesCount - 1))
        {
            mParticles[icurr] = mParticles[mAliveParticlesCount - 1];
        }
        --mAliveParticlesCount;
    }
}

void ParticleEffect::SetRenderdata(ParticleRenderdata* renderdata)
{
    mRenderdata = renderdata;
}
