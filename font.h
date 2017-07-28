/* font.h - Copyright (C) 2001-2002 Jacob Kroon, see COPYING for details */

#ifndef _FONT_H
#define _FONT_H

/* includes */
#include <SDL_video.h>

/* structs */
typedef struct _Font Font;

/* functions */
Font *font_create(Uint8 *, Uint32 *);
void font_free(Font *);
void font_print(Font *, char *, SDL_Rect *);
Uint32 font_calc_width(Font *, char *);
Uint32 font_calc_height(Font *, char *);
void font_move_pen(Font *, Uint16, Uint16);
void font_set_pen(Font *, Uint16, Uint16);
void font_set_color(Font *, Uint8, Uint8, Uint8);
void font_set_alpha(Font *, Uint8);

#endif /* _FONT_H */
