#ifndef OUTLINER
#define OUTLINER

#include "common.hpp"

struct Vertex
{
    Vertex( const vec3& pos, const vec3& bc ) : pos( pos ), bc( bc ) {}
    vec3 pos;
    vec3 bc;
};

struct Outline
{
  vector<vector<FT_Vector>> contours;
  FT_Glyph_Metrics metrics;
  vector<vector<Vertex>> conics;
  FT_Vector shadePos;
};


class Outliner
{
private:

  enum State {ST_POINT, ST_FIRST_CONIC, ST_SECOND_CONIC};
  static State state;
  static inline const FT_Pos spaceWidth = 1300;
  static const FT_Pos textStartPos = -13000;
  static inline const GLfloat density = 1.1f;
  static inline const FT_Pos shadeOffset = 1000;

  static void CheckError(const int error, const string where)
  {
    if ( error )
    {
      cerr << "ERROR: " << where << " returns: " << error << endl;
      exit(-1);
    }  
  }

  static void AddConic( const vec3& p1, const vec3& p2, const vec3& p3, vector< Vertex >& conics )
  {
    conics.push_back( Vertex( p1, vec3( 1, 0, 0 ) ) );
    conics.push_back( Vertex( p2, vec3( 0, 1, 0 ) ) );
    conics.push_back( Vertex( p3, vec3( 0, 0, 1 ) ) );
  }

public:
  static void Fetch(const string& text, const string& fontPath, vector <Outline>& outlines)
  {
    FT_Library library;
    FT_Face    face;
    FT_Glyph   glyph;
    FT_UInt    glyph_index;

    CheckError( FT_Init_FreeType( &library ), "FT_Init_FreeType()" );
    CheckError( FT_New_Face( library, fontPath.c_str(), 0, &face ), "FT_New_Face()" );
    CheckError( FT_Set_Char_Size( face, 0, 16*64, 300, 300 ), "FT_Set_Char_Size()" );

    FT_Pos glyphPos = textStartPos;
    size_t len = strlen( text.c_str() );
    for (int g = 0; g < len; ++g)
    {
      // cout << "---- Glyph ---- " << endl;
      glyph_index = FT_Get_Char_Index(face, text[g]);
    
      CheckError( FT_Load_Glyph( face, glyph_index, FT_LOAD_DEFAULT ), "FT_Load_Glyph()" );
      CheckError( FT_Get_Glyph( face->glyph, &glyph ), "FT_Get_Glyph()" );

      if( glyph->format == FT_GLYPH_FORMAT_BITMAP )
      {
        cerr << "ERROR: glyph->format == FT_GLYPH_FORMAT_BITMAP" << endl;
        exit(-1);
      }

      Outline outline;
      outline.metrics    = face->glyph->metrics;
      outline.shadePos.x = glyphPos + shadeOffset;
      outline.shadePos.y = -shadeOffset;

      short* contours   = ((FT_OutlineGlyph)glyph)->outline.contours;
      short maxContour  = ((FT_OutlineGlyph)glyph)->outline.n_contours;
      FT_Vector* points = ((FT_OutlineGlyph)glyph)->outline.points;
      char* tags        = ((FT_OutlineGlyph)glyph)->outline.tags;

      for(int c = 0, p = 0; c < maxContour; ++c)
      {
        // cout << "---- Contour ---- " << endl;
        vector<Vertex> conic;
        vector<FT_Vector> contour;

        /* TEST
        AddConic( vec3(glyphPos, 1000, 0),
                  vec3(1000 + glyphPos, 1000, 0),
                  vec3(1000 + glyphPos, 2000, 0), conic );
        */

        int contourStart = p;
        int counter = 0;
        short maxPoint = contours[c];
        for(; p <= maxPoint; ++p)
        {
          points[p].x += glyphPos;

          // DEBUG: TODO - Finish conics processing
          contour.push_back(points[p]);
          continue;

          if(tags[p] == FT_CURVE_TAG(FT_CURVE_TAG_ON))
          {
            // cout << counter++ << " point" << endl;
            contour.push_back(points[p]);
          } else if(tags[p] == FT_CURVE_TAG(FT_CURVE_TAG_CONIC))
          {
            // cout << counter++ << " conics" << endl;
            if(( tags[p-1] == FT_CURVE_TAG(FT_CURVE_TAG_ON) ) &&
               ( tags[p+1] == FT_CURVE_TAG(FT_CURVE_TAG_ON) ))
              AddConic( vec3(points[p-1].x, points[p-1].y, 0),
                        vec3(points[p].x  , points[p].y,   0),
                        vec3(points[p+1].x, points[p+1].y, 0), conic );

            else if( tags[p-1] == FT_CURVE_TAG(FT_CURVE_TAG_CONIC) )
            {
              FT_Vector np;
              np.x = (points[p-1].x + points[p].x ) / 2;
              np.y = (points[p-1].y + points[p].y ) / 2;
              contour.push_back(np);

              AddConic( vec3(points[p-1].x, points[p-1].y, 0),
                        vec3(points[p].x  , points[p].y,   0),
                        vec3(points[p+1].x, points[p+1].y, 0), conic );
            }

          } else
          {
           // contour.push_back(points[p]);
           // cout << "TAG: " << (int)tags[p] << endl;
          }
        }
        
        outline.contours.push_back(contour);
        outline.conics.push_back(conic);
      }

      outlines.push_back(outline);
      glyphPos += (outline.metrics.width == 0 ? spaceWidth : outline.metrics.width  * density);
    }
  }
};

Outliner::State Outliner::state;
#endif
