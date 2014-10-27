#  Clementine Window Manager
#   Copyright 2002 Dave Berton <db@mosey.org>
#
#   based on aewm Copyright 1998-2001 Decklin Foster <decklin@red-bean.com>
#
#   This program is free software; see LICENSE for details.

# This should be set to the location of the X installation you want to
# compile against.
XROOT = /usr

# Uncomment for debugging info
DEFINES += -DDEBUG

CC       = g++
CFLAGS   = -g -O2 -Wall -fpermissive

BINDIR   = $(DESTDIR)$(XROOT)/bin
MANDIR   = $(DESTDIR)$(XROOT)/man/man1
CFGDIR   = $(DESTDIR)/etc/X11/clementine
INCLUDES = -I$(XROOT)/include
LDPATH   = -L$(XROOT)/lib
LIBS     = -lX11

PROG     = clementinewm
#MANPAGE  = clementine.1x
OBJS     = client.o keys.o look.o main.o menu.o painter.o windowmanager.o windowsystem.o
HEADERS  = client.h keys.h look.h menu.h painter.h tokenizer.hpp windowmanager.h windowsystem.h

all: $(PROG) 

$(PROG): $(OBJS)
	$(CC) $(OBJS) $(LDPATH) $(LIBS) -o $@

$(OBJS): %.o: %.cpp $(HEADERS)
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

install: all
	install -s $(PROG) $(BINDIR)
#	install -m 644 $(MANPAGE) $(MANDIR)
#	gzip -9vf $(MANDIR)/$(MANPAGE)
#	mkdir -p $(CFGDIR) && cp aewmrc.sample $(CFGDIR)/aewmrc

clean: 
	rm -f $(PROG) $(OBJS)

.PHONY: all install clean
