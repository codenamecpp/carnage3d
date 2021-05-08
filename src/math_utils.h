#pragma once

#ifndef FLT_EPSILON
    #define FLT_EPSILON 0.0000001f
#endif

namespace cxx
{
    // get next power of two value
    inline unsigned int get_next_pot(unsigned int value)
    {
        if (value > 0)
        {
            --value;
            value |= value >> 1;
            value |= value >> 2;
            value |= value >> 4;
            value |= value >> 8;
            value |= value >> 16;
            ++value;
        }
        return value;
    }

    // test number is power of two
    inline bool is_pot(unsigned int value)
    {
        return value && !(value & (value - 1));
    }

    // test is number even or odd
    inline bool is_even(unsigned int value) { return (value % 2) == 0; }
    inline bool is_odd(unsigned int value)  { return (value % 2)  > 0; }

    // normalize angle to [-180,180), degrees
    inline float normalize_angle_180(float angleDegrees)
    {
        angleDegrees = std::fmod(angleDegrees + 180.0f, 360.0f);
        if (angleDegrees < 0.0f)
        {
            angleDegrees += 360.0f;
        }
        return angleDegrees - 180.0f;
    }

    // normalize angle to range [0, 360), degrees
    inline float normalize_angle_360(float angleDegrees)
    {
        angleDegrees = fmod(angleDegrees, 360.0f);
        if (angleDegrees < 0.0f)
        {
            angleDegrees += 360.0f;
        }
        return angleDegrees;
    }

    // round up value
    inline unsigned int round_up_to(unsigned int value, unsigned int pad)
    {
        if (pad > 0 && (value % pad))
        {
            const unsigned int value_to_add = pad - (value % pad);
            return value + value_to_add;
        }
        return value;
    }

    inline unsigned int align_up(unsigned int value, unsigned int alignment)
    {
        if (alignment > 0)
        {
            value = ((value) + (alignment - 1)) & ~(alignment - 1);
        }
        return value;
    }

    inline unsigned int aligh_down(unsigned int value, unsigned int alignment)
    {
        if (alignment > 0)
        {
            value = value & ~(alignment - 1);
        }
        return value;
    }

    inline bool is_aligned(unsigned int value, unsigned alignment)
    {
        return ((value) & (alignment - 1)) == 0;
    }

    // loops the value within range [0, 1]
    inline float repeat(float value, float length)
    {
        return value - glm::floor(value / length) * length;
    }

    // ping-pongs the value in range [0, 1]
    inline float ping_pong(float value, float length)
    {
        value = repeat(value, length * 2.0f);
        return length - glm::abs(value - length);
    }

    // 2d cross product
    template <typename TTuple2>
    inline float cross2(const TTuple2& lhs, const TTuple2& rhs)
    {
        return lhs.x * rhs.y - lhs.y * rhs.x;
    }

    // floats comparsion
    inline bool equals(float lhs, float rhs)
    {
        return fabsf(lhs - rhs) < FLT_EPSILON;
    }

    inline bool equals_zero(float value)
    {
        return fabsf(value) < FLT_EPSILON;
    }

} // namespace cxx