/* Clementine Window Manager
   Copyright 2002 Dave Berton <db@mosey.org>

   based on aewm Copyright 1998-2001 Decklin Foster <decklin@red-bean.com>

   This program is free software; see LICENSE for details. */

/* This structure keeps track of top-level windows (hereinafter
 * 'clients'). The clients we know about (i.e. all that don't set
 * override-redirect) are kept track of in linked list starting at the
 * global pointer called, appropriately, 'clients'. 
 *
 * window and parent refer to the actual client window and the larget
 * frame into which we will reparent it respectively. trans is set to
 * None for regular windows, and the window's 'owner' for a transient
 * window. Currently, we don't actually do anything with the owner for
 * transients; it's just used as a boolean.
 *
 * ignore_unmap is for our own purposes and doesn't reflect anything
 * from X. Whenever we unmap a window intentionally, we increment
 * ignore_unmap. This way our unmap event handler can tell when it
 * isn't supposed to do anything. */

#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

class WindowSystem;

class Client 
{
public:
    Client( WindowSystem* windowsystem, Window w, bool viewable,
	    int wx, int wy, int wwidth, int wheight,
	    long wcolormap );
    ~Client();

    std::string name() const {return nm;}
    void setName( const std::string& name ) { nm = name; redraw(); }
    int titleHeight() const;

    void redraw() { redraw(active); }
    void redraw( bool act, bool force = false );
    void iconize();
    void uniconize();
    void map();
    void mapRaised();
    void takeFocus();
    void takeButtonPress( Window w, unsigned int button, bool mod1,
			  int x, int y);
    void takeButtonRelease( Window w, unsigned int button, bool mod1,
			    int x, int y );
    void installColormap( long colormap );
    long state();
    void move();
    void moveResize();
    void raise();
    void lower();
    void drag();
    void resize();
    bool checkUnmap();
    bool isActive() const { return active; }

    // db more get rid ?
    void takeConfigureRequest( int x, int y,
			       unsigned int width, unsigned int height,
			       unsigned int valuemask,
			       Window sibling, int stack_mode );


    bool isFrame( Window w ) { return w==frame; }
    bool isTitlebar( Window w ) { return w==titlebar; }
    bool isCloseButton( Window w ) { return w==close_button; }
    bool isIconizeButton( Window w ) { return w==iconize_button; }
    bool isWindow( Window w ) { return (w==window); }

private:
    WindowSystem* ws;
    std::string   nm;
    Colormap      cmap;
    Window        window;
    Window        frame;
    Window        close_button;
    Window        iconize_button;
    Window        titlebar;
    Window        trans;
    int           x;
    int           y;
    int           width;
    int           height;
    int           ignore_unmap;
    XSizeHints*   size;
    bool          active;
    int           button_size; // width/height of button
    int           titlebar_width;

    void initPosition();
    void drawOutline();
    void recalcResize(int x1, int y1, int x2, int y2);
    int getIncSize( int *x_ret, int *y_ret, int mode);
    void configure();
    void tryDelete();
    bool isInCloseBox( int mx, int my ) {
	return (mx >= width - titleHeight()) && (my <= titleHeight())
	    && (mx >= 0) && (my >=0);
    }
    bool isInIconizeBox( int mx, int my ) {
	return (mx <= titleHeight()) && (my <= titleHeight())
	    && (mx >= 0) && (my >=0);
    }

    enum GravityType {
	APPLY_GRAVITY=1,
	REMOVE_GRAVITY=-1
    };
    void gravitate( GravityType type );

    bool awaiting_close;
    bool awaiting_iconize;
};

#endif
