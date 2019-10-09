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

    // thin float angle wrapper
    struct angle_t final
    {
    public:
        enum class units { degrees, radians };

        angle_t() = default;
        // init angle
        explicit angle_t(float angleValue, units angleUnits)
        {
            if (angleUnits == units::radians)
            {
                mDegrees = glm::degrees(angleValue);
            }
            else
            {
                mDegrees = angleValue;
            }
        }

        // convert angle value to angle_t
        static angle_t from_radians(float angleValue) { return angle_t { angleValue, units::radians }; }
        static angle_t from_degrees(float angleValue) { return angle_t { angleValue, units::degrees }; }

        // convert angle_t to angle value
        inline float to_radians() const { return glm::radians(mDegrees); }
        inline float to_degrees() const { return mDegrees; }

        // init angle
        inline void set_angle(float angleValue, units angleUnits)
        {
            if (angleUnits == units::radians)
            {
                mDegrees = glm::degrees(angleValue);
            }
            else
            {
                mDegrees = angleValue;
            }
        }

        // normalize angle to range [-180, 180)
        inline void normalize_angle_180()
        {
            mDegrees = fmod(mDegrees + 180.0f, 360.0f);
            if (mDegrees < 0.0f)
            {
                mDegrees += 360.0f;
            }
            mDegrees = mDegrees - 180.0f;
        }

        // get sin and cos for angle
        // @param outSin: Output sin value
        // @param outCos: Output cos value
        inline void get_sin_cos(float& outSin, float& outCos) const
        {
            float angleRadians = to_radians();

            outSin = sin(angleRadians);
            outCos = cos(angleRadians);
        }

        // test whether angle is nearly zero
        inline bool is_zero() const { return fabs(mDegrees) < 0.01f; }
        inline bool non_zero() const { return fabs(mDegrees) >= 0.01f; }

        // operators
        inline angle_t operator + (angle_t rhs) const { return from_degrees(mDegrees + rhs.mDegrees); }
        inline angle_t operator - (angle_t rhs) const { return from_degrees(mDegrees - rhs.mDegrees); }
        inline angle_t operator - () const { return from_degrees(-mDegrees); }
        inline angle_t operator * (angle_t rhs) const { return from_degrees(mDegrees * rhs.mDegrees); }
        inline angle_t operator / (angle_t rhs) const { return from_degrees(mDegrees / rhs.mDegrees); }
        inline angle_t operator % (angle_t rhs) const { return from_degrees(fmod(mDegrees, rhs.mDegrees)); }

        inline angle_t& operator %= (angle_t rhs) { mDegrees = fmod(mDegrees, rhs.mDegrees); return *this; }
        inline angle_t& operator *= (angle_t rhs) { mDegrees *= rhs.mDegrees; return *this; }
        inline angle_t& operator /= (angle_t rhs) { mDegrees /= rhs.mDegrees; return *this; }
        inline angle_t& operator += (angle_t rhs) { mDegrees += rhs.mDegrees; return *this; }
        inline angle_t& operator -= (angle_t rhs) { mDegrees -= rhs.mDegrees; return *this; }

        inline bool operator < (angle_t rhs) const { return mDegrees < rhs.mDegrees; }
        inline bool operator > (angle_t rhs) const { return mDegrees > rhs.mDegrees; }
        inline bool operator <= (angle_t rhs) const { return mDegrees <= rhs.mDegrees; }
        inline bool operator >= (angle_t rhs) const { return mDegrees >= rhs.mDegrees; }
        inline bool operator == (angle_t rhs) const { return mDegrees == rhs.mDegrees; }
        inline bool operator != (angle_t rhs) const { return mDegrees != rhs.mDegrees; }

    public:
        float mDegrees = 0.0f;
    };

    template<typename TVector2D>
    inline TVector2D rotate_around_center(TVector2D point, const TVector2D& center, angle_t rotationAngle) 
    {
        // substract center to use simplyfied rotation
        point -= center;
        
        float cos_a;
        float sin_a;
        rotationAngle.get_sin_cos(sin_a, cos_a);
        
        float tmp_x = cos_a*point.x - sin_a*point.y;
        float tmp_y = sin_a*point.x + cos_a*point.y;
        
        point.x = tmp_x;
        point.y = tmp_y;
        
        // add center to move point to its original center position
        point += center;
        return point;
    }


} // namespace cxx