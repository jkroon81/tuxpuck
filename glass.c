/* glass.c - Copyright (C) 2001-2002 Jacob Kroon, see COPYING for details */

#include <stdlib.h>
#include <string.h>
#include "video.h"
#include "audio.h"
#include "tuxpuck.h"

/* defines */
#define GRAVITY			((float)-0.001)

/* externals */
extern unsigned char glass_png[];
extern unsigned char piece1_png[];
extern unsigned char piece2_png[];
extern unsigned char piece3_png[];
extern unsigned char piece4_png[];
extern unsigned char piece5_png[];
extern unsigned char crash_ogg[];

/* structs */
typedef struct {
  float x, y, z, dx, dy;
  Uint32 id;
  SDL_Surface *sdl_image;
  SDL_Rect rect;
} Piece;

/* statics */
static float _x, _z;
static SDL_Surface *_sdl_glass = NULL;
static SDL_Rect _rect;
static Piece *_piece[5];
static Sound *_snd_crash = NULL;

/* functions */
static Piece *_piece_create(Uint32 id)
{
  Piece *piece = NULL;
  SDL_Surface *image = NULL;

  switch (id) {
  case 0:
    image = video_create_png_surface(piece1_png, NULL);
    break;
  case 1:
    image = video_create_png_surface(piece2_png, NULL);
    break;
  case 2:
    image = video_create_png_surface(piece3_png, NULL);
    break;
  case 3:
    image = video_create_png_surface(piece4_png, NULL);
    break;
  case 4:
    image = video_create_png_surface(piece5_png, NULL);
    break;
  default:
    break;
  }
  if (id == 1)
    SDL_SetColorKey(image, SDL_SRCCOLORKEY, 1);
  piece = malloc(sizeof(Piece));
  memset(piece, 0, sizeof(Piece));
  piece->sdl_image = image;
  piece->id = id;
  return piece;
}

static void _piece_free(Piece * piece)
{
  SDL_FreeSurface(piece->sdl_image);
  free(piece);
}

static void _piece_erase(Piece * piece)
{
  video_erase(&piece->rect);
}

static void _piece_blit(Piece * piece)
{
  SDL_Surface *scaled_surface = NULL;
  float scale, y;

  scale = board_calc_scale(piece->z);
  y = board_calc_y(piece->z) - piece->y * scale;
  scaled_surface = video_scale_surface(piece->sdl_image, scale);
  piece->rect.x = (Sint16) (SCREEN_W / 2 + scale * piece->x * 12.7);
  piece->rect.y = (Sint16) (y - scaled_surface->h);
  video_blit(scaled_surface, NULL, &piece->rect);
  SDL_FreeSurface(scaled_surface);
}

static Uint8 _piece_update(Piece * piece, Uint32 time)
{
  piece->dy += GRAVITY * time;
  piece->x += piece->dx * time;
  piece->y += piece->dy * time;
  if (piece->y < -10)
    return 0;
  return 1;
}

static void _piece_reset(Piece * piece)
{
  float x, z, dx;

  entity_get_velocity((Entity *) board_get_puck(), &dx, NULL);
  glass_get_position(&x, &z);
  switch (piece->id) {
  case 0:
    piece->x = x - 2.5;
    piece->y = 0;
    piece->z = z;
    piece->dx = -0.01;
    piece->dy = 0.25;
    break;
  case 1:
    piece->x = x - 1.9;
    piece->y = 0.2;
    piece->z = z;
    piece->dx = -0.008;
    piece->dy = 0.28;
    break;
  case 2:
    piece->x = x + 0.5;
    piece->y = 0;
    piece->z = z;
    piece->dx = 0.001;
    piece->dy = 0.3;
    break;
  case 3:
    piece->x = x + 1.0;
    piece->y = 0.7;
    piece->z = z;
    piece->dx = 0.007;
    piece->dy = 0.27;
    break;
  case 4:
    piece->x = x + 1.1;
    piece->y = 0.1;
    piece->z = z;
    piece->dx = 0.012;
    piece->dy = 0.26;
    break;
  default:
    break;
  }
  piece->dx += dx * 0.1;
}

void glass_init(void)
{
  Uint8 i;

  _x = _z = 0;
  _sdl_glass = video_create_png_surface(glass_png, NULL);
  for (i = 0; i < 5; i++)
    _piece[i] = _piece_create(i);
  memset(&_rect, 0, sizeof(SDL_Rect));
  _snd_crash = audio_create_sound(crash_ogg, NULL);
}

void glass_deinit(void)
{
  Uint8 i = 0;

  SDL_FreeSurface(_sdl_glass);
  for (i = 0; i < 5; i++)
    _piece_free(_piece[i]);
  audio_free_sound(_snd_crash);
}

void glass_smash(float x, float z)
{
  Uint8 i;

  _x = x - 1.2;
  if (z < 0)
    _z = -40;
  else
    _z = 40;
  for (i = 0; i < 5; i++)
    _piece_reset(_piece[i]);
  audio_play_sound(_snd_crash);
}

Uint8 glass_update(Uint32 time)
{
  Uint8 done = 1, i;

  for (i = 0; i < 5; i++)
    if (_piece_update(_piece[i], time) != 0)
      done = 0;
  return (!done);
}

void glass_get_position(float *x, float *z)
{
  if (x)
    *x = _x;
  if (z)
    *z = _z;
}

void glass_erase(void)
{
  Uint8 i;

  video_erase(&_rect);
  for (i = 0; i < 5; i++)
    _piece_erase(_piece[i]);
}

void glass_blit(void)
{
  SDL_Surface *scaled_surface = NULL;
  float scale, y;
  Uint8 i;

  scale = board_calc_scale(_z);
  y = board_calc_y(_z);
  scaled_surface = video_scale_surface(_sdl_glass, scale);
  _rect.x =
    (Sint16) (SCREEN_W / 2 - scaled_surface->w / 2 + scale * _x * 12.7);
  _rect.y = (Sint16) (y - scaled_surface->h);
  video_blit(scaled_surface, NULL, &_rect);
  SDL_FreeSurface(scaled_surface);
  for (i = 0; i < 5; i++)
    _piece_blit(_piece[i]);
}
