/* sprite.c - Copyright (C) 2001-2002 Jacob Kroon, see COPYING for details */

#include <stdlib.h>
#include <string.h>
#include "video.h"
#include "tuxpuck.h"

/* structs */
typedef struct _Animation Animation;

struct _Animation {
  Uint8 nbr_of_frames, current_frame;
  Uint8 *frame;
  Uint16 *time;
  Uint32 time_stamp;
};

struct _Sprite {
  Uint8 nbr_of_frames, nbr_of_animations, current_animation;
  Uint32 x, y;
  SDL_Rect rect;
  SDL_Surface **sdl_frame;
  Animation **animation;
};

/* functions */
static Animation *_animation_create(Uint8 * data, Uint32 * memcounter)
{
  Animation *animation = NULL;
  Uint32 index = 0;
  Uint16 *time = NULL;
  Uint8 nbr_of_frames = 0, current_frame = 0, i;
  Uint8 *frame = NULL;

  nbr_of_frames = data[index++];
  current_frame = 0;
  time = (Uint16 *) malloc(nbr_of_frames * sizeof(Uint16));
  frame = (Uint8 *) malloc(nbr_of_frames * sizeof(Uint8));
  for (i = 0; i < nbr_of_frames; i++) {
    frame[i] = data[index++];
    memcpy(&time[i], &data[index], sizeof(Uint16));
    index += 2;
  }
  if (memcounter)
    *memcounter += index;
  animation = malloc(sizeof(Animation));
  animation->nbr_of_frames = nbr_of_frames;
  animation->current_frame = current_frame;
  animation->frame = frame;
  animation->time = time;
  animation->time_stamp = 0;
  return animation;
}

static void _animation_free(Animation * animation)
{
  free(animation->time);
  free(animation->frame);
  free(animation);
}

static void _animation_reset(Animation * animation)
{
  animation->time_stamp = 0;
  animation->current_frame = 0;
}

static Uint8 _animation_update(Animation * animation, Uint32 time)
{
  animation->time_stamp += time;
  while (animation->time_stamp > animation->time[animation->current_frame]) {
    animation->time_stamp -= animation->time[animation->current_frame];
    animation->current_frame++;
    if (animation->current_frame >= animation->nbr_of_frames) {
      animation->current_frame = 0;
      return 0;
    }
  }
  return 1;
}

Sprite *sprite_create(Uint8 * data, Uint32 * memcounter)
{
  SDL_Surface **sdl_frame = NULL;
  SDL_Rect rect = { 0, 0, 0, 0 };
  Uint32 index = 0, size;
  Uint8 current_animation = 0, nbr_of_frames = 0, nbr_of_animations = 0, i;
  Animation **animation = NULL;
  Sprite *sprite = NULL;

  memcpy(&size, data, sizeof(Uint32));
  if (memcounter)
    *memcounter += size + sizeof(Uint32);
  data += sizeof(Uint32);
  nbr_of_frames = data[index++];
  sdl_frame =
    (SDL_Surface **) malloc((nbr_of_frames + 1) * sizeof(SDL_Surface *));
  memset(sdl_frame, 0, (nbr_of_frames + 1) * sizeof(SDL_Surface *));
  for (i = 1; i < nbr_of_frames + 1; i++)
    sdl_frame[i] = video_create_png_surface(&data[index], &index);
  nbr_of_animations = data[index++];
  animation =
    (Animation **) malloc((nbr_of_animations + 1) * sizeof(Animation *));
  memset(animation, 0, (nbr_of_animations + 1) * sizeof(Animation *));
  for (i = 1; i < nbr_of_animations + 1; i++)
    animation[i] = _animation_create(&data[index], &index);
  sprite = malloc(sizeof(Sprite));
  sprite->nbr_of_frames = nbr_of_frames;
  sprite->nbr_of_animations = nbr_of_animations;
  sprite->current_animation = current_animation;
  sprite->x = sprite->y = 0;
  sprite->rect = rect;
  sprite->sdl_frame = sdl_frame;
  sprite->animation = animation;
  return sprite;
}

void sprite_free(Sprite * sprite)
{
  Uint8 i;

  for (i = 1; i < sprite->nbr_of_frames + 1; i++)
    SDL_FreeSurface(sprite->sdl_frame[i]);
  free(sprite->sdl_frame);
  for (i = 1; i < sprite->nbr_of_animations + 1; i++)
    _animation_free(sprite->animation[i]);
  free(sprite->animation);
  free(sprite);
}

void sprite_blit(Sprite * sprite)
{
  Uint8 frame;

  if (sprite->current_animation == 0) {
    memset(&sprite->rect, 0, sizeof(SDL_Rect));
    return;
  }
  frame = sprite->animation[sprite->current_animation]->current_frame;
  frame = sprite->animation[sprite->current_animation]->frame[frame];
  video_erase(&sprite->rect);
  if (frame) {
    sprite->rect.x = sprite->x;
    sprite->rect.y = sprite->y;
    video_blit(sprite->sdl_frame[frame], NULL, &sprite->rect);
  } else
    memset(&sprite->rect, 0, sizeof(SDL_Rect));
}

void sprite_erase(Sprite * sprite)
{
  video_erase(&sprite->rect);
  memset(&sprite->rect, 0, sizeof(SDL_Rect));
}

void sprite_set_animation(Sprite * sprite, Uint8 animation)
{
  sprite->current_animation = animation;
  if (animation != 0)
    _animation_reset(sprite->animation[sprite->current_animation]);
}

Uint8 sprite_update(Sprite * sprite, Uint32 time)
{
  Uint8 done;

  if (sprite->current_animation == 0)
    return 0;
  done =
    _animation_update(sprite->animation[sprite->current_animation], time);
  if (done == 0)
    sprite->current_animation = 0;
  return done;
}

void sprite_set_position(Sprite * sprite, Uint32 x, Uint32 y)
{
  sprite->x = x;
  sprite->y = y;
}
