#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>

#include "ogl.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../stb/stb_image.h"

OGLErrHandler* externalOGL_ErrHandler = nullptr;

// ----------------------------------------------------------------------------
// Data for a regular tetrahedron with edge length 200, centered at the origin
// ----------------------------------------------------------------------------
const GLfloat gVerts[] = { 
    0.8165f, -0.333f, -0.4714f,
    0.0f, -0.333f, 0.9428f,
    -0.8165f, -0.333f, -0.4714f,
    0.0f, 1.0f,   0.0f 
};

// Transparency (to see also inner faces) is in the last triangle only,
// so that glEnable(GL_BLEND) works well
const GLfloat gColors[] = { 0.0f, 1.0f, 0.0f, 1.0f,
                             1.0f, 0.0f, 0.0f, 1.0f,
                             0.0f, 0.0f, 1.0f, 1.0f,
                             1.0f, 1.0f, 0.0f, 1.0f }; //With transparency

// Texture UV coordinates.
const GLfloat gUV[] = { 0.0f, 0.0f,
                        0.0f, 1.0f,
                        1.0f, 0.0f,
                        1.f, 1.0f };

// Normals heading outside of the tetrahedron
const GLfloat gNormals[] = { 0.0f,    -1.0f,      0.0f,     /* face 0 1 2 */
                            -0.81650f, 0.33333f,  0.47140f, /* face 1 2 3 */
                            0.0f,     0.33333f, -0.94281f, /* face 2 3 0 */
                            0.81650f, 0.33333f,  0.47140f  /* face 3 0 1 */ };

// Order would be important if we were using face culling
const unsigned int gIndices[] = { 0, 1, 2, 
                                  1, 2, 3,
                                  2, 3, 0,
                                  3, 0, 1 };

const GLfloat cubeVerts[] = {
    -0.5f, -0.5f, -0.5f, 
    0.5f, -0.5f, -0.5f,  
    0.5f,  0.5f, -0.5f,  
    0.5f,  0.5f, -0.5f,  
    -0.5f,  0.5f, -0.5f, 
    -0.5f, -0.5f, -0.5f, 

    -0.5f, -0.5f,  0.5f, 
    0.5f, -0.5f,  0.5f,  
    0.5f,  0.5f,  0.5f,  
    0.5f,  0.5f,  0.5f,  
    -0.5f,  0.5f,  0.5f, 
    -0.5f, -0.5f,  0.5f, 

    -0.5f,  0.5f,  0.5f, 
    -0.5f,  0.5f, -0.5f, 
    -0.5f, -0.5f, -0.5f, 
    -0.5f, -0.5f, -0.5f, 
    -0.5f, -0.5f,  0.5f, 
    -0.5f,  0.5f,  0.5f, 

    0.5f,  0.5f,  0.5f,  
    0.5f,  0.5f, -0.5f,  
    0.5f, -0.5f, -0.5f,  
    0.5f, -0.5f, -0.5f,  
    0.5f, -0.5f,  0.5f,  
    0.5f,  0.5f,  0.5f,  

    -0.5f, -0.5f, -0.5f, 
    0.5f, -0.5f, -0.5f,  
    0.5f, -0.5f,  0.5f,  
    0.5f, -0.5f,  0.5f,  
    -0.5f, -0.5f,  0.5f, 
    -0.5f, -0.5f, -0.5f, 

    -0.5f,  0.5f, -0.5f, 
    0.5f,  0.5f, -0.5f,  
    0.5f,  0.5f,  0.5f,  
    0.5f,  0.5f,  0.5f,  
    -0.5f,  0.5f,  0.5f, 
    -0.5f,  0.5f, -0.5f, 
};

const GLfloat cubeNorms[] = {
    0.0f,  0.0f, -1.0f,
    0.0f,  0.0f, -1.0f,
    0.0f,  0.0f, -1.0f,
    0.0f,  0.0f, -1.0f,
    0.0f,  0.0f, -1.0f,
    0.0f,  0.0f, -1.0f,

    0.0f,  0.0f,  1.0f,
    0.0f,  0.0f,  1.0f,
    0.0f,  0.0f,  1.0f,
    0.0f,  0.0f,  1.0f,
    0.0f,  0.0f,  1.0f,
    0.0f,  0.0f,  1.0f,

    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,

    1.0f,  0.0f,  0.0f,
    1.0f,  0.0f,  0.0f,
    1.0f,  0.0f,  0.0f,
    1.0f,  0.0f,  0.0f,
    1.0f,  0.0f,  0.0f,
    1.0f,  0.0f,  0.0f,

    0.0f, -1.0f,  0.0f,
    0.0f, -1.0f,  0.0f,
    0.0f, -1.0f,  0.0f,
    0.0f, -1.0f,  0.0f,
    0.0f, -1.0f,  0.0f,
    0.0f, -1.0f,  0.0f,

    0.0f,  1.0f,  0.0f,
    0.0f,  1.0f,  0.0f,
    0.0f,  1.0f,  0.0f,
    0.0f,  1.0f,  0.0f,
    0.0f,  1.0f,  0.0f,
    0.0f,  1.0f,  0.0f
};

const GLfloat cubeColors[] = {
    1.0f, 0.5f, 0.31f, 1.0f,
    1.0f, 0.5f, 0.31f, 1.0f,
    1.0f, 0.5f, 0.31f, 1.0f,
    1.0f, 0.5f, 0.31f, 1.0f,
    1.0f, 0.5f, 0.31f, 1.0f,
    1.0f, 0.5f, 0.31f, 1.0f,

    1.0f, 0.5f, 0.31f, 1.0f,
    1.0f, 0.5f, 0.31f, 1.0f,
    1.0f, 0.5f, 0.31f, 1.0f,
    1.0f, 0.5f, 0.31f, 1.0f,
    1.0f, 0.5f, 0.31f, 1.0f,
    1.0f, 0.5f, 0.31f, 1.0f,

    1.0f, 0.5f, 0.31f, 1.0f,
    1.0f, 0.5f, 0.31f, 1.0f,
    1.0f, 0.5f, 0.31f, 1.0f,
    1.0f, 0.5f, 0.31f, 1.0f,
    1.0f, 0.5f, 0.31f, 1.0f,
    1.0f, 0.5f, 0.31f, 1.0f,

    1.0f, 0.5f, 0.31f, 1.0f,
    1.0f, 0.5f, 0.31f, 1.0f,
    1.0f, 0.5f, 0.31f, 1.0f,
    1.0f, 0.5f, 0.31f, 1.0f,
    1.0f, 0.5f, 0.31f, 1.0f,
    1.0f, 0.5f, 0.31f, 1.0f,

    1.0f, 0.5f, 0.31f, 1.0f,
    1.0f, 0.5f, 0.31f, 1.0f,
    1.0f, 0.5f, 0.31f, 1.0f,
    1.0f, 0.5f, 0.31f, 1.0f,
    1.0f, 0.5f, 0.31f, 1.0f,
    1.0f, 0.5f, 0.31f, 1.0f,

    1.0f, 0.5f, 0.31f, 1.0f,
    1.0f, 0.5f, 0.31f, 1.0f,
    1.0f, 0.5f, 0.31f, 1.0f,
    1.0f, 0.5f, 0.31f, 1.0f,
    1.0f, 0.5f, 0.31f, 1.0f,
    1.0f, 0.5f, 0.31f, 1.0f
};

glm::vec3 pyramidPositions[] = {
    glm::vec3( 0.0f,  0.0f,  0.0f),
    glm::vec3( 2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3( 2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3( 1.3f, -2.0f, -2.5f),
    glm::vec3( 1.5f,  2.0f, -2.5f),
    glm::vec3( 1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
};

// ----------------------------------------------------------------------------
// Shaders
// ----------------------------------------------------------------------------
// Note: We use GLSL 1.50 which is the minimum starting with OpenGL >= 3.2 (2009)
// Apple supports OpenGL 3.2 since OS X 10.7 "Lion" (2011)

// Vertex shader for the triangles
const GLchar* triangVertexShader =
{
    "#version 150 \n"

    "in vec3 in_Position; \n"
    "in vec4 in_Color; \n"
    "in vec3 in_Normal; \n"
    "uniform mat4 mMVP; \n"
    "uniform mat4 mToViewSpace; \n"

    "flat out vec4 theColor; \n"
    "flat out vec3 theNormal; \n"
    "out vec3 pointPos; \n"

    "void main(void) \n"
    "{\n"
    "    gl_Position = mMVP * vec4(in_Position, 1.0); \n"
    "    theColor = in_Color; \n"

    "    // Operations in View Space \n"
    "    vec4 temp4 = mToViewSpace * vec4(in_Position, 1.0); \n"
    "    pointPos = temp4.xyz; \n"
    "    temp4 = mToViewSpace * vec4(in_Normal, 0.0); \n"
    "    theNormal = normalize(temp4.xyz); \n"
    "}\n"
};

// Common function for fragment shaders
const GLchar* illuminationShader =
{
    "#version 15 \n"

    "vec3 Illuminate(in vec4 LiProps, in vec3 LiColor, in vec4 PColor, \n"
    "                in vec3 PNormal, in vec3 PPos) \n"
    "{\n"
    "    // Ambient illumination. Hardcoded \n"
    "    vec3 liAmbient = vec3(0.2, 0.2, 0.2); \n"

    "    // Operations in View Space \n"
    "    vec3 lightDirec = LiProps.xyz - PPos; \n"
    "    float lightDist = length(lightDirec); \n"
    "    // Normalize. Attention: No lightDist > 0 check \n"
    "    lightDirec = lightDirec / lightDist; \n"
    "    // Attenuation. Hardcoded for this sample distances \n"
    "    float attenu = 260.0 / lightDist; \n"
    "    attenu = attenu * attenu; \n"

    "    // Lambertian diffuse illumination \n"
    "    float diffuse = dot(lightDirec, PNormal); \n"
    "    diffuse = max(0.0, diffuse); \n"
    "    vec3 liDiffuse = LiColor * LiProps.w * diffuse * attenu; \n"

    "    // Gaussian specular illumination. Harcoded values again \n"
    "    // We avoid it for interior faces \n"
    "    vec3 viewDir = vec3(0.0, 0.0, 1.0); \n"
    "    vec3 halfDir = normalize(lightDirec + viewDir); \n"
    "    float angleHalf = acos(dot(halfDir, PNormal)); \n"
    "    float exponent = angleHalf / 0.05; \n"
    "    float specular = 0.0; \n"
    "    if (diffuse > 0.0) \n"
    "        specular = exp(-exponent * exponent); \n"

    "    vec3 lightRes = PColor.rgb * ( liAmbient + liDiffuse ); \n"
    "    // Specular color is quite similar as light color \n"
    "    lightRes += (0.2 * PColor.xyz + 0.8 * LiColor) * specular * attenu; \n"
    "    lightRes = clamp(lightRes, 0.0, 1.0); \n"

    "     return lightRes; \n"
    "}\n"
};

// Fragment shader for the triangles
const GLchar* triangFragmentShader =
{
    "#version 150 \n"

    "uniform vec4 lightProps; // Position in View space, and intensity \n"
    "uniform vec3 lightColor; \n"

    "flat in vec4 theColor; \n"
    "flat in vec3 theNormal; \n"
    "in vec3 pointPos; \n"

    "out vec4 fragColor; \n"

    "// Declare this function \n"
    "vec3 Illuminate(in vec4 LiProps, in vec3 LiColor, in vec4 PColor, \n"
    "                in vec3 PNormal, in vec3 PPos); \n"

    "void main(void) \n"
    "{\n"
    "    vec3 lightRes = Illuminate(lightProps, lightColor, theColor, \n"
    "                               theNormal, pointPos); \n "

    "    fragColor = vec4(lightRes, theColor.a); \n"
    "}\n"
};

// // Vertex shader for strings (textures) with illumination
// const GLchar* stringsVertexShader =
// {
//     "#version 150 \n"

//     "in vec3 in_sPosition; \n"
//     "in vec3 in_sNormal; \n"
//     "in vec2 in_TextPos; \n"
//     "uniform mat4 mMVP; \n"
//     "uniform mat4 mToViewSpace; \n"

//     "flat out vec3 theNormal; \n"
//     "out vec3 pointPos; \n"
//     "out vec2 textCoord; \n"

//     "void main(void) \n"
//     "{\n"
//     "    gl_Position = mMVP * vec4(in_sPosition, 1.0); \n"
//     "    textCoord = in_TextPos; \n"

//     "    // Operations in View Space \n"
//     "    vec4 temp4 = mToViewSpace * vec4(in_sPosition, 1.0); \n"
//     "    pointPos = temp4.xyz; \n"
//     "    temp4 = mToViewSpace * vec4(in_sNormal, 0.0); \n"
//     "    theNormal = normalize(temp4.xyz); \n"
//     "}\n"
// };

// // Fragment shader for strings (textures) with illumination
// const GLchar* stringsFragmentShader =
// {
//     "#version 150 \n"

//     "uniform vec4 lightProps; // Position in View space, and intensity \n"
//     "uniform vec3 lightColor; \n"
//     "uniform sampler2D stringTexture; \n"

//     "flat in vec3 theNormal; \n"
//     "in vec3 pointPos; \n"
//     "in vec2 textCoord; \n"

//     "out vec4 fragColor; \n"

//     "// Declare this function \n"
//     "vec3 Illuminate(in vec4 LiProps, in vec3 LiColor, in vec4 PColor, \n"
//     "                in vec3 PNormal, in vec3 PPos); \n"

//     "void main(void) \n"
//     "{\n"
//     "    vec4 colo4 = texture(stringTexture, textCoord); \n"
//     "    vec3 lightRes = Illuminate(lightProps, lightColor, colo4, \n"
//     "                               theNormal, pointPos); \n "

//     "    fragColor = vec4(lightRes, colo4.a); \n"
//     "}\n"
// };

// Vertex shader for immutable strings (textures)
// const GLchar* stringsImmutableVS =
// {
//     "#version 150 \n"

//     "in vec3 in_sPosition; \n"
//     "in vec2 in_TextPos; \n"
//     "uniform mat4 mMVP; \n"
//     "out vec2 textCoord; \n"

//     "void main(void) \n"
//     "{\n"
//     "    gl_Position = mMVP * vec4(in_sPosition, 1.0); \n"
//     "    textCoord = in_TextPos; \n"
//     "}\n"
// };

// // Fragment shader for immutable strings (textures)
// const GLchar* stringsImmutableFS =
// {
//     "#version 150 \n"

//     "uniform sampler2D stringTexture; \n"
//     "in vec2 textCoord; \n"
//     "out vec4 fragColor; \n"

//     "void main(void) \n"
//     "{\n"
//     "    fragColor= texture(stringTexture, textCoord); \n"
//     "}\n"
// };



// ----------------------------------------------------------------------------
// A point light
// ----------------------------------------------------------------------------

void myLight::Set(const glm::vec3& position, GLfloat intensity,
                  GLfloat R, GLfloat G, GLfloat B)
{
    _PosAndIntensisty[0] = (GLfloat) position.x;
    _PosAndIntensisty[1] = (GLfloat) position.y;
    _PosAndIntensisty[2] = (GLfloat) position.z;
    _PosAndIntensisty[3] = (GLfloat) intensity;
    _Color[0] = R;
    _Color[1] = G;
    _Color[2] = B;
}


// ----------------------------------------------------------------------------
// OGLMesh
// ----------------------------------------------------------------------------
OGLMesh::OGLMesh()
{
    _VAO = _VBO = _EBO = 0;
}

OGLMesh::~OGLMesh()
{
    Clear();
}

void OGLMesh::Clear()
{
    // Clear graphics card memory
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    if ( _EBO )
        glDeleteBuffers(1, &_EBO);
    if ( _VBO )
        glDeleteBuffers(1, &_VBO);

    // Unbind from context
    glBindVertexArray(0);
    if ( _VAO )
        glDeleteVertexArrays(1, &_VAO);

    glFlush(); //Tell GL to execute those commands now, but we don't wait for them

    _VAO = _VBO = _EBO = 0;
}

void OGLMesh::SetBuffers(Shader& shader,
                                GLsizei numPoints, GLsizei numFaces,
                                const GLfloat* verts, const GLfloat* colors, 
                                const GLfloat* norms, const GLfloat* uvs, const unsigned int* indices)
{
    OnGLError(OGL_ERR_CLEAR); //clear error stack
    for (int i = 0; i<numPoints; ++i) {
        Vertex vertex;
        vertex.position = vec3(verts[i*3], verts[i*3 + 1], verts[i*3 + 2]);
        if (colors) {
            vertex.color = vec4(colors[i*4], colors[i*4+1], colors[i*4+2], colors[i*4+3]);
        }
        if (norms) {
            vertex.normal = vec3(norms[i*3], norms[i*3+1], norms[i*3+2]);
        }
        // vertex.texCoord = vec2(uvs[i*3], uvs[i*3+1]);
        _vertices.push_back(vertex);
    }

    if (indices) {
        for (int i = 0; i < numFaces * 3; ++i) {
            _indices.push_back(indices[i]);
        }
    }

    SetupMesh(shader);
}

void OGLMesh::SetupMesh(Shader& shader)
{
    OnGLError(OGL_ERR_CLEAR);

    // create buffers/arrays
    glGenVertexArrays(1, &_VAO);
    glGenBuffers(1, &_VBO);
    glGenBuffers(1, &_EBO);

    glBindVertexArray(_VAO);
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    // A great thing about structs is that their memory layout is sequential for all its items.
    // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
    // again translates to 3/2 floats which translates to a byte array.
    glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(Vertex), &_vertices[0], GL_STATIC_DRAW);  

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(unsigned int), &_indices[0], GL_STATIC_DRAW);

    // set the vertex attribute pointers
    // vertex positions
    GLuint loc = shader.GetAttribLoc("aPos");
    if (loc != -1) {
        glEnableVertexAttribArray(loc);	
        glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    }
    // vertex colors
    loc = shader.GetAttribLoc("aColor");
    if (loc != -1) {
        glEnableVertexAttribArray(loc);	
        glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    }
    // vertex normals
    loc = shader.GetAttribLoc("aNormal");
    if (loc != -1) {
        glEnableVertexAttribArray(loc);	
        glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    }
    // // vertex texture coords
    // loc = shader.GetAttribLoc("aUV");
    // if (loc != -1) {
    //     glEnableVertexAttribArray(loc);	
    //     glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    // }
}
// void OGLMesh::SetBuffers(Shader* theShader,
//                                 GLsizei numPoints, GLsizei numFaces,
//                                 const GLfloat* vert, const GLfloat* colo, 
//                                 const GLfloat* norm, const GLfloat* uv, const GLushort* indices)
// {
//     OnGLError(OGL_ERR_CLEAR); //clear error stack

//     // NOTE: have you realized that I fully trust on parameters being != 0 and != NULL?

//     // Part 1: Buffers - - - - - - - - - - - - - - - - - - -

//     // Graphics card buffer for vertices.
//     // Not shared buffer with colors and normals, why not? Just for fun.
//     glGenBuffers(1, &_bufVertId);
//     glBindBuffer(GL_ARRAY_BUFFER, _bufVertId);
//     // Populate the buffer with the array "vert"
//     GLsizeiptr nBytes = numPoints * 3 * sizeof(GLfloat); //3 components {x,y,z}
//     glBufferData(GL_ARRAY_BUFFER, nBytes, vert, GL_STATIC_DRAW);

//     if ( ! OnGLError(OGL_ERR_BUFFER) )
//     {
//         // Likely the GPU got out of memory
//         Clear();
//         return;
//     }

//     // Graphics card buffer for colors and normals.
//     glGenBuffers(1, &_bufColNorUVId);
//     glBindBuffer(GL_ARRAY_BUFFER, _bufColNorUVId);
//     // Allocate space for both arrays
//     nBytes = (numPoints * 4 + numFaces * 3 + numPoints * 2) * sizeof(GLfloat);
//     glBufferData(GL_ARRAY_BUFFER, nBytes, NULL, GL_STATIC_DRAW);
//     if ( ! OnGLError(OGL_ERR_BUFFER) )
//     {
//         // Likely the GPU got out of memory
//         Clear();
//         return;
//     }
//     // Populate part of the buffer with the array "colo"
//     nBytes = numPoints * 4 * sizeof(GLfloat); // rgba components
//     glBufferSubData(GL_ARRAY_BUFFER, 0, nBytes, colo);
//     // Add the array "norm" to the buffer
//     GLsizeiptr normBufOffset = nBytes;
//     nBytes = numFaces * 3 * sizeof(GLfloat);
//     glBufferSubData(GL_ARRAY_BUFFER, normBufOffset, nBytes, norm);
//     // Add the array "uv" to the buffer
//     GLsizeiptr uvBufOffset = normBufOffset + nBytes;
//     nBytes = numPoints * 2 * sizeof(GLfloat);
//     glBufferSubData(GL_ARRAY_BUFFER, uvBufOffset, nBytes, uv);

//     // Buffer for texture
//     glGenTextures(1, &_textureId);
//     glBindTexture(GL_TEXTURE_2D, _textureId);
//     // set the texture wrapping/filtering options (on the currently bound
//     // texture object)
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//     // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
//     //                 GL_LINEAR_MIPMAP_LINEAR);
//     // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//     // load and generate the texture
//     int width, height, nrChannels;
//     unsigned char *img_data =
//         stbi_load("/Users/chrisschilling/wxwidget/graphics/assets/container.jpg", &width, &height, &nrChannels, 0);
//     if (img_data) {
//       glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
//                    GL_UNSIGNED_BYTE, img_data);
//       glGenerateMipmap(GL_TEXTURE_2D);
//     } else {
//       std::cout << "Failed to load texture" << std::endl;
//     }
//     stbi_image_free(img_data);

//     // Graphics card buffer for indices.
//     glGenBuffers(1, &_bufIndexId);
//     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _bufIndexId);
//     // Populate the buffer with the array "indices"
//     // We use "triangle strip". An index for each additional vertex.
//     nBytes = (3 + numFaces - 1) * sizeof(GLushort); //Each triangle needs 3 indices
//     glBufferData(GL_ELEMENT_ARRAY_BUFFER, nBytes, indices, GL_STATIC_DRAW);

//     if ( ! OnGLError(OGL_ERR_BUFFER) )
//     {
//         // Likely the GPU got out of memory
//         Clear();
//         return;
//     }

//     // Unbind buffers. We will bind them one by one just now, at VAO creation
//     glBindBuffer(GL_ARRAY_BUFFER, 0);
//     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
//     glBindTexture(GL_TEXTURE_2D, 0);

//     _numFaces = numFaces;

//     OnGLError(OGL_ERR_CLEAR); //clear error stack

//     // Part 2: VAO - - - - - - - - - - - - - - - - - - -

//     // Vertex Array Object (VAO) that stores the relationship between the
//     // buffers and the shader input attributes
//     glGenVertexArrays(1, &_VAO);
//     glBindVertexArray(_VAO);

//     glBindTexture(GL_TEXTURE_2D, _textureId);
//     // Set the way of reading (blocks of n floats each) from the current bound
//     // buffer and passing data to the shader (through the index of an attribute).
//     // Vertices positions
//     glBindBuffer(GL_ARRAY_BUFFER, _bufVertId);
//     GLuint loc = theShader->GetAttribLoc("aPos");
//     glEnableVertexAttribArray(loc);
//     glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)0);
//     // Colors
//     glBindBuffer(GL_ARRAY_BUFFER, _bufColNorUVId);
//     loc = theShader->GetAttribLoc("aColor");
//     glEnableVertexAttribArray(loc);
//     glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *)0);
//     // Normals. Their position in buffer starts at bufoffset
//     loc = theShader->GetAttribLoc("aNormal");
//     glEnableVertexAttribArray(loc);
//     glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)normBufOffset);
//     // Textures. 
//     loc = theShader->GetAttribLoc("aUV");
//     glEnableVertexAttribArray(loc);
//     glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)uvBufOffset);
//     // Indices
//     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _bufIndexId);

//     // Unbind
//     glBindVertexArray(0);
//     glBindBuffer(GL_ARRAY_BUFFER, 0);
//     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
//     glBindTexture(GL_TEXTURE_2D, 0);

//     // Some log
//     OnGLError(OGL_ERR_JUSTLOG, "Triangles data loaded into GPU.");
// }

void OGLMesh::Draw(bool useIndices)
{
    if ( !_VAO )
        return;

    // Bind the source data for the shader
    glBindVertexArray(_VAO);
    //glDrawArrays(GL_TRIANGLES, 0, 36);
    //glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, (GLvoid *)0);
    if (useIndices) {
        glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, (GLvoid *)0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, _vertices.size());
    }    
    OnGLError(OGL_ERR_DRAWING_TRI);
    glBindVertexArray(0);
}


// // ----------------------------------------------------------------------------
// // myOGLString
// // ----------------------------------------------------------------------------
// myOGLString::myOGLString()
// {
//     _bufPosId = _textureId = _stringVAO = _textureUnit = 0;
//     _stringShaders = NULL;
// }

// myOGLString::~myOGLString()
// {
//     Clear();
// }

// void myOGLString::Clear()
// {
//     if ( _stringShaders )
//         _stringShaders->DisableGenericVAA();

//     // Clear graphics card memory
//     glBindBuffer(GL_ARRAY_BUFFER, 0);
//     if ( _bufPosId )
//         glDeleteBuffers(1, &_bufPosId);

//     // Unbind from context
//     glBindVertexArray(0);
//     glDeleteVertexArrays(1, &_stringVAO);

//     if ( _textureUnit && _textureId )
//     {
//         glActiveTexture(GL_TEXTURE0 + _textureUnit);
//         glBindTexture(GL_TEXTURE_2D, 0);
//         glDeleteTextures(1, &_textureId);
//     }
//     glActiveTexture(GL_TEXTURE0);

//     glFlush(); //Tell GL to execute those commands now, but we don't wait for them

//     _bufPosId = _textureId = _stringVAO = _textureUnit = 0;
//     _stringShaders = NULL;
// }

// void myOGLString::SetStringWithVerts(Shader* theShader,
//                              const unsigned char* tImage, int tWidth, int tHeigh,
//                              const GLfloat* vert, const GLfloat* norm)
// {
//     OnGLError(OGL_ERR_CLEAR); //clear error stack

//     if ( !tImage )
//         return;

//     // Part 1: Buffers - - - - - - - - - - - - - - - - - - -

//     // Graphics card buffer for vertices, normals, and texture coords
//     glGenBuffers(1, &_bufPosId);
//     glBindBuffer(GL_ARRAY_BUFFER, _bufPosId);
//     // (4+4) (vertices + normals) x 3 components + 4 text-vertices x 2 components
//     GLsizeiptr nBytes = (8 * 3 + 4 * 2) * sizeof(GLfloat);
//     glBufferData(GL_ARRAY_BUFFER, nBytes, NULL, GL_STATIC_DRAW);

//     if ( ! OnGLError(OGL_ERR_BUFFER) )
//     {
//         // Likely the GPU got out of memory
//         Clear();
//         return;
//     }

//     // Populate part of the buffer with the array "vert"
//     nBytes = 12 * sizeof(GLfloat);
//     glBufferSubData(GL_ARRAY_BUFFER, 0, nBytes, vert);
//     // Add the array "norm" to the buffer
//     GLsizeiptr bufoffset = nBytes;
//     if ( norm )
//     {
//         // Just for string on face, not immutable string
//         glBufferSubData(GL_ARRAY_BUFFER, bufoffset, nBytes, norm);
//     }

//     // Add the array of texture coordinates to the buffer.
//     // Order is set accordingly with the vertices
//     // See myOGLManager::SetStringOnPyr()
//     GLfloat texcoords[8] = { 0.0, 1.0,  0.0, 0.0,  1.0, 1.0,  1.0, 0.0 };
//     bufoffset += nBytes;
//     nBytes = 8 * sizeof(GLfloat);
//     glBufferSubData(GL_ARRAY_BUFFER, bufoffset, nBytes, texcoords);

//     _stringShaders = theShader;

//     OnGLError(OGL_ERR_CLEAR); //clear error stack

//     // Part 2: VAO - - - - - - - - - - - - - - - - - - -

//     // Vertex Array Object (VAO) that stores the relationship between the
//     // buffers and the shader input attributes
//     glGenVertexArrays(1, &_stringVAO);
//     glBindVertexArray(_stringVAO);

//     // Set the way of reading (blocks of n floats each) from the current bound
//     // buffer and passing data to the shader (through the index of an attribute).
//     // Vertices positions
//     GLuint loc = _stringShaders->GetAttribLoc("in_sPosition");
//     glEnableVertexAttribArray(loc);
//     glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)0);
//     // Normals. Their position in buffer starts at bufoffset
//     bufoffset = 12 * sizeof(GLfloat);
//     if ( norm )
//     {
//         // Just for string on face, not immutable string
//         loc = _stringShaders->GetAttribLoc("in_sNormal");
//         glEnableVertexAttribArray(loc);
//         glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)bufoffset);
//     }
//     // Texture coordinates
//     bufoffset *= 2; //Normals take same amount of space as vertices
//     loc = _stringShaders->GetAttribLoc("in_TextPos");
//     glEnableVertexAttribArray(loc);
//     glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)bufoffset);

//     // Part 3: The texture with the string as an image - - - - - - - -

//     // Create the bind for the texture
//     // Same unit for both textures (strings) since their characteristics are the same.
//     _textureUnit = 1;
//     glActiveTexture(GL_TEXTURE0 + _textureUnit);
//     glGenTextures(1, &_textureId); //"Name" of the texture object
//     glBindTexture(GL_TEXTURE_2D, _textureId);
//     // Avoid some artifacts
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//     // Do this before glTexImage2D because we only have 1 level, no mipmap
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
//     // For RGBA default alignment (4) is good. In other circumstances, we may
//     // need glPixelStorei(GL_UNPACK_ALIGNMENT, 1)
//     // Load texture into card. No mipmap, so 0-level
//     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
//                  (GLsizei)tWidth, (GLsizei)tHeigh, 0,
//                   GL_RGBA, GL_UNSIGNED_BYTE, tImage);
//     if ( ! OnGLError(OGL_ERR_TEXTIMAGE) )
//     {
//         // Likely the GPU got out of memory
//         Clear();
//         return;
//     }

//     // Unbind
//     glBindVertexArray(0);
//     glBindBuffer(GL_ARRAY_BUFFER, 0);
//     glBindTexture(GL_TEXTURE_2D, 0);
//     glActiveTexture(GL_TEXTURE0);

//     // Some log
//     OnGLError(OGL_ERR_JUSTLOG, "Texture for string loaded into GPU.");
// }

// void myOGLString::Draw(const glm::mat4& unifMvp, const glm::mat4& unifToVw,
//                        const myLight* theLight)
// {
//     if ( !_stringVAO )
//         return;

//     OnGLError(OGL_ERR_CLEAR); //clear error stack

//     if ( ! _stringShaders->Use() )
//         return;

//     // Bind the source data for the shader
//     glBindVertexArray(_stringVAO);

//     // Pass matrices to the shader in column-major order
//     glUniformMatrix4fv(_stringShaders->GetUnifLoc("mMVP"), 1, GL_FALSE, &unifMvp[0][0]);
//     if ( theLight )
//     {
//         // Just for string on face, not immutable string
//         glUniformMatrix4fv(_stringShaders->GetUnifLoc("mToViewSpace"), 1, GL_FALSE, &unifToVw[0][0]);
//         // Pass the light, in View coordinates in this sample
//         glUniform4fv(_stringShaders->GetUnifLoc("lightProps"), 1, theLight->GetFLightPos());
//         glUniform3fv(_stringShaders->GetUnifLoc("lightColor"), 1, theLight->GetFLightColor());

//         // We have a flat shading, and we want the first vertex normal as the flat value
//         glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
//     }

//     // Use our texture unit
//     glActiveTexture(GL_TEXTURE0 + _textureUnit);
//     glBindTexture(GL_TEXTURE_2D, _textureId);
//     // The fragment shader will read texture values (pixels) from the texture
//     // currently active
//     glUniform1i(_stringShaders->GetUnifLoc("stringTexture"), _textureUnit);

//     // Draw the rectangle made up of two triangles
//     glDrawArrays(GL_TRIANGLE_STRIP, 0,  4);

//     OnGLError(OGL_ERR_DRAWING_STR);

//     // Unbind
//     glBindVertexArray(0);
//     glBindTexture(GL_TEXTURE_2D, 0);
//     glActiveTexture(GL_TEXTURE0);

//     _stringShaders->StopUse();
// }

// // ----------------------------------------------------------------------------
// // myOGLImmutString
// // ----------------------------------------------------------------------------
// void myOGLImmutString::SetImmutString(Shader* theShader,
//                         const unsigned char* tImage, int tWidth, int tHeigh)
// {
//     // Make a rectangle of the same size as the image. Order of vertices matters.
//     // Set a 2 pixels margin
//     double imaVerts[12];
//     imaVerts[0] = 2.0         ;   imaVerts[1] = 2.0         ; imaVerts[2]  = -1.0;
//     imaVerts[3] = 2.0         ;   imaVerts[4] = 2.0 + tHeigh; imaVerts[5]  = -1.0;
//     imaVerts[6] = 2.0 + tWidth;   imaVerts[7] = 2.0         ; imaVerts[8]  = -1.0;
//     imaVerts[9] = 2.0 + tWidth;  imaVerts[10] = 2.0 + tHeigh; imaVerts[11] = -1.0;

//     // GLFloat version
//     GLfloat fimaVerts[12];
//     SetAsGLFloat4x4(imaVerts, fimaVerts, 12);

//     // Call the base class without normals, it will handle this case
//     SetStringWithVerts(theShader, tImage, tWidth, tHeigh, fimaVerts, NULL);
// }

// void myOGLImmutString::SetOrtho(float winWidth, float winHeight)
// {
//     // We want an image always of the same size, regardless of window size.
//     // The orthogonal projection with the whole window achieves it.
//     _Ortho = glm::ortho(0.0f, winWidth, 0.0f, winHeight, -1.0f, 1.0f);
// }


// ----------------------------------------------------------------------------
// myOGLCamera
// ----------------------------------------------------------------------------
myOGLCamera::myOGLCamera()
{
    InitCamera();
}


void myOGLCamera::InitCamera()
{
    // The radius of the bounding sphere
    _worldRadius = 20.0f;
    _worldUp = vec3(0.0f, 1.0f, 0.0f);
    _worldCenter = vec3(0.0f, 0.0f, 0.0f);

    _camPosition = vec3(0.0f, 0.0f, _worldRadius);
    _fov = 45.0;
    _camYaw = -90.0f;
    _camPitch = 0.0f;

    _camMovementSpeed = 2.5f;  // pixel per sec?
    _mouseSensitivity = 0.1f;

    // Init motion
    _motion.forward = false;
    _motion.backward = false;
    _motion.strafe_left = false;
    _motion.strafe_right = false;

    UpdateCameraOrientation();
}

void myOGLCamera::UpdateCameraOrientation() {

    glm::vec3 front;
    front.x = cos(glm::radians(_camYaw)) * cos(glm::radians(_camPitch));
    front.y = sin(glm::radians(_camPitch));
    front.z = sin(glm::radians(_camYaw)) * cos(glm::radians(_camPitch));
    _camFront = glm::normalize(front);

    _camRight = glm::normalize(glm::cross(_camFront, _worldUp));  
    _camUp    = glm::normalize(glm::cross(_camRight, _camFront));

    // float tmpv = glm::distance(_camPosition, _worldCenter);
    // // Calculate distances, not coordinates, with some margins
    // // Near clip-plane distance to the camera
    // _nearD = tmpv - 1.10 * _worldRadius - 5.0;
    // // Far clip-plane distance to the camera
    // _farD = tmpv + 1.10 * _worldRadius + 5.0;
}

void myOGLCamera::ViewSizeChanged(int newWidth, int newHeight)
{
    // These values are also used for MouseRotation()
    _winWidth = newWidth;
    _winHeight = newHeight;

    // Calculate the projection matrix
    _aspect = (double) newWidth / newHeight;
}

void myOGLCamera::GetViewAndProjection(mat4* view, mat4* proj) {
    UpdateCameraPosition();
    *view = glm::lookAt(_camPosition, _camPosition + _camFront, _camUp);
    *proj = glm::perspective(glm::radians(_fov), _aspect, 0.1, 100.0);
}

void myOGLCamera::UpdateCameraPosition() {
    float deltaPos = _deltaT/1000.0f * _camMovementSpeed;
    if (_motion.forward)
        _camPosition += deltaPos * _camFront;
    if (_motion.backward)
        _camPosition -= deltaPos * _camFront;
    if (_motion.strafe_right)
        _camPosition += deltaPos * _camRight;
    if (_motion.strafe_left)
        _camPosition -= deltaPos * _camRight;
}

void myOGLCamera::KeyboardPan(CAMERA_MOVEMENT direction, bool start)
{
    if (direction == FORWARD)
        _motion.forward = start;
    if (direction == BACKWARD)
        _motion.backward = start;
    if (direction == LEFT)
         _motion.strafe_left = start;
    if (direction == RIGHT)
        _motion.strafe_right = start;
}

void myOGLCamera::MouseRotation(int fromX, int fromY, int toX, int toY, GLboolean constrainPitch)
{
    if ( fromX == toX && fromY == toY )
        return; //no rotation
        float xoffset = static_cast<float>(toX - fromX);
        float yoffset = static_cast<float>(toY - fromY);
        xoffset *= _mouseSensitivity;
        yoffset *= _mouseSensitivity;

        _camYaw   += xoffset;
        _camPitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (_camPitch > 89.0f)
                _camPitch = 89.0f;
            if (_camPitch < -89.0f)
                _camPitch = -89.0f;
        }

        // update Front, Right and Up Vectors using the updated Euler angles
        UpdateCameraOrientation();
}

void myOGLCamera::TimerRotation(float angle) {
    //_camPitch = 0.0f;
    //_camPosition = vec3(0.0f, 0.0f, _worldRadius);
    _camYaw = angle;
}

void myOGLCamera::MouseScroll(int delta) {
    float deltaf = _mouseSensitivity * delta;
    _fov -= deltaf;
    if (_fov < 1.0f)
        _fov = 1.0f;
    if (_fov > 45.0f)
        _fov = 45.0f;
}

// ----------------------------------------------------------------------------
// myOGLManager
// ----------------------------------------------------------------------------

myOGLManager::myOGLManager(OGLErrHandler* extErrHnd)
{
    externalOGL_ErrHandler = extErrHnd;
    OnGLError(OGL_ERR_CLEAR); //clear error stack
}

myOGLManager::~myOGLManager()
{
    OnGLError(OGL_ERR_CLEAR); //clear error stack

    // Force GPU finishing before the context is deleted
    glFinish();
}

/* Static */
bool myOGLManager::Init()
{
    // Retrieve the pointers to OGL functions we use in this sample
    if (glewInit() != GLEW_OK) {
        return false;
    }
    return true;
}

const GLubyte* myOGLManager::GetGLVersion()
{
    return glGetString(GL_VERSION);
}

const GLubyte* myOGLManager::GetGLVendor()
{
    return glGetString(GL_VENDOR);
}

const GLubyte* myOGLManager::GetGLRenderer()
{
    return glGetString(GL_RENDERER);
}

void myOGLManager::SetShadersAndTriangles()
{
    // The shaders attributes and uniforms
    _pyramidShaders.AddAttrib("aPos");
    _pyramidShaders.AddAttrib("aColor");
    _pyramidShaders.AddAttrib("aNormal");
    // _pyramidShaders.AddAttrib("aUV");
    _pyramidShaders.AddUnif("projection");
    _pyramidShaders.AddUnif("view");
    _pyramidShaders.AddUnif("model");
    _pyramidShaders.AddUnif("lightColor");
    _pyramidShaders.AddUnif("lightPos");
    _pyramidShaders.AddUnif("viewPos");
    _pyramidShaders.LoadCode("/Users/chrisschilling/wxwidget/graphics/SimpleVertexShader.vs", GL_VERTEX_SHADER);
    _pyramidShaders.LoadCode("/Users/chrisschilling/wxwidget/graphics/SimpleFragmentShader.fs", GL_FRAGMENT_SHADER);
    _pyramidShaders.Init();
    _lightCubeShaders.AddAttrib("aPos");
    _lightCubeShaders.AddUnif("aColor");
    _lightCubeShaders.AddUnif("MVP");
    _lightCubeShaders.LoadCode("/Users/chrisschilling/wxwidget/graphics/LightCube.vs", GL_VERTEX_SHADER);
    _lightCubeShaders.LoadCode("/Users/chrisschilling/wxwidget/graphics/LightCube.fs", GL_FRAGMENT_SHADER);
    _lightCubeShaders.Init();
    // _StringShaders.AddAttrib("in_sPosition");
    // _StringShaders.AddAttrib("in_sNormal");
    // _StringShaders.AddAttrib("in_TextPos");
    // _StringShaders.AddUnif("mMVP");
    // _StringShaders.AddUnif("mToViewSpace");
    // _StringShaders.AddUnif("lightProps");
    // _StringShaders.AddUnif("lightColor");
    // _StringShaders.AddUnif("stringTexture");
    // _StringShaders.AddCode(stringsVertexShader, GL_VERTEX_SHADER);
    // _StringShaders.AddCode(illuminationShader, GL_FRAGMENT_SHADER);
    // _StringShaders.AddCode(stringsFragmentShader, GL_FRAGMENT_SHADER);
    // _StringShaders.Init();
    // _ImmutStringSha.AddAttrib("in_sPosition");
    // _ImmutStringSha.AddAttrib("in_TextPos");
    // _ImmutStringSha.AddUnif("mMVP");
    // _ImmutStringSha.AddUnif("stringTexture");
    // _ImmutStringSha.AddCode(stringsImmutableVS, GL_VERTEX_SHADER);
    // _ImmutStringSha.AddCode(stringsImmutableFS, GL_FRAGMENT_SHADER);
    // _ImmutStringSha.Init();
    // The point light. Set its color as full white.
    // In this sample we set the light position same as the camera position
    // In View space, camera position is {0, 0, 0}
    //_Light.Set(glm::vec3(0.0, 0.0, 0.0), 1.0, 1.0, 1.0, 1.0);
    // The triangles data
    // _LightCubeMesh.SetBuffers(&_LightCubeShaders, 36, 12, lightCubevVertices, nullptr, nullptr, nullptr, nullptr);
    // _LightCubeMesh.AddPosition(vec3(1.2f, 1.0f, 2.0f));
    _pyramidMesh.SetBuffers(_pyramidShaders, 4, 4, gVerts, gColors, gNormals, gUV, gIndices);
    _cubeMesh.SetBuffers(_pyramidShaders, 36, 6, cubeVerts, cubeColors, cubeNorms, nullptr, nullptr);
    _lightCubeMesh.SetBuffers(_lightCubeShaders, 36, 6, cubeVerts, nullptr, nullptr, nullptr, nullptr);
}

// void myOGLManager::SetStringOnPyr(const unsigned char* strImage, int iWidth, int iHeigh)
// {
//     // Some geometry. We want a rectangle close to face 0-1-2 (X-Z plane).
//     // The rectangle must preserve strImage proportions. If the height of the
//     // rectangle is "h" and we want to locate it with its largest side parallel
//     // to the edge of the face and at distance= h/2, then the rectangle width is
//     // rw = edgeLength - 2 * ((h/2 + h + h/2)/tan60) = edgeLength - 4*h/sqrt(3)
//     // If h/rw = Prop then
//     //    rw = edgeLength / (1+4/sqrt(3)*Prop) and h = Prop * rw

//     double edgeLen = MyDistance(glm::vec3(gVerts[0], gVerts[1], gVerts[2]),
//                                  glm::vec3(gVerts[6], gVerts[7], gVerts[8]));
//     GLfloat prop = ((GLfloat) iHeigh) / ((GLfloat) iWidth);
//     GLfloat rw = float(edgeLen) / (1 + 4 * prop / std::sqrt(3.0f));
//     GLfloat h = prop * rw;
//     GLfloat de = 2 * h / std::sqrt(3.0f);
//     // A bit of separation of the face so as to avoid z-fighting
//     GLfloat rY = gVerts[1] - 0.01f; // Towards outside
//     GLfloat sVerts[12];
//     // The image was created top to bottom, but OpenGL axis are bottom to top.
//     // The image would display upside down. We avoid it choosing the right
//     // order of vertices and texture coords. See myOGLString::SetStringWithVerts()
//     sVerts[0] = gVerts[6] + de;  sVerts[1] = rY;   sVerts[2] = gVerts[8] + h / 2;
//     sVerts[3] = sVerts[0]     ;  sVerts[4] = rY;   sVerts[5] = sVerts[2] + h;
//     sVerts[6] = sVerts[0] + rw;  sVerts[7] = rY;   sVerts[8] = sVerts[2];
//     sVerts[9] = sVerts[6]     ; sVerts[10] = rY;  sVerts[11] = sVerts[5];

//     // Normals for the rectangle illumination, same for the four vertices
//     const GLfloat strNorms[] = { gNormals[0], gNormals[1], gNormals[2],
//                                  gNormals[0], gNormals[1], gNormals[2],
//                                  gNormals[0], gNormals[1], gNormals[2],
//                                  gNormals[0], gNormals[1], gNormals[2]};

//     // The texture data for the string on the face of the pyramid
//     _StringOnPyr.SetStringWithVerts(&_StringShaders, strImage, iWidth, iHeigh,
//                                      sVerts, strNorms);
// }

// void myOGLManager::SetImmutableString(const unsigned char* strImage,
//                                       int iWidth, int iHeigh)
// {
//     _ImmString.SetImmutString(&_ImmutStringSha, strImage, iWidth, iHeigh);
// }

void myOGLManager::SetViewport(int x, int y, int width, int height)
{
    if (width < 1) width = 1;
    if (height < 1) height = 1;

    glViewport(x, y, (GLsizei)width, (GLsizei)height);

    // The camera handles perspective projection
    _Camera.ViewSizeChanged(width, height);
    // // And this object handles its own orthogonal projection
    // _ImmString.SetOrtho(width, height);
}

void myOGLManager::Render()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor((GLfloat)0.15, (GLfloat)0.15, 0.0, (GLfloat)1.0); // Dark, but not black.
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    OnGLError(OGL_ERR_CLEAR);

    mat4 view, projection;
    _Camera.GetViewAndProjection(&view, &projection);
    vec3 lightPos(1.2f, 1.0f, 2.0f);
    // period = 5s

    lightPos.x = 1.0f + sin(_frameCnt*.021) * 2.0f;
    lightPos.y = sin(_frameCnt*.021 / 2.0f) * 1.0f;
    vec3 lightColor(1.0f, 1.0f, 1.0f);

    if ( ! _pyramidShaders.Use() )
        OnGLError(OGL_ERR_DRAWING_TRI);
    float angle = 0.0;
    for (int i = 0; i<10; ++i) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, pyramidPositions[i]);
        angle += 0.6*_frameCnt;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
        angle += 20.0f;
        glUniformMatrix4fv(_pyramidShaders.GetUnifLoc("projection"), 1, GL_FALSE, &projection[0][0]);
        glUniformMatrix4fv(_pyramidShaders.GetUnifLoc("view"), 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(_pyramidShaders.GetUnifLoc("model"), 1, GL_FALSE, &model[0][0]);
        glUniform3fv(_pyramidShaders.GetUnifLoc("lightColor"), 1, &lightColor[0]);
        glUniform3fv(_pyramidShaders.GetUnifLoc("lightPos"), 1, &lightPos[0]);
        glUniform3fv(_pyramidShaders.GetUnifLoc("viewPos"), 1, &(_Camera.GetCameraPosition())[0]);

        // TODO(experiment with flat color interpolation (uses provoking))
        // We have a flat shading, and we want the first vertex data as the flat value
        glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
        _pyramidMesh.Draw();
    }


    if ( ! _pyramidShaders.Use() )
        OnGLError(OGL_ERR_DRAWING_TRI);
    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(_pyramidShaders.GetUnifLoc("projection"), 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(_pyramidShaders.GetUnifLoc("view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(_pyramidShaders.GetUnifLoc("model"), 1, GL_FALSE, &model[0][0]);
    glUniform3fv(_pyramidShaders.GetUnifLoc("lightColor"), 1, &lightColor[0]);
    glUniform3fv(_pyramidShaders.GetUnifLoc("lightPos"), 1, &lightPos[0]);
    glUniform3fv(_pyramidShaders.GetUnifLoc("viewPos"), 1, &(_Camera.GetCameraPosition())[0]);
    _cubeMesh.Draw(false);

    if ( ! _lightCubeShaders.Use() )
        OnGLError(OGL_ERR_DRAWING_TRI);
    model = glm::mat4(1.0f);
    model = glm::translate(model, lightPos);
    model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
    mat4 mvp = projection * view * model;
    glUniformMatrix4fv(_lightCubeShaders.GetUnifLoc("MVP"), 1, GL_FALSE, &mvp[0][0]);
    glUniform3fv(_lightCubeShaders.GetUnifLoc("aColor"), 1, &lightColor[0]);
    _lightCubeMesh.Draw(false);
    //_Triangles.Draw(_Camera.GetFloatMVP(), _Camera.GetFloatToVw(), &_Light);
    //_StringOnPyr.Draw(_Camera.GetFloatMVP(), _Camera.GetFloatToVw(), &_Light);
    // This string is at the very front, whatever z-coords are given
    //glDisable(GL_DEPTH_TEST);
    //_ImmString.Draw(_ImmString.GetFloatMVP(), NULL, NULL);
}
