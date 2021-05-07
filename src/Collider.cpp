#include "stdafx.h"
#include "Collider.h"
#include "Box2D_Helpers.h"
#include "PhysicsBody.h"

Collider::Collider(PhysicsBody* physicsBody, const CollisionShape& shapeData, const PhysicsMaterial& shapeMaterial,
    CollisionGroup shapeCollisionGroup,
    CollisionGroup shapeCollidesWith, ColliderFlags shapeFlags)
        : mPhysicsBody(physicsBody)
        , mShapeData(shapeData)
        , mGameObject(physicsBody->mGameObject)
{
    debug_assert(mPhysicsBody);
    debug_assert(mGameObject);

    // allocate new shape
    b2FixtureDef fixtureDef;
    fixtureDef.userData = this;
    fixtureDef.friction = shapeMaterial.mFriction;
    fixtureDef.restitution = shapeMaterial.mRestitution;
    fixtureDef.density = shapeMaterial.mDensity;
    fixtureDef.filter.categoryBits = shapeCollisionGroup;
    fixtureDef.filter.maskBits = shapeCollidesWith;

    // setup fixture shape
    struct FixtureShapeDefs
    {
        b2CircleShape mCircleShape;
        b2EdgeShape mEdgeShape;
        b2PolygonShape mPolygonShape;
    };

    FixtureShapeDefs fixtureShapeDefs;
    
    switch (shapeData.mType)
    {
        case eCollisionShape_Circle:
            fixtureShapeDefs.mCircleShape.m_radius = shapeData.mCircle.mRadius;
            fixtureDef.shape = &fixtureShapeDefs.mCircleShape;
        break;

        case eCollisionShape_Edge:
            fixtureShapeDefs.mEdgeShape.Set(
                convert_vec2(shapeData.mEdge.mEdgePoint0),
                convert_vec2(shapeData.mEdge.mEdgePoint1));
            fixtureDef.shape = &fixtureShapeDefs.mEdgeShape;
        break;

        case eCollisionShape_Box:
        {
            b2Vec2 b2CenterOffset (shapeData.mBox.mCenter.x, shapeData.mBox.mCenter.z);
            fixtureShapeDefs.mPolygonShape.SetAsBox(shapeData.mBox.mHalfExtents.x, shapeData.mBox.mHalfExtents.z, b2CenterOffset, 0.0f);
            fixtureDef.shape = &fixtureShapeDefs.mPolygonShape;
        }
        break;

        default:
            debug_assert(false);
        break;
    }

    mBox2Fixture = mPhysicsBody->mBox2Body->CreateFixture(&fixtureDef);
    debug_assert(mBox2Fixture);
}

void Collider::SetShapeMaterial(const PhysicsMaterial& shapeMaterial)
{
    mBox2Fixture->SetFriction(shapeMaterial.mFriction);
    mBox2Fixture->SetRestitution(shapeMaterial.mRestitution);
    mBox2Fixture->SetDensity(shapeMaterial.mDensity);
}

void Collider::SetShapeCollisionGroup(CollisionGroup collisionGroup, CollisionGroup collidesWith)
{
    b2Filter filterData;
    filterData.categoryBits = collisionGroup;
    filterData.maskBits = collidesWith;
    mBox2Fixture->SetFilterData(filterData);
}

void Collider::GetShapeMaterial(PhysicsMaterial& shapeMaterial) const
{
    shapeMaterial.mFriction = mBox2Fixture->GetFriction();
    shapeMaterial.mRestitution = mBox2Fixture->GetRestitution();
    shapeMaterial.mDensity = mBox2Fixture->GetDensity();
}

void Collider::GetShapeCollisionGroup(CollisionGroup& collisionGroup, CollisionGroup& collidesWith) const
{
    const b2Filter& filterData = mBox2Fixture->GetFilterData();
    collisionGroup = (CollisionGroup) filterData.categoryBits;
    collidesWith = (CollisionGroup) filterData.maskBits;
}

void Collider::SetShapeCollisionFlags(ColliderFlags flags)
{
}

void Collider::ChangeShapeCollisionFlags(ColliderFlags enableFlags, ColliderFlags disableFlags)
{
    ColliderFlags shapeFlags;
    GetShapeCollisionFlags(shapeFlags);

    shapeFlags = (shapeFlags | enableFlags) & ~disableFlags;
    SetShapeCollisionFlags(shapeFlags);
}

void Collider::GetShapeCollisionFlags(ColliderFlags& flags) const
{
    flags = ColliderFlags_None;
}

void Collider::DestroyFixture()
{
    if (mBox2Fixture)
    {
        mPhysicsBody->mBox2Body->DestroyFixture(mBox2Fixture);
        mBox2Fixture = nullptr;
    }
}
