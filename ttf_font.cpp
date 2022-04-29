/*  $Id: ttf_font.cpp 1274 2006-09-05 19:10:18Z grumbel $
**   __      __ __             ___        __   __ __   __
**  /  \    /  \__| ____    __| _/_______/  |_|__|  | |  |   ____
**  \   \/\/   /  |/    \  / __ |/  ___/\   __\  |  | |  | _/ __ \
**   \        /|  |   |  \/ /_/ |\___ \  |  | |  |  |_|  |_\  ___/
**    \__/\  / |__|___|  /\____ /____  > |__| |__|____/____/\___  >
**         \/          \/      \/    \/                         \/
**  Copyright (C) 2005 Ingo Ruhnke <grumbel@gmx.de>
**
**  This program is free software; you can redistribute it and/or
**  modify it under the terms of the GNU General Public License
**  as published by the Free Software Foundation; either version 2
**  of the License, or (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
** 
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <iostream>
#include <iomanip>
#include <fstream>
#include <assert.h>
#include <vector>
#include <stdexcept>
#include <sstream>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "ttf_font.hpp"

TTFCharacter::TTFCharacter(FT_GlyphSlot glyph)
{
  x_offset =  glyph->bitmap_left;
  y_offset = -glyph->bitmap_top;

  advance = (glyph->advance.x >> 6);

  bitmap = new Bitmap(glyph->bitmap.width, glyph->bitmap.rows);

  for(int y = 0; y < bitmap->get_height(); ++y)
    for(int x = 0; x < bitmap->get_width(); ++x)
      {
        bitmap->get_data()[y * bitmap->get_width() + x] = 255 - glyph->bitmap.buffer[y * glyph->bitmap.pitch + x];
      }
}

class TTFFontImpl
{
public:
  /// the global FreeType library handle
  static FT_Library      library;

  /** Array of characters available in this font, current limited to
      256 characters, no full unicode */
  std::vector<TTFCharacter*> characters;

  /** The original size of the font in pixels */
  int size;

  /** The amount of pixels to advance in y direction after a finished
      line, this can be transformed by FontEffect */
  int height;

  ~TTFFontImpl()
  {
    for(std::vector<TTFCharacter*>::size_type i = 0; i < characters.size(); ++i)
      delete characters[i];
  }
};

FT_Library TTFFontImpl::library;

TTFFont::TTFFont(const std::string& filename, int size_)
  : impl(new TTFFontImpl())
{
  assert(size_ > 0);

  impl->size = size_;

  // Read the TTF font file content into buffer
  std::ifstream fin(filename.c_str());
  std::istreambuf_iterator<char> first(fin), last;
  std::vector<char> buffer(first, last); 

  FT_Face face;
  if (FT_New_Memory_Face(TTFFontImpl::library, 
                         reinterpret_cast<FT_Byte*>(&*buffer.begin()), buffer.size(), 
                         0, &face))
    {
      throw std::runtime_error("Couldn't load font: '" + filename + "'");
    }
  
  FT_Set_Pixel_Sizes(face, impl->size, impl->size);

  FT_Select_Charmap(face,  FT_ENCODING_UNICODE);

  impl->height = impl->size;

  std::cout << "BBox: " << impl->size << " "
            << impl->size * face->bbox.xMin/face->units_per_EM << " " 
            << impl->size * face->bbox.yMin/face->units_per_EM << " " 
            << impl->size * face->bbox.xMax/face->units_per_EM << " " 
            << impl->size * face->bbox.yMax/face->units_per_EM << " " 
            << face->units_per_EM
            << std::endl;

  // We limit ourself to 256 characters for the momemnt
  for(int glyph_index = 0; glyph_index < 256; glyph_index += 1)
    {
      if (FT_Load_Char( face,  glyph_index, FT_LOAD_RENDER))//| FT_LOAD_FORCE_AUTOHINT))
        {
          std::cerr << "couldn't load char: " << glyph_index << " '" << char(glyph_index) << "'" << std::endl;
          impl->characters.push_back(0);
        }
      else
        {
          impl->characters.push_back(new TTFCharacter(face->glyph));
        }
      //std::cout << "Char: " << int(glyph_index) << " " << face->glyph->bitmap_top << std::endl;
    }

  FT_Done_Face(face);
}

TTFFont::~TTFFont()
{
  delete impl;
}

const TTFCharacter&
TTFFont::get_character(char c) const
{
  if (impl->characters[(unsigned char)c] == 0)
    return *impl->characters['?'];
  else
    return *impl->characters[(unsigned char)c];

  /*  unsigned char c = c_;
  if (!(c >= 0 && c < 256) || impl->characters[c] == 0)
    {
      std::cout << "Unknown character: " << c << " '" << c << "'" << std::endl;
      return *impl->characters['?'];
    }
  else
    {
      return *impl->characters[c];
      }*/
}

int
TTFFont::get_height() const
{
  return impl->size;
}

int
TTFFont::get_width(const std::string& text) const
{
  int width = 0;
  for(std::string::const_iterator i = text.begin(); i != text.end(); ++i)
    width += get_character(*i).advance;
  return width;
}

void
TTFFont::init()
{
  FT_Error  error;
  
  error = FT_Init_FreeType( &TTFFontImpl::library );
  if ( error )
    throw std::runtime_error( "could not initialize FreeType" );
}

void
TTFFont::deinit()
{
  FT_Done_FreeType( TTFFontImpl::library );
}

/* EOF */
