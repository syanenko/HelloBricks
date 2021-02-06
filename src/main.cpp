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

char* pathVertexDummy    = "shaders/dummy.vert";
char* pathFragmentDummy  = "shaders/dummy.frag";
char* pathVertexShade    = "shaders/shade.vert";
char* pathFragmentShade  = "shaders/shade.frag";
char* pathVertexSmooth   = "shaders/smooth.vert";
char* pathFragmentSmooth = "shaders/smooth.frag";

// View control
int winWidth  = 1024;
int winHeight = 768;

GLdouble origX = 0.0;
GLdouble origY = 0.0;
GLdouble moveStep = 150;

GLdouble defaultZoom = 14000.0;
GLdouble zoom = defaultZoom;

// Text rendering control
GLfloat spaceWidth = 1300.0;
GLfloat density = 1.1f;
GLfloat textStartPos = -13000.0;

GLfloat scaledColor = 1.0f / 255.0f;

vector <Outline> outlines;

GLuint texBuffer, texture;
GLuint progShade = 0;
GLuint progDummy = 0;
GLuint progSmooth = 0;

GLfloat conics[4096];

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
  // glBindFramebuffer(GL_FRAMEBUFFER, 0); // -- DEBUG

  glClear(GL_COLOR_BUFFER_BIT);

  // Projection
  double ar = glutGet( GLUT_WINDOW_WIDTH ) / glutGet( GLUT_WINDOW_HEIGHT );
  mat4 projection = glm::ortho< float >( -zoom * ar + origX, zoom * ar + origX, -zoom + origY, zoom + origY, -1.0, 1.0 );
  mat4 modelview = mat4( 1.0 );

  glUseProgram(progDummy);
  GLint loc = glGetUniformLocation( progDummy, "u_projection" );
  glUniformMatrix4fv( loc, 1, GL_FALSE, glm::value_ptr( projection ) );
  loc = glGetUniformLocation( progDummy, "u_modelview" );
  glUniformMatrix4fv( loc, 1, GL_FALSE, glm::value_ptr( modelview) );

  // Draw
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE);
  glBlendEquation(GL_FUNC_ADD);

  glColor3f(scaledColor, scaledColor, scaledColor);

  // -- DEBUG: Lines
  /*
  for (int maxOutline = outlines.size(), i=0; i < maxOutline; ++i)
  {
    cout << "-- Outline -----" << endl;
    for(int maxContour = outlines[i].contours.size(), c = 0; c < maxContour; ++c)
    {
      cout << "-- Contour -----" << endl;
      glBegin(GL_LINE_LOOP);
      for(int maxPoint = outlines[i].contours[c].size(), p = 0; p < maxPoint; ++p)
      {
        cout << outlines[i].contours[c][p].x << " " << outlines[i].contours[c][p].y << endl;
        GLfloat x = outlines[i].contours[c][p].x;
        GLfloat y = outlines[i].contours[c][p].y;
        glVertex2f(x, y);
      }
      glEnd();
    }
  }
  */

  // Outlines
  for (int i = 0; i < outlines.size(); ++i)
  {
    // Contours
    vector<vector<FT_Vector>> contours;
    for(size_t maxContour = outlines[i].contours.size(), c = 0; c < maxContour; ++c)
    {
      size_t maxPoint = outlines[i].contours[c].size();
      if(maxPoint == 0)
        continue;

      glBegin(GL_TRIANGLES);
      for(int p = 1; p < maxPoint; ++p)
      {
        glVertex2f(outlines[i].contours[c][p-1].x,
                   outlines[i].contours[c][p-1].y);

        glVertex2f(outlines[i].contours[c][p].x,
                   outlines[i].contours[c][p].y);

        glVertex2f(outlines[i].shadePos.x,
                   outlines[i].shadePos.y);
      }

      glVertex2f(outlines[i].contours[c][0].x,
                 outlines[i].contours[c][0].y);

      glVertex2f(outlines[i].contours[c][maxPoint-1].x,
                 outlines[i].contours[c][maxPoint-1].y);

      glVertex2f(outlines[i].shadePos.x,
                 outlines[i].shadePos.y);

      glEnd();
    }
  }

  // -- DEBUG
  // glutSwapBuffers();
  // return;
  
  glDisable(GL_BLEND);

  // Display texture
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClear(GL_COLOR_BUFFER_BIT);

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, texture);

  glUseProgram(progShade);

  loc = glGetUniformLocation( progShade, "u_projection" );
  projection = glm::ortho< float >( -1, 1, -1, 1, -1.0, 1.0 );
  glUniformMatrix4fv( loc, 1, GL_FALSE, glm::value_ptr( projection ) );

  loc = glGetUniformLocation( progShade, "u_modelview" );
  glUniformMatrix4fv( loc, 1, GL_FALSE, glm::value_ptr( modelview ) );

  loc = glGetUniformLocation(progShade, "u_resolution");
  if (loc >= 0)
    glUniform2f( loc, (GLfloat)winWidth, (GLfloat)winHeight );
  
  loc = glGetUniformLocation(progShade, "u_texture");
  if (loc >= 0)
    glUniform1i( loc, 0 );
  
  glBegin(GL_QUADS);
  glTexCoord2f(0, 0); glVertex2f(-1, -1);
  glTexCoord2f(0, 1); glVertex2f(-1, +1);
  glTexCoord2f(1, 1); glVertex2f(+1, +1);
  glTexCoord2f(1, 0); glVertex2f(+1, -1);
  glEnd();

  // Apply smooth
  glUseProgram(progSmooth);

  loc = glGetUniformLocation( progSmooth, "u_projection" );
  projection = glm::ortho< float >( -zoom * ar + origX, zoom * ar + origX, -zoom + origY, zoom + origY, -1.0, 1.0 );
  glUniformMatrix4fv( loc, 1, GL_FALSE, glm::value_ptr( projection ) );
  
  loc = glGetUniformLocation( progSmooth, "u_modelview" );
  glUniformMatrix4fv( loc, 1, GL_FALSE, glm::value_ptr( modelview ) );
  
  for (int i = 0; i < outlines.size(); ++i)
  {
    for (int c = 0; c < outlines[i].conics.size(); ++c)
    {
      if(outlines[i].conics[c].size() > 0)
      {
        loc = glGetAttribLocation( progSmooth, "a_position" );
        glVertexAttribPointer( loc, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), &(outlines[i].conics[c][0].pos ));
        glEnableVertexAttribArray( loc );
    
        loc = glGetAttribLocation( progSmooth, "a_bc" );
        glVertexAttribPointer( loc, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), &outlines[i].conics[c][0].bc );
        glEnableVertexAttribArray( loc );
    
        glDrawArrays( GL_TRIANGLES, 0, outlines[i].conics[c].size() );
      }
    }
  }

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
    case GLUT_KEY_RIGHT: origX -= moveStep; break;
    case GLUT_KEY_LEFT:  origX += moveStep; break;
    case GLUT_KEY_UP:    origY -= moveStep; break;
    case GLUT_KEY_DOWN:  origY += moveStep; break;
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
    case '-': zoom *= 1.1f;       break;
    case '+': zoom /= 1.1f;       break;
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

  Outliner::Fetch(text, fontPath, outlines);

  InitGlut(argc, argv);

  progShade  = Shader::CreateProgram(pathVertexShade,  pathFragmentShade);
  progDummy  = Shader::CreateProgram(pathVertexDummy,  pathFragmentDummy);
  progSmooth = Shader::CreateProgram(pathVertexSmooth, pathFragmentSmooth);

  initTexture();

  glutMainLoop();
}
