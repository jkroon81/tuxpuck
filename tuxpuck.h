/* tuxpuck.h - Copyright (C) 2001-2002 Jacob Kroon, see COPYING for details */

#ifndef _TUXPUCK_H
#define _TUXPUCK_H

/* includes */
#include <SDL_video.h>

/* defines */
#define SLEEP		10
#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2		1.57079632679489661923
#endif
#define PAD_W			((float)9.4)
#define PAD_H			((float)0.02)
#define PUCK_W			((float)2.8)
#define PUCK_STATE_ALIVE	1
#define PUCK_STATE_DEAD		2
#define BOARD_STATE_PLAY	1
#define BOARD_STATE_NEW_PUCK	2
#define BOARD_STATE_CRASH_GLASS	3
#define BOARD_TOP		((float)175)
#define BOARD_BOTTOM		((float)465)
#define BOARD_LEFT		((float)203)
#define BOARD_RIGHT		((float)517)
#define PLAYER_STATE_IDLE		1
#define PLAYER_STATE_SERVE		2
#define PLAYER_STATE_AIM		3
#define PLAYER_STATE_HIT		4
#define PLAYER_STATE_BACKUP		5
#define PLAYER_STATE_WIN_POINT		6
#define PLAYER_STATE_LOOSE_POINT	7
#define PLAYER_STATE_WIN_GAME		8
#define PLAYER_STATE_LOOSE_GAME		9

/* structs */
typedef struct _Menu Menu;
typedef struct _Entity Entity;
typedef struct _Puck Puck;
typedef Entity Pad;
typedef struct _HumanPlayer HumanPlayer;
typedef struct _AIPlayer AIPlayer;
typedef struct _Strategy Strategy;
typedef struct _Sprite Sprite;
typedef struct _Timer Timer;

struct _Strategy {
  void (*idle) (AIPlayer *, Uint32);
  void (*serve) (AIPlayer *, Uint32);
  void (*backup) (AIPlayer *, Uint32);
  void (*aim) (AIPlayer *, Uint32);
  void (*hit) (AIPlayer *, Uint32);
};

struct _AIPlayer {
  char *name;
  SDL_Surface *sdl_image;
  SDL_Rect rect;
  Pad *pad;
  Puck *puck;
  float speed, hit_power;
  Uint8 state, points;
  void (*free) (AIPlayer *);
    Uint8(*reblit) (AIPlayer *, Uint32);
  void (*set_state) (AIPlayer *, Uint8);
    Uint8(*ready) (void);
  Strategy strategy;
};

/* functions */
Menu *menu_create(int);
void menu_add_field(Menu *, int, int, char *);
int menu_get_selected(Menu *);
void menu_free(Menu *);
void entity_blit(Entity *);
void entity_erase(Entity *);
void entity_move(Entity *, Uint32);
Uint8 entity_move_towards(Entity *, float, float, float, Uint32);
void entity_set_position(Entity *, float, float);
void entity_get_position(Entity *, float *, float *);
void entity_set_velocity(Entity *, float, float);
void entity_get_velocity(Entity *, float *, float *);
void entity_set_alpha(Entity *, Uint8);
Pad *pad_create(Uint8);
void pad_free(Pad *);
Puck *puck_create(void);
void puck_free(Puck *);
void puck_move(Puck *, Uint32);
int puck_will_get_hit_by(Puck *, Pad *, Uint32);
void puck_get_hit_by(Puck *, Pad *, Uint32);
int puck_is_dead(Puck *);
void puck_set_state(Puck *, Uint8);
void board_init(void);
void board_deinit(void);
void board_blit(void);
void board_reblit(void);
void board_clean_up(void);
Uint8 board_update(Uint32);
Pad *board_get_pad(Uint8);
Puck *board_get_puck(void);
Uint8 board_get_state(void);
Uint8 board_get_turn(void);
float board_calc_y(float);
float board_calc_scale(float);
void glass_init(void);
void glass_deinit(void);
void glass_blit(void);
void glass_erase(void);
void glass_get_position(float *, float *);
Uint8 glass_update(Uint32);
void glass_smash(float, float);
void scoreboard_init(void);
void scoreboard_deinit(void);
void scoreboard_blit(void);
void scoreboard_erase(void);
void scoreboard_reblit(void);
void scoreboard_clean_up(void);
void scoreboard_set_alpha(Uint8);
void scoreboard_update(Uint32);
void scoreboard_add_point(Uint8);
void scoreboard_set_mousebar(Uint8);
HumanPlayer *human_create(Pad *, char *);
void human_free(HumanPlayer *);
void human_set_speed(HumanPlayer *, Uint8);
void human_give_point(HumanPlayer *);
Uint8 human_get_points(HumanPlayer *);
void human_update(HumanPlayer *, Uint32);
void aiplayer_blit(AIPlayer *);
void aiplayer_erase(AIPlayer *);
void aiplayer_update(AIPlayer *, Uint32);
void aiplayer_set_alpha(AIPlayer *, Uint8);
void dumb_idle(AIPlayer *, Uint32);
void dumb_serve(AIPlayer *, Uint32);
void dumb_backup(AIPlayer *, Uint32);
void dumb_aim(AIPlayer *, Uint32);
void dumb_hit(AIPlayer *, Uint32);
void smart_idle(AIPlayer *, Uint32);
void smart_backup(AIPlayer *, Uint32);
Sprite *sprite_create(Uint8 *, Uint32 *);
void sprite_free(Sprite *);
void sprite_blit(Sprite *);
void sprite_erase(Sprite *);
Uint8 sprite_update(Sprite *, Uint32);
void sprite_set_position(Sprite *, Uint32, Uint32);
void sprite_set_animation(Sprite *, Uint8);
Timer *timer_create(void);
void timer_free(Timer *);
void timer_reset(Timer *);
void timer_update(Timer *);
Uint32 timer_elapsed(Timer *);

#endif /* _TUXPUCK_H */
