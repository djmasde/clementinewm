/* Clementine Window Manager
   Copyright 2002 Dave Berton <db@mosey.org>

   based on aewm Copyright 1998-2001 Decklin Foster <decklin@red-bean.com>

   This program is free software; see LICENSE for details. */

#include "painter.h"
#include <cassert>
#include <algorithm>
#include <cmath>
#include <iostream>

#define DEF_FONT "fixed"
#define DEF_ACTIVE_FG   "white"
#define DEF_ACTIVE_BG   "slategrey"
#define DEF_INACTIVE_FG "black"
#define DEF_INACTIVE_BG   "grey"

Image::Image( Display* d, Pixmap p )
    : dpy(d), pixmap(p)
{
    assert(dpy);
}

Image::Image( const Image& other )
    : dpy(other.dpy), pixmap(other.pixmap)
{
    ((Image&)other).pixmap = None;
}

Image::~Image()
{
    if ( pixmap != None )
	XFreePixmap( dpy, pixmap );
}

Image& Image::operator=( const Image& other )
{
    dpy = other.dpy;
    pixmap = other.pixmap;
    ((Image&)other).pixmap = None;
    return *this;
}

///////////

Painter::Painter( Display* d, Window r, int s )
    : dpy(d), root(r), screen(s)
{
    assert(dpy);

    /* font */
    const char* opt_font = DEF_FONT;
    font = XLoadQueryFont(dpy, opt_font);
    assert(font);


    /* GCs */
    const char* opt_active_fg = DEF_ACTIVE_FG;
    const char* opt_active_bg = DEF_ACTIVE_BG;
    const char* opt_inactive_fg = DEF_INACTIVE_FG;
    const char* opt_inactive_bg = DEF_INACTIVE_BG;
    
    XGCValues gv;
    XColor dummyc;
    XAllocNamedColor(dpy, DefaultColormap(dpy, screen), opt_active_fg, 
		     &active_fg, &dummyc);
    XAllocNamedColor(dpy, DefaultColormap(dpy, screen), opt_active_bg, 
		     &active_bg, &dummyc);
    XAllocNamedColor(dpy, DefaultColormap(dpy, screen), opt_inactive_fg, 
		     &inactive_fg, &dummyc);
    XAllocNamedColor(dpy, DefaultColormap(dpy, screen), opt_inactive_bg, 
		     &inactive_bg, &dummyc);
    // db more error checking for above 

    gv.function = GXcopy;
    gv.foreground = active_fg.pixel;
    gv.background = active_bg.pixel;
    gv.font = font->fid;
    gv.line_width = 1;
    gc = XCreateGC(dpy, root, 
		   GCFunction|GCForeground|GCBackground|GCFont|GCLineWidth, 
		   &gv);
    gv.function = GXinvert;
    gv.subwindow_mode = IncludeInferiors;
    invert_gc = XCreateGC(dpy, root, GCFunction|GCForeground|
			  GCSubwindowMode|GCLineWidth|GCFont, 
			  &gv);

    assert(gc);
    assert(invert_gc);
    look = new Look( dpy, screen );

}

Painter::~Painter()
{
    assert(look);
    delete look;
    XFreeFont(dpy, font);
    XFreeGC( dpy, gc );
    XFreeGC(dpy, invert_gc);
}

void Painter::setForeground( int r, int g, int b )
{
    RGBColor c = look->colorManager()->alloc( r, g, b );
    setForeground( c.pixel() );
}

void Painter::rubberBand( int x, int y, 
			  unsigned int width, unsigned int height )
{
    XDrawRectangle(dpy, root, invert_gc,
		   x, y, width, height );
}

void Painter::drawRectangle( Drawable d, 
			     int x, int y, int width, int height )
{
    XDrawRectangle(dpy, d, gc,
		   x, y, width, height );
}
    
void Painter::setForeground( unsigned long pixel )
{
    XSetForeground( dpy, gc, pixel );
}

void Painter::setBackground( unsigned long pixel )
{
    XSetBackground( dpy, gc, pixel );
}

void Painter::setForegroundBackground( unsigned long foreground, 
				       unsigned long background )
{
    setForeground( foreground );
    setBackground( background );
}

void Painter::setActiveColors()
{
    setForegroundBackground( active_fg.pixel, active_bg.pixel );
}

void Painter::setInactiveColors()
{
    setForegroundBackground( inactive_fg.pixel, inactive_bg.pixel );
}
    

void Painter::drawGradient( Drawable d,
			    int in_x, int in_y,
			    unsigned int in_width, unsigned int in_height, 
			    ColorSet cs,
			    bool horizontal, bool ascend )
{
    static int NCOLORSHADES=128; // this many shades in gradient

    int cxCap = in_width;
    int cyCap = in_height;

    // Get the intensity values for the ending colormap
    unsigned int r1, g1, b1;
    r1 = g1 = b1 = 111;
    cs.dark.RGB( r1, g1, b1 );
    
    // Get the intensity values for the begining color
    unsigned int r2, g2, b2;
    r2 = g2 = b2 = 111;
    cs.light.RGB( r2, g2, b2 );

    unsigned int r, g, b;
    r = g = b = 0;

/*
    diagonal
    int x = 0;
    int y = 0;
    int width = cxCap;
    int height = cyCap;
    int xDelta = std::max(width/NCOLORSHADES,1);
    int yDelta = std::max(height/NCOLORSHADES,1);
    for ( ; x < cxCap; x += xDelta ) {
	for ( y = 0; y < cyCap; y += yDelta ) {
	    r = (( r1 + (( r2 - r1) / 2.0)
		   * (((double) x / (double) width)
		      + ((double) y / (double) height)))
		 + 0.5);
	    g = (( g1 + (( g2 - g1) / 2.0)
		   * (((double) x / (double) width)
		      + ((double) y / (double) height)))
		 + 0.5);
	    b = ((b1 + ((b2 - b1) / 2.0)
		  * (((double) x / (double) width)
		     + ((double) y / (double) height)))
		 + 0.5);
	    setForeground( r, g, b );
	    fillRectangle( image.pixmap, x, y,
			   xDelta, yDelta );
	}
    }
    return;

*/
    if( horizontal )  { //paint horizontal rect;
	int x = cxCap + std::max(cxCap/NCOLORSHADES,1);	
	int w = x; // width of area to shade
	int xDelta= std::max(w/NCOLORSHADES,1);	// width of one shade band
	while ( x > in_x - xDelta ) {
	    x -= xDelta;
	    if (r1 > r2)
		r = r1 - (r1-r2)*(w-x)/w;
	    else
		r = r1 + (r2-r1)*(w-x)/w;
	    
	    if (g1 > g2)
		g = g1 - (g1-g2)*(w-x)/w;
	    else
		g = g1 + (g2-g1)*(w-x)/w;
	    
	    if (b1 > b2)
		b = b1 - (b1-b2)*(w-x)/w;
	    else
		b = b1 + (b2-b1)*(w-x)/w;

	    setForeground( r, g, b );
            if(ascend) { // Paint from  left to right;
		drawFill( d, x, in_y,
			  xDelta, cyCap );
	    } else  {   // Paint from  right to left;
		drawFill( d, in_width-x-xDelta, in_y,
			       xDelta, cyCap );
	    }
	}
    } else  {   //paint vertical rect;
	int y = cyCap + std::max(cyCap/NCOLORSHADES,1);	
	int w = y; // height of area to shade
	int yDelta= std::max(w/NCOLORSHADES,1);	// height of one shade band
	    //while (y >= yDelta) {
        while (y > in_y - yDelta) {
	    y -= yDelta;
	    if (r1 > r2)
		r = r1 - (r1-r2)*(w-y)/w;
	    else
		r = r1 + (r2-r1)*(w-y)/w;
	    
	    if (g1 > g2)
		g = g1 - (g1-g2)*(w-y)/w;
	    else
		g = g1 + (g2-g1)*(w-y)/w;
	    
	    if (b1 > b2)
		b = b1 - (b1-b2)*(w-y)/w;
	    else
		b = b1 + (b2-b1)*(w-y)/w;
	    
	    setForeground( r, g, b );
            if(ascend) { // Paint from top to bottom
		drawFill( d, in_x, y,
			       cxCap, yDelta );
	    } else  {   // Paint from bottom to top
		drawFill( d, in_x, in_height-y-yDelta,
			       cxCap, yDelta );
	    }
	}
    }


}

void Painter::drawFrame( Drawable d, ColorSet cs, bool reverse, 
			  int x, int y, int width, int height )
{
    if ( reverse )
	setForeground( cs.light.pixel() );
    else
	setForeground( cs.dark.pixel() );
    drawLine( d, x, y+height-1, x+width-1, y+height-1);
    drawLine( d,  x+width-1, y+1, x+width-1, y+height);
    if ( reverse )
	setForeground( cs.dark.pixel() );
    else
	setForeground( cs.light.pixel() );
    drawLine( d, x, y, x+width-1, y);
    drawLine( d, x, y+1, x, y+height);
    setForeground( cs.color.pixel() );
    drawPoint( d, x + width - 1, y );
    drawPoint( d, x, y + height-1 );
}

void Painter::drawFill( Drawable d, 
			int x, int y,
			int width, int height )
{
    XFillRectangle(dpy, d, gc, x, y, width, height);
}

void Painter::setBackground( Drawable w, const Image& image )
{
    XSetWindowBackgroundPixmap(dpy, w, image.pixmap );
    XClearWindow(dpy,w);
}


Image Painter::createBlankImage( int width, int height )
{
    Pixmap pixmap = XCreatePixmap(dpy,root, width,
				  height, DefaultDepth(dpy, screen) );
    if (pixmap == None) {
	std::cerr << "Painter::createBlankImage unable to create pixmap" << 
	    std::endl;
	return Image( dpy, None );
    }
    return Image(dpy,pixmap);
}

void Painter::drawString( Drawable d, bool active,
			  int x, int y, const std::string& s )
{
    RGBColor c;
    if ( active )
	c = look->activeFacet()->color().color;
    else
	c = look->inactiveFacet()->color().color;
    drawString( d, c, x, y, s );
}

void Painter::drawString( Drawable d, RGBColor c, 
			  int x, int y, const std::string& s )
{
    setForeground( c.pixel() );
    XDrawString( dpy, d, gc, x, y, (char*)s.c_str(), s.length() );
}


void Painter::drawBackground( Drawable d, Facet* facet,
			      bool down,
			      int width, int height )
{
    assert( facet );
    ColorSet cs;
    Style style;
    style = facet->style();
    int borderOffset = 0;
    if ( style.isFlat() ) {
	cs = facet->background();
	setForeground( cs.color.pixel() );
	drawFill( d, borderOffset, borderOffset,
		  width - 2*borderOffset, height );
    }
    if ( style.isGradient() ) {
	cs = facet->color();
	drawGradient( d, borderOffset, borderOffset,
		      width-(2*borderOffset), height, 
		      cs, style.isHorizontal(), style.isAscending() );
    }
    if ( style.isBeveled() ) {
	cs = facet->background();
	drawFrame( d, cs, down, 0, 0, width, height );
    }
}

void Painter::setTitlebarBackground( Drawable d, bool active, 
				     int width, int height )
{
    Image image = createBlankImage( width, height );
    Facet* f;
    if ( active )
	f = look->activeFacet();
    else 
	f = look->inactiveFacet();
    drawBackground( image.pixmap, f, false, width, height );
    setBackground( d, image );
}   

void Painter::setBorderBackground( Drawable d, bool active, 
				   int width, int height, int titleHeight )
{
    Image image = createBlankImage( width, height );
    ColorSet cs;
    if ( active )
	cs = look->activeFacet()->border();
    else
	cs = look->inactiveFacet()->border();
    setForeground( cs.color.pixel() );
    drawFill( image.pixmap,0,0,width,height);
//    drawRectangle( image.pixmap, 0, 0, width-1, height-1 );
//    drawLine( image.pixmap, 0, titleHeight-1, width, titleHeight-1 );
    setBackground( d, image );
}

void Painter::drawCloseControl( Drawable d, bool active,
				int x, int y, 
				int width, int height )
{
    Facet* f;
    if ( active )
	f = look->activeButtonFacet();
    else 
	f = look->inactiveButtonFacet();
    ColorSet cs = f->color();
    setForeground( cs.color.pixel() );
    drawLine( d, x, y, x + width, y + height );
    drawLine( d, x + 1, y, x + width, y + height - 1 );
    drawLine( d, x, y + 1, x + width -1 , y + height );

    drawLine( d, x, y + height-1, x + width, y-1 );
    drawLine( d, x, y + height - 2, x + width - 1, y-1 );
    drawLine( d, x + 1, y + height-1, x + width, y  );
}

void Painter::drawIconizeControl( Drawable d, bool active,
				  int x, int y, 
				  int width, int height )
{
    ColorSet cs;
    if ( active )
	cs = look->activeButtonFacet()->color();
    else
	cs = look->inactiveButtonFacet()->color();
    if ( std::fmod(width,2) == 0 )
	width--;
    height = width;
    int half = height/2;
    // use bottom half of area
    y += half - int(.2*height); // move down a bit
    height = half;              // reduce height
    setForeground( cs.color.pixel() );
    drawLine( d, x, y, x + std::ceil(width/2) + 1, y + height + 1 );
    drawLine( d, x + 1, y, x + std::ceil(width/2) + 1, y + height );
    drawLine( d, x, y + 1,  x + std::ceil(width/2), y + height + 1 );
    drawLine( d, x + std::ceil(width/2), y + height, x + width, y -1  );
    drawLine( d, x + std::ceil(width/2), y + height-1, x +width-1, y-1 );
    drawLine( d, x + std::ceil(width/2)+1, y + height, x +width, y );
    drawPoint( d, x + std::ceil(width/2), y + height + 1 );
}

void Painter::setButtonBackground( Drawable d, Painter::Button b, bool active, 
				   bool down, int width, int height )
{
    Image image = createBlankImage( width, height );
    Facet* f;
    if ( active )
	f = look->activeButtonFacet();
    else 
	f = look->inactiveButtonFacet();
    /* This will handle the button background and border, 'depressing'
     * the button if down */
    drawBackground( image.pixmap, f, down, width, height );

    /* Paint the control differently if there is no bevel (hence, no
     * way to 'depress' the button), which provides feedback in this
     * case. */
    if ( down && !f->style().isBeveled() )
	active = !active;

    switch( b ) {
    case CLOSE_BUTTON:
	drawCloseControl( image.pixmap, active, 
			  4, 4, width-8, height-8 );
	break;
    case ICONIZE_BUTTON:
	drawIconizeControl( image.pixmap, active, 
			  4, 4, width-8, height-8 );
	break;
    }

    setBackground( d, image );    
}

void Painter::drawLine( Drawable d, int x1, int y1, int x2, int y2 )
{
    XDrawLine( dpy, d, gc, x1, y1, x2, y2 );
}

void Painter::drawPoint( Drawable d, int x, int y )
{
    XDrawPoint( dpy, d, gc, x, y );
}

