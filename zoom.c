/* zoom.c - Copyright (C) 2001-2002 Jacob Kroon, see COPYING for details */
/* Zoom code originally by A. Schiffler, aschiffler@cogeco.ca */

#include <stdlib.h>
#include <SDL_video.h>

/* defines */
#define VALUE_LIMIT     0.001

/* zoom code */
static int zoomSurfaceY(SDL_Surface * src, SDL_Surface * dst)
{
  Uint32 x, y, sx, sy, *sax, *say, *csax, *csay, csx, csy;
  Uint8 *sp, *dp, *csp;
  int dgap;

  /* Variable setup */
  sx = (Uint32) (65536.0 * (float) src->w / (float) dst->w);
  sy = (Uint32) (65536.0 * (float) src->h / (float) dst->h);

  /* Allocate memory for row increments */
  if ((sax = (Uint32 *) malloc(dst->w * sizeof(Uint32))) == NULL) {
    return (-1);
  }
  if ((say = (Uint32 *) malloc(dst->h * sizeof(Uint32))) == NULL) {
    if (sax != NULL) {
      free(sax);
    }
    return (-1);
  }

  /* Precalculate row increments */
  csx = 0;
  csax = sax;
  for (x = 0; x < dst->w; x++) {
    csx += sx;
    *csax = (csx >> 16);
    csx &= 0xffff;
    csax++;
  }
  csy = 0;
  csay = say;
  for (y = 0; y < dst->h; y++) {
    csy += sy;
    *csay = (csy >> 16);
    csy &= 0xffff;
    csay++;
  }

  csx = 0;
  csax = sax;
  for (x = 0; x < dst->w; x++) {
    csx += (*csax);
    csax++;
  }
  csy = 0;
  csay = say;
  for (y = 0; y < dst->h; y++) {
    csy += (*csay);
    csay++;
  }

  /* Pointer setup */
  sp = csp = (Uint8 *) src->pixels;
  dp = (Uint8 *) dst->pixels;
  dgap = dst->pitch - dst->w;

  /* Draw */
  csay = say;
  for (y = 0; y < dst->h; y++) {
    csax = sax;
    sp = csp;
    for (x = 0; x < dst->w; x++) {
      /* Draw */
      *dp = *sp;
      /* Advance source pointers */
      sp += (*csax);
      csax++;
      /* Advance destination pointer */
      dp++;
    }
    /* Advance source pointer (for row) */
    csp += ((*csay) * src->pitch);
    csay++;
    /* Advance destination pointers */
    dp += dgap;
  }

  /* Remove temp arrays */
  free(sax);
  free(say);

  return (0);
}

SDL_Surface *zoomSurface(SDL_Surface * src, float zoom)
{
  SDL_Surface *rz_src;
  SDL_Surface *rz_dst;
  int dstwidth, dstheight;
  int i;

  /* Sanity check */
  if (src == NULL)
    return (NULL);

  if (src->format->BitsPerPixel != 8)
    return NULL;
  rz_src = src;

  /* Sanity check zoom factors */
  if (zoom < VALUE_LIMIT) {
    zoom = VALUE_LIMIT;
  }

  /* Calculate target size and set rect */
  dstwidth = (int) ((double) rz_src->w * zoom);
  dstheight = (int) ((double) rz_src->h * zoom);
  if (dstwidth < 1) {
    dstwidth = 1;
  }
  if (dstheight < 1) {
    dstheight = 1;
  }

  /* Alloc space to completely contain the zoomed surface */
  rz_dst = NULL;
  /* Target surface is 8bit */
  rz_dst =
    SDL_CreateRGBSurface(SDL_SWSURFACE, dstwidth, dstheight, 8, 0, 0, 0, 0);
  /* Lock source surface */
  SDL_LockSurface(rz_src);
  /* Check which kind of surface we have */
  /* Copy palette and colorkey info */
  for (i = 0; i < rz_src->format->palette->ncolors; i++) {
    rz_dst->format->palette->colors[i] = rz_src->format->palette->colors[i];
  }
  rz_dst->format->palette->ncolors = rz_src->format->palette->ncolors;
  /* Call the 8bit transformation routine to do the zooming */
  zoomSurfaceY(rz_src, rz_dst);
  SDL_SetColorKey(rz_dst, SDL_SRCCOLORKEY | SDL_RLEACCEL,
		  rz_src->format->colorkey);
  /* Unlock source surface */
  SDL_UnlockSurface(rz_src);

  /* Return destination surface */
  return (rz_dst);
}
