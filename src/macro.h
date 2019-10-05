#pragma once

#define OS_WINDOWS  1
#define OS_LINUX    2
#define OS_UNIX     3
#define OS_UNKNOWN  0    

#if defined(_WIN32)
    #define OS_NAME OS_WINDOWS
#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
    #define OS_NAME OS_LINUX
#else
    #define OS_NAME OS_UNKNOWN
#endif

#if OS_NAME == OS_WINDOWS
    #define _CRT_SECURE_NO_WARNINGS
    #define _SCL_SECURE_NO_WARNINGS

#ifdef _DEBUG
    #define _CRTDBG_MAP_ALLOC
    #include <crtdbg.h>
#endif

#elif OS_NAME == OS_LINUX
    #include <assert.h>
#endif

#ifdef _DEBUG
    #if OS_NAME == OS_WINDOWS
        #define debug_assert(expr) _ASSERTE(expr)
        #define release_assert(expr)
    #else
        #define debug_assert(expr) assert(expr)
        #define release_assert(expr)
    #endif
#else
    #define debug_assert(expr)
    #define release_assert(expr)
#endif

#if OS_NAME == OS_WINDOWS
    #define cxx_stricmp _stricmp
    #define cxx_strnicmp _strnicmp
#else
    #include <strings.h>
    #define cxx_stricmp strcasecmp
    #define cxx_strnicmp strncasecmp
#endif