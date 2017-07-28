/* tux.c - Copyright (C) 2001-2002 Jacob Kroon, see COPYING for details */

#include <stdlib.h>
#include <string.h>
#include "video.h"
#include "audio.h"
#include "tuxpuck.h"

/* externals */
extern unsigned char tux_png[];
extern unsigned char tux_eyes_spr[];
extern unsigned char tux_boos_ogg[];
extern unsigned char tux_apps_ogg[];

/* statics */
static char _name[] = "Tux";
static Uint8 _is_ready = 1;
static Sprite *_spr_eyes = NULL;
static Sound *_snd_boos = NULL, *_snd_apps = NULL;

/* functions */
static void _tux_free(AIPlayer * player)
{
  SDL_FreeSurface(player->sdl_image);
  sprite_free(_spr_eyes);
  audio_free_sound(_snd_apps);
  audio_free_sound(_snd_boos);
  free(player);
}

static void _tux_set_state(AIPlayer * player, Uint8 state)
{
  player->state = state;
  switch (state) {
  case PLAYER_STATE_WIN_POINT:
    audio_play_sound(_snd_apps);
    sprite_set_animation(_spr_eyes, 1);
    _is_ready = 0;
    break;
  case PLAYER_STATE_LOOSE_POINT:
    audio_play_sound(_snd_boos);
    sprite_set_animation(_spr_eyes, 1);
    _is_ready = 0;
    break;
  default:
    break;
  }
}

static Uint8 _tux_reblit(AIPlayer * player, Uint32 time)
{
  switch (player->state) {
  case PLAYER_STATE_WIN_POINT:
  case PLAYER_STATE_LOOSE_POINT:
    sprite_blit(_spr_eyes);
    if (sprite_update(_spr_eyes, time) == 0) {
      sprite_erase(_spr_eyes);
      player->state = PLAYER_STATE_IDLE;
      _is_ready = 1;
    }
    break;
  case PLAYER_STATE_WIN_GAME:
  case PLAYER_STATE_LOOSE_GAME:
    return 0;
  default:
    break;
  }
  return 1;
}

static Uint8 _ready(void)
{
  return _is_ready;
}

AIPlayer *tux_create(Pad * pad, Puck * puck)
{
  AIPlayer *player = NULL;

  player = malloc(sizeof(AIPlayer));
  memset(player, 0, sizeof(AIPlayer));
  player->sdl_image = video_create_png_surface(tux_png, NULL);
  player->rect.x = SCREEN_W / 2 - player->sdl_image->w / 2;
  player->rect.y = BOARD_TOP - player->sdl_image->h;
  player->rect.w = player->rect.h = 0;
  _spr_eyes = sprite_create(tux_eyes_spr, NULL);
  sprite_set_position(_spr_eyes, 292, 47);
  _snd_apps = audio_create_sound(tux_apps_ogg, NULL);
  _snd_boos = audio_create_sound(tux_boos_ogg, NULL);
  if (_snd_apps)
    audio_set_single(_snd_apps, 0);
  if (_snd_boos)
    audio_set_single(_snd_boos, 0);
  player->name = _name;
  player->pad = pad;
  player->puck = puck;
  player->speed = 3.0;
  player->hit_power = 2.5;
  player->state = PLAYER_STATE_IDLE;
  player->reblit = _tux_reblit;
  player->set_state = _tux_set_state;
  player->ready = _ready;
  player->free = _tux_free;
  player->strategy.idle = smart_idle;
  player->strategy.serve = dumb_serve;
  player->strategy.backup = dumb_backup;
  player->strategy.aim = dumb_aim;
  player->strategy.hit = dumb_hit;
  return player;
}
