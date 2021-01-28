/*
 * What: "Hello Bricks" - Text rendering with OpenGL
 * Who:  Sergey Yanenko
 * Why:  Homework for "Bricks" project
 * When: 01/2021
 *
 *  Usage: HelloBricks [-h][-t <text>[-f [<font>]]
 *
 *  Keys:
 *  -h: Show this help
 *  -t: Text to render
 *  -f: Font to use ('fonts/times.ttf' by default)
 *  
 *  Control:
 *  +/- Zoom in/out
 *  Arrows: Pan
 *  Enter: Set default view
 *  Esc: Exit
*/

#include "common.hpp"
#include "outliner.hpp"
#include "shader.hpp"

char* text = "Hello Bricks";
char* fontPath = "fonts/times.ttf";

char* pathVertexShader   = "shaders/text.vert";
char* pathFragmentShader = "shaders/text.frag";

// View control
int winWidth  = 1024;
int winHeight = 768;
GLdouble origX = 0.0;
GLdouble origY = 0.0;
float defaultZoom = 0.35;
float zoom = defaultZoom;

// Text rendering control
GLfloat glyphScale = 5.0;
GLfloat spaceWidth = 1300.0;
GLfloat density = 1.1;
GLfloat glyphStartPos = -13000.0;
GLfloat shadeOffsetX =  0.1;
GLfloat shadePosY = -0.1;
GLfloat scaledColor = 1.0 / 255.0;

GLfloat scaleX = 1.0 / (glyphScale * winWidth);
GLfloat scaleY = 1.0 / (glyphScale * winHeight);

// vector <FT_OutlineGlyph> outlines;
vector <Outline> outlines;
vector <FT_Glyph_Metrics> metrics;

GLuint texBuffer, texture;
GLuint shaderProg = 0;

GLuint debug_texture;

//
// Init texture
//
void initTexture()
{
  glGenFramebuffers(1, &texBuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, texBuffer);

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, winWidth, winHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if(status != GL_FRAMEBUFFER_COMPLETE)
  {
    cerr << "ERROR: " << "glCheckFramebufferStatus()" << " returns " << status << endl;
    exit(-1);
  }
}

//
//  GLUT display callback
//
void onDisplay()
{
  // Draw to texture
  glBindTexture(GL_TEXTURE_2D, 0);
  glEnable(GL_TEXTURE_2D);
  glBindFramebuffer(GL_FRAMEBUFFER, texBuffer);

  glClear(GL_COLOR_BUFFER_BIT);
  
  glLoadIdentity();
  glScaled(zoom, zoom, 1);
  glTranslated(origX, origY, 0);

  glUseProgram(0);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE);
  glBlendEquation(GL_FUNC_ADD);

  glColor3f(scaledColor, scaledColor, scaledColor);

  GLfloat glyphPos = glyphStartPos;
  GLfloat shadePosX = glyphPos * scaleX + shadeOffsetX;

  // DEBUG: Lines
  /*
  for (int maxOutline = outlines.size(), i=0; i < maxOutline; ++i)
  {
    cout << "-- Outline -----" << endl;
    vector<vector<FT_Vector>> contours;
    for(int maxContour = outlines[i].contours.size(), c = 0; c < maxContour; ++c)
    {
      cout << "-- Contour -----" << endl;
      glBegin(GL_LINE_LOOP);
      for(int maxPoint = outlines[i].contours[c].size(), p = 0; p < maxPoint; ++p)
      {
        cout << outlines[i].contours[c][p].x << " " << outlines[i].contours[c][p].y << endl;
        GLfloat x = (outlines[i].contours[c][p].x + glyphPos) * scaleX;
        GLfloat y =  outlines[i].contours[c][p].y * scaleY;
        glVertex2f(x, y);
      }
      glEnd();
    }
    
    glyphPos += (outlines[i].metrics.width == 0 ? spaceWidth : outlines[i].metrics.width * density);
    shadePosX = glyphPos * scaleX + shadeOffsetX;
  }
  */

  for (int maxOutline = outlines.size(), i=0; i < maxOutline; ++i)
  {
    // cout << "-- Outline -----" << endl;
    vector<vector<FT_Vector>> contours;
    for(int maxContour = outlines[i].contours.size(), c = 0; c < maxContour; ++c)
    {
      // cout << "-- Contour -----" << endl;
      glBegin(GL_TRIANGLES);
      int maxPoint = outlines[i].contours[c].size();
      for(int p = 1; p < maxPoint; ++p)
      {
        // cout << outlines[i].contours[c][p].x << " " << outlines[i].contours[c][p].y << endl;
        GLfloat x1 = (outlines[i].contours[c][p-1].x + glyphPos) * scaleX;
        GLfloat y1 =  outlines[i].contours[c][p-1].y * scaleY;

        GLfloat x2 = (outlines[i].contours[c][p].x + glyphPos) * scaleX;
        GLfloat y2 =  outlines[i].contours[c][p].y * scaleY;

        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
        glVertex2f(shadePosX, shadePosY);
      }

      GLfloat x1 = (outlines[i].contours[c][0].x + glyphPos) * scaleX;
      GLfloat y1 =  outlines[i].contours[c][0].y * scaleY;

      GLfloat x2 = (outlines[i].contours[c][maxPoint-1].x + glyphPos) * scaleX;
      GLfloat y2 =  outlines[i].contours[c][maxPoint-1].y * scaleY;

      glVertex2f(x1, y1);
      glVertex2f(x2, y2);
      glVertex2f(shadePosX, shadePosY);

      glEnd();
    }
    
    glyphPos += (outlines[i].metrics.width == 0 ? spaceWidth : outlines[i].metrics.width * density);
    shadePosX = glyphPos * scaleX + shadeOffsetX;
  }

  // Display texture
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClear(GL_COLOR_BUFFER_BIT);

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, texture);

  glUseProgram(shaderProg);

  GLint loc = glGetUniformLocation(shaderProg, "u_resolution");
  if (loc >= 0)
    glUniform2f( loc, (GLfloat)winWidth, (GLfloat)winHeight );

  loc = glGetUniformLocation(shaderProg, "u_texture");
  if (loc >= 0)
    glUniform1i( loc, 0 );

  glLoadIdentity();
  glScaled(1, 1, 1);
  glTranslated(0, 0, 0);

  glBegin(GL_QUADS);
  glTexCoord2f(0, 0); glVertex2f(-1, -1);
  glTexCoord2f(0, 1); glVertex2f(-1, +1);
  glTexCoord2f(1, 1); glVertex2f(+1, +1);
  glTexCoord2f(1, 0); glVertex2f(+1, -1);
  glEnd();

  CheckGlError("onDisplay()");
  glutSwapBuffers();
}

//
//  GLUT special keys callback
//
void onKeysSpecial(int key, int x, int y)
{
  switch(key)
  {
    case GLUT_KEY_RIGHT: origX -= (0.03f / zoom); break;
    case GLUT_KEY_LEFT:  origX += (0.03f / zoom); break;
    case GLUT_KEY_UP:    origY -= (0.03f / zoom); break;
    case GLUT_KEY_DOWN:  origY += (0.03f / zoom); break;
  }

  glutPostRedisplay();
}

//
//  GLUT common keys callback
//
void onKeysCommon(unsigned char key, int x,int y)
{
  switch(key)
  {
    case '-': zoom /= 1.1f;       break;
    case '+': zoom *= 1.1f;       break;
    case 13:  origX = origY = 0;
              zoom = defaultZoom; break;
    case 27:  exit(0);
  }

  glutPostRedisplay();
}

//
// GLUT reshape callback
//
void onReshape(int width, int height)
{
   glutReshapeWindow(winWidth, winHeight);
}

//
// Print usage
//
void printUsage()
{
  printf("Usage: HelloBricks [-h][-t <text>[-f [<font>]]\n\n\
Keys:\n\n\
-h: Show this help\n\
-t: Text to render\n\
-f: Font to use ('fonts/times.ttf' by default)\n\n\
Control:\n\n\
+/-: Zoom in/out\n\
Arrows: Pan\n\
Enter: Set default view\n\
Esc: Exit\n\n");
}

//
// Process command line
//
void procCommandLine(int argc, char* argv[])
{
  if( argc < 3)
  {
    printUsage();
    exit(0);
  }

  if(!strcmp(argv[1], "-h"))
  {
    printUsage();
    exit(0);
  } else if( !strcmp(argv[1], "-t") )
  {
    text = argv[2];
    if( argc == 5 )
    {
      if( !strcmp(argv[3], "-f") )
        fontPath = argv[4];
    }
  }
}

void onIdle()
{
    glutPostRedisplay();
}

//
// Init Glut
//
void InitGlut(int argc, char* argv[])
{
  glutInit( &argc, argv );
  glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
  glutInitWindowSize(winWidth, winHeight);
  glutCreateWindow("Hello Bricks !");

  // Callbacks
  glutDisplayFunc(onDisplay);
  glutReshapeFunc(onReshape);
  glutKeyboardFunc(onKeysCommon);
  glutSpecialFunc(onKeysSpecial);
  // glutIdleFunc(onIdle);

  if (glewInit() != GLEW_OK)
  {
    cerr << "ERROR: Error initializing GLEW" << endl;
    exit(-1);
  }
  
  CheckGlError("InitGlut()");
}


//
// Main
//
int main(int argc, char* argv[])
{
  procCommandLine(argc, argv);
  printUsage();

  // Outliner::FetchOutlines(text, fontPath, outlines, metrics );
  Outliner::Fetch(text, fontPath, outlines);
  // return 0;

  InitGlut(argc, argv);

  shaderProg = Shader::CreateProgram(pathVertexShader, pathFragmentShader);

  initTexture();

  glutMainLoop();
}
