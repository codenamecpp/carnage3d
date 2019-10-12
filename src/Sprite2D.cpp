#include "stdafx.h"
#include "Sprite2D.h"

void Sprite2D::SetNull()
{
    mTexture = nullptr;

    mTextureRegion.SetNull();

    mOrigin.x = 0.0f;
    mOrigin.y = 0.0f;
    mPosition.x = 0.0f;
    mPosition.y = 0.0f;
    mHeight = 0.0f;
    mScale = 1.0f;

    mRotateAngle = cxx::angle_t::from_degrees(0.0f);
}

void Sprite2D::GetCorners(glm::vec2 positions[4]) const
{
    positions[0] = mOrigin;

    positions[1].x = mOrigin.x + (mTextureRegion.mRectangle.w * mScale);
    positions[1].y = mOrigin.y;

    positions[2].x = mOrigin.x;
    positions[2].y = mOrigin.y + (mTextureRegion.mRectangle.h * mScale);

    positions[3].x = mOrigin.x + (mTextureRegion.mRectangle.w * mScale);
    positions[3].y = mOrigin.y + (mTextureRegion.mRectangle.h * mScale);


    if (mRotateAngle.non_zero()) // has rotation
    {
        for (int icorner = 0; icorner < 4; ++icorner)
        {
            glm::vec2 posTransformed = glm::rotate(positions[icorner], mRotateAngle.to_radians());
            positions[icorner] = mPosition + posTransformed;
        }
    }
}
