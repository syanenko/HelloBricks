#ifndef COMMON
#define COMMON

#ifdef USEGLEW
  #include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES

#ifdef __APPLE__
  #include <GLUT/glut.h>
#else
  #include <GL/glut.h>
#endif

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H

#include <iostream>
#include <string>
#include <vector>

using namespace std;

static void CheckGlError(string where)
{
  int err = glGetError();
  if (err)
  {
    cerr << "ERROR: " << gluErrorString(err) << " at " << where.c_str() << endl;
    exit(-1);
  }
}

#endif