#pragma once

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

    // test whether value is power of two
    inline bool is_pot(unsigned int value)
    {
        unsigned int nextPowerOfTwo = get_next_pot(value);
        return nextPowerOfTwo == value;
    }

    // normalize angle to range [-180, 180)
    // @param angleDegrees: Angle in degrees
    inline float normalize_angle_180(float angleDegrees)
    {
        angleDegrees = std::fmod(angleDegrees + 180.0f, 360.0f);
        if (angleDegrees < 0.0f)
        {
            angleDegrees += 360.0f;
        }
        return angleDegrees - 180.0f;
    }
} // namespace cxx