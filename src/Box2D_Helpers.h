#pragma once

#include "GameObject.h"
#include "PhysicsBody.h"
#include "Collider.h"

// b2Vec <- -> glm::vec2 conversion

//////////////////////////////////////////////////////////////////////////

inline glm::vec2 convert_vec2(const b2Vec2& vector_value)
{
    return { vector_value.x, vector_value.y };
}

inline b2Vec2 convert_vec2(const glm::vec2& vector_value)
{
    return { vector_value.x, vector_value.y };
}

//////////////////////////////////////////////////////////////////////////

inline Collider* b2Fixture_get_collider(b2Fixture* fixture)
{
    debug_assert(fixture);
    Collider* collisionShape = (Collider*) fixture->GetUserData();

    return collisionShape;
}

inline PhysicsBody* b2Fixture_get_physics_body(b2Fixture* fixture)
{
    debug_assert(fixture);

    PhysicsBody* physicsBoxy = (PhysicsBody*) fixture->GetBody()->GetUserData();
    return physicsBoxy;
}

inline GameObject* b2Fixture_get_game_object(b2Fixture* fixture)
{
    debug_assert(fixture);

    PhysicsBody* physicsBoxy = (PhysicsBody*) fixture->GetBody()->GetUserData();
    GameObject* gameObject = nullptr;
    if (physicsBoxy)
    {
        gameObject = physicsBoxy->mGameObject;
    }
    return gameObject;
}

//////////////////////////////////////////////////////////////////////////