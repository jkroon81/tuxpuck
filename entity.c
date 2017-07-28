/* entity.c - Copyright (C) 2001-2002 Jacob Kroon, see COPYING for details */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "video.h"
#include "audio.h"
#include "tuxpuck.h"

/* defines */
#define ABS(x)			((x)<0 ? -(x) : (x))
#define MAX_PUCK_SPEED		((float)0.20)
#define THRESHOLD		((float)0.02)
#define HIT_FRICTION		((float)0.60)

/* externals */
extern unsigned char pad_png[];
extern unsigned char puck_png[];
extern unsigned char nock_ogg[];

/* structs */
typedef struct _MoveArea MoveArea;

struct _MoveArea {
  float x1, x2, z1, z2;
};

struct _Entity {
  Uint8 alpha;
  SDL_Rect rect;
  SDL_Surface *sdl_image;
  MoveArea move_area;
  float x, z, dx, dz, w, h, y_scale;
};

struct _Puck {
  Entity entity;
  Uint8 state;
  Sound *snd_nock;
};

/* functions */
static Entity *_entity_create(void)
{
  Entity *e = NULL;
  MoveArea move_area = { -20, 20, -42, 42 };

  e = (Entity *) malloc(sizeof(Entity));
  memset(e, 0, sizeof(Entity));
  e->move_area = move_area;
  return e;
}

static void _entity_free(Entity * e)
{
  free(e);
}

void entity_move(Entity * e, Uint32 time)
{
  MoveArea *move_area;
  float *x, *z, *w, *h, *dx, *dz;

  move_area = &e->move_area;
  x = &e->x;
  z = &e->z;
  w = &e->w;
  h = &e->h;
  dx = &e->dx;
  dz = &e->dz;

  *x += *dx * time;
  *z += *dz * time;
  if (*x < move_area->x1 + *w / 2) {
    *x = move_area->x1 + *w / 2;
    *dx = -*dx;
  } else if (*x > move_area->x2 - *w / 2) {
    *x = move_area->x2 - *w / 2;
    *dx = -*dx;
  }
  if (*z < move_area->z1 + *h / 2) {
    *z = move_area->z1 + *h / 2;
    *dz = -*dz;
  } else if (*z > move_area->z2 - *h / 2) {
    *z = move_area->z2 - *h / 2;
    *dz = -*dz;
  }
}

void entity_blit(Entity * e)
{
  SDL_Surface *scaled_surface = NULL;
  float scale;

  scale = board_calc_scale(e->z);
  scaled_surface = video_scale_surface(e->sdl_image, scale);
  e->rect.x =
    (Sint16) (SCREEN_W / 2 - scaled_surface->w / 2 + scale * e->x * 12.7);
  e->rect.y = (Sint16) (board_calc_y(e->z) - scaled_surface->h * e->y_scale);
  video_set_alpha(scaled_surface, e->alpha);
  video_blit(scaled_surface, NULL, &e->rect);
  SDL_FreeSurface(scaled_surface);
}

void entity_set_alpha(Entity * e, Uint8 alpha)
{
  e->alpha = alpha;
}

void entity_set_position(Entity * e, float x, float z)
{
  e->x = x;
  e->z = z;
}

void entity_get_position(Entity * e, float *x, float *z)
{
  if (x)
    *x = e->x;
  if (z)
    *z = e->z;
}

void entity_set_velocity(Entity * e, float dx, float dz)
{
  e->dx = dx;
  e->dz = dz;
}

void entity_get_velocity(Entity * e, float *dx, float *dz)
{
  if (dx)
    *dx = e->dx;
  if (dz)
    *dz = e->dz;
}

void entity_erase(Entity * e)
{
  video_erase(&e->rect);
}

Uint8 entity_move_towards(Entity * e, float x, float z, float speed,
			  Uint32 time)
{
  float dx = 0, dz = 0;

  if (x > e->x) {
    dx = speed;
    if (x < e->x + dx * time) {
      e->x = x;
      dx = 0;
    }
  } else if (x < e->x) {
    dx = -speed;
    if (x > e->x + dx * time) {
      e->x = x;
      dx = 0;
    }
  }
  if (z > e->z) {
    dz = speed;
    if (z < e->z + dz * time) {
      e->z = z;
      dz = 0;
    }
  } else if (z < e->z) {
    dz = -speed;
    if (z > e->z + dz * time) {
      e->z = z;
      dz = 0;
    }
  }
  e->dx = dx;
  e->dz = dz;
  entity_move(e, time);
  if (e->x == x && e->z == z)
    return 0;
  return 1;
}

Pad *pad_create(Uint8 place)
{
  Pad *pad = NULL;

  pad = _entity_create();
  pad->w = PAD_W;
  pad->h = PAD_H;
  pad->y_scale = 1.0;
  pad->sdl_image = video_create_png_surface(pad_png, NULL);
  SDL_SetColorKey(pad->sdl_image, SDL_SRCCOLORKEY, 2);
  if (place == 1) {
    pad->move_area.z1 = -40;
    pad->move_area.z2 = -20.8;
    pad->z = -36;
  } else {
    pad->move_area.z1 = 20.8;
    pad->move_area.z2 = 40;
    pad->z = 36;
  }
  pad->alpha = 128;
  return pad;
}

void pad_free(Pad * pad)
{
  SDL_FreeSurface(pad->sdl_image);
  free(pad);
}

Puck *puck_create(void)
{
  Puck *puck = NULL;
  Entity *e = NULL;

  puck = malloc(sizeof(Puck));
  e = _entity_create();
  e->w = PUCK_W;
  e->h = PUCK_W;
  e->y_scale = 0.8;
  e->sdl_image = video_create_png_surface(puck_png, NULL);
  memcpy(&puck->entity, e, sizeof(Entity));
  _entity_free(e);
  puck->state = PUCK_STATE_DEAD;
  puck->snd_nock = audio_create_sound(nock_ogg, NULL);
  return puck;
}

void puck_free(Puck * puck)
{
  SDL_FreeSurface(puck->entity.sdl_image);
  audio_free_sound(puck->snd_nock);
  free(puck);
}

void puck_set_state(Puck * puck, Uint8 state)
{
  puck->state = state;
}

int puck_will_get_hit_by(Puck * puck, Pad * pad, Uint32 time)
{
  float pad_next_x, pad_next_z, puck_next_x, puck_next_z;
  Entity *e = NULL;

  e = &puck->entity;
  pad_next_x = pad->x + pad->dx * time;
  pad_next_z = pad->z + pad->dz * time;
  puck_next_x = e->x + e->dx * time;
  puck_next_z = e->z + e->dz * time;
  if (pad->move_area.z1 < 0) {
    if (pad->move_area.z2 < puck_next_z - PUCK_W / 2)
      return 0;
  } else if (pad->move_area.z1 > puck_next_z + PUCK_W / 2)
    return 0;
  if (ABS(pad_next_x - puck_next_x) > PAD_W / 2 + PUCK_W / 2)
    return 0;
  if (pad->z < e->z) {
    if (pad_next_z + PAD_H / 2 > puck_next_z - PUCK_W / 2)
      return 1;
    else
      return 0;
  } else {
    if (pad_next_z - PAD_H / 2 < puck_next_z + PUCK_W / 2)
      return 1;
    else
      return 0;
  }
}

void puck_move(Puck * puck, Uint32 time)
{
  Entity *e = NULL;

  e = &puck->entity;
  if (e->x + e->dx * time < e->move_area.x1 + e->w / 2
      || e->x + e->dx * time > e->move_area.x2 - e->w / 2)
    audio_play_sound(puck->snd_nock);
  entity_move(e, time);
}

int puck_is_dead(Puck * puck)
{
  if (puck->state == PUCK_STATE_DEAD)
    return 1;
  else
    return 0;
}

void puck_get_hit_by(Puck * puck, Pad * pad, Uint32 time)
{
  float x, z, dx, dz, speed;
  Entity *e = NULL;

  e = &puck->entity;
  x = pad->x;
  z = pad->z;
  dx = pad->dx;
  dz = pad->dz;
  e->dx += dx * HIT_FRICTION;
  e->dx *= HIT_FRICTION;
  if (z < e->z) {
    if (e->dz < 0)
      e->dz = -e->dz;
    e->z = z + dz * time + PUCK_W / 2 + PAD_H / 2 + THRESHOLD;
  } else {
    if (e->dz > 0)
      e->dz = -e->dz;
    e->z = z + dz * time - PUCK_W / 2 - PAD_H / 2 - THRESHOLD;
  }
  e->dz += dz * HIT_FRICTION;
  e->dz *= HIT_FRICTION;
  if (e->dx * e->dx + e->dz * e->dz > MAX_PUCK_SPEED * MAX_PUCK_SPEED) {
    speed = sqrt(e->dx * e->dx + e->dz * e->dz);
    e->dx = e->dx / speed * MAX_PUCK_SPEED;
    e->dz = e->dz / speed * MAX_PUCK_SPEED;
  }
  audio_play_sound(puck->snd_nock);
}
