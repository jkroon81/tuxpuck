/* video.h - Copyright (C) 2001-2002 Jacob Kroon, see COPYING for details */

#ifndef _VIDEO_H
#define _VIDEO_H

/* includes */
#include <SDL_video.h>

/* defines */
#define SCREEN_W	((Uint32)640)
#define SCREEN_H	((Uint32)480)

/* functions */
int video_init(void);
void video_deinit(void);
SDL_Surface *video_create_png_surface(Uint8 *, Uint32 *);
SDL_Surface *video_create_jpg_surface(Uint8 *, Uint32 *);
SDL_Surface *video_scale_surface(SDL_Surface *, float);
SDL_Surface *video_duplicate(void);
void video_blit(SDL_Surface *, SDL_Rect *, SDL_Rect *);
void video_set_alpha(SDL_Surface *, Uint8);
void video_fill(Uint32, Uint8, SDL_Rect *);
void video_erase(SDL_Rect *);
void video_save(void);
void video_update(void);
void video_restore(void);
void video_toggle_fullscreen(void);
Uint32 video_map_rgb(Uint8, Uint8, Uint8);
/* effects */
void video_box_up(SDL_Surface *, Uint32);
void video_fade(SDL_Surface *, Uint32);

#endif /* _VIDEO_H */
