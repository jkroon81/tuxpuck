/* player.c - Copyright (C) 2001-2002 Jacob Kroon, see COPYING for details */

#include <stdlib.h>
#include <math.h>
#include <SDL_mouse.h>
#include "video.h"
#include "tuxpuck.h"

/* defines */
#define MIN_MOUSE_SPEED		((float)0.05)
#define MAX_MOUSE_SPEED		((float)0.50)

/* structs */
struct _HumanPlayer {
  Uint8 points;
  char *name;
  Pad *pad;
  float speed;
};

/* functions */
static float _mod(float a, float b)
{
  return (a - floor(a / b) * b);
}

/* It is not absolutely exactly (+- 1 bounce difference),
 * but I think it needn't be.
 * -- Ernst Moritz Hahn
 *
 * This function is temporarly commented since it isnt used (yet),
 * and thus gives a warning when compiling.
 * -- Jacob Kroon

static float _calc_bounces(float z, float dx, float dz)
{
  float n, x;

  if(dz==0) return 0;
  n = (35 - z) / dz;
  x = n * dx;
  return fabs(x / 40);
}
*/

static float _calc_puck_x(float x, float z, float dx, float dz, float ax)
{
  float board_w;
  float raw_x, rx, g, h, n;

  board_w = 40 - PUCK_W;
  n = (ax - z) / dz;
  raw_x = n * dx + x + board_w / 2;
  rx = _mod(raw_x, board_w);
  g = raw_x / board_w;
  h = fabs(_mod(g, 2));
  if ((h > 1))
    rx = board_w - rx;
  rx = rx - board_w / 2;
  return rx;
}

static void _dumb_really_idle(AIPlayer * player, Uint32 time)
{
  static float counter = M_PI_2;
  float dx, dz, puck_x, puck_z, pad_x, pad_z;

  counter += 0.005 * time;
  entity_get_position(player->pad, &pad_x, &pad_z);
  entity_get_position((Entity *) player->puck, &puck_x, &puck_z);
  dx = (sin(counter / 2.0) * 0.03 + (puck_x - pad_x) * 0.005) * player->speed;
  dz = (sin(counter / 4.0) * 0.01 + (35 - pad_z) * 0.005) * player->speed;
  entity_set_velocity(player->pad, dx, dz);
}

static void _smart_really_idle(AIPlayer * player, Uint32 time)
{
  static float counter = M_PI_2;
  float dx, dz, puck_x, puck_z, puck_dx, puck_dz, pad_x, pad_z, x;

  counter += 0.005 * time;
  entity_get_position(player->pad, &pad_x, &pad_z);
  entity_get_position((Entity *) player->puck, &puck_x, &puck_z);
  entity_get_velocity((Entity *) player->puck, &puck_dx, &puck_dz);
  if (puck_dz != 0) {
    x = _calc_puck_x(puck_x, puck_z, puck_dx, puck_dz, 35);
    dx =
      (sin(counter / 2.0) * 0.03 + (puck_x - pad_x) * 0.005) * player->speed;
    dz = (sin(counter / 4.0) * 0.01 + (40 - pad_z) * 0.005) * player->speed;
    if (puck_dz > 0)
      dx = ((x - pad_x) * 0.005) * player->speed;
    else
      dx = (0 - pad_x) * 0.005;
    dz = ((35 - pad_z) * 0.005) * player->speed;
    entity_set_velocity(player->pad, dx, dz);
  }
}

HumanPlayer *human_create(Pad * pad, char *name)
{
  HumanPlayer *human = NULL;

  human = malloc(sizeof(HumanPlayer));
  human->name = name;
  human->pad = pad;
  human->points = 0;
  human->speed = (MAX_MOUSE_SPEED + MIN_MOUSE_SPEED) / 2.0;
  return human;
}

void human_give_point(HumanPlayer * human)
{
  human->points++;
}

Uint8 human_get_points(HumanPlayer * human)
{
  return human->points;
}

void human_free(HumanPlayer * human)
{
  free(human);
}

void human_set_speed(HumanPlayer * human, Uint8 speed)
{
  human->speed =
    MIN_MOUSE_SPEED + (MAX_MOUSE_SPEED - MIN_MOUSE_SPEED) * speed / 10.0;
}

void human_update(HumanPlayer * human, Uint32 time)
{
  int dx, dy;

  SDL_GetRelativeMouseState(&dx, &dy);
  if (time != 0)
    entity_set_velocity(human->pad, (float) dx / time * human->speed,
			(float) -dy / time * human->speed);
  else
    entity_set_velocity(human->pad, 0, 0);
}

void aiplayer_blit(AIPlayer * player)
{
  video_blit(player->sdl_image, NULL, &player->rect);
}

void aiplayer_erase(AIPlayer * player)
{
  video_erase(&player->rect);
}

void aiplayer_set_alpha(AIPlayer * player, Uint8 alpha)
{
  video_set_alpha(player->sdl_image, alpha);
}

void aiplayer_update(AIPlayer * player, Uint32 time)
{
  switch (player->state) {
  case PLAYER_STATE_IDLE:
    player->strategy.idle(player, time);
    break;
  case PLAYER_STATE_SERVE:
    player->strategy.serve(player, time);
    break;
  case PLAYER_STATE_BACKUP:
    player->strategy.backup(player, time);
    break;
  case PLAYER_STATE_AIM:
    player->strategy.aim(player, time);
    break;
  case PLAYER_STATE_HIT:
    player->strategy.hit(player, time);
    break;
  case PLAYER_STATE_WIN_POINT:
  case PLAYER_STATE_LOOSE_POINT:
    _dumb_really_idle(player, time);
    if (player->ready() && board_get_state() == BOARD_STATE_PLAY) {
      if (board_get_turn() == 2)
	player->set_state(player, PLAYER_STATE_SERVE);
      else
	player->set_state(player, PLAYER_STATE_IDLE);
    }
    break;
  default:
    _dumb_really_idle(player, time);
    break;
  }
}

void dumb_idle(AIPlayer * player, Uint32 time)
{
  float puck_z;

  _dumb_really_idle(player, time);
  entity_get_position((Entity *) player->puck, NULL, &puck_z);
  if (puck_z > 20)
    player->set_state(player, PLAYER_STATE_BACKUP);
}

void dumb_serve(AIPlayer * player, Uint32 time)
{
  player->set_state(player, PLAYER_STATE_BACKUP);
}

void dumb_backup(AIPlayer * player, Uint32 time)
{
  float pad_x;

  entity_get_position(player->pad, &pad_x, NULL);
  if (entity_move_towards
      (player->pad, pad_x, 38, 0.03 * player->speed, time) == 0)
    player->set_state(player, PLAYER_STATE_AIM);
}

void dumb_aim(AIPlayer * player, Uint32 time)
{
  float hit_speed, dx, dz, puck_x, puck_z, pad_x, pad_z;

  entity_get_position(player->pad, &pad_x, &pad_z);
  entity_get_position((Entity *) player->puck, &puck_x, &puck_z);
  dx = (puck_x - pad_x);
  dz = (puck_z - pad_z);
  if (dz > -0.4) {
    player->set_state(player, PLAYER_STATE_BACKUP);
    return;
  }
  hit_speed = sqrt(dx * dx + dz * dz);
  dx = dx / hit_speed * player->hit_power * 0.1;
  dz = dz / hit_speed * player->hit_power * 0.1;
  player->set_state(player, PLAYER_STATE_HIT);
  entity_set_velocity(player->pad, dx, dz);
}

void dumb_hit(AIPlayer * player, Uint32 time)
{
  float pad_z;

  entity_get_position(player->pad, NULL, &pad_z);
  if (pad_z < 21)
    player->set_state(player, PLAYER_STATE_IDLE);
}

void smart_idle(AIPlayer * player, Uint32 time)
{
  float puck_z;

  _smart_really_idle(player, time);
  entity_get_position((Entity *) player->puck, NULL, &puck_z);
  if (puck_z > 20)
    player->set_state(player, PLAYER_STATE_BACKUP);
}

void smart_backup(AIPlayer * player, Uint32 time)
{
  float pad_x, puck_x, puck_z, puck_dx, puck_dz, x;

  entity_get_position(player->pad, &pad_x, NULL);
  entity_get_position((Entity *) player->puck, &puck_x, &puck_z);
  entity_get_velocity((Entity *) player->puck, &puck_dx, &puck_dz);

  if (fabs(puck_dx) > 0.15) {
    x = _calc_puck_x(puck_x, puck_z, puck_dx, puck_dz, 35);
    entity_move_towards(player->pad, x, 35, 0.03 * player->speed, time);
    player->set_state(player, PLAYER_STATE_BACKUP);
  } else {
    if (entity_move_towards
	(player->pad, pad_x, 38, 0.03 * player->speed, time) == 0) {
      player->set_state(player, PLAYER_STATE_AIM);
    }
  }
}
