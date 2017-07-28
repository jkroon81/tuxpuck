# Makefile for TuxPuck Utils , Copyright Jacob Kroon 2001-2002
CC		= gcc
CFLAGS		+= -g -Wall -Werror
#############################################################

all : ttf2font data2c anim

ttf2font : ttf2font.c
	$(CC) $(CFLAGS) ttf2font.c `freetype-config --cflags --libs` -o ttf2font

data2c : data2c.c
	$(CC) $(CFLAGS) data2c.c `sdl-config --cflags --libs` -o data2c

anim : anim.c
	$(CC) $(CFLAGS) anim.c `sdl-config --cflags --libs` -o anim

clean :
	rm -f *~ data2c ttf2font anim

indent :
	indent -br -brs -sob -ce -c50 -npsl -npcs data2c.c ttf2font.c anim.c
	rm -f *~
