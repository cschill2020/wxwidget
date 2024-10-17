#ifndef OGL_H
#define OGL_H

// Include GLEW
#include <GL/glew.h>

#include <string>
#include <vector>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "ogl_error.h"
#include "shader.h"

/*
  ************  NOTES  *******************************************************
  This is not an OGL tutorial, I mean, this is just a sample of how OGL stuff
  may be arranged. I tried to keep it simple.
  It's likely that the novice to OGL reads this sample, so here are some brief
  notes:
  * Starting with OpenGL >= 2.0 the user defines some special programs that are
    going to be executed in the GPU, not in the CPU. These programs are called
    "Shaders". Since OGL >= 3.2, and if a "Core Profile" context is set, the use
    of shaders is the only way to make an OGL application.
  * A program consists (at least) of a 'vertex shader' who operates on the
    vertices that define the primitive (a triangle, a line, etc) and a 'fragment
    shader' which deals with fragments (points) interpolated from the previously
    processed vertices in the vertex shader.
  * Shaders must be compiled and linked, both operations are done on the GPU.
  * Shaders are written in the GLSL language, that looks like C, but it isn't C.
  * Data (vertices, textures) are stored in GPU memory, so they don't need to be
    loaded each time the scene must be redrawn.
  * Rotations and translations are matrix operations that the GPU may do in the
    [vertex]shaders. The user must define the needed matrices.
  * A vertex shader defines special vars ("attributes") used for reading the
    data from the GPU buffers. Other special vars ("uniforms") are used for
    values that don't change with each vertex (i.e. the matrices).
  * The relationship between a data buffer and the input attributes in a vertex
    shader can be saved in a "Vertex Array Object" (VAO).

  I use several classes for typical OGL jobs: Shaders, Data, Camera.
  Because different GPU cards may behave on their own, I also use very often
  an error-helper. It will inform of the point where something went wrong.

  I decided to keep apart all of this from wxWidgets. You won't find anything
  related to wxWidgets in the oglstuff[.h][.cpp] files.
  That's why I use std::vector and std::string instead of those provided by wx.
*/



//-----------------------------------------------------------------------------
// A "point light"
class myLight
{
public:
    myLight() {}
    ~myLight() {}

    void Set(const glm::vec3& position, GLfloat intensity,
             GLfloat R, GLfloat G, GLfloat B);
    // Return position and intensity
    const GLfloat* GetFLightPos() const
        { return _PosAndIntensisty; }
    // Return color
    const GLfloat* GetFLightColor() const
        { return _Color; }

private:
    // Light position and intensity
    GLfloat _PosAndIntensisty[4];
    // Light color
    GLfloat _Color[3];
};

//-----------------------------------------------------------------------------
// The "camera", or the point of view

// Defines several possible options for camera movement.
enum CAMERA_MOVEMENT {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

class myOGLCamera
{
public:
    myOGLCamera();
    ~myOGLCamera() {}

    // Initial positions
    void InitCamera();
    void SetDeltaT(long long deltaT) {
        _deltaT = deltaT;
    }
    // When the size of the window changes
    void ViewSizeChanged(int newWidth, int newHeight);
    // Get View and Projection matrices from camera
    void GetViewAndProjection(glm::mat4* view, glm::mat4* proj);
    vec3 GetCameraPosition() { return _camPosition; }

    // Rotates Camera around the world when the timer is active.
    // Not currently used, but could make a good screensaver
    void TimerRotation(float angle);
    // Translate the camera when mouse is pressed.
    void KeyboardPan(CAMERA_MOVEMENT direction, bool start);
    // Simulates a virtual trackball and rotates the 'world'
    void MouseRotation(int fromX, int fromY, int toX, int toY, GLboolean constrainPitch = true);
    void MouseScroll(int delta);

private:
    void UpdateCameraPosition();
    void UpdateCameraOrientation();

    struct motion {
        bool forward;
        bool backward;
        bool strafe_left;
        bool strafe_right;
    } _motion;
    long long _deltaT;

    // Window size in pixels
    int _winWidth;
    int _winHeight;

    // Parameters for the projection
    double _fov;
    double _aspect;
    double _nearD;
    double _farD;

    // World attributes
    double _worldRadius;
    glm::vec3 _worldCenter;
    glm::vec3 _worldUp;

    // Camera Attributes
    glm::vec3 _camPosition;
    glm::vec3 _camFront;
    glm::vec3 _camUp;
    glm::vec3 _camRight;

    // euler Angles
    float _camYaw;
    float _camPitch;

    // camera options
    float _camMovementSpeed;
    float _mouseSensitivity;
};

struct Vertex {
    // position
    glm::vec3 position;
    // color
    glm::vec4 color;
    // // normal
    glm::vec3 normal;
    // // texCoords
    // glm::vec2 texCoord;
};
//-----------------------------------------------------------------------------
// An object for triangle meshes.
class OGLMesh
{
public:
    OGLMesh();
    ~OGLMesh();

    // Clean up
    void Clear();
    // Load data into the GPU
    // void SetBuffers(Shader* theShader, GLsizei numPoints, GLsizei numFaces,
    //                 const GLfloat* vert, const GLfloat* colo, 
    //                 const GLfloat* norm, const GLfloat* uv, const GLushort* indices);
    void SetBuffers(Shader& shader, GLsizei numPoints, GLsizei numFaces,
                    const GLfloat* verts, const GLfloat* colors, 
                    const GLfloat* norms, const GLfloat* uvs, const unsigned int* indices);
    //Draw the triangles
    void Draw(bool useIndices = true);

private:
    void SetupMesh(Shader& shader);

    std::vector<Vertex> _vertices;
    std::vector<unsigned int> _indices;

    //GLsizei _numFaces;
    GLuint _VAO, _VBO, _EBO;
};

//-----------------------------------------------------------------------------
// An object for strings
class myOGLString
{
public:
    myOGLString();
    ~myOGLString();

    // Clean up
    void Clear();
    // Load data into the GPU
    void SetStringWithVerts(Shader* theShader,
                             const unsigned char* tImage, int tWidth, int tHeigh,
                             const GLfloat* vert, const GLfloat* norm);
    // Draw the string
    void Draw(const glm::mat4& unifMvp, const glm::mat4& unifToVw,
              const myLight* theLight);


private:
    GLuint _bufPosId;    // Buffer id
    GLuint _stringVAO;   // Vertex Arrays Object
    GLuint _textureUnit; // The context unit
    GLuint _textureId;   // Texture name
    // GPU Program used to draw the texture
    Shader* _stringShaders;
};

//-----------------------------------------------------------------------------
// An object for the immutable string
class myOGLImmutString : public myOGLString
{
public:
    myOGLImmutString(){}
    ~myOGLImmutString(){}

    // Load data into the GPU
    void SetImmutString(Shader* theShader,
                        const unsigned char* tImage, int tWidth, int tHeigh);
    // Update orthogonal projection matrix
    void SetOrtho(float winWidth, float winHeight);
    // The transformation matrix
    const glm::mat4& GetFloatMVP() { return _Ortho; }

private:
    glm::mat4 _Ortho; // The orthogonal projection matrix
};

//-----------------------------------------------------------------------------
// General manager
class myOGLManager
{
public:
    myOGLManager(OGLErrHandler* extErrHnd = NULL);
    ~myOGLManager();

    // Constants, prototypes and pointers to OGL functions
    static bool Init();
    // Strings describing the current GL connection
    const GLubyte* GetGLVersion();
    const GLubyte* GetGLVendor();
    const GLubyte* GetGLRenderer();

    myOGLCamera& GetCamera() { return _Camera; }

    // Load data into the GPU
    void SetShadersAndTriangles();
    // // Load the texture for the string in the pyramid
    // void SetStringOnPyr(const unsigned char* strImage, int iWidth, int iHeigh);
    // // Load the texture for the immutable string
    // void SetImmutableString(const unsigned char* strImage, int iWidth, int iHeigh);

    // For window size change
    void SetViewport(int x, int y, int width, int height);
    void SetFrameCount(unsigned long frameCnt) {
        _frameCnt = frameCnt;
    }

    void Render(); // Total rendering

private:
    myLight        _Light;
    myOGLCamera    _Camera;

    Shader   _pyramidShaders;
    Shader   _lightCubeShaders;
    // Shader   _StringShaders;
    // Shader   _ImmutStringSha;

    OGLMesh    _pyramidMesh;
    OGLMesh    _cubeMesh;
    OGLMesh    _lightCubeMesh;

    unsigned long _frameCnt;
};

#endif //OGL_H

