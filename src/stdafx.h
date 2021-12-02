#pragma once

#include "macro.h"

#ifdef _UNICODE
    #error Unicode is unsupported!
#endif
#ifdef UNICODE
    #error Unicode is unsupported!
#endif

#if OS_NAME == OS_WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <windows.h>
    #include <mmsystem.h> // for multimedia timers
#elif OS_NAME == OS_LINUX || OS_NAME == OS_MACOS
    #include <limits.h>
    #include <unistd.h>
#endif

#ifdef __EMSCRIPTEN__
    #include <emscripten/emscripten.h>
    #define GLFW_INCLUDE_ES3
#endif

#pragma warning ( disable : 4351 ) // new behavior: elements of array will be default initialized
#pragma warning ( disable : 4201 ) // nonstandard extension used: nameless struct/union
#pragma warning ( disable : 4100 ) // unreferenced formal parameter

#include <stdlib.h>

// stdlib
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
#include <sstream>
#include <stdint.h>
#include <cctype>
#include <chrono>
#include <thread>
#include <functional>

// opengl
#include <GL/glew.h>
#if OS_NAME == OS_MACOS
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include "GLFW/glfw3.h"

// glm
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/glm.hpp>

// physics
#include <box2d/box2d.h>

// lib
#include "common_utils.h"
#include "rtti.h"
#include "enum_utils.h"
#include "math_defs.h"
#include "math_utils.h"
#include "handle.h"
#include "intrusive_list.h"
#include "memory_istream.h"
#include "noncopyable.h"
#include "object_pool.h"
#include "randomizer.h"
#include "strings.h"
#include "path_utils.h"
#include "json_document.h"
#include "mem_allocators.h"
#include "iostream_utils.h"

#include "game_version.h"
// app
#include "CommonTypes.h"
#include "Console.h"
#include "InputsManager.h"
#include "System.h"
#include "FileSystem.h"
#include "GraphicsDevice.h"
#include "GameCamera.h"
#include "SpriteAnimation.h"
#include "GameParams.h"
#include "GameMapHelpers.h"
#include "GuiManager.h"
#include "PixelsArray.h"
#include "GameDefs.h"
#include "Convert.h"
