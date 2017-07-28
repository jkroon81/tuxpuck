/* ttf2font.c - Copyright (C) 2001-2002 Jacob Kroon, see COPYING for details */

#include <stdlib.h>
#include <stdio.h>
#include <ft2build.h>
#include FT_FREETYPE_H

/* defines */
#define START_CHAR	' '
#define END_CHAR	'z'

void errorc(char *msg)
{
  fprintf(stderr, "%s\n", msg);
  exit(1);
}

void errorcc(char *msg1, char *msg2)
{
  fprintf(stderr, "%s%s\n", msg1, msg2);
  exit(1);
}

void write_glyph(FILE * file, FT_GlyphSlot glyph)
{
  short int x, y;
  short int xsize, ysize;
  short int xpos, ypos, advance;

  FT_Bitmap bitmap = glyph->bitmap;
  xsize = bitmap.width;
  ysize = bitmap.rows;
  xpos = glyph->bitmap_left;
  ypos = glyph->bitmap_top;
  advance = glyph->advance.x >> 6;
  fwrite(&xsize, sizeof(xsize), 1, file);
  fwrite(&ysize, sizeof(ysize), 1, file);
  fwrite(&xpos, sizeof(xpos), 1, file);
  fwrite(&ypos, sizeof(ypos), 1, file);
  fwrite(&advance, sizeof(advance), 1, file);
  for (y = 0; y < bitmap.rows; y++)
    for (x = 0; x < bitmap.width; x++)
      fwrite(&bitmap.buffer[y * bitmap.pitch + x], 1, 1, file);
}

int main(int argc, char **argv)
{
  FT_Library ftl;
  FT_Face face;
  FT_UInt xsize, ysize;
  unsigned char ch;
  FILE *out = NULL;

  if (argc != 5)
    errorc("Usage: ttf2font <in> <out> <x-size> <y_size>");
  if (sscanf(argv[3], "%d", &xsize) != 1
      || sscanf(argv[4], "%d", &ysize) != 1)
    errorc("Wrong format of font size");
  if (FT_Init_FreeType(&ftl))
    errorc("Could't initialize FreeType2 library");
  if (FT_New_Face(ftl, argv[1], 0, &face))
    errorcc("Couldn't open font ", argv[1]);
  if (FT_Set_Charmap(face, face->charmaps[0]))
    errorc("Error setting first charmap");
  if (FT_Set_Pixel_Sizes(face, xsize, ysize))
    errorc("Error setting font size");
  out = fopen(argv[2], "wb");
  for (ch = START_CHAR; ch <= END_CHAR; ch++) {
    if (FT_Load_Char(face, ch, FT_LOAD_RENDER))
      errorc("Error rendering glyph");
    fwrite(&ch, 1, 1, out);
    write_glyph(out, face->glyph);
  }
  fclose(out);
  return 0;
}
