/* Clementine Window Manager
   Copyright 2002 Dave Berton <db@mosey.org>

   based on aewm Copyright 1998-2001 Decklin Foster <decklin@red-bean.com>

   This program is free software; see LICENSE for details. */

#include "windowmanager.h"
#include "painter.h"

#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#define XK_MISCELLANY
#include <X11/keysymdef.h>

/* global window manager */
WindowManager* windowManager = 0;

/* Modes for find_client */
#define WINDOW 0
#define FRAME 1

#define DEF_NEW1 "urxvt"
#define DEF_NEW2 "xterm"
#define DEF_NEW3 "dmenu_run -b" //dmenu launcher

WindowManager::WindowManager()
    : WindowSystem(),
      menu(0),
      active(0)
{
    opt_new1 = DEF_NEW1;
    opt_new2 = DEF_NEW2;
    opt_new3 = DEF_NEW3;
    if ( painter()->rootCommand().length() )
	exec( (char*)painter()->rootCommand().c_str() );
}

/* After pulling my hair out trying to find some way to tell if a
 * window is still valid, I've decided to instead carefully ignore any
 * errors raised by this function. We know that the X calls are, and
 * we know the only reason why they could fail -- a window has removed
 * itself completely before the Unmap and Destroy events get through
 * the queue to us. It's not absolutely perfect, but it works.
 *
 * The 'withdrawing' argument specifes if the client is actually
 * (destroying itself||being destroyed by us) or if we are merely
 * cleaning up its data structures when we exit mid-session. */
WindowManager::~WindowManager()
{
    shutdown();
}

void WindowManager::shutdown()
{
#ifdef DEBUG
    std::cout << "WindowManager shutting down" << std::endl;
    dumpClients();
#endif
    unsigned int nwins, i;
    Window dummyw1, dummyw2, *wins;
    Client *c;

    queryTree( &dummyw1, &dummyw2, &wins, &nwins);
    grabServer();
    ignoreErrors();
    for (i = 0; i < nwins; i++) {
	c = findClient(wins[i], FRAME);
	if (c) {
	    c->map();
	    std::vector<Client*>::iterator it = std::find(clients.begin(), 
						  clients.end(), 
						  c);
	    assert( it != clients.end() );
	    clients.erase( it );
    
	    delete c;
	}
    }
    sync();
    unignoreErrors();
    ungrabServer();

    xFree(wins);
    assert( clients.size() == 0 );
    //assert( icons.size() == 0 ); db more
}

void WindowManager::exec(char *cmd)
{
    pid_t pid = fork();

    switch (pid) {
    case 0:
	execlp("/bin/sh", "sh", "-c", cmd, NULL);
	err("exec failed, cleaning up child");
	exit(1);
    case -1:
	err("can't fork");
    }
}

void WindowManager::withdraw( Client* c )
{
    grabServer();
    ignoreErrors();
    
#ifdef DEBUG
    err("withdrawing " + c->name());
#endif
    
    if ( active && c == active )
	active = 0;

    std::vector<Client*>::iterator it = std::find(clients.begin(), 
						  clients.end(), 
						  c);
    assert( it != clients.end() );
    clients.erase( it );
    
    delete c;
    
    sync();
    unignoreErrors();
    ungrabServer();
}

/* Set up a client structure for the new (not-yet-mapped) window. The
 * confusing bit is that we have to ignore 2 unmap events if the
 * client was already mapped but has IconicState set (for instance,
 * when we are the second window manager in a session).  That's
 * because there's one for the reparent (which happens on all viewable
 * windows) and then another for the unmapping itself. */

void WindowManager::newWindow( Window w, bool viewable,
			       int x, int y, int width, int height,
			       long colormap)
{
    Client *c;

    grabServer();

    c = new Client( this, w, viewable, x, y, width, height, colormap );
    if ( c->state() == IconicState )
	icons.push_back( c );
    else
	clients.push_back( c );

#ifdef DEBUG
    dump(c);
#endif

    sync();
    ungrabServer();
}

void WindowManager::newIcon( Window w, bool viewable,
			     int x, int y, int width, int height,
			     long colormap)
{
    Client *c = findClient(w, WINDOW);
    if (c) {
	/* add to icons */
	std::vector<Client*>::iterator it = std::find( icons.begin(),
						       icons.end(),
						       c);
	assert( it == icons.end() );
	icons.push_back( c );
	/* remove from clients */
	it = std::find(clients.begin(), 
		       clients.end(), 
		       c);
	assert( it != clients.end() );
	    clients.erase( it );
    } else {
	assert(false);
    }
#ifdef DEBUG
	dumpClients();
#endif
}

#ifdef DEBUG

/* Bleh, stupid macro names. I'm not feeling creative today. */

#define SHOW(name) \
case name: return #name;

const char * WindowManager::showState(Client *c)
{
    switch ( c->state() ) {
	SHOW(WithdrawnState)
	SHOW(NormalState)
	SHOW(IconicState)
	default: return "unknown state";
    }
}

/*
const char * WindowManager::showGrav(Client *c)
{
    if (!c->size || !(c->size->flags & PWinGravity))
	return "no grav (NW)";

    switch (c->size->win_gravity) {
	SHOW(UnmapGravity)
	    SHOW(NorthWestGravity)
	    SHOW(NorthGravity)
	    SHOW(NorthEastGravity)
	    SHOW(WestGravity)
	    SHOW(CenterGravity)
	    SHOW(EastGravity)
	    SHOW(SouthWestGravity)
	    SHOW(SouthGravity)
	    SHOW(SouthEastGravity)
	    SHOW(StaticGravity)
	    default: return "unknown grav";
    }

}
*/

void WindowManager::dump(Client *c)
{
    err( c->name() + std::string( "\n\t" ) +
	 /*std::string( showState(c) )+*/ std::string( ", ignore\n" ));
    // db more
/* +
	 convertToString( c->ignore_unmap ) + std::string( "\tframe " ) +
	 convertToString( c->frame ) + std::string( ", win " ) +
	 convertToString( c->window ) + std::string( ", geom " ) +
	 convertToString( c->width ) + std::string( "," ) +
	 convertToString( c->height ) + std::string( "," ) +
	 convertToString( c->x ) + std::string( "," ) +
	 convertToString( c->y ));
*/
}

void WindowManager::dumpClients()
{
    std::cout << "CLIENTS:-----------------" << std::endl;
    std::vector<Client*>::iterator it = clients.begin();
    for( ; it != clients.end(); ++it ) 
	dump(*it);
    std::cout << "ICONS:-------------------" << std::endl;
    it = icons.begin();
    for( ; it != icons.end(); ++it ) 
	dump(*it);
    std::cout << "-------------------------" << std::endl;    

}
#endif

bool WindowManager::managedWindow( Window w )
{
    return findClient( w, WINDOW ) != 0;
}

Client* WindowManager::findClient(Window w, int mode)
{
    Client *c;
    std::vector<Client*>::const_iterator it = clients.begin();
    for( ; it != clients.end(); ++it ) {
	c = *it;
	if ( mode == FRAME ) {
	    if ( c->isFrame( w ) ||
		c->isCloseButton( w ) ||
		c->isIconizeButton( w ) ||
		c->isTitlebar( w ) )
		return c;
	} else { /* WINDOW */
	    if ( c->isWindow( w ) )
		return c;
	}
    }
    return 0;
}

/* find a client based on client window or frame window */
Client* WindowManager::findClient(Window w)
{
    Client* c = findClient( w, FRAME );
    if ( c )
	return c;
    return findClient( w, WINDOW );
}

void WindowManager::takeButtonPress( Window w, unsigned int button, 
				     bool mod1,
				     int x, int y )
{
    if ( menu && menu->isWindow( w ) ) {
	int	pointer_x;
	int	pointer_y;
	pointerPosition(&pointer_x, &pointer_y);
	int item = menu->item( x,y );
	delete menu;
	menu = 0;
	assert( item <= (int)icons.size() );
	Client* c = icons[item];
	assert(c);
	c->mapRaised();
	c->redraw( true );
	clients.push_back( c );
	icons.erase( std::find( icons.begin(), icons.end(), c ) );
	dumpClients();
    } else {
	if ( menu ) {
	    delete menu;
	    menu = 0;
	}
	Client *c = findClient(w);
	if ( c )
	    c->takeButtonPress( w, button, mod1, x, y );
    }
}

void WindowManager::takeButtonRelease( Window w, unsigned int button, 
				       bool mod1,
				       int x, int y )
{
    Client *c = findClient(w);
    if ( c )
	c->takeButtonRelease( w, button, mod1, x, y );
}

void WindowManager::takeRootButton( unsigned int button, 
				    bool mod1, 
				    int x, int y )
{
    if ( menu ) {
	delete menu;
	menu = 0;
    }
    switch (button) {
    case Button1: 
	exec(opt_new1); 
	break;
    case Button2: 
//	if ( button == 2 ) {
//	    shutdown();
//	    WindowSystem::shutdown();
//	    const char* cmd = "./clementine"; // db more, path?
//	    execlp("/bin/sh", "sh", "-c", cmd, NULL);
//	    err("exec failed, cleaning up child");
//	    exit(1);
	exec(opt_new3); 
	break;
//
//    }
    case Button3: 
	if ( icons.size() )
	    menu = new Menu( this, icons, x, y );
	break;
    }
}


/* Because we are redirecting the root window, we get ConfigureRequest
 * events from both clients we're handling and ones that we aren't.
 * For clients we manage, we need to fiddle with the frame and the
 * client window, and for unmanaged windows we have to pass along
 * everything unchanged. Thankfully, we can reuse (a) the
 * XWindowChanges struct and (c) the code to configure the client
 * window in both cases.
 *
 * Most of the assignments here are going to be garbage, but only the
 * ones that are masked in by e->value_mask will be looked at by the X
 * server. */
void WindowManager::takeConfigureRequest( Window w, int x, int y, 
					  int width, int height,
					  unsigned int valuemask,
					  Window sibling, int stack_mode )
{
    int x1,y1;
    Client *c = findClient(w, WINDOW);
    if (c) {
	c->takeConfigureRequest( x, y, width, height, valuemask, sibling, 
				 stack_mode );
	/* start setting up the next call */
	x1 = 0;
	y1 = c->titleHeight();
    } else {
	x1 = x;
	y1 = y;
    }

    configure( w, x1, y1, width, height, valuemask, sibling, stack_mode );
}

/* Two possiblilies if a client is asking to be mapped. One is that
 * it's a new window, so we handle that if it isn't in our clients
 * list anywhere. The other is that it already exists and wants to
 * de-iconify, which is simple to take care of. */
void WindowManager::takeMap( Window w )
{

    Client *c = findClient(w, WINDOW);

    if (!c) 
	throw ("Client: bad takeMap request");
    else
	c->mapRaised();
}

/* See aewm.h for the intro to this one. If this is a window we
 * unmapped ourselves, decrement c->ignore_unmap and casually go on as
 * if nothing had happened. If the window unmapped itself from under
 * our feet, however, get rid of it.
 *
 * If you spend a lot of time with -DDEBUG on, you'll realize that
 * because most clients unmap and destroy themselves at once, they're
 * gone before we even get the Unmap event, never mind the Destroy
 * one. This will necessitate some extra caution in remove_client.
 *
 * Personally, I think that if Map events are intercepted, Unmap
 * events should be intercepted too. No use arguing with a standard
 * that's almost as old as I am though. :-( */
void WindowManager::takeUnmap( Window w )
{
    Client *c = findClient(w, WINDOW);

    if (!c) {
	return;
    }
    if ( c->checkUnmap() ) {
	withdraw(c);
    }
}

/* This happens when a window is iconified and destroys itself. An
 * Unmap event wouldn't happen in that case because the window is
 * already unmapped. */
void WindowManager::takeDestroy( Window w )
{
    Client *c = findClient(w, WINDOW);
    if (!c) {
	return;
    }
    withdraw(c);
}

/* All that we have cached is the name and the size hints, so we only
 * have to check for those here. A change in the name means we have to
 * immediately wipe out the old name and redraw; size hints only get
 * used when we need them. */
void WindowManager::takeNameChange( Window w, std::string name )
{
    Client *c = findClient(w, WINDOW);
    if (!c) 
	return;
    c->setName(name);
}


/* X's default focus policy is follows-mouse, but we have to set it
 * anyway because some sloppily written clients assume that (a) they
 * can set the focus whenever they want or (b) that they don't have
 * the focus unless the keyboard is grabbed to them. OTOH it does
 * allow us to keep the previous focus when pointing at the root,
 * which is nice.
 *
 * We also implement a colormap-follows-mouse policy here. That, on
 * the third hand, is *not* X's default. */

void WindowManager::takeFocus( Window w )
{
    Client *c = findClient(w, FRAME);
    if (!c) 
	return;
    if ( active && active != c ) {
	active->redraw( false );
    }
    active = c;
    c->takeFocus();
    c->redraw( true );
}

/* Here's part 2 of our colormap policy: when a client installs a new
 * colormap on itself, set the display's colormap to that. Arguably,
 * this is bad, because we should only set the colormap if that client
 * has the focus. However, clients don't usually set colormaps at
 * random when you're not interacting with them, so I think we're
 * safe. If you have an 8-bit display and this doesn't work for you,
 * by all means yell at me, but very few people have 8-bit displays
 * these days. */
void WindowManager::takeNewColormap( Window w, long colormap )
{
    Client *c = findClient(w, WINDOW);
    if (c)
	c->installColormap( colormap );
}

/* If we were covered by multiple windows, we will usually get
 * multiple expose events, so ignore them unless e->count (the number
 * of outstanding exposes) is zero. */
void WindowManager::takeExpose( Window w )
{
    if ( menu )
	menu->expose();
    else {
	Client *c = findClient(w);
	if (c) 
	    c->redraw();
    }
}

void WindowManager::takeMotion( Window w, int x, int y )
{
    if ( menu && menu->isWindow( w ) ) {
	int	pointer_x;
	int	pointer_y;
	pointerPosition(&pointer_x, &pointer_y);
	menu->pointerMotion( pointer_x, pointer_y );
    } 
}

void WindowManager::takeKeyPress( Window w, unsigned int state,
				  KeySym keysym )
{
    if ( menu ) {
	if ( keysym == XK_Escape ) {
	    delete menu;
	    menu = 0;
	}
    }
    if ( keysym == XK_Tab && clients.size() ) {
	if ( clients.size() == 1 )
	    (*clients.begin())->redraw( true ); 
    }
	

}



