TEMPLATE	= app
CONFIG		= warn_on debug
DEFINES         = DEBUG
TARGET		= clementine
SOURCES		= windowsystem.cpp \
                  client.cpp \
                  painter.cpp \
                  menu.cpp \
                  windowmanager.cpp \
                  main.cpp
HEADERS		= windowsystem.h \
                  client.h \
                  painter.h \
                  menu.h \
                  windowmanager.h

OBJECTS_DIR	= .obj

LIBS		+= -L/usr/X11R6/lib -lX11



