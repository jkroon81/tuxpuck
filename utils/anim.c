/* anim.c - Copyright (C) 2001-2002 Jacob Kroon, see COPYING for details */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <SDL_endian.h>
#include <SDL_types.h>

void errorc(char *msg)
{
  fprintf(stderr, "%s\n", msg);
  exit(1);
}

void errorcc(char *msg1, char *msg2)
{
  fprintf(stderr, "%s%s\n", msg1, msg2);
  exit(1);
}

void write_file(FILE * out, char *filename)
{
  FILE *in = NULL;
  struct stat theStat;
  Uint8 *data;
  Uint32 size;

  if ((in = fopen(filename, "rb")) == NULL)
    errorcc("Couldn't open file, ", filename);
  stat(filename, &theStat);
  data = (Uint8 *) malloc(theStat.st_size);
  size = theStat.st_size;
  fwrite(&size, sizeof(Uint32), 1, out);
  fread(data, theStat.st_size, 1, in);
  fwrite(data, theStat.st_size, 1, out);
  free(data);
  fclose(in);
}

int main(int argc, char **argv)
{
  FILE *in = NULL, *out = NULL;
  char buffer1[100];
  char buffer2[100];
  char *ptr;
  Uint8 nbrOfFrames, nbrOfAnimations;
  Uint32 uint32;
  int j = 0, i = 0;

  if (argc != 3)
    errorc("Usage : anim <infile> <outfile>");
  if ((in = fopen(argv[1], "rb")) == NULL)
    errorcc("Couldn't open file for reading: ", argv[1]);
  if ((out = fopen(argv[2], "wb")) == NULL)
    errorcc("Couldn't open file for writing: ", argv[2]);
  if (fscanf(in, "NbrOfFrames: %d\n", &uint32) != 1)
    errorc("Wrong number of frames!");
  nbrOfFrames = (Uint8) uint32;
  fwrite(&nbrOfFrames, 1, 1, out);
  ptr = strrchr(argv[1], '/');
  if (ptr)
    argv[1][strlen(argv[1]) - strlen(ptr) + 1] = 0;
  else
    argv[1][0] = 0;
  for (i = 0; i < nbrOfFrames; i++) {
    if (fscanf(in, "%s\n", buffer1) != 1)
      errorc("Couldnt find image name!");
    sprintf(buffer2, "%s%s", argv[1], buffer1);
    write_file(out, buffer2);
  }
  if (fscanf(in, "NbrOfAnimations: %d\n", &uint32) != 1)
    errorc("Wrong number of animations!");
  nbrOfAnimations = (Uint8) uint32;
  fwrite(&nbrOfAnimations, 1, 1, out);
  for (i = 0; i < nbrOfAnimations; i++) {
    Uint32 n;
    Uint8 n2;

    if (fscanf(in, "%d\n", &n) != 1)
      errorc("Couldnt read number of frames in animation!");
    n2 = (Uint8) n;
    fwrite(&n2, 1, 1, out);
    for (j = 0; j < n2; j++) {
      Uint32 frame;
      Uint32 time;
      Uint8 frame2;
      Uint16 time2;

      if (fscanf(in, "%d %d\n", &frame, &time) != 2)
	errorc("Error reading frames");
      frame2 = (Uint8) frame;
      time2 = (Uint16) time;
      fwrite(&frame2, 1, 1, out);
      fwrite(&time2, sizeof(time2), 1, out);
    }
  }
  fclose(in);
  fclose(out);
  return 0;
}
