/* Clementine Window Manager
   Copyright 2002 Dave Berton <db@mosey.org>

   based on aewm Copyright 1998-2001 Decklin Foster <decklin@red-bean.com>

   This program is free software; see LICENSE for details. */

#ifndef MENU_H
#define MENU_H

#include "windowsystem.h"
#include "client.h"
#include <vector>

class Menu
{
public:
    Menu( WindowSystem* windowsystem, std::vector< Client* > clients,
	  int x, int y );
    ~Menu();

    int item( int x, int y );
    bool isWindow( Window w ) { return w == popup; }
    void expose();
    void pointerMotion( int x, int y );


private:
    WindowSystem* ws;
    std::vector< Client* > entries;
    int currentItem;  /* Last known selected menu item. -1 if none. */
    Window popup;

    int start_x;
    int start_y;

    int width;	/* Width of menu. */
    int item_height;	/* Height of each menu item. */

    void setMenuDimensions();

    void drawLabels();
    void drawLabel( int num, std::string l );
};


#endif
