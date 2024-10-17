#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <vector>

#include <GL/glew.h>

// For shader attributes
struct shaVars {
  GLuint loc; // The attribute "location", some kind of index in the shader
  std::string name; // The name of the attribute
};

typedef std::vector<shaVars> shaVars_v;

// For shader code
struct shaShas {
  GLuint shaId;
  GLenum typeSha;    // The type of shader
  std::string scode; // The NULL terminated GLSL code
};

typedef std::vector<shaShas> shaShas_v;

//-----------------------------------------------------------------------------
// This object builds a GPU program by joining several shaders.
class Shader {
public:
  Shader();
  ~Shader();

  void Init();
  bool Use();
  void StopUse();
  void CleanUp();

  void AddCode(std::string &shaString, GLenum shaType);
  bool LoadCode(const char *path, GLenum shaType);
  void AddAttrib(const std::string &name);
  void AddUnif(const std::string &name);
  GLuint GetAttribLoc(const std::string &name);
  GLuint GetUnifLoc(const std::string &name);
  GLuint GetProgramId() { return _proId; }

private:
  void SetAttribLocations();
  bool AskUnifLocations();
  bool Compile(GLuint shaId);
  bool LinkProg(GLuint proId);

  shaVars_v _shaAttrib; // 'attributes' names and locations
  shaVars_v _shaUnif;   // 'uniforms' names and locations
  shaShas_v _shaCode;   // shaders code and their types
  GLuint _proId;        // program Id

  bool _SHAinitializated;
};

#endif // SHADER_H
