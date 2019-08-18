#pragma once

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

#ifdef _DEBUG
    #define _CRTDBG_MAP_ALLOC
#endif

#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
    #define debug_assert(expr) _ASSERTE(expr)
    #define release_assert(expr) _ASSERTE(expr)
#else
    #define debug_assert(expr)
    #define release_assert(expr) _ASSERTE(expr)
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
#include <unordered_map>

// opengl
#include <gl/glew.h>
#include <gl/wglew.h>
#include <gl/GL.h>
#include <glfw/glfw3.h>

// glm
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/glm.hpp>

// lib
#include "aux_math.h"
#include "geometries.h"
#include "frustum.h"
#include "heap.h"
#include "handle.h"
#include "intrusive_list.h"
#include "memory_istream.h"
#include "aux_stl.h"
#include "object_pool.h"
#include "randomizer.h"
#include "refcounted.h"
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
#include "CarnageGame.h"
#include "PixelsArray.h"
#include "GameDefs.h"