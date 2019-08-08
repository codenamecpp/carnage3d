#pragma once

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

// target opengl version code
#define OPENGL_CONTEXT_MAJOR_VERSION 3
#define OPENGL_CONTEXT_MINOR_VERSION 2

#define BUFFER_OFFSET(offset) ((unsigned char*)0 + (offset))