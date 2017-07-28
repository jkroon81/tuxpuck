/* video.c - Copyright (C) 2001-2002 Jacob Kroon, see COPYING for details */

#include <stdlib.h>
#include <SDL_video.h>
#include <SDL_mouse.h>
#include <SDL_events.h>
#include <SDL_timer.h>
#include "video.h"
#include "tuxpuck.h"

/* defines */
#define MAX_RECTS	500

/* externals */
extern SDL_Surface *loadPNG(Uint8 *, Uint32 *);
extern SDL_Surface *loadJPG(Uint8 *, Uint32 *);
extern SDL_Surface *zoomSurface(SDL_Surface *, float);

/* statics */
static SDL_Surface *_screen = NULL, *_background = NULL;
static SDL_Rect *_dirty_rect = NULL;
static Uint32 _no_dirty_rects = 0;

/* functions */
int video_init(void)
{
  if ((_screen =
       SDL_SetVideoMode(SCREEN_W, SCREEN_H, 0,
			SDL_SWSURFACE | SDL_HWSURFACE)) < 0)
    return -1;
  SDL_WM_SetCaption("TuxPuck v" _VERSION, "TuxPuck v" _VERSION);
  SDL_ShowCursor(0);
  _dirty_rect = (SDL_Rect *) malloc(MAX_RECTS * sizeof(SDL_Rect));
  return 0;
}

void video_deinit(void)
{
  if (_background != NULL)
    SDL_FreeSurface(_background);
  free(_dirty_rect);
}

SDL_Surface *video_create_png_surface(Uint8 * data, Uint32 * memcounter)
{
  return loadPNG(data, memcounter);
}

SDL_Surface *video_create_jpg_surface(Uint8 * data, Uint32 * memcounter)
{
  return loadJPG(data, memcounter);
}

SDL_Surface *video_scale_surface(SDL_Surface * surface, float scale)
{
  return zoomSurface(surface, scale);
}

void video_update(void)
{
  SDL_UpdateRects(_screen, _no_dirty_rects, _dirty_rect);
  _no_dirty_rects = 0;
}

void video_erase(SDL_Rect * rect)
{
  SDL_BlitSurface(_background, rect, _screen, rect);
  _dirty_rect[_no_dirty_rects++] = *rect;
}

void video_set_alpha(SDL_Surface * surface, Uint8 alpha)
{
  if (alpha == 255)
    SDL_SetAlpha(surface, 0, 0);
  else
    SDL_SetAlpha(surface, SDL_SRCALPHA | SDL_RLEACCEL, alpha);
}

void video_fill(Uint32 color, Uint8 alpha, SDL_Rect * rect)
{
  SDL_Surface *fill = NULL;
  SDL_Rect rect2;

  if (rect == NULL) {
    rect2.x = rect2.y = 0;
    rect2.w = SCREEN_W;
    rect2.h = SCREEN_H;
    rect = &rect2;
  }
  fill =
    SDL_CreateRGBSurface(SDL_SWSURFACE, rect->w, rect->h, 16, 0, 0, 0, 0);
  SDL_FillRect(fill, NULL, color);
  video_set_alpha(fill, alpha);
  video_blit(fill, NULL, rect);
  SDL_FreeSurface(fill);
}

void video_blit(SDL_Surface * surface, SDL_Rect * src, SDL_Rect * dst)
{
  SDL_Rect own_rect;

  if (!dst) {
    own_rect.x = own_rect.y = 0;
    dst = &own_rect;
  }
  SDL_BlitSurface(surface, src, _screen, dst);
  _dirty_rect[_no_dirty_rects++] = *dst;
}

void video_restore(void)
{
  SDL_BlitSurface(_background, NULL, _screen, NULL);
  _dirty_rect[0].x = _dirty_rect[0].y = 0;
  _dirty_rect[0].w = SCREEN_W;
  _dirty_rect[0].h = SCREEN_H;
  _no_dirty_rects = 1;
}

void video_save(void)
{
  if (_background != NULL)
    SDL_FreeSurface(_background);
  _background =
    SDL_ConvertSurface(_screen, _screen->format,
		       SDL_SWSURFACE | SDL_HWSURFACE);
}

SDL_Surface *video_duplicate(void)
{
  SDL_Surface *screen = NULL;

  screen =
    SDL_ConvertSurface(_screen, _screen->format,
		       SDL_SWSURFACE | SDL_HWSURFACE);
  return screen;
}

void video_toggle_fullscreen(void)
{
  SDL_WM_ToggleFullScreen(_screen);
}

Uint32 video_map_rgb(Uint8 r, Uint8 g, Uint8 b)
{
  return SDL_MapRGB(_screen->format, r, g, b);
}

/* effects */

void video_box_up(SDL_Surface * surface, Uint32 time_limit)
{
  Uint8 loop = 1;
  Uint32 elapsed_time = 0;
  SDL_Event event;
  SDL_Rect rect = { SCREEN_W / 2 + 1, SCREEN_H / 2 + 1, 0, 0 };
  float time_ratio;
  Sint16 temp_x, temp_y;
  Timer *timer = NULL;

  timer = timer_create();
  timer_reset(timer);
  while (loop && elapsed_time < time_limit) {
    while (SDL_PollEvent(&event))
      if (event.type == SDL_KEYDOWN || event.type == SDL_MOUSEBUTTONDOWN)
	loop = 0;
    timer_update(timer);
    elapsed_time = timer_elapsed(timer);
    time_ratio = (float) elapsed_time / time_limit;
    temp_x = rect.x;
    temp_y = rect.y;
    rect.x = SCREEN_W / 2.0 * (1.0 - time_ratio) + 1;
    rect.w = temp_x - rect.x + 1;
    rect.y = SCREEN_H / 2.0 * (1.0 - time_ratio) + 1;
    rect.h = time_ratio * SCREEN_H;
    video_blit(surface, &rect, &rect);
    rect.x = SCREEN_W - rect.x;
    video_blit(surface, &rect, &rect);
    rect.x = SCREEN_W - rect.x;
    rect.w = time_ratio * SCREEN_W;
    rect.h = temp_y - rect.y + 1;
    video_blit(surface, &rect, &rect);
    rect.y = SCREEN_H - rect.y;
    rect.w += temp_x - rect.x;
    video_blit(surface, &rect, &rect);
    rect.y = SCREEN_H - rect.y;
    SDL_Delay(SLEEP);
    video_update();
  }
  video_blit(surface, NULL, NULL);
  video_update();
  timer_free(timer);
}

void video_fade(SDL_Surface * to, Uint32 time_limit)
{
  SDL_Event event;
  Timer *timer = NULL;
  float alpha = 0.0;
  Uint32 elapsed_time = 0;
  Uint8 loop = 1;

  timer = timer_create();
  timer_reset(timer);
  while (loop) {
    while (SDL_PollEvent(&event))
      if (event.type == SDL_KEYDOWN || event.type == SDL_MOUSEBUTTONDOWN)
	loop = 0;
    timer_update(timer);
    elapsed_time = timer_elapsed(timer);
    alpha = (float) elapsed_time / time_limit;
    if (alpha > 1.0) {
      alpha = 1.0;
      loop = 0;
    }
    video_set_alpha(to, (Uint8) (alpha * 255));
    video_restore();
    video_blit(to, NULL, NULL);
    video_update();
    SDL_Delay(SLEEP);
  }
  timer_free(timer);
  video_set_alpha(to, 255);
  video_blit(to, NULL, NULL);
  video_update();
}
