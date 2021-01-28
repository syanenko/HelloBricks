#ifndef OUTLINER
#define OUTLINER

#include "common.hpp"

struct Outline
{
  vector<vector<FT_Vector>> contours;
  vector<vector<FT_Vector>> quadsegs;
  FT_Glyph_Metrics metrics;
};

class Outliner
{
  static void CheckFtError(const int error, const string where)
  {
    if ( error )
    {
      cerr << "ERROR: " << where << " returns: " << error << endl;
      exit(-1);
    }  
  }

public:
  static void Fetch(const string& text, const string& fontPath, vector <Outline>& outlines )
  {
    FT_Library library;
    FT_Face    face;
    FT_Glyph   glyph;
    FT_UInt    glyph_index;

    CheckFtError( FT_Init_FreeType( &library ), "FT_Init_FreeType()" );
    CheckFtError( FT_New_Face( library, fontPath.c_str(), 0, &face ), "FT_New_Face()" );
    CheckFtError( FT_Set_Char_Size( face, 0, 16*64, 300, 300 ), "FT_Set_Char_Size()" );

    size_t len = strlen( text.c_str() );
    for (int g = 0; g < len; ++g)
    {
      glyph_index = FT_Get_Char_Index(face, text[g]);
    
      CheckFtError( FT_Load_Glyph( face, glyph_index, FT_LOAD_DEFAULT ), "FT_Load_Glyph()" );
      CheckFtError( FT_Get_Glyph( face->glyph, &glyph ), "FT_Get_Glyph()" );

      if( glyph->format == FT_GLYPH_FORMAT_BITMAP )
      {
        cerr << "ERROR: glyph->format == FT_GLYPH_FORMAT_BITMAP" << endl;
        exit(-1);
      }

      Outline outline;
      outline.metrics   = face->glyph->metrics;
      short* contours   = ((FT_OutlineGlyph)glyph)->outline.contours;
      short maxContour  = ((FT_OutlineGlyph)glyph)->outline.n_contours;
      FT_Vector* points = ((FT_OutlineGlyph)glyph)->outline.points;

      for(int c = 0, p = 0; c < maxContour; ++c)
      {
        vector<FT_Vector> contour;
        for(short maxPoint = contours[c]; p <= maxPoint; ++p)
          contour.push_back(points[p]);
        
        outline.contours.push_back(contour);
      }

      outlines.push_back(outline);
    }
  }
};
#endif
