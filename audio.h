/* audio.h - Copyright (C) 2001-2002 Jacob Kroon, see COPYING for details */

#ifndef _AUDIO_H
#define _AUDIO_H

/* includes */
#include <SDL_types.h>

/* structs */
typedef struct _Sound Sound;

/* functions */
void audio_init(void);
void audio_deinit(void);
Sound *audio_create_sound(Uint8 *, Uint32 *);
void audio_free_sound(Sound *);
void audio_play_sound(Sound *);
void audio_set_mute(Uint8);
void audio_set_single(Sound *, Uint8);

#endif /* _AUDIO_H */
