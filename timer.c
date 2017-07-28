/* timer.c - Copyright (C) 2001-2002 Jacob Kroon, see COPYING for details */

#include <stdlib.h>
#include <SDL_timer.h>
#include "tuxpuck.h"

/* structs */
struct _Timer {
  Uint32 start, end, elapsed_time;
};

/* functions */
Timer *timer_create(void)
{
  Timer *timer = NULL;

  timer = malloc(sizeof(Timer));
  timer->start = SDL_GetTicks();
  timer->end = timer->start;
  timer->elapsed_time = 0;
  return timer;
}

void timer_free(Timer * timer)
{
  free(timer);
}

void timer_reset(Timer * timer)
{
  timer->start = SDL_GetTicks();
  timer->end = 0;
}

void timer_update(Timer * timer)
{
  timer->end = SDL_GetTicks();
  timer->elapsed_time = timer->end - timer->start;
}

Uint32 timer_elapsed(Timer * timer)
{
  return timer->elapsed_time;
}
