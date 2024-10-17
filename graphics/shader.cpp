#include <iostream>
#include <fstream>
#include <sstream>

#include "ogl_error.h"
#include "shader.h"

Shader::Shader()
{
    _proId = 0;
    _SHAinitializated = false;
}

Shader::~Shader()
{
    if ( _proId )
        CleanUp();
}

void Shader::CleanUp()
{
    StopUse();

    glDeleteProgram(_proId);

    glFlush();
}

void Shader::AddCode(std::string& shaString, GLenum shaType)
{
    // The code is a null-terminated string
    shaShas sv = {0, shaType, shaString};
    _shaCode.push_back(sv);
}

bool Shader::LoadCode(const char* path, GLenum shaType) {

	// Read the Vertex Shader code from the file
	std::string code;
	std::ifstream stream(path, std::ios::in);
	if(stream.is_open()){
		std::stringstream sstr;
		sstr << stream.rdbuf();
		code = sstr.str();
		stream.close();
	} else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", path);
		getchar();
		return false;
	}

    shaShas sv = {0, shaType, code};
    _shaCode.push_back(sv);
    return true;
}

void Shader::AddAttrib(const std::string& name)
{
    shaVars sv = {0, name}; //We will set the location later
    _shaAttrib.push_back(sv);
    // We don't check the max number of attribute locations (usually 16)
}

void Shader::AddUnif(const std::string& name)
{
    shaVars sv = {0, name};
    _shaUnif.push_back(sv);
}

// Inform GL of the locations in program for the vars for buffers used to feed
// the shader. We use glBindAttribLocation (before linking the gl program) with
// the location we want.
// Since GL 3.3 we could avoid this using in the shader "layout(location=x)...".
// The same names as in the shader must be previously set with AddAttrib()
void Shader::SetAttribLocations()
{
    GLuint loc = 0;
    for(shaVars_v::iterator it = _shaAttrib.begin(); it != _shaAttrib.end(); ++it)
    {
        it->loc = loc++;
        glBindAttribLocation(_proId, it->loc, it->name.c_str());
    }
}

GLuint Shader::GetAttribLoc(const std::string& name)
{
    for (shaVars_v::iterator it = _shaAttrib.begin(); it != _shaAttrib.end(); ++it)
    {
        if ( it->name == name && it->loc != (GLuint)-1 )
            return it->loc;
    }

    return (GLuint) -1;
}

// Store the locations in program for uniforms vars
bool Shader::AskUnifLocations()
{
    for (shaVars_v::iterator it = _shaUnif.begin(); it != _shaUnif.end(); ++it)
    {
        GLint glret = glGetUniformLocation(_proId, it->name.c_str());
        if ( glret == -1 )
        {
			std::cout<<"Unif error: "<<it->name.c_str()<<std::endl;
            // Return now, this GPU program cannot be used because we will
            // pass data to unknown/unused uniform locations
            return false;
        }
        it->loc = glret;
    }

    return true;
}

GLuint Shader::GetUnifLoc(const std::string& name)
{
    for (shaVars_v::iterator it = _shaUnif.begin(); it != _shaUnif.end(); ++it)
    {
        if ( it->name == name && it->loc != (GLuint)-1 )
            return it->loc;
    }

    return (GLuint) -1;
}

// Create a GPU program from the given shaders
void Shader::Init()
{
    OnGLError(OGL_ERR_CLEAR); //clear error stack

    bool resC = false;
    bool resL = false;

    // GLSL code load and compilation
    for (shaShas_v::iterator it = _shaCode.begin(); it != _shaCode.end(); ++it)
    {
        it->shaId = glCreateShader(it->typeSha);
        const GLchar* charPointer = it->scode.c_str();
        glShaderSource(it->shaId, 1, &charPointer, nullptr);
        OnGLError(OGL_ERR_SHADERCREATE);

        resC = Compile(it->shaId);
        if ( !resC )
            break;
    }

    if ( resC )
    {
        // The program in the GPU
        _proId = glCreateProgram();
        for (shaShas_v::iterator it = _shaCode.begin(); it != _shaCode.end(); ++it)
        {
            glAttachShader(_proId, it->shaId);
        }

        SetAttribLocations(); //Before linking

        resL = LinkProg(_proId);
    }

    // We don't need them any more
    for (shaShas_v::iterator it = _shaCode.begin(); it != _shaCode.end(); ++it)
    {
        if ( resC && it->shaId )
        {
            glDetachShader(_proId, it->shaId);
        }
        glDeleteShader(it->shaId);
    }

    if ( !resC || !resL )
        return;

    // Log that shaders are OK
    OnGLError(OGL_ERR_JUSTLOG, "Shaders successfully compiled and linked.");

    // After linking, we can get locations for uniforms
    _SHAinitializated = AskUnifLocations();
    if ( !_SHAinitializated )
        OnGLError(OGL_ERR_SHADERLOCATION, " Unused or unrecognized uniform.");
}

// Useful while developing: show shader compilation errors
bool Shader::Compile(GLuint shaId)
{
    glCompileShader(shaId);

    GLint Param = 0;
    glGetShaderiv(shaId, GL_COMPILE_STATUS, &Param);

    if ( Param == GL_FALSE )
    {
        glGetShaderiv(shaId, GL_INFO_LOG_LENGTH, &Param);

        if ( Param > 0 )
        {
            GLchar* InfoLog = new GLchar[Param];
            int nChars = 0;
            glGetShaderInfoLog(shaId, Param, &nChars, InfoLog);
            OnGLError(OGL_ERR_SHADERCOMPILE, InfoLog);
            delete [] InfoLog;
        }
        return false;
    }
    return true;
}

// Useful while developing: show shader program linkage errors
bool Shader::LinkProg(GLuint proId)
{
    glLinkProgram(proId);

    GLint Param = 0;
    glGetProgramiv(proId, GL_LINK_STATUS, &Param);

    if ( Param == GL_FALSE )
    {
        glGetProgramiv(proId, GL_INFO_LOG_LENGTH, &Param);

        if ( Param > 0 )
        {
            GLchar* InfoLog = new GLchar[Param];
            int nChars = 0;
            glGetProgramInfoLog(proId, Param, &nChars, InfoLog);
            OnGLError(OGL_ERR_SHADERLINK, InfoLog);
            delete [] InfoLog;
        }
        return false;
    }
    return true;
}

bool Shader::Use()
{
    if ( !_SHAinitializated )
        return false;

    glUseProgram(_proId);
    return true;
}

void Shader::StopUse()
{
    glUseProgram(0);
}

