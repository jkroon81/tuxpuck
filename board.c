/* board.c - Copyright (C) 2001-2002 Jacob Kroon, see COPYING for details */

#include "video.h"
#include "tuxpuck.h"

/* externals */
extern unsigned char board_jpg[];

/* statics */
static SDL_Surface *_sdl_board;
static Uint8 _state = 0, _turn = 0;
static float _coeff[4];
static Pad *_pad_1, *_pad_2;
static Puck *_puck;
static SDL_Rect _rect = { 0, 0, 0, 0 };

/* functions */
static Uint8 _get_scorer(void)
{
  float puck_z;

  if (puck_is_dead(_puck))
    return 0;
  entity_get_position((Entity *) _puck, NULL, &puck_z);
  if (puck_z > 40)
    return 1;
  else if (puck_z < -40)
    return 2;
  return 0;
}

void board_init(void)
{
  _state = BOARD_STATE_NEW_PUCK;
  _sdl_board = video_create_jpg_surface(board_jpg, NULL);
  _rect.x = SCREEN_W / 2 - _sdl_board->w / 2;
  _rect.y = SCREEN_H - _sdl_board->h;
  _rect.w = _rect.h = 0;
  _pad_1 = pad_create(1);
  _pad_2 = pad_create(2);
  _puck = puck_create();
  _turn = 1;
  glass_init();
  _coeff[0] = 80 * (BOARD_BOTTOM * BOARD_TOP) / (BOARD_BOTTOM - BOARD_TOP);
  _coeff[1] = 40 * (BOARD_BOTTOM + BOARD_TOP) / (BOARD_BOTTOM - BOARD_TOP);
  _coeff[2] =
    80 * (1.0 * BOARD_LEFT / BOARD_RIGHT) / (1.0 - BOARD_LEFT / BOARD_RIGHT);
  _coeff[3] =
    40 * (1.0 + BOARD_LEFT / BOARD_RIGHT) / (1.0 - BOARD_LEFT / BOARD_RIGHT);
  video_fade(_sdl_board, 1000);
}

void board_deinit(void)
{
  SDL_FreeSurface(_sdl_board);
  pad_free(_pad_1);
  pad_free(_pad_2);
  puck_free(_puck);
  glass_deinit();
}

Pad *board_get_pad(Uint8 who)
{
  if (who == 1)
    return _pad_1;
  else if (who == 2)
    return _pad_2;
  else
    return NULL;
}

Puck *board_get_puck(void)
{
  return _puck;
}

void board_blit(void)
{
  video_blit(_sdl_board, NULL, &_rect);
}

Uint8 board_update(Uint32 time)
{
  Uint8 scorer;
  float puck_x, puck_z;

  switch (_state) {
  case BOARD_STATE_PLAY:
    if (puck_will_get_hit_by(_puck, _pad_1, time))
      puck_get_hit_by(_puck, _pad_1, time);
    else if (puck_will_get_hit_by(_puck, _pad_2, time))
      puck_get_hit_by(_puck, _pad_2, time);
    puck_move(_puck, time);
    break;
  case BOARD_STATE_CRASH_GLASS:
    if (glass_update(time) == 0) {
      glass_erase();
      _state = BOARD_STATE_NEW_PUCK;
      if (_turn == 1)
	_turn = 2;
      else
	_turn = 1;
    }
    break;
  case BOARD_STATE_NEW_PUCK:
    if (_turn == 1) {
      if (entity_move_towards((Entity *) _puck, 0, -21, 0.05, time) == 0) {
	_state = BOARD_STATE_PLAY;
	entity_set_velocity((Entity *) _puck, 0, 0);
	puck_set_state(_puck, PUCK_STATE_ALIVE);
      }
    } else {
      if (entity_move_towards((Entity *) _puck, 0, 21, 0.05, time) == 0) {
	_state = BOARD_STATE_PLAY;
	entity_set_velocity((Entity *) _puck, 0, 0);
	puck_set_state(_puck, PUCK_STATE_ALIVE);
      }
    }
    break;
  default:
    break;
  }
  if ((scorer = _get_scorer()) != 0) {
    _state = BOARD_STATE_CRASH_GLASS;
    entity_get_position((Entity *) _puck, &puck_x, &puck_z);
    glass_smash(puck_x, puck_z);
    puck_set_state(_puck, PUCK_STATE_DEAD);
  }
  entity_move(_pad_1, time);
  entity_move(_pad_2, time);
  return scorer;
}

void board_clean_up(void)
{
  entity_erase(_pad_1);
  entity_erase(_pad_2);
  entity_erase((Entity *) _puck);
  if (_state == BOARD_STATE_CRASH_GLASS)
    glass_erase();
}

void board_reblit(void)
{
  float p1, p2, p, g = 0;

  entity_get_position(_pad_1, NULL, &p1);
  entity_get_position(_pad_2, NULL, &p2);
  entity_get_position((Entity *) _puck, NULL, &p);
  if (_state == BOARD_STATE_CRASH_GLASS) {
    glass_get_position(NULL, &g);
    if (g > 0)
      glass_blit();
  }
  if (p < p1) {
    entity_blit(_pad_2);
    entity_blit(_pad_1);
    entity_blit((Entity *) _puck);
  } else if (p < p2) {
    entity_blit(_pad_2);
    entity_blit((Entity *) _puck);
    entity_blit(_pad_1);
  } else {
    entity_blit((Entity *) _puck);
    entity_blit(_pad_2);
    entity_blit(_pad_1);
  }
  if (_state == BOARD_STATE_CRASH_GLASS) {
    if (g < 0)
      glass_blit();
  }
}

Uint8 board_get_state(void)
{
  return _state;
}

Uint8 board_get_turn(void)
{
  return _turn;
}

float board_calc_y(float y)
{
  return (_coeff[0] / (_coeff[1] + y));
}

float board_calc_scale(float y)
{
  return _coeff[2] / (_coeff[3] + y);
}
