#pragma once

namespace box2d
{
    // simple wrapper for seamlessly cast between math libraries
    struct vec2: public b2Vec2
    {
    public:
        vec2() = default;
        vec2(float xIn, float yIn): b2Vec2(xIn, yIn) 
        {
        }
        template<typename TVec2>
        vec2(const TVec2& in_vec2): b2Vec2(in_vec2.x, in_vec2.y) 
        { 
        }
        template<typename TVec2>
        inline vec2& operator = (const TVec2& in_vec2)
        {
            x = in_vec2.x;
            y = in_vec2.y;
            return *this;
        }
        inline operator glm::vec2 () const
        {
            return {x, y};
        }
        inline vec2 operator * (float scalar) const
        {
            return {x * scalar, y * scalar};
        }
    };

    // vectors
    static const vec2 NullVector { 0.0f, 0.0f };
    static const vec2 ForwardVector (1.0f, 0.0f);
    static const vec2 LateralVector (0.0f, 1.0f);

} // namespace box2d