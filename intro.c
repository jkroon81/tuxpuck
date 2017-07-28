/* intro.c - Copyright (C) 2001-2002 Jacob Kroon, see COPYING for details */

#include <string.h>
#include <SDL_events.h>
#include <SDL_timer.h>
#include "video.h"
#include "font.h"
#include "tuxpuck.h"

/* defines */
#define FADE_SPEED	((float)100.0)

/* externals */
extern unsigned char title_jpg[];
extern unsigned char logo_jpg[];
extern unsigned char monos_fnt[];

/* functions */
void run_intro(void)
{
  Uint8 loop = 1, state = 0;
  Uint32 elapsed_time = 0;
  float alpha = 0.0;
  SDL_Event event;
  SDL_Surface *sdl_title = NULL, *sdl_logo = NULL;
  SDL_Rect text_rect, logo_rect;
  Font *font = NULL;
  Timer *timer = NULL;
  char txt_coder[] = "written by Jacob Kroon";
  char txt_gfx[] = "graphics by m0ns00n";
  char txt_version[] = "v" _VERSION;

  sdl_title = video_create_jpg_surface(title_jpg, NULL);
  sdl_logo = video_create_jpg_surface(logo_jpg, NULL);
  logo_rect.x = 0;
  logo_rect.y = SCREEN_H / 2 - sdl_logo->h / 2 - 9;
  logo_rect.w = logo_rect.h = 0;
  video_box_up(sdl_title, 8000);
  video_save();
  font = font_create(monos_fnt, NULL);
  font_set_color(font, 170, 140, 110);
  font_set_pen(font, 70, logo_rect.y + sdl_logo->h);
  timer = timer_create();
  timer_reset(timer);
  while (loop) {
    while (SDL_PollEvent(&event))
      if (event.type == SDL_MOUSEBUTTONDOWN)
	loop = 0;
    timer_update(timer);
    timer_reset(timer);
    elapsed_time = timer_elapsed(timer);
    switch (state) {
    case 0:
      alpha += FADE_SPEED * 2.0 * elapsed_time / 1000.0;
      if (alpha > 255.0)
	alpha = 255;
      video_erase(&logo_rect);
      video_set_alpha(sdl_logo, (Uint8) alpha);
      video_blit(sdl_logo, NULL, &logo_rect);
      font_set_alpha(font, alpha);
      video_erase(&text_rect);
      font_print(font, txt_version, &text_rect);
      if (alpha == 255.0) {
	alpha = 0.0;
	state = 1;
	font_set_pen(font,
		     SCREEN_W / 2 - font_calc_width(font,
						    txt_coder) / 2,
		     SCREEN_H - 80);
	memset(&text_rect, 0, sizeof(SDL_Rect));
      }
      break;
    case 1:
      alpha += FADE_SPEED * elapsed_time / 1000.0;
      if (alpha > 255.0) {
	state = 2;
	alpha = 255.0;
      }
      font_set_alpha(font, (Uint8) alpha);
      video_erase(&text_rect);
      font_print(font, txt_coder, &text_rect);
      break;
    case 2:
      alpha -= FADE_SPEED * elapsed_time / 1000.0;
      if (alpha < 0.0) {
	state = 3;
	alpha = 0.0;
      }
      font_set_alpha(font, (Uint8) alpha);
      video_erase(&text_rect);
      font_print(font, txt_coder, &text_rect);
      if (alpha == 0.0)
	font_set_pen(font,
		     SCREEN_W / 2 - font_calc_width(font,
						    txt_gfx) / 2,
		     SCREEN_H - 80);
      break;
    case 3:
      alpha += FADE_SPEED * elapsed_time / 1000.0;
      if (alpha > 255.0) {
	state = 4;
	alpha = 255.0;
      }
      font_set_alpha(font, (Uint8) alpha);
      video_erase(&text_rect);
      font_print(font, txt_gfx, &text_rect);
      break;
    case 4:
      alpha -= FADE_SPEED * elapsed_time / 1000.0;
      if (alpha < 0.0) {
	state = 5;
	alpha = 0.0;
      }
      font_set_alpha(font, (Uint8) alpha);
      video_erase(&text_rect);
      font_print(font, txt_gfx, &text_rect);
      if (alpha == 0.0)
	font_set_pen(font,
		     SCREEN_W / 2 - font_calc_width(font,
						    txt_coder) / 2,
		     SCREEN_H - 80);
      break;
    case 5:
      alpha += elapsed_time / 1000.0;
      if (alpha > 5.0) {
	state = 1;
	alpha = 0.0;
      }
      break;
    }
    video_update();
    SDL_Delay(SLEEP);
  }
  SDL_FreeSurface(sdl_title);
  SDL_FreeSurface(sdl_logo);
  font_free(font);
  timer_free(timer);
}
