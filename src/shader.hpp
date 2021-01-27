#include <iostream>

#include "common.hpp"

class Shader
{
  static char* LoadSource(const char *sourceFile)
  {
    long size;
    char* buffer;
    FILE* file = fopen(sourceFile, "rt");
     
    if (!file)
    {
      cerr << "Cannot open file '" << sourceFile << "'" << endl;
      exit(-1);
    }

    fseek(file, 0, SEEK_END);
    size = ftell(file);
    rewind(file);
     
    buffer = (char*)malloc(size + 1);
    if (!buffer)
    {
      cerr << "Cannot allocate " << size + 1 << " bytes for file '" << sourceFile << "'" << endl;
      exit(-1);
    }
     
    if (fread(buffer, size, 1, file) == -1)
    {
      cerr << "Cannot read " << size + 1 << " bytes from file '" << sourceFile << "'" << endl;
      exit(-1);
    }
     
    buffer[size] = 0;
    fclose(file);

    return buffer;
  }


  static void PrintShaderLog(const GLuint shader, const char* fileName)
  {
    GLsizei maxLen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLen);

    if (maxLen > 1)
    {
      GLsizei len = 0;
      char* buffer = (char *)malloc(maxLen);
      if (!buffer)
      {
        cerr << "Cannot allocate " << maxLen << " bytes for shader log" << endl;
        exit(-1);
      }

      glGetShaderInfoLog(shader, maxLen, &len, buffer);
      cerr << fileName << buffer << endl;
      free(buffer);
    }

    glGetShaderiv(shader, GL_COMPILE_STATUS, &maxLen);
    if (!maxLen)
    {
      cerr << "Error compiling '" << fileName << "'" << endl;
      exit(-1);
    }
  }


  static void PrintProgramLog(const GLuint program)
  {
     GLsizei maxLen=0;
     glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLen);
   
     if (maxLen > 1)
     {
        GLsizei len = 0;
        char* buffer = (char *)malloc(maxLen);
        if (!buffer)
        {
          cerr << "Cannot allocate " << maxLen << "bytes of text for program log" << endl;
          exit(-1);
        }
        
        glGetProgramInfoLog(program, maxLen, &len, buffer);
        cerr << buffer << endl;
     }
   
     glGetProgramiv(program, GL_LINK_STATUS, &maxLen);
     if (!maxLen)
       cerr << "Error linking program" << endl;
  }


  static GLuint CreateShader(const GLenum type, const char* fileName)
  {
     GLuint shader = glCreateShader(type);

     char* source = LoadSource(fileName);
     glShaderSource(shader, 1, (const char**)&source, NULL);
     free(source);

     glCompileShader(shader);

     PrintShaderLog(shader, fileName);

     return shader;
  }


public:
  static GLuint CreateProgram(const char* vertFileName, const char* fragFileName)
  {
     GLuint prog = glCreateProgram();

     if (vertFileName)
     {
        GLuint vert = CreateShader(GL_VERTEX_SHADER, vertFileName);
        glAttachShader(prog, vert);
     }
   
     if (fragFileName)
     {
        GLuint frag = CreateShader(GL_FRAGMENT_SHADER, fragFileName);
        glAttachShader(prog, frag);
     }

     glLinkProgram(prog);
     PrintProgramLog(prog);

     return prog;
  }


  static GLuint CreateShaderProgGeom(const char* vertFileName, const char* geomFileName, const char* fragFileName,
                                     const GLint in, const GLint out, const GLint n)
  {
     GLuint prog = glCreateProgram();

     if (vertFileName)
     {
        GLuint vert = CreateShader(GL_VERTEX_SHADER, vertFileName);
        glAttachShader(prog, vert);
     }

     if (geomFileName)
     {
        #ifdef GL_GEOMETRY_SHADER_EXT
          GLuint geom = CreateShader(GL_GEOMETRY_SHADER_EXT, geomFileName);
          glAttachShader(prog, geom);
          glProgramParameteriEXT(prog, GL_GEOMETRY_INPUT_TYPE_EXT,   in);
          glProgramParameteriEXT(prog, GL_GEOMETRY_OUTPUT_TYPE_EXT,  out);
          glProgramParameteriEXT(prog, GL_GEOMETRY_VERTICES_OUT_EXT, n);
        #else
          cerr << "Geometry shaders not supported" << endl;
        #endif
     }

     if (fragFileName)
     {
        GLuint frag = CreateShader(GL_FRAGMENT_SHADER, fragFileName);
        glAttachShader(prog, frag);
     }

     glLinkProgram(prog);
     PrintProgramLog(prog);

     return prog;
  }


  static int CreateShaderProgLoc(const char* vertFileName, const char* fragFileName, const GLchar* name[])
  {
     int prog = glCreateProgram();
   
     if (vertFileName)
     {
        GLuint vert = CreateShader(GL_VERTEX_SHADER, vertFileName);
        glAttachShader(prog, vert);
     }
   
     if (fragFileName)
     {
        GLuint frag = CreateShader(GL_FRAGMENT_SHADER, fragFileName);
        glAttachShader(prog, frag);
     }
     
     for (GLuint i=0; name[i]; ++i)
     {
      if (name[i][0])
        glBindAttribLocation(prog, i, name[i]);
     }

     CheckGlError("CreateShaderProgLoc()");

     glLinkProgram(prog);

     PrintProgramLog(prog);

     return prog;
  }
};