#pragma once

#include "GraphicsDefs.h"

#define BUFFER_OFFSET(offset) ((unsigned char*)0 + (offset))

// target opengl version code
#define OPENGL_CONTEXT_MAJOR_VERSION 3
#define OPENGL_CONTEXT_MINOR_VERSION 2

// checks current opengl error code
inline void glCheckError()
{
#ifdef _DEBUG
    GLenum errcode = ::glGetError();
    debug_assert(errcode == GL_NO_ERROR);
#else
    // nothing
#endif
}

// resets current opengl error code
inline void glClearError()
{
    for (GLenum glErrorCode = ::glGetError(); glErrorCode != GL_NO_ERROR; glErrorCode = ::glGetError()) 
    {
    }
}

// map buffer content enum to opengl specific value
inline GLenum EnumToGL(eBufferContent bufferContent)
{
    switch (bufferContent)
    {
        case eBufferContent_Vertices: return GL_ARRAY_BUFFER;
        case eBufferContent_Indices: return GL_ELEMENT_ARRAY_BUFFER;
    }
    debug_assert(false);
    return GL_ARRAY_BUFFER;
}

inline GLenum EnumToGL(eBufferUsage usageHint)
{
    switch (usageHint)
    {
        case eBufferUsage_Static: return GL_STATIC_DRAW;
        case eBufferUsage_Dynamic: return GL_DYNAMIC_DRAW;
        case eBufferUsage_Stream: return GL_STREAM_DRAW;
    }
    debug_assert(false);
    return GL_STATIC_DRAW;
}

inline GLenum EnumToGL(ePrimitiveType primitiveType)
{
    switch (primitiveType)
    {
        case ePrimitiveType_TriangleStrip: return GL_TRIANGLE_STRIP;
        case ePrimitiveType_Points: return GL_POINTS;
        case ePrimitiveType_Lines: return GL_LINES;
        case ePrimitiveType_LineLoop: return GL_LINE_LOOP;
        case ePrimitiveType_Triangles: return GL_TRIANGLES;
        case ePrimitiveType_TriangleFan: return GL_TRIANGLE_FAN;
    }
    debug_assert(false);
    return GL_TRIANGLES;
}