/* Clementine Window Manager
   Copyright 2002 Dave Berton <db@mosey.org>
   Copyright 2014 Dj_Dexter/djmasde <djdexter@gentoo-es.com>

   based on aewm Copyright 1998-2001 Decklin Foster <decklin@red-bean.com>

   This program is free software; see LICENSE for details. */


#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <string>
#include <vector>
#include <assert.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "windowsystem.h"
#include "menu.h"
#include "client.h"

class WindowManager : public WindowSystem
{
public:
    WindowManager();
    virtual ~WindowManager();
    
    void shutdown();
    
private:
    std::vector<Client*> clients;
    std::vector<Client*> icons;
    char*        opt_new1;
    const char*        opt_new2;
    const char*        opt_new3;
    Menu* menu;
    Client* active;

    void exec(char *cmd);

    void withdraw( Client* c );
    Client* findClient(Window w, int mode);
    Client* findClient(Window w);

    void takeButtonPress( Window w, unsigned int button, bool mod1,
			  int x, int y);
    void takeButtonRelease( Window w, unsigned int button, bool mod1,
			    int x, int y);
    void takeRootButton( unsigned int button, 
			 bool mod1, 
			 int x, int y );
    void takeConfigureRequest( Window w, int x, int y, 
			       int width, int height,
			       unsigned int valuemask,
			       Window sibling, int stack_mode );
    void takeKeyPress( Window w, unsigned int state,
		       KeySym keysym );
    void takeMap( Window w ); 
    void takeUnmap( Window w ); 
    void takeDestroy( Window w ); 
    void takeFocus( Window w ); 
    void takeNewColormap( Window w, long colormap ); 
    void takeNameChange( Window w, std::string name );
    void takeExpose( Window w ); 
    void takeMotion( Window w, int x, int y );

    bool managedWindow( Window w );
    void newWindow( Window w, bool viewable,
		    int x, int y, int width, int height, long colormap);
    void newIcon( Window w, bool viewable,
		  int x, int y, int width, int height, long colormap);


#ifdef DEBUG
    const char * showState(Client *c);
//    const char * showGrav(Client *c);
    void dump(Client *c);
    void dumpClients();
#endif

};

extern WindowManager* windowManager;

#endif
