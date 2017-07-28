/* menu.c - Copyright (C) 2001-2002 Jacob Kroon, see COPYING for details */

#include <stdlib.h>
#include <string.h>
#include <SDL_events.h>
#include <SDL_timer.h>
#include "video.h"
#include "font.h"
#include "tuxpuck.h"

/* defines */
#define BORDER_SIZE	5
#define BORDER_COLOR	(video_map_rgb(170, 140, 110))
#define BORDER_SPACING	20
#define FIELD_SPACING	15

/* structs */
struct _Menu {
  int nbr_of_fields;
  int *selectable;
  int selected;
  char **text;
};

/* externals */
extern unsigned char monob_fnt[];

/* static variables */
static Font *_font = NULL;

/* static functions */
static void _menu_select_previous(Menu * menu)
{
  int selected = menu->selected;

  do {
    selected--;
    if (selected < 0)
      selected = menu->nbr_of_fields - 1;
  } while (menu->selectable[selected] == 0);
  menu->selected = selected;
}

static void _menu_select_next(Menu * menu)
{
  int selected = menu->selected;
  do {
    selected++;
    if (selected >= menu->nbr_of_fields)
      selected = 0;
  } while (menu->selectable[selected] == 0);
  menu->selected = selected;
}

static void _menu_blit(Menu * menu, SDL_Rect * erase_rect)
{
  int total_height = 0, total_width = 0, i, ruler_y;
  SDL_Rect rect;

  for (i = 0; i < menu->nbr_of_fields; i++) {
    total_height += font_calc_height(_font, menu->text[i]);
    if (font_calc_width(_font, menu->text[i]) > total_width)
      total_width = font_calc_width(_font, menu->text[i]);
  }
  total_width += 2 * BORDER_SPACING;
  total_height +=
    2 * BORDER_SPACING + (menu->nbr_of_fields - 1) * FIELD_SPACING;
  rect.w = total_width;
  rect.h = total_height;
  rect.x = SCREEN_W / 2 - total_width / 2;
  rect.y = SCREEN_H / 2 - total_height / 2;
  if (erase_rect != NULL) {
    erase_rect->x = rect.x - BORDER_SIZE;
    erase_rect->y = rect.y - BORDER_SIZE;
    erase_rect->w = rect.w + 2 * BORDER_SIZE;
    erase_rect->h = rect.h + 2 * BORDER_SIZE;
  }
  video_fill(0, 128, &rect);
  rect.x -= BORDER_SIZE;
  rect.y -= BORDER_SIZE;
  rect.w = BORDER_SIZE;
  rect.h += 2 * BORDER_SIZE;
  video_fill(BORDER_COLOR, 255, &rect);
  rect.x += BORDER_SIZE + total_width;
  video_fill(BORDER_COLOR, 255, &rect);
  rect.x -= total_width;
  rect.w = total_width;
  rect.h = BORDER_SIZE;
  video_fill(BORDER_COLOR, 255, &rect);
  rect.y += total_height + BORDER_SIZE;
  video_fill(BORDER_COLOR, 255, &rect);
  ruler_y = SCREEN_H / 2 - total_height / 2 + BORDER_SPACING;
  font_set_alpha(_font, 255);
  font_set_color(_font, 255, 255, 255);
  for (i = 0; i < menu->nbr_of_fields; i++) {
    ruler_y += font_calc_height(_font, menu->text[i]);
    font_set_pen(_font,
		 SCREEN_W / 2 - font_calc_width(_font, menu->text[i]) / 2,
		 ruler_y);
    ruler_y += FIELD_SPACING;
    if (!menu->selectable[i]) {
      font_set_color(_font, 20, 155, 55);
      font_print(_font, menu->text[i], NULL);
      font_set_color(_font, 255, 255, 255);
    } else if (menu->selected == i) {
      font_set_color(_font, 255, 0, 0);
      font_print(_font, menu->text[i], NULL);
      font_set_color(_font, 255, 255, 255);
    } else
      font_print(_font, menu->text[i], NULL);
  }
}

/* functions */
Menu *menu_create(int nbr_of_fields)
{
  Menu *menu;

  menu = malloc(sizeof(*menu));
  menu->selected = -1;
  menu->nbr_of_fields = nbr_of_fields;
  menu->text = malloc(nbr_of_fields * sizeof(char *));
  memset(menu->text, 0, nbr_of_fields * sizeof(char *));
  menu->selectable = malloc(nbr_of_fields * sizeof(int));
  memset(menu->selectable, 0, nbr_of_fields * sizeof(int));
  if (_font == NULL)
    _font = font_create(monob_fnt, NULL);
  return menu;
}

void menu_add_field(Menu * menu, int position, int selectable, char *text)
{
  menu->text[position] = malloc(strlen(text) + 1);
  strcpy(menu->text[position], text);
  menu->selectable[position] = selectable;
  if (menu->selected == -1 && selectable)
    menu->selected = position;
}

int menu_get_selected(Menu * menu)
{
  int loop = 1;
  SDL_Event event;
  SDL_Rect rect;
  SDL_Surface *background;

  SDL_WM_GrabInput(SDL_GRAB_OFF);
  background = video_duplicate();
  if (menu->selected == -1)
    _menu_select_next(menu);
  _menu_blit(menu, &rect);
  video_update();
  while (loop) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_KEYDOWN:
	switch (event.key.keysym.sym) {
	case SDLK_ESCAPE:
	  menu->selected = -1;
	  loop = 0;
	  break;
	case SDLK_RETURN:
	  loop = 0;
	  break;
	case SDLK_UP:
	  _menu_select_previous(menu);
	  video_blit(background, &rect, &rect);
	  _menu_blit(menu, NULL);
	  video_update();
	  break;
	case SDLK_DOWN:
	  _menu_select_next(menu);
	  video_blit(background, &rect, &rect);
	  _menu_blit(menu, NULL);
	  video_update();
	default:
	  break;
	}
	break;
      }
    }
    SDL_Delay(SLEEP);
  }
  SDL_WM_GrabInput(SDL_GRAB_ON);
  video_blit(background, &rect, &rect);
  SDL_FreeSurface(background);
  return menu->selected;
}

void menu_free(Menu * menu)
{
  int i;

  for (i = 0; i < menu->nbr_of_fields; i++)
    if (menu->text[i] != NULL)
      free(menu->text[i]);
  free(menu->selectable);
  free(menu);
}
