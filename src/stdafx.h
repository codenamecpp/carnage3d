#pragma once

#include "macro.h"

#ifdef _UNICODE
    #error Unicode is unsupported!
#endif
#ifdef UNICODE
    #error Unicode is unsupported!
#endif

#pragma warning ( disable : 4351 ) // new behavior: elements of array will be default initialized
#pragma warning ( disable : 4201 ) // nonstandard extension used: nameless struct/union

#include <stdlib.h>

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