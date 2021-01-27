#ifndef OUTLINER
#define OUTLINER

#include "common.hpp"

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
  static void FetchOutlines(const string& text, const string& fontPath, vector <FT_OutlineGlyph>& outlines, vector <FT_Glyph_Metrics>& metrics )
  {
    FT_Library library;
    FT_Face    face;
    FT_Glyph   glyph;
    FT_UInt    glyph_index;

    CheckFtError( FT_Init_FreeType( &library ), "FT_Init_FreeType()" );
    CheckFtError( FT_New_Face( library, fontPath.c_str(), 0, &face ), "FT_New_Face()" );
    CheckFtError( FT_Set_Char_Size( face, 0, 16*64, 300, 300 ), "FT_Set_Char_Size()" );

    size_t len = strlen( text.c_str() );
    for (int i = 0; i < len; ++i)
    {
      glyph_index = FT_Get_Char_Index(face, text[i]);
    
      CheckFtError( FT_Load_Glyph( face, glyph_index, FT_LOAD_DEFAULT ), "FT_Load_Glyph()" );
      CheckFtError( FT_Get_Glyph( face->glyph, &glyph ), "FT_Get_Glyph()" );

      if( glyph->format == FT_GLYPH_FORMAT_BITMAP )
      {
        cerr << "ERROR: glyph->format == FT_GLYPH_FORMAT_BITMAP" << endl;
        exit(-1);
      }

      outlines.push_back( (FT_OutlineGlyph)glyph );
      metrics.push_back( face->glyph->metrics );
    }
  }
};

#endif
