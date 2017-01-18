#ifndef SHADER__HEADER_INCLUDED
#define SHADER__HEADER_INCLUDED

#include "common.hpp" // GL
#include <iostream>
#include <string>
#include <fstream>



class Shader
{
public:

  inline Shader() { }
  inline Shader(std::string vertexSourceFilename, std::string fragmentSourceFilename) 
         : vertexSourceFilename_(vertexSourceFilename), fragmentSourceFilename_(fragmentSourceFilename)
         , vertex_(0), fragment_(0), program_(0), initOK_(false) 
  { }
  inline Shader(Shader const &shader)
         : vertexSourceFilename_(shader.vertexSourceFilename_), fragmentSourceFilename_(shader.fragmentSourceFilename_)
         , vertex_(0), fragment_(0), program_(0), initOK_(shader.initOK_)
  {   
    if(initOK_)
      init(); 
  }
  inline ~Shader() 
  {
     destroy();
  }

  bool init();
  void destroy();
  GLuint getProgramID() const;
  Shader& operator=(Shader const &shader);

private:

  bool initShaderStage(GLuint &shader, GLenum shader_stage_type, std::string const &source);
  //void bindAttribLocation_();

private:

  std::string vertexSourceFilename_;
  std::string fragmentSourceFilename_;
  GLuint vertex_;
  GLuint fragment_;
  GLuint program_;
  bool initOK_;
};



#endif // GFX_SHADER__HEADER_INCLUDED