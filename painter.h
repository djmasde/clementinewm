/* Clementine Window Manager
   Copyright 2002 Dave Berton <db@mosey.org>

   based on aewm Copyright 1998-2001 Decklin Foster <decklin@red-bean.com>

   This program is free software; see LICENSE for details. */

#ifndef PAINTER_H
#define PAINTER_

#include <X11/Xlib.h>
#include <string>
#include "look.h"

class WindowSystem;
class Painter;
class Look;

class Image
{
public:
    Image( Display* d, Pixmap p );
    Image( const Image& other ); // destructive
    Image& operator=( const Image& other ); // destructive
    ~Image();
    friend class Painter;
private:
    Display* dpy;
    Pixmap pixmap;
};


class Painter
{
public:
    ~Painter();

    void setTitlebarBackground( Drawable d, bool active, int width, 
				int height );
    enum Button {
	CLOSE_BUTTON,
	ICONIZE_BUTTON
    };
    void setButtonBackground( Drawable d, Button b, bool active, bool down,
			      int width, int height );
    void setBorderBackground( Drawable d, bool active, 
			      int width, int height, int titleHeight );

    void setActiveColors();
    void setInactiveColors();

    void drawString( Drawable d, bool active,
		     int x, int y, const std::string& s );

    int fontAscent() const { return font->ascent; }
    int fontDescent() const { return font->descent; }
    int textWidth( const std::string& t ) { // db more move this
	return XTextWidth( font, t.c_str(), t.length()) + 4;
    }

    void rubberBand( int x, int y, 
		     unsigned int width, unsigned int height );

    void clear( Window w ) { XClearWindow( dpy, w ); }

    std::string rootCommand() const { return look->rootCommand(); }

private:
    Painter( Display* d, Window r, int s );
    Painter( const Painter& p );

    Display*     dpy;
    Window       root;
    int          screen;
    Look*        look;
    GC           gc;
    GC           invert_gc;
    XColor       active_fg;
    XColor       active_bg;
    XColor       inactive_fg;
    XColor       inactive_bg;

    XFontStruct* font;
    
    void setForeground( int r, int g, int b );
    void setForeground( unsigned long pixel );
    void setBackground( unsigned long pixel );
    void setForegroundBackground( unsigned long foreground, 
				  unsigned long background );



    void drawLine( Drawable d, int x1, int y1, int x2, int y2 );
    void drawPoint( Drawable d, int x, int y );

    void clearArea( Window w, int x, int y, int width, int height ) {
	XClearArea( dpy, w, x, y, width, height, false );
    }

    Image createBlankImage( int width, int height );

    void drawGradient( Drawable d, 
		       int x, int y,
		       unsigned int in_width, unsigned int in_height, 
		       ColorSet cs,
		       bool horizontal, bool ascend );
    void drawFrame( Drawable d, ColorSet cs, bool reverse, 
		     int x, int y, int width, int height );
    void drawFill( Drawable d, int x, int y, 
		   int width, int height );

    void drawRectangle( Drawable d,
			int x, int y, int width, int height );
    void drawString( Drawable d, RGBColor c, 
		     int x, int y, const std::string& s );

    void setBackground( Drawable w, const Image& image );

    void drawCloseControl( Drawable d, bool active,
			   int x, int y, 
			   int width, int height );
    void drawIconizeControl( Drawable d, bool active, 
			     int x, int y, 
			     int width, int height );

    void drawBackground( Drawable d, Facet* facet,
			 bool down, 
			 int width, int height );

    friend class WindowSystem;


};

#endif
