/* Clementine Window Manager
   Copyright 2002 Dave Berton <db@mosey.org>

   based on aewm Copyright 1998-2001 Decklin Foster <decklin@red-bean.com>

   This program is free software; see LICENSE for details. */


#ifndef WINDOWSYSTEM_H
#define WINDOWSYSTEM_H

#include <string>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

class Painter;  // forward declare, for drawing
class Keys;     // forward declare. for key bindings

class WindowSystem
{
public:
    WindowSystem();
    virtual ~WindowSystem();

    void shutdown();

    Painter* painter() { return paint; }

    void setQuitting( bool q ) { quit = q;}
    bool quitting() const { return quit; }

    void eventLoop();

    void grabServer() { XGrabServer(dpy); }
    void sync() { XSync(dpy, False); }
    void flush() { XFlush(dpy); }
    void ungrabServer() { XUngrabServer(dpy); }
    int displayWidth() { return DisplayWidth(dpy, screen); }
    int displayHeight() { return DisplayHeight(dpy, screen); }

    void setInputFocus( Window w ) { 
	XSetInputFocus( dpy, w, RevertToPointerRoot, CurrentTime);
    }
    void installColormap( long colormap ) {
	XInstallColormap( dpy, colormap );
    }

    bool grabPointer( Cursor c );
    void ungrabPointer();
    void pointerPosition( int* x, int* y );
    void waitForMouse( XEvent* ev );
   
    void transientForHint( Window w, Window* prop_window_return ) {
	XGetTransientForHint(dpy, w, prop_window_return);
    }
    std::string windowName( Window w );

    void windowNormalHints( Window w, XSizeHints* hints_return, 
			    long* supplied_return ) {
	XGetWMNormalHints(dpy, w, hints_return, supplied_return );
    }
    XWMHints* windowHints( Window w ) {
	return XGetWMHints(dpy, w);
    }
    
    void unmap( Window w ) {
	XUnmapWindow(dpy, w);
    }
    void map( Window w ) {
	XMapWindow(dpy, w);
    }
    void mapRaised( Window w ) {
	XMapRaised(dpy, w);
    }
    Window createFrame( int x, int y,
			unsigned int width, unsigned int height );
    Window createMenu();
    Window createWindow( int x, int y, 
			 unsigned int width, unsigned int height );

    void setBackPixmapParentRelative( Window w );
    
    void addToSaveSet( Window w ) {
	XAddToSaveSet(dpy, w);
    }
    void removeFromSaveSet( Window w ) {
	XRemoveFromSaveSet( dpy, w );
    }
    void selectInput( Window w, long event_mask ) {
	XSelectInput(dpy, w, event_mask);
    }
    void grabButtons( Window w ) {
	//###### db more
	XGrabButton(dpy, Button1, Mod1Mask, w, True,
		    ButtonReleaseMask | ButtonMotionMask, GrabModeAsync,
		    GrabModeAsync, None, None);
	XGrabButton(dpy, Button3, Mod1Mask, w, True,
		    ButtonReleaseMask | ButtonMotionMask, GrabModeAsync,
		    GrabModeAsync, None, None);
    }
    void setBorderWidth( Window w, unsigned int width ) {
	XSetWindowBorderWidth(dpy, w, width);
    }
    void resize( Window w, unsigned int width, unsigned int height ) {
	XResizeWindow(dpy, w, width, height);
    }
    void reparent( Window w, Window parent, int x, int y ) {
	XReparentWindow(dpy, w, parent, x, y);
    }
    void reparentToRoot( Window w, int x, int y ) {
	reparent( w, root, x, y );
    }
    void move( Window w, int x, int y ) {
    	XMoveWindow(dpy, w, x, y);
    }
    void moveResize( Window w, int x, int y,
		     unsigned int width, unsigned int height ){
	XMoveResizeWindow(dpy, w, x, y, width, height);
    }
    void lower( Window w ) {
	(void)XLowerWindow( dpy, w );
    }
    void raise( Window w ) {
	(void)XRaiseWindow( dpy, w );
    }

    void destroy( Window w ) {
	XDestroyWindow( dpy, w );
    }
    void tryDelete( Window w );

    //## db more : improve, use std::vector
    void queryTree( Window* w1, Window* w2, Window** children, 
		    unsigned int* nwins ) {
	XQueryTree(dpy, root, w1, w2, children, nwins);
    }

    //## db more: eliminate
    void xFree( void* data ) {
	XFree( data );
    }

    // ## db more
    XSizeHints* allocSizeHints(){
	return XAllocSizeHints();
    }

    void kill( Window w ) {
	XKillClient( dpy, w );
    }
	
    void sendConfigureEvent( Window w, int x, int y,
			     unsigned int width, unsigned int height );

    Status getProtocols( Window w, Atom** protocols_return, int* count_return){
	return XGetWMProtocols(dpy, w, protocols_return, count_return);
    }
    void setState( Window w, long state );
    long getState( Window w );
    void iconize( Window w );
    void handleXerror( XErrorEvent *e);

    /* visual properties */
    int pad() const { return opt_pad; }
    Cursor moveCursor() const { return move_curs; }
    Cursor resizeCursor() const { return resize_curs; }
    Cursor arrowCursor() const { return arrow_curs; }

    void configure( Window w, int x, int y, 
		    int width, int height,
		    unsigned int valuemask,
		    Window sibling, int stack_mode);

    GC createGC( unsigned long valuemask, XGCValues* value ) {
	return XCreateGC(dpy, root, valuemask, value);
    }
    void freeGC( GC gc ) {
	XFreeGC( dpy, gc );
    }

protected:

    void err( std::string e );


    virtual bool managedWindow( Window w )=0;
    virtual void newWindow( Window w, bool viewable,
			    int x, int y, int width, int height,
			    long colormap) = 0;
    virtual void newIcon( Window w, bool viewable,
			  int x, int y, int width, int height,
			  long colormap) = 0;

    void ignoreErrors();
    void unignoreErrors();

    /* event handling */
    virtual void takeButtonPress( Window w, unsigned int button, 
				  bool mod1, 
				  int x, int y )=0;
    virtual void takeButtonRelease( Window w, unsigned int button, 
				    bool mod1, 
				    int x, int y )=0;
    virtual void takeRootButton( unsigned int button, 
				 bool mod1, 
				 int x, int y )=0;
    virtual void takeConfigureRequest( Window w, int x, int y, 
				       int width, int height,
				       unsigned int valuemask,
				       Window sibling, int stack_mode )=0;
    virtual void takeKeyPress( Window w, unsigned int state,
			       KeySym keysym )=0; 
    virtual void takeMap( Window w )=0; 
    virtual void takeUnmap( Window w )=0;
    virtual void takeDestroy( Window w )=0;
    virtual void takeFocus( Window w )=0; 
    virtual void takeNewColormap( Window w, long colormap )=0; 
    virtual void takeNameChange( Window w, std::string name )=0;
    virtual void takeExpose( Window w )=0; 
    virtual void takeMotion( Window w, int x, int y )=0;

private:
    Display*     dpy;
    Window       root;
    int          screen;
    Painter*     paint;
    Keys*        keys;
    Cursor       move_curs;
    Cursor       resize_curs;
    Cursor       arrow_curs;
    Atom         wm_protos;
    Atom         wm_delete;
    Atom         wm_state;
    Atom         wm_change_state;
    int          opt_pad;
    bool         quit;

    int sendDeleteMessage( Window w );
    void scanWindows();


#ifdef DEBUG
    void showEvent(XEvent e);
#endif

    virtual void handleButtonPress(XButtonEvent *);
    virtual void handleButtonRelease(XButtonEvent *);
    virtual void handleConfigure(XConfigureRequestEvent *) ;
    virtual void handleKeyPress(XKeyEvent *) ;
    virtual void handleMap(XMapRequestEvent *) ;
    virtual void handleUnmap(XUnmapEvent *) ;
    virtual void handleDestroy(XDestroyWindowEvent *) ;
    virtual void handleClientMessage(XClientMessageEvent *) ;
    virtual void handlePropertyChange(XPropertyEvent *);
    virtual void handleEnter(XCrossingEvent *);
    virtual void handleColormapChange(XColormapEvent *) ;
    virtual void handleExpose(XExposeEvent *) ;
    virtual void handleMotion(XMotionEvent *) ;

};



#endif
