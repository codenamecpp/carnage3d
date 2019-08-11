#pragma once

// combine rgba bytes into single unsigned int value
#define MAKE_RGBA(r,g,b,a) (((unsigned int)(r)) | (((unsigned int)(g)) << 8) | (((unsigned int)(b)) << 16) | (((unsigned int)(a)) << 24))
#define MAKE_RGB_SET_ALPHA(rgb, a) ((((unsigned int)(rgb) & 0x00FFFFFFU)) | (((unsigned int)(a)) << 24))

// predefined rgba colors

#define COLOR_RED           MAKE_RGBA(0xFF,0x00,0x00,0xFF)
#define COLOR_GREEN         MAKE_RGBA(0x00,0xFF,0x00,0xFF)
#define COLOR_DARK_GREEN    MAKE_RGBA(0x00,0x80,0x00,0xFF)
#define COLOR_ORANGE        MAKE_RGBA(0xFF,0xA5,0x00,0xFF)
#define COLOR_BLUE          MAKE_RGBA(0x00,0x00,0xFF,0xFF)
#define COLOR_DARK_BLUE     MAKE_RGBA(0x00,0x00,0xA0,0xFF)
#define COLOR_WHITE         MAKE_RGBA(0xFF,0xFF,0xFF,0xFF)
#define COLOR_BLACK         MAKE_RGBA(0x00,0x00,0x00,0xFF)
#define COLOR_CYAN          MAKE_RGBA(0x00,0xFF,0xFF,0xFF)
#define COLOR_YELLOW        MAKE_RGBA(0xFF,0xFF,0x00,0xFF)

// defines rgba color
struct Color32
{
public:
    Color32() = default;
    Color32(unsigned int rgba): mRGBA(rgba) {}
    Color32(unsigned char cr, unsigned char cg, unsigned char cb, unsigned char ca)
        : mR(cr)
        , mG(cg)
        , mB(cb)
        , mA(ca)
    {
    }
    // @param rgba: Source rgba color
    inline Color32& operator = (const Color32& rgba)
    {
        mRGBA = rgba.mRGBA;
        return *this;
    }
    // Set color components
    // @param theR, theG, theB, theA: Color components
    inline void SetComponents(unsigned char theR, unsigned char theG, unsigned char theB, unsigned char theA)
    {
        mA = theA;
        mR = theR;
        mG = theG;
        mB = theB;
    }
    // Set color components
    // @param theR, theG, theB, theA: Normalized color components in range [0, 1]
    inline void SetComponentsF(float theR, float theG, float theB, float theA)
    {
        mA = (unsigned char) (theA * 255);
        mR = (unsigned char) (theR * 255);
        mG = (unsigned char) (theG * 255);
        mB = (unsigned char) (theB * 255);
    }
public:
    union
    {
        struct
        {
            unsigned char mR;
            unsigned char mG;
            unsigned char mB;
            unsigned char mA;
        };
        unsigned int mRGBA;
    };
};

inline bool operator == (const Color32& LHS, const Color32& RHS) { return LHS.mRGBA == RHS.mRGBA; }
inline bool operator != (const Color32& LHS, const Color32& RHS) { return LHS.mRGBA != RHS.mRGBA; }

// defines point in 2d space
struct Point2D
{
public:
    Point2D() = default;
    Point2D(int posx, int posy)
        : x(posx)
        , y(posy)
    {
    }
    inline Point2D& operator += (const Point2D& point)
    {
        x += point.x;
        y += point.y;
        return *this;
    }
    inline Point2D& operator -= (const Point2D& point)
    {
        x -= point.x;
        y -= point.y;
        return *this;
    }
    inline Point2D operator -() const { return { -x, -y }; }
public:
    int x, y;
};

inline bool operator == (const Point2D& theL, const Point2D& theR) { return theL.x == theR.x && theL.y == theR.y; }
inline bool operator != (const Point2D& theL, const Point2D& theR) { return theL.x != theR.x || theL.y != theR.y; }

inline Point2D operator - (const Point2D& theL, const Point2D& theR) { return {theL.x - theR.x, theL.y - theR.y}; }
inline Point2D operator + (const Point2D& theL, const Point2D& theR) { return {theL.x + theR.x, theL.y + theR.y}; }

// defines size in 2d space
struct Size2D
{
public:
    Size2D() = default;
    Size2D(int sizex, int sizey)
        : x(sizex)
        , y(sizey)
    {
    }
    inline Size2D& operator += (const Size2D& point)
    {
        x += point.x;
        y += point.y;
        return *this;
    }
    inline Size2D& operator -= (const Size2D& point)
    {
        x -= point.x;
        y -= point.y;
        return *this;
    }
    inline Size2D operator -() const { return { -x, -y }; }
public:
    int x, y;
};

inline bool operator == (const Size2D& theL, const Size2D& theR) { return theL.x == theR.x && theL.y == theR.y; }
inline bool operator != (const Size2D& theL, const Size2D& theR) { return theL.x != theR.x || theL.y != theR.y; }

inline Size2D operator - (const Size2D& theL, const Size2D& theR) { return {theL.x - theR.x, theL.y - theR.y}; }
inline Size2D operator + (const Size2D& theL, const Size2D& theR) { return {theL.x + theR.x, theL.y + theR.y}; }

// defines rectangle in 2d space
struct Rect2D
{
public:
    Rect2D() = default;
    Rect2D(int posx, int posy, int sizex, int sizey)
        : x(posx)
        , y(posy)
        , w(sizex)
        , h(sizey)
    {
    }
public:
    int x, y;
    int w, h;
};

inline bool operator == (const Rect2D& theL, const Rect2D& theR) 
{ 
    return (theL.x == theR.x) && (theL.y == theR.y) && (theL.w == theR.w) && (theL.h == theR.h);
}

inline bool operator != (const Rect2D& theL, const Rect2D& theR)
{
    return (theL.x != theR.x) || (theL.y != theR.y) || (theL.w != theR.w) || (theL.h != theR.h);
}

enum eLogMessage
{
    eLogMessage_Debug,
    eLogMessage_Info,
    eLogMessage_Warning,
    eLogMessage_Error,
    eLogMessage_COUNT
};

inline const char* ToString(eLogMessage messageType)
{
    switch (messageType)
    {
        case eLogMessage_Debug: return "debug";
        case eLogMessage_Info: return "info";
        case eLogMessage_Warning: return "warning";
        case eLogMessage_Error: return "error";
    }
    debug_assert(false);
    return "";
}

// defines time interval in milliseconds
struct Timespan
{
public:    
    // @param timeMilliseconds: Milliseconds
    Timespan() = default;
    Timespan(long long timeMilliseconds) : mMilliseconds(timeMilliseconds)
    {
    }
    inline Timespan& operator += (const Timespan& timespan)
    {
        mMilliseconds += timespan.mMilliseconds;
        return *this;
    }
    inline Timespan& operator -= (const Timespan& timespan)
    {
        mMilliseconds -= timespan.mMilliseconds;
        return *this;
    }

	static const long long MillisecondsPerSecond = 1000LL;
	static const long long MillisecondsPerMinute = 60000LL;
	static const long long MillisecondsPerHour = 3600000LL;
	static const long long MillisecondsPerDay = 86400000LL;

    // convert time to timespan
    // @param intime: Input amount of time specified in days/hours/minutes/seconds
    static Timespan FromDays(float intime)
    {
        return (long long)(intime * MillisecondsPerDay);
    }

    static Timespan FromHours(float intime)
    {
        return (long long)(intime * MillisecondsPerHour);
    }

    static Timespan FromMinutes(float intime)
    {
        return (long long)(intime * MillisecondsPerMinute);
    }

    static Timespan FromSeconds(float intime)
    {
        return (long long)(intime * MillisecondsPerSecond);
    }

    // convert timespan to specific time values
    inline float ToMinutes() const { return (mMilliseconds * 1.0f) / (MillisecondsPerMinute * 1.0f); }
    inline float ToSeconds() const { return (mMilliseconds * 1.0f) / (MillisecondsPerSecond * 1.0f); }
    inline float ToDays() const { return (mMilliseconds * 1.0f) / (MillisecondsPerDay * 1.0f); }
    inline float ToHours() const { return (mMilliseconds * 1.0f) / (MillisecondsPerHour * 1.0f); }

public:
    long long mMilliseconds = 0;
};

inline bool operator == (const Timespan& theL, const Timespan& theR) { return theL.mMilliseconds == theR.mMilliseconds; }
inline bool operator != (const Timespan& theL, const Timespan& theR) { return theL.mMilliseconds != theR.mMilliseconds; }

inline bool operator > (const Timespan& theL, const Timespan& theR) { return theL.mMilliseconds > theR.mMilliseconds; }
inline bool operator < (const Timespan& theL, const Timespan& theR) { return theL.mMilliseconds < theR.mMilliseconds; }

inline Timespan operator - (const Timespan& theL, const Timespan& theR) { return theL.mMilliseconds - theR.mMilliseconds; }
inline Timespan operator + (const Timespan& theL, const Timespan& theR) { return theL.mMilliseconds + theR.mMilliseconds; }