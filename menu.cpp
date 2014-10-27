/* Clementine Window Manager
   Copyright 2002 Dave Berton <db@mosey.org>

   based on aewm Copyright 1998-2001 Decklin Foster <decklin@red-bean.com>

   This program is free software; see LICENSE for details. */

#include "menu.h"
#include "client.h"
#include "windowsystem.h"
#include "painter.h"
#include <assert.h>

Menu::Menu( WindowSystem* windowsystem,
	    std::vector< Client* > clients,
	    int x, int y )
    : ws(windowsystem),
      entries( clients ),
      popup(0)
{
    assert(ws);
    assert(entries.size());
    popup = ws->createMenu();
    assert( popup );
    /* set menu dimensions */
    int w = 100;
    std::vector< Client* >::const_iterator it;
    for ( it = entries.begin(); it != entries.end(); ++it ) {
	std::string label = (*it)->name();
	int tw = ws->painter()->textWidth( label );
	if ( tw > w )
	    w = tw;
    }

    width = w;
    item_height = ws->painter()->fontAscent() + 
	ws->painter()->fontDescent() + 
	ws->pad();

    /* Arrange for centre of first menu item to be under pointer. */
    start_x = x - width / 2;
    start_y = y - item_height / 2;

    // db more adjust for edge of screen
    if (start_x < 0)
	start_x = 0;
    if (start_x + width > ws->displayWidth() )
	start_x = ws->displayWidth() - width;
    if (start_y < 0)
	start_y = 0;

    x -= start_x;
    y -= start_y;

    currentItem = item( x, y );

    ws->moveResize( popup, start_x, start_y,
		    width, (entries.size()) * item_height );
    ws->mapRaised( popup );
}

Menu::~Menu()
{
    if ( popup )
	ws->destroy( popup );
}

int Menu::item(int x, int y)
{
    if ( x < 0 || x > (int)width || 
	 y < 0 || y >= (int)(entries.size()*item_height) ) 
	return -1;
    return y / item_height;
}

void Menu::expose()
{
    drawLabels();
}

void Menu::pointerMotion( int x, int y )
{
    int old = currentItem;
    x -= start_x;
    y -= start_y;
    currentItem = item(x, y);

    if ( currentItem == old )
	return;

    /* erase old */
    if ( old > -1 && old < (int)entries.size() ) {
        drawLabel( old, entries[old]->name() );
    }
    /* draw new */
    if ( currentItem > -1 && currentItem < (int)entries.size() ) {
	drawLabel( currentItem, entries[currentItem]->name() );
    }
}


void Menu::drawLabels()
{
    ws->grabServer();
    ws->painter()->setTitlebarBackground( popup, true, 
					  width, item_height*entries.size() );
    ws->painter()->clear( popup );
    std::vector< Client* >::const_iterator it;
    for ( it = entries.begin(); it != entries.end(); ++it ) {
	std::string label = (*it)->name();
	int entryNum = it - entries.begin();
	drawLabel( entryNum, label );
    }
    ws->ungrabServer();
    ws->sync();
}

void Menu::drawLabel( int num, std::string l )
{
    int tx = (width - ws->painter()->textWidth( l )) / 2 + ws->pad();
    int ty = num * item_height + ws->painter()->fontAscent() + 1;
    ws->painter()->drawString( popup, num == currentItem, tx, ty, l );
}


