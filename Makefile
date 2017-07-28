# Makefile for TuxPuck , Copyright Jacob Kroon 2001-2002
NAME		= tuxpuck
VERSION		= 0.8.2
CC		= gcc
CFLAGS		+= -D_VERSION=\"$(VERSION)\" -g -Wall
CSOURCES	= tuxpuck.c video.c audio.c menu.c sprite.c font.c timer.c \
		  board.c entity.c glass.c scoreboard.c player.c zoom.c png.c \
		  jpg.c intro.c tux.c arcana.c 
INCLUDES	= tuxpuck.h video.h audio.h font.h
	   	  

#############################################################

OBJS=$(CSOURCES:.c=.o)
ifdef COMSPEC
  OBJS += w32icon.o
endif

%.o	: %.c
	$(CC) $(CFLAGS) `sdl-config --cflags` -c -o $@ $<

$(NAME) : $(OBJS)
	cd data; $(MAKE)
	$(CC) $(CFLAGS) $(OBJS) data/libdata.a `sdl-config --libs` -lm -lpng \
	-ljpeg -lz -lvorbisfile -lvorbis -logg -o $(NAME)

w32icon.o : data/icons/tuxpuck.ico
	echo AppIcon ICON "data/icons/tuxpuck.ico" > temp.rc
	windres -i temp.rc -o w32icon.o
	rm temp.rc

clean :
	cd utils; $(MAKE) clean;
	cd data; $(MAKE) clean;
	rm -f *~ $(OBJS) $(NAME)

indent :
	cd utils; $(MAKE) indent;
	indent -br -brs -sob -ce -c50 -npsl -npcs $(CSOURCES) $(INCLUDES)
	rm -f *~

dist :
	$(MAKE) clean
	mkdir $(NAME)-$(VERSION)
	cp $(CSOURCES) $(INCLUDES) readme.txt todo.txt bugs.txt thanks.txt \
	  COPYING Makefile $(NAME)-$(VERSION)
	cp -R man utils data $(NAME)-$(VERSION)
	tar -cf $(NAME)-$(VERSION).tar $(NAME)-$(VERSION)
	tar -f $(NAME)-$(VERSION).tar --delete \
	  `tar -tf $(NAME)-*.tar | grep -w -e ".svn/"`
	gzip -9 $(NAME)-$(VERSION).tar
	rm -Rf $(NAME)-$(VERSION)

install : $(NAME)
	install -d $(DESTDIR)/usr/bin
	install -d $(DESTDIR)/usr/man/man6
	install -m755 $(NAME) $(DESTDIR)/usr/bin
	install -m644 man/$(NAME).6.gz $(DESTDIR)/usr/man/man6
