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
        angleDegrees = fmod(angleDegrees + 180.0f, 360.0f);
        if (angleDegrees < 0.0f)
        {
            angleDegrees += 360.0f;
        }
        return angleDegrees - 180.0f;
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

    template<typename TVector2D>
    inline TVector2D rotate_around_center(TVector2D point, const TVector2D& center, float angleRadians) 
    {
        // substract center to use simplyfied rotation
        point -= center;
        
        float cos_a = cos(angleRadians);
        float sin_a = sin(angleRadians);
        
        float tmp_x = cos_a*point.x - sin_a*point.y;
        float tmp_y = sin_a*point.x + cos_a*point.y;
        
        point.x = tmp_x;
        point.y = tmp_y;
        
        // add center to move point to its original center position
        point += center;
        return point;
    }

} // namespace cxx