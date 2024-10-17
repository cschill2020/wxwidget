#ifndef OGL_ERROR_H
#define OGL_ERROR_H

// Include GLEW
#include <GL/glew.h>

// Define our own GL errors
enum
{
    OGL_ERR_CLEAR = 0,
    OGL_ERR_JUSTLOG,

    OGL_ERR_SHADERCREATE,
    OGL_ERR_SHADERCOMPILE,
    OGL_ERR_SHADERLINK,
    OGL_ERR_SHADERLOCATION,

    OGL_ERR_BUFFER,
    OGL_ERR_TEXTIMAGE,

    OGL_ERR_DRAWING_TRI,
    OGL_ERR_DRAWING_STR
};

// Used to handle GL errors in other part of the app.
typedef void OGLErrHandler(int err, int glerr, const GLchar* glMsg);
// External function for GL errors
extern OGLErrHandler* externalOGL_ErrHandler; // = nullptr;
// Allow GL errors to be handled in other part of the app.
inline bool OnGLError(int err, const GLchar* glMsg = nullptr) {
    GLenum GLErrorVal = glGetError();

    if ( err == OGL_ERR_CLEAR )
    {
        // Clear previous errors
        while ( GLErrorVal != GL_NO_ERROR )
            GLErrorVal = glGetError();
        return true;
    }

    if ( (GLErrorVal == GL_NO_ERROR) && (glMsg == NULL) )
        return true;

    if ( externalOGL_ErrHandler )
    {
        // Use the external error message handler. We pass our err-enum value.
        externalOGL_ErrHandler(err, GLErrorVal, glMsg);
    }

    return err == OGL_ERR_JUSTLOG ? true : false;
}

#endif //OGL_ERROR_H
