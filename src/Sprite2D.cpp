#include "stdafx.h"
#include "Sprite2D.h"

void Sprite2D::Clear()
{
    mTexture = nullptr;

    mTextureRegion.Clear();

    mOriginMode = eOriginMode_Center;
    mPosition.x = 0.0f;
    mPosition.y = 0.0f;
    mHeight = 0.0f;
    mScale = 1.0f;
    mPaletteIndex = 0;

    mRotateAngle.set_zero();
}

void Sprite2D::GetCorners(glm::vec2 positions[4]) const
{
    float rectw = mTextureRegion.mRectangle.w * mScale;
    float recth = mTextureRegion.mRectangle.h * mScale;

    positions[0] = GetOriginPoint();

    positions[1].x = positions[0].x + rectw;
    positions[1].y = positions[0].y;

    positions[2].x = positions[0].x;
    positions[2].y = positions[0].y + recth;

    positions[3].x = positions[1].x;
    positions[3].y = positions[2].y;

    if (mRotateAngle.non_zero()) // has rotation
    {
        for (int icorner = 0; icorner < 4; ++icorner)
        {
            glm::vec2 posTransformed = glm::rotate(positions[icorner], mRotateAngle.to_radians());
            positions[icorner] = mPosition + posTransformed;
        }
    }
    else
    {
        for (int icorner = 0; icorner < 4; ++icorner)
        {
            positions[icorner] += mPosition;
        } 
    }
}

void Sprite2D::GetMaxRectPoints(glm::vec2 positions[2]) const
{
    float rectw = (mTextureRegion.mRectangle.w * mScale);
    float recth = (mTextureRegion.mRectangle.h * mScale);
    float maxside = std::max(rectw, recth);

    positions[0] = mPosition + GetOriginPoint();
    positions[1].x = positions[0].x + maxside;
    positions[1].y = positions[0].y + maxside;
}
