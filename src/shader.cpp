#include "shader.hpp"

void Shader::destroy()
{ 
  glDeleteShader(vertex_);
  glDeleteShader(fragment_);
  glDeleteProgram(program_);
  vertex_ = 0;
  fragment_ = 0;
  program_ = 0;
  initOK_ = false;
}

bool Shader::init()
{ 
  //------- Instanciation -------
  if(initShaderStage(vertex_, GL_VERTEX_SHADER, vertexSourceFilename_) == false)
    return false;
  if(initShaderStage(fragment_, GL_FRAGMENT_SHADER, fragmentSourceFilename_) == false)
    return false;
  
  program_ = glCreateProgram();
  glAttachShader(program_, vertex_);
  glAttachShader(program_, fragment_);
   //bindAttribLocation_();
  glLinkProgram(program_);

  //------- Link & check -------
  GLint link(0);
  glGetProgramiv(program_, GL_LINK_STATUS, &link);
  if(link != GL_TRUE)
  {   
    GLint err_size(0);
    char *err_msg(NULL);
    glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &err_size);
    err_msg = new char[err_size + 1];
    glGetProgramInfoLog(program_, err_size, &err_size, err_msg);
    err_msg[err_size] = '\0';   
    std::cerr << "--------------------------------------------------------------" << std::endl;
    std::cerr << "+ GLSL ERROR - linking program + \n" << err_msg << std::endl;
    std::cerr << "--------------------------------------------------------------" << std::endl;
    delete [] err_msg;
    return false;
  }   
  return (initOK_ = true);
}

bool Shader::initShaderStage(GLuint &shader, GLenum shader_stage_type, std::string const &source_filename)
{ 
  if(shader_stage_type != GL_VERTEX_SHADER && shader_stage_type != GL_FRAGMENT_SHADER) {
    std::cerr << "GFX ERROR, Shader stage not supported : " << shader_stage_type << std::endl;
    glDeleteShader(shader);
    return false;
  }
  shader = glCreateShader(shader_stage_type);
    
  //------- Parse file -------
  std::string code, code_line;
  std::ifstream source_file(source_filename.c_str());
  if(!source_file) {
    std::cerr << "Error - unable to open file \'" << source_filename << "\' - in gfx::Shader::initShaderStage" << std::endl;
    glDeleteShader(shader);
    return false;
  } 
  while(getline(source_file, code_line)) {
    code += code_line + '\n';
  }
  source_file.close();

  //------- Build & check -------
  GLint build_err(0), err_size(0);
  const GLchar* code_cstr = code.c_str();
  glShaderSource(shader, 1, &code_cstr, NULL);
  glCompileShader(shader);  
  glGetShaderiv(shader, GL_COMPILE_STATUS, &build_err);
  if(build_err != GL_TRUE)
  {   
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &err_size);
    char *err_msg = new char[err_size + 1];
    err_msg[err_size] = '\0';
    glGetShaderInfoLog(shader, err_size, &err_size, err_msg);
    std::cerr << "--------------------------------------------------------------" << std::endl;
    std::cerr << "+ GLSL ERROR - shader stage (" << source_filename << "::" << shader_stage_type << ") +" << std::endl;
    std::cerr << "--------------------------------------------------------------" << std::endl;
    std::cerr << err_msg << std::endl;
    std::cerr << "--------------------------------------------------------------" << std::endl;   
    delete [] err_msg;
    return false;
  }

  //--- OK ---
  return true;
}

/*
void Shader::bindAttribLocation_()
{
  //glBindAttribLocation(program_, 0, "inVertex");
  //glBindAttribLocation(program_, 1, "inNormal");
  //glBindAttribLocation(program_, 2, "in_TexCoord0");
  //glBindAttribLocation(program_, 3, "in_Normal");
}
*/

Shader& Shader::operator=(Shader const &shader)
{
  if(this != &shader) {
    vertexSourceFilename_   = shader.vertexSourceFilename_;
    fragmentSourceFilename_ = shader.fragmentSourceFilename_;
    initOK_ = shader.initOK_;
    destroy();    
    init();
  }
  return *this;
}

GLuint Shader::getProgramID() const {
  return program_;
}
