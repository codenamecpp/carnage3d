#pragma once

// detect operation system

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

#ifdef _UNICODE
    #error Unicode is unsupported!
#endif
#ifdef UNICODE
    #error Unicode is unsupported!
#endif

#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS

#pragma warning ( disable : 4351 ) // new behavior: elements of array will be default initialized
#pragma warning ( disable : 4201 ) // nonstandard extension used: nameless struct/union

#include <stdlib.h>

#if OS_NAME == OS_WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <windows.h>
#ifdef _DEBUG
    #define _CRTDBG_MAP_ALLOC
#endif
    #include <crtdbg.h>
#elif OS_NAME == OS_LINUX
    #include <limits.h>
    #include <unistd.h>
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

// small c++ std templates library extensions

template <typename Array> struct ArrayType;
template <typename TElement, int NumElements> 
struct ArrayType<TElement[NumElements]>
{
    enum { Countof = NumElements };
};
template <typename TElement, int NumElements>
constexpr int CountOf(const TElement(&)[NumElements])
{
    return NumElements;
}
template <typename TElement>
inline void SafeDelete(TElement*& elementPointer)
{
    if (elementPointer)
    {
        delete elementPointer;
        elementPointer = nullptr;
    }
}
template<typename TElement>
inline void SafeDeleteArray(TElement*& elementPointer)
{
    if (elementPointer)
    {
        delete [] elementPointer;
        elementPointer = nullptr;
    }
}

// stdlib
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdarg.h>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <deque>
#include <list>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <cctype>
#include <chrono>
#include <thread>

// physics
#include <Box2D.h>

// opengl
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

// glm
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/glm.hpp>

// lib
#include "enum_utils.h"
#include "aux_math.h"
#include "geometries.h"
#include "frustum.h"
#include "handle.h"
#include "intrusive_list.h"
#include "memory_istream.h"
#include "noncopyable.h"
#include "object_pool.h"
#include "randomizer.h"
#include "strings.h"
#include "path_utils.h"
#include "config_document.h"

// app
#include "CommonTypes.h"
#include "Console.h"
#include "Inputs.h"
#include "System.h"
#include "FileSystem.h"
#include "GraphicsDevice.h"
#include "GameCamera.h"
#include "SpriteAnimation.h"
#include "GameRules.h"
#include "GameMapHelpers.h"
#include "GuiSystem.h"
#include "PixelsArray.h"
#include "GameDefs.h"