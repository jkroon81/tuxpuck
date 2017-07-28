/* tuxpuck.c - Copyright (C) 2001-2002 Jacob Kroon, see COPYING for details */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <SDL.h>
#include "video.h"
#include "audio.h"
#include "tuxpuck.h"

/* defines */
#ifdef windows
#define SETTINGSFILE "tuxpuck.ini"
#else
#define SETTINGSFILE _settings_file
#endif

/* externals */
extern time_t time(time_t *);
extern void run_intro(void);
extern AIPlayer *tux_create(Pad *, Puck *);
extern AIPlayer *arcana_create(Pad *, Puck *);

/* structs */
typedef struct {
  Uint8 sound;
  Uint8 fullscreen;
  Uint8 mouse_speed;
} Settings;

/* statics */
static Settings *_settings = NULL;
#ifndef windows
static char _settings_file[200];
#endif

/* functions */
static int _play_match(Uint8 opponent)
{
  int next_opponent;
  SDL_Event event;
  Uint8 loop = 1, scorer = 0;
  Uint32 elapsed_time = 0;
  char buffer[50];
  HumanPlayer *p1 = NULL;
  AIPlayer *p2 = NULL;
  Timer *timer = NULL;
  float alpha = 0.0;
  Menu *exit_menu;
  Menu *again_menu;

  memset(buffer, 0, 50);
  board_init();
  scoreboard_init();
  video_save();
  p1 = human_create(board_get_pad(1), "Human");
  switch (opponent) {
  case 1:
    p2 = tux_create(board_get_pad(2), board_get_puck());
    break;
  case 2:
    p2 = arcana_create(board_get_pad(2), board_get_puck());
    break;
  }
  exit_menu = menu_create(2);
  menu_add_field(exit_menu, 0, 1, "Continue");
  menu_add_field(exit_menu, 1, 1, "Surrender");
  again_menu = menu_create(2);
  menu_add_field(again_menu, 0, 1, "Play Again");
  menu_add_field(again_menu, 1, 1, "Main Menu");
  timer = timer_create();
  timer_reset(timer);
  while (loop) {
    while (SDL_PollEvent(&event))
      if (event.type == SDL_MOUSEBUTTONDOWN) {
	loop = 0;
	alpha = 1.0;
      }
    SDL_Delay(SLEEP);
    timer_update(timer);
    timer_reset(timer);
    elapsed_time = timer_elapsed(timer);
    alpha += elapsed_time * 0.001;
    if (alpha > 1.0) {
      loop = 0;
      alpha = 1.0;
    }
    board_clean_up();
    scoreboard_erase();
    aiplayer_erase(p2);
    aiplayer_set_alpha(p2, (Uint8) (alpha * 255));
    scoreboard_set_alpha((Uint8) (alpha * 255));
    entity_set_alpha((Entity *) board_get_puck(), (Uint8) (alpha * 255));
    entity_set_alpha(board_get_pad(1), (Uint8) (alpha * 255.0 / 2.0));
    entity_set_alpha(board_get_pad(2), (Uint8) (alpha * 255.0 / 2.0));
    aiplayer_blit(p2);
    board_reblit();
    scoreboard_blit();
    video_update();
  }
  loop = 1;
  board_clean_up();
  aiplayer_blit(p2);
  video_save();
  board_reblit();
  video_update();
  SDL_PumpEvents();
  SDL_GetRelativeMouseState(NULL, NULL);
#ifndef _DEBUG
  SDL_WM_GrabInput(SDL_GRAB_ON);
#endif
  human_set_speed(p1, _settings->mouse_speed);
  timer_reset(timer);
  while (loop) {
    while (SDL_PollEvent(&event))
      switch (event.type) {
      case SDL_KEYDOWN:
	switch (event.key.keysym.sym) {
	case SDLK_ESCAPE:
	  if (menu_get_selected(exit_menu) == 1)
	    loop = 0;
	  timer_reset(timer);
	  break;
	case SDLK_F1:
	  _settings->sound = !_settings->sound;
	  audio_set_mute(!_settings->sound);
	  break;
	case SDLK_F5:
	  if (_settings->mouse_speed > 1)
	    _settings->mouse_speed--;
	  human_set_speed(p1, _settings->mouse_speed);
	  scoreboard_set_mousebar(_settings->mouse_speed);
	  break;
	case SDLK_F6:
	  if (_settings->mouse_speed < 10)
	    _settings->mouse_speed++;
	  human_set_speed(p1, _settings->mouse_speed);
	  scoreboard_set_mousebar(_settings->mouse_speed);
	  break;
	case SDLK_f:
	  _settings->fullscreen = !_settings->fullscreen;
	  video_toggle_fullscreen();
	  break;
	default:
	  break;
	}
	break;
      case SDL_QUIT:
	loop = 0;
	break;
      }
    SDL_Delay(SLEEP);
    timer_update(timer);
    timer_reset(timer);
    elapsed_time = timer_elapsed(timer);
    human_update(p1, elapsed_time);
    aiplayer_update(p2, elapsed_time);
    scoreboard_update(elapsed_time);
    if ((scorer = board_update(elapsed_time)) != 0) {
      scoreboard_add_point(scorer);
      if (scorer == 1) {
	human_give_point(p1);
	p2->set_state(p2, PLAYER_STATE_LOOSE_POINT);
      } else {
	p2->points++;
	p2->set_state(p2, PLAYER_STATE_WIN_POINT);
      }
      if (human_get_points(p1) >= 15 || p2->points >= 15) {
	if (human_get_points(p1) == 15)
	  p2->set_state(p2, PLAYER_STATE_LOOSE_GAME);
	else
	  p2->set_state(p2, PLAYER_STATE_WIN_GAME);
      }
    }
    board_clean_up();
    scoreboard_clean_up();
    scoreboard_reblit();
    if (p2->reblit(p2, elapsed_time) == 0)
      loop = 0;
    board_reblit();
    video_update();
  }
#ifndef _DEBUG
  SDL_WM_GrabInput(SDL_GRAB_OFF);
#endif
  menu_free(exit_menu);
  timer_free(timer);
  human_free(p1);
  p2->free(p2);
  board_deinit();
  scoreboard_deinit();
  if (menu_get_selected(again_menu) == 0)
    next_opponent = opponent;
  else
    next_opponent = -1;
  menu_free(again_menu);
  return next_opponent;
}

static void _read_settings(void)
{
  FILE *file = NULL;
  char buffer[100], buffer2[100];
  Uint32 uint32 = 0;

  if ((file = fopen(SETTINGSFILE, "r")) == NULL)
    return;
  while (fgets(buffer, 100, file) != 0) {
    if (sscanf(buffer, "%s %d\n", buffer2, &uint32) != 2) {
      fclose(file);
      return;
    } else if (strcmp(buffer2, "SOUND") == 0)
      _settings->sound = (Uint8) uint32;
    else if (strcmp(buffer2, "FULLSCREEN") == 0)
      _settings->fullscreen = (Uint8) uint32;
    else if (strcmp(buffer2, "MOUSESPEED") == 0)
      _settings->mouse_speed = (Uint8) uint32;
  }
  fclose(file);
}

static void _save_settings(void)
{
  FILE *file = NULL;

  if ((file = fopen(SETTINGSFILE, "w")) == NULL)
    return;
  fprintf(file, "SOUND %d\n", _settings->sound);
  fprintf(file, "FULLSCREEN %d\n", _settings->fullscreen);
  fprintf(file, "MOUSESPEED %d\n", _settings->mouse_speed);
  fclose(file);
}

static void _tuxpuck_init(void)
{
#ifndef windows
  char *homeDir = NULL;
#endif
  srand(time(NULL));
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
  audio_init();
  video_init();

  video_save();
  _settings = (Settings *) malloc(sizeof(Settings));
  memset(_settings, 0, sizeof(Settings));
  _settings->sound = 1;
  _settings->fullscreen = 0;
  _settings->mouse_speed = 5;
#ifndef windows
  homeDir = getenv("HOME");
  sprintf(_settings_file, "%s/.tuxpuckrc", homeDir);
#endif
  _read_settings();
  audio_set_mute(!_settings->sound);
  if (_settings->fullscreen)
    video_toggle_fullscreen();

  run_intro();
  video_save();
}

static void _tuxpuck_deinit(void)
{
  audio_deinit();
  video_deinit();
  SDL_Quit();
  _save_settings();
  free(_settings);
}

int main(int argc, char *argv[])
{
  int next_opponent;
  Menu *main_menu, *op_menu;

  _tuxpuck_init();
  main_menu = menu_create(2);
  menu_add_field(main_menu, 0, 1, "Play Match");
  menu_add_field(main_menu, 1, 1, "Exit");
  op_menu = menu_create(3);
  menu_add_field(op_menu, 0, 0, "Opponent");
  menu_add_field(op_menu, 1, 1, "Tux");
  menu_add_field(op_menu, 2, 1, "Arcana");
  while (menu_get_selected(main_menu) == 0) {
    next_opponent = menu_get_selected(op_menu);
    while (next_opponent != -1)
      next_opponent = _play_match(next_opponent);
  }
  menu_free(op_menu);
  menu_free(main_menu);
  _tuxpuck_deinit();
  return 0;
}
