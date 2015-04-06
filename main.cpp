/* Clementine Window Manager
   Copyright 2002 Dave Berton <db@mosey.org>
   Copyright 2014 Dj_Dexter/djmasde <djdexter@gentoo-es.com>

   based on aewm Copyright 1998-2001 Decklin Foster <decklin@red-bean.com>

   This program is free software; see LICENSE for details. */


#include <iostream>
#include "windowmanager.h"
#include "windowsystem.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char **argv) {
   
  try {
        // show version (-v) flag, close wm (-exit) flag...
	if(argc == 2 && !strcmp("-v", argv[1]))
		printf("clementine version 0.0.7.1 see LICENSE for details\n");
        else if(argc != 1)
		printf("\n");

        if(argc == 2 && !strcmp("-exit", argv[1]))
	        execlp("/bin/sh", "sh", "-c", "pkill clementinewm", NULL);
        else if(argc != 1)
                printf("\n");
		printf("Please use one of the options:\n");
                printf("usage: clementinewm [-v -exit]\n");                
	WindowManager wm;
	windowManager = &wm; // set global reference
	wm.eventLoop();
  } catch( const char*  e ) {
	std::cerr << "Unhandled exception: " << e << std::endl;
  }
  return 0;
}

