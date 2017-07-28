/* data2c - Copyright (C) 2001-2002 Jacob Kroon, see COPYING for details */

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <SDL_types.h>
#include <SDL_endian.h>

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

int main(int argc, char **argv)
{
  FILE *in = NULL, *out = NULL;
  char buffer[100];
  unsigned char ch;
  int i = 0;
  Uint32 size;
  struct stat theStat;

  if (argc != 3)
    errorc("Usage : data2c <file> <array>");
  if ((in = fopen(argv[1], "rb")) == NULL)
    errorcc("Couldn't open file for reading : ", argv[1]);
  stat(argv[1], &theStat);
  size = theStat.st_size;
  sprintf(buffer, "%s.c", argv[2]);
  if ((out = fopen(buffer, "wb")) == NULL)
    errorcc("Couldn't open file for writing : ", buffer);
  fprintf(out, "/* %s */\n", buffer);
  fprintf(out, "unsigned char %s[] = {\n", argv[2]);
  for (i = 0; i < 4; i++)
    fprintf(out, "%d,", ((Uint8 *) & size)[i]);
  while (fread(&ch, 1, 1, in) != 0)
    fprintf(out, "%d,", ch);
  fseek(out, -1, SEEK_CUR);
  fprintf(out, "};\n");
  fclose(in);
  fclose(out);
  return 0;
}
