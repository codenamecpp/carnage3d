#pragma once

//////////////////////////////////////////////////////////////////////////

// map direction
enum eMapDirection2D
{
    eMapDirection2D_None, // null direction
    // straight
    eMapDirection2D_N,
    eMapDirection2D_E,
    eMapDirection2D_S,
    eMapDirection2D_W,
    // diagonals
    eMapDirection2D_NE,
    eMapDirection2D_NW,
    eMapDirection2D_SE,
    eMapDirection2D_SW,
};

//////////////////////////////////////////////////////////////////////////

inline bool IsMapDirectionStraight(eMapDirection2D direction)
{
    return (direction == eMapDirection2D_N) ||
        (direction == eMapDirection2D_E) ||
        (direction == eMapDirection2D_S) ||
        (direction == eMapDirection2D_W);
}

inline bool IsMapDirectionDiagonal(eMapDirection2D direction)
{
    return (direction == eMapDirection2D_NE) ||
        (direction == eMapDirection2D_SE) ||
        (direction == eMapDirection2D_SW) ||
        (direction == eMapDirection2D_NW);
}

//////////////////////////////////////////////////////////////////////////

inline eMapDirection2D GetStraightMapDirectionFromHeading(cxx::angle_t headingAngle)
{
    static const std::pair<float, eMapDirection2D> Directions[] =
    {
        {360.0f, eMapDirection2D_E},
        {  0.0f, eMapDirection2D_E},
        { 90.0f, eMapDirection2D_S},
        {180.0f, eMapDirection2D_W},
        {270.0f, eMapDirection2D_N},
    };

    float angleDegrees = cxx::wrap_angle_360(headingAngle.mDegrees);

    for (const auto& curr: Directions)
    {
        if (fabs(curr.first - angleDegrees) <= 45.0f)
            return curr.second;
    }
    debug_assert(false);
    return eMapDirection2D_E;
}

//////////////////////////////////////////////////////////////////////////

inline eMapDirection2D GetStraightMapDirectionCW(eMapDirection2D dir)
{
    switch (dir)
    {    
        case eMapDirection2D_N: return eMapDirection2D_E;
        case eMapDirection2D_E: return eMapDirection2D_S;
        case eMapDirection2D_S: return eMapDirection2D_W;
        case eMapDirection2D_W: return eMapDirection2D_N;
    }
    debug_assert(false);
    return eMapDirection2D_E;
}

inline eMapDirection2D GetStraightMapDirectionCCW(eMapDirection2D dir)
{
    switch (dir)
    {
        case eMapDirection2D_N: return eMapDirection2D_W;
        case eMapDirection2D_E: return eMapDirection2D_N;
        case eMapDirection2D_S: return eMapDirection2D_E;
        case eMapDirection2D_W: return eMapDirection2D_S;
    }
    debug_assert(false);
    return eMapDirection2D_E;
}

inline eMapDirection2D GetStraightMapDirectionOpposite(eMapDirection2D dir)
{
    switch (dir)
    {
        case eMapDirection2D_N: return eMapDirection2D_S;
        case eMapDirection2D_E: return eMapDirection2D_W;
        case eMapDirection2D_S: return eMapDirection2D_N;
        case eMapDirection2D_W: return eMapDirection2D_E;
    }
    debug_assert(false);
    return eMapDirection2D_E;
}

//////////////////////////////////////////////////////////////////////////