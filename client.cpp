/* Clementine Window Manager
   Copyright 2002 Dave Berton <db@mosey.org>

   based on aewm Copyright 1998-2001 Decklin Foster <decklin@red-bean.com>

   This program is free software; see LICENSE for details. */

#include "client.h"
#include "windowsystem.h"
#include "painter.h"
#include <iostream>
#include <assert.h>
#include <iostream> // ## db more
#include <cstdlib>

#define PIXELS 0
#define INCREMENTS 1

Client::Client( WindowSystem* windowsystem, Window w, bool viewable,
		int wx, int wy, int wwidth, int wheight,
		long wcolormap )
    : ws(windowsystem)
{
    assert(ws);
    ws->transientForHint( w, &trans );
    nm = ws->windowName( w );
    window = w;
    ignore_unmap = 0;
    x = wx;
    y = wy;
    width = wwidth;
    height = wheight;
    cmap = wcolormap;
    size = ws->allocSizeHints();
    active = false;
    awaiting_close = false;
    awaiting_iconize = false;

    long dummy;
    ws->windowNormalHints( window, size, &dummy );

    if ( viewable ) {
	ignore_unmap++;
    } else {
	initPosition();
	ws->setState(window, NormalState);
	// consolidate
	XWMHints *hints;
	if ((hints = ws->windowHints(w))) {
	    if (hints->flags & StateHint) 
		ws->setState(window, hints->initial_state);
	    ws->xFree(hints);
	}
    }

    gravitate( APPLY_GRAVITY );

    frame = ws->createFrame( x-1, y - titleHeight(), 
			     width + 2,  // 1 pixel border, each side
			     height + titleHeight() + 1 ); // 1 pixel border, top
    button_size = titleHeight() - 2;
    titlebar_width = width - 2*button_size;

    close_button = ws->createWindow( 0,0, button_size, button_size );
    iconize_button = ws->createWindow( 0,0, button_size, button_size );
    titlebar = ws->createWindow( 0,0, titlebar_width, button_size );

    ws->addToSaveSet( window );
    ws->selectInput( window, ColormapChangeMask|PropertyChangeMask);
    ws->grabButtons( window );
    ws->setBorderWidth( window, 0);
    ws->resize( window, width, height); //db more needed?

    ws->reparent( close_button, frame, width + 1 - button_size, 1 );
    ws->reparent( iconize_button, frame, 1, 1 );
    ws->reparent( titlebar, frame, button_size + 1, 1 );
    ws->reparent( window, frame, 1, titleHeight()); // for 1 pixel border
    configure();

    // db more fix: initial iconized windows
    if ( viewable ) {
	if ( ws->getState(window) == IconicState ) {
	    ignore_unmap++;
	    ws->unmap( window );
	} else {
	    mapRaised();
	}
    } else {
	if ( ws->getState(window) == NormalState ) {
	    ws->map( window );
	    ws->map( close_button );
	    ws->map( iconize_button );
	    ws->map( titlebar );
	    ws->mapRaised( frame );
	}
    }
    redraw( false, true );
    
}

Client::~Client()
{
//    if ( ignore_unmap ) 
//	throw ( "Client withdrawing directly!" ); // ## db more
    ws->setState( window, WithdrawnState );
    gravitate( REMOVE_GRAVITY );
    ws->setBorderWidth( window, 1 );
    ws->removeFromSaveSet( window );
    ws->reparentToRoot( window, x, y);
    ws->destroy( close_button );
    ws->destroy( iconize_button );
    ws->destroy( titlebar );
    ws->destroy( frame );
    if (size) 
	ws->xFree(size);
}

int Client::titleHeight() const
{
    return ws->painter()->fontAscent() + 
	ws->painter()->fontDescent() +2*ws->pad();
    return ( trans ? 3 : 
	     ws->painter()->fontAscent() + 
	     ws->painter()->fontDescent() +2*ws->pad());
}


/* Window gravity is a mess to explain, but we don't need to do much
 * about it since we're using X borders. For NorthWest et al, the top
 * left corner of the window when there is no WM needs to match up
 * with the top left of our fram once we manage it, and likewise with
 * SouthWest and the bottom right (these are the only values I ever
 * use, but the others should be obvious.) Our titlebar is on the top
 * so we only have to adjust in the first case. */
void Client::gravitate( Client::GravityType type )
{
    int dy = 0;
    int gravity = (size->flags & PWinGravity) ?
	size->win_gravity : NorthWestGravity;
    
    switch (gravity) {
    case NorthWestGravity:
    case NorthEastGravity:
    case NorthGravity: 
	dy = titleHeight(); 
	break;
    case CenterGravity: 
	dy = titleHeight()/2; 
	break;
    }
    
    y += type * dy;
}


/* Figure out where to map the window. c->x, c->y, c->width, and
 * c->height actually start out with values in them (whatever the
 * client passed to XCreateWindow).  Program-specified hints will
 * override these, but anything set by the program will be
 * sanity-checked before it is used. PSize is ignored completely,
 * because GTK sets it to 200x200 for almost everything. User-
 * specified hints will of course override anything the program says.
 *
 * If we can't find a reasonable position hint, we make up a position
 * using the mouse co-ordinates and window size. If the mouse is in
 * the center, we center the window; if it's at an edge, the window
 * goes on the edge. To account for window gravity while doing this,
 * we add theight into the calculation and then degravitate. Don't
 * think about it too hard, or your head will explode.
 *
 * If we are using the gnome_pda hint, the entire process is done over
 * a smaller "pretend" root window, and then at the very end we shift
 * the window into the right place based using the left/top offsets. */

void Client::initPosition()
{
    int xmax = ws->displayWidth();
    int ymax = ws->displayHeight();;

    if (size->flags & (USSize)) {
	if (size->width) width = size->width;
	if (size->height) height = size->height;
    } else {
	/* make sure it's big enough to click at */
	if (width < 2 * titleHeight()) width = 2 * titleHeight();
	if (height < titleHeight()) height = titleHeight();
    }

    if (size->flags & USPosition) {
	x = size->x;
	y = size->y;
    } else {
	if (size->flags & PPosition) {
	    x = size->x;
	    y = size->y;
	}
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x > xmax) x = xmax - titleHeight();
	if (y > ymax) y = ymax - titleHeight();
	
	if (x == 0 && y == 0) {
	    int mouse_x, mouse_y;
	    ws->pointerPosition(&mouse_x, &mouse_y);
	    if (width < xmax)
		x = int( (mouse_x < xmax ? 
			(mouse_x / (float)xmax) : 1)
		    * (xmax - width - 
		       2 ) );
	    if (height + titleHeight() < ymax)
		y = int( (mouse_y < ymax ? 
			(mouse_y / (float)ymax) : 1)
		    * (ymax - height - titleHeight() - 
		       2) );
	    y += titleHeight();
	    gravitate( REMOVE_GRAVITY );
	}
    }
    
}

void Client::iconize()
{
//    if ( !ignore_unmap ) // db more this used to be required, but seems to not be anymore.  hmm.
//	ignore_unmap++;
    ws->unmap( frame );
    ws->unmap( close_button );
    ws->unmap( iconize_button );
    ws->unmap( titlebar );
    ws->unmap( window );
    ws->iconize( window );
}

void Client::uniconize()
{
    mapRaised();
}

void Client::map()
{
    ws->map( window );
    ws->map( close_button );
    ws->map( iconize_button );
    ws->map( titlebar );
}

void Client::mapRaised()
{
    map();
    ws->mapRaised( frame );
    ws->setState( window, NormalState );
}

void Client::redraw( bool act, bool force )
{
    bool cleared = false;
    if ( force || act != active ) {
	active = act;
	ws->painter()->setBorderBackground( frame, active,
					    width + 2, 
					    height + titleHeight() + 1,
					    titleHeight() );
	ws->painter()->setTitlebarBackground( titlebar, active, 
					      titlebar_width, 
					      button_size);
	ws->painter()->setButtonBackground( close_button, 
					    Painter::CLOSE_BUTTON,
					    active,
					    false,
					    button_size,
					    button_size );
	ws->painter()->setButtonBackground( iconize_button, 
					    Painter::ICONIZE_BUTTON,
					    active,
					    false,
					    button_size,
					    button_size );
	cleared = true;
    }
    if (!trans && name().length()) {
	if ( !cleared ) 
	    ws->painter()->clear( titlebar );
	ws->painter()->drawString( titlebar, 
				   active,
				   ws->pad(), 
				   ws->painter()->fontAscent() + ws->pad(),
				   name());
    }
    ws->sync();
}

void Client::takeFocus()
{
    ws->setInputFocus( window );
    ws->installColormap( cmap );
}

void Client::installColormap( long colormap )
{
    cmap = colormap;
    ws->installColormap( cmap );
}

long Client::state()
{
    return ws->getState( window );
}

void Client::move()
{
    ws->move( frame, x, y - titleHeight() );
    configure();
} 

void Client::moveResize()
{
    titlebar_width = width - 2*button_size;
    ws->moveResize( frame, x-1, 
		    y - titleHeight(), 
		    width + 2, height + titleHeight() + 1);
    ws->moveResize( close_button, width + 1 - button_size, 1, 
		    button_size, button_size);
    ws->moveResize( iconize_button, 1, 1, 
		    button_size, button_size);
    ws->moveResize( titlebar, button_size + 1, 1, 
		    titlebar_width, button_size);
    ws->moveResize( window, 1, titleHeight(), 
		    width, height);
    configure();
}

void Client::configure()
{
    ws->sendConfigureEvent( window, x, y,
			    width, height );
    // db more needed?
//    ws->sendConfigureEvent( close_button, width + 1 - button_size, 1, 
//			    button_size, button_size );
//   ws->sendConfigureEvent( iconize_button_button, 1, 1, 
//			    button_size, button_size );
}

void Client::raise()
{
    ws->raise( frame );
}

void Client::lower()
{
    ws->lower( frame );
}

void Client::takeConfigureRequest( int x1, int y1,
				   unsigned int width1, unsigned int height1,
				   unsigned int valuemask,
				   Window sibling, int stack_mode )
{
    gravitate( REMOVE_GRAVITY );
    if (valuemask & CWX) x = x1;
    if (valuemask & CWY) y = y1;
    if (valuemask & CWWidth) width = width1;
    if (valuemask & CWHeight) height = height1;
    gravitate( APPLY_GRAVITY );
    /* configure the frame */
    ws->configure( frame, x, y - titleHeight(),
		   width, height + titleHeight(),
		   valuemask, sibling, stack_mode );
    configure();
}

void Client::drag()
{
    XEvent ev;
    int x1, y1;
    int old_cx = x;
    int old_cy = y;    
    if (!ws->grabPointer(ws->moveCursor())) 
	return;
    ws->pointerPosition(&x1, &y1);
    ws->grabServer();
    bool motion = false;
    for (;;) {
	ws->waitForMouse( &ev );
	switch (ev.type) {
	case MotionNotify:
	    if ( !motion ) {
		drawOutline();
		motion = true;
	    }
	    drawOutline(); /* clear */
	    x = old_cx + (ev.xmotion.x_root - x1);
	    y = old_cy + (ev.xmotion.y_root - y1);
	    drawOutline(); /* redraw */
	    break;
	case ButtonRelease:
	    if ( motion )
		drawOutline(); /* clear */
	    ws->ungrabServer();
	    ws->ungrabPointer();
	    move();
	    return; /* get out of loop */
	}
    }
}

void Client::drawOutline()
{
    ws->painter()->rubberBand( x-1, 
			       y - titleHeight(),
			       width+1, 
			       height + titleHeight() );
}

void Client::resize()
{
    XEvent ev;
    int old_cx = x;
    int old_cy = y;
    
    if (!ws->grabPointer(ws->resizeCursor())) 
	return;
    ws->grabServer();    

    int	pointer_x;
    int	pointer_y;
    ws->pointerPosition(&pointer_x, &pointer_y);
    int offset_x = x + width - pointer_x;
    int offset_y = y + height - pointer_y;
    drawOutline();
    for (;;) {
	ws->waitForMouse( &ev );
	switch (ev.type) {
	case MotionNotify:
	    drawOutline(); /* clear */
	    x = ev.xmotion.x_root + offset_x;
	    y = ev.xmotion.y_root + offset_y;
	    recalcResize(old_cx, old_cy, 
			 x, y );
	    drawOutline();
	    break;
	case ButtonRelease:
	    drawOutline(); /* clear */
	    moveResize();
	    redraw( active, true );
	    ws->ungrabServer();
	    ws->ungrabPointer();
	    return; /* get out of loop */
	}
    }
}

void Client::recalcResize(int x1, int y1, int x2, int y2)
{
    width = abs(x1 - x2) - 2;
    height = abs(y1 - y2) - 2;
    
    getIncSize( &width, &height, PIXELS);
    
    if (size->flags & PMinSize) {
	if (width < size->min_width) 
	    width = size->min_width;
	if (height < size->min_height) 
	    height = size->min_height;
    }
    
    if (size->flags & PMaxSize) {
	if (width > size->max_width) 
	    width = size->max_width;
	if (height > size->max_height) 
	    height = size->max_height;
    }
    
    x = (x1 <= x2) ? x1 : x1 - width;
    y = (y1 <= y2) ? y1 : y1 - height;
}

/* If the window in question has a ResizeInc int, then it wants to be
 * resized in multiples of some (x,y). Here we set x_ret and y_ret to
 * the number of multiples (if mode == INCREMENTS) or the correct size
 * in pixels for said multiples (if mode == PIXELS). */

int Client::getIncSize( int *x_ret, int *y_ret, int mode)
{
    int basex, basey;
    
    if (size->flags & PResizeInc) {
	basex = (size->flags & PBaseSize) ? size->base_width :
	    (size->flags & PMinSize) ? size->min_width : 0;
	basey = (size->flags & PBaseSize) ? size->base_height :
	    (size->flags & PMinSize) ? size->min_height : 0;
	if (mode == PIXELS) {
	    *x_ret = width - ((width - basex) % size->width_inc);
	    *y_ret = height - ((height - basey) % size->height_inc);
	} else /* INCREMENTS */ {
	    *x_ret = (width - basex) / size->width_inc;
	    *y_ret = (height - basey) / size->height_inc;
	}
	return 1;
    }
    
    return 0;
}

bool Client::checkUnmap()
{
    if ( ignore_unmap ) {
	ignore_unmap--;
	return false;
    } else {
	return true;
    }
}

void Client::takeButtonPress( Window w, unsigned int button, bool mod1,
			      int x, int y)
{
    if ( w == close_button ) {
	awaiting_close = true;
	ws->painter()->setButtonBackground( close_button,
					    Painter::CLOSE_BUTTON,
					    active,
					    true,
					    button_size, 
					    button_size );
    }
    if ( w == iconize_button ) {
	awaiting_iconize = true;
	ws->painter()->setButtonBackground( iconize_button,
					    Painter::ICONIZE_BUTTON,
					    active,
					    true,
					    button_size,
					    button_size );
    }
    if ( w == titlebar || w == frame ) {
	raise();
	redraw( true, true ); 
	if ( button == 1 && !awaiting_close && !awaiting_iconize )
	    drag();
    } else if ( isWindow( w ) ) { 
	if ( mod1 ) { /* mod click */
	    switch (button) {
	    case 1:
		raise();
		redraw( true, true ); 
		drag();
		break;
	    case 2:
		break;
	    case 3:
 		raise();
		redraw( true, true ); 
		resize();
		break;
	    }
	}
    }
}

void Client::takeButtonRelease( Window w, unsigned int button, bool mod1,
				int x, int y )
{
    if ( w == close_button ) {
 	if ( awaiting_close &&
	    ( x > 0 && y > 0 && x < button_size && y < button_size )) {
	    ws->tryDelete( window );
	}
	ws->painter()->setButtonBackground( close_button, 
					    Painter::CLOSE_BUTTON,
					    active, 
					    false,
					    button_size,
					    button_size );
	awaiting_close = false;
    }
    if ( w == iconize_button ) {
 	if ( awaiting_iconize &&
	    ( x > 0 && y > 0 && x < button_size && y < button_size )) {
	    iconize();
	}
	ws->painter()->setButtonBackground( iconize_button, 
					    Painter::ICONIZE_BUTTON,
					    active, 
					    false,
					    button_size,
					    button_size );
	awaiting_iconize = false;
    }
    if ( !isFrame( w ) )
    {
	awaiting_close = false;
	awaiting_iconize = false;
    }
}
