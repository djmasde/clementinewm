/* Clementine Window Manager
   Copyright 2002 Dave Berton <db@mosey.org>
   Copyright 2014 Dj_Dexter/djmasde <djdexter@gentoo-es.com>

   based on aewm Copyright 1998-2001 Decklin Foster <decklin@red-bean.com>

   This program is free software; see LICENSE for details. */

#include "windowsystem.h"
#include "painter.h"
#include "keys.h"
#include "look.h"

#include <X11/cursorfont.h>
#include <X11/Xmd.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
//#include <X11/XF86keysym.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlocale.h>

#include <signal.h>
#include <sys/wait.h>

#include <stdio.h>

#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>

#define DEF_PAD  3

// ## db more : get rid of these
#define ChildMask (SubstructureRedirectMask|SubstructureNotifyMask)
#define ButtonMask (ButtonPressMask|ButtonReleaseMask)
#define MouseMask (ButtonMask|PointerMotionMask)

#include <iostream>
#include <sstream>

std::string convertToString(int x)
{
   std::ostringstream o;
   if (o << x)
     return o.str();
   // db more some sort of error handling goes here...
   return "string conversion error";
} 

int handle_xerror(Display *dpy, XErrorEvent *e)
{
//    if ( !windowManager ) {
	char msg[255];
	XGetErrorText(dpy, e->error_code, msg, sizeof msg);
	std::cerr<<"X error:" + std::string(msg) << std::endl;//e->resourceid
	//db more dump core
//	std::cout << ((std::string*)0)->size();
	exit(1);
//    }
	//   windowManager->handleXerror( e );
	// return 0;
}

void quit_nicely(void)
{
    printf("quit_nicely\n");
//    if ( !windowManager ) {
//	std::cerr << "internal error, exiting" << std::endl;
	exit(1);
//    }
//    windowManager->setQuitting( true ); // force graceful exit
//    windowManager->flush();
}

void sig_handler(int signal)
{
    switch (signal) {
    case SIGINT:
    case SIGTERM:
    case SIGHUP:
	quit_nicely(); break;
    case SIGCHLD:
	wait(NULL); break;
    }
}

WindowSystem::WindowSystem()
{
    struct sigaction act;
    act.sa_handler = sig_handler;
    act.sa_flags = 0;
    sigaction(SIGTERM, &act, NULL);
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGHUP, &act, NULL);
    sigaction(SIGCHLD, &act, NULL);

    opt_pad = DEF_PAD;
    quit = false;

    XSetWindowAttributes sattr;

    dpy = XOpenDisplay(NULL);

    if (!dpy) {
	err("clementine: can't open display (is $DISPLAY set properly?)");
	    // ,getenv("DISPLAY"));
	exit(1);
    }

    XSetErrorHandler(handle_xerror);
    screen = DefaultScreen(dpy);
    root = RootWindow(dpy, screen);

    wm_protos = XInternAtom(dpy, "WM_PROTOCOLS", False);
    wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    wm_state = XInternAtom(dpy, "WM_STATE", False);
    wm_change_state = XInternAtom(dpy, "WM_CHANGE_STATE", False);

    move_curs = XCreateFontCursor(dpy, XC_fleur);
    resize_curs = XCreateFontCursor(dpy, XC_sizing);
    arrow_curs = XCreateFontCursor(dpy, XC_left_ptr);

    paint = new Painter( dpy, root, screen );
    keys = new Keys();

    sattr.cursor = arrow_curs;
    sattr.event_mask = ChildMask|ColormapChangeMask|KeyPressMask|ButtonMask;
    XChangeWindowAttributes(dpy, root, CWCursor|CWEventMask, &sattr);

    /* Window cycling */
    XGrabKey( dpy, XKeysymToKeycode(dpy, XK_Tab), Mod1Mask,
	      root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey( dpy, XKeysymToKeycode(dpy, XK_Tab), ( Mod1Mask | ShiftMask ),
	      root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey( dpy, XKeysymToKeycode(dpy, XK_Tab), ControlMask,
	      root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey( dpy, XKeysymToKeycode(dpy, XK_Tab), ( ControlMask | ShiftMask ),
	      root, True, GrabModeAsync, GrabModeAsync);
}

WindowSystem::~WindowSystem()
{
    shutdown();
}

void WindowSystem::shutdown()
{
#ifdef DEBUG
    std::cout << "WindowSystem shutting down" << std::endl;
#endif

    delete paint;
    delete keys;
    XFreeCursor(dpy, move_curs);
    XFreeCursor(dpy, resize_curs);
    XFreeCursor(dpy, arrow_curs);
    

    XInstallColormap(dpy, DefaultColormap(dpy, screen));
    XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
    
    XCloseDisplay(dpy);
}

void WindowSystem::scanWindows()
{
    unsigned int nwins, i;
    Window dummyw1, dummyw2, *wins;
    XWindowAttributes attr;
    XQueryTree(dpy, root, &dummyw1, &dummyw2, &wins, &nwins);
    for (i = 0; i < nwins; i++) {
	XGetWindowAttributes(dpy, wins[i], &attr);
	if (!attr.override_redirect && attr.map_state == IsViewable) {
	    newWindow( wins[i], true, attr.x, attr.y, 
		       attr.width, attr.height, attr.colormap );
	}
    }
    XFree(wins);
}

std::string WindowSystem::windowName( Window w )
{
    char* n;
    if ( XFetchName(dpy, w, &n) ) {
	std::string name( n );
	XFree(n);
	return name;
    }
    return std::string();
}

   /* creates a window parented to the root window */
Window WindowSystem::createFrame( int x, int y, 
				  unsigned int width, unsigned int height )
{
    XSetWindowAttributes pattr;

    pattr.override_redirect = True;
    pattr.background_pixel = BlackPixel( dpy, screen );
    //   pattr.border_pixel = BlackPixel( dpy, screen );
    pattr.event_mask = ChildMask|ButtonPressMask|ButtonReleaseMask|
	ExposureMask|EnterWindowMask|KeyPressMask;
 
    return XCreateWindow(dpy, root,
			 x, y, width, height,
			 0, /* border width */ 
			 DefaultDepth(dpy, screen), 
			 CopyFromParent,
			 DefaultVisual(dpy, screen),
			 CWOverrideRedirect|CWBackPixel|
			 /*CWBorderPixel|*/CWEventMask, &pattr);
}

/* creates a window parented to the root window */
Window WindowSystem::createWindow( int x, int y, 
				  unsigned int width, unsigned int height )
{
    XSetWindowAttributes pattr;

    pattr.override_redirect = True;
    pattr.event_mask = ChildMask|ButtonPressMask|ButtonReleaseMask|
	ExposureMask|EnterWindowMask|KeyPressMask;
 
    Window w = XCreateWindow(dpy, root,
			     x, y, width, height,
			     0, /* border width */ 
			     DefaultDepth(dpy, screen), 
			     CopyFromParent,
			     DefaultVisual(dpy, screen),
			     CWOverrideRedirect|CWEventMask, &pattr);
    setBackPixmapParentRelative( w );
    return w;
}

void WindowSystem::setBackPixmapParentRelative( Window w )
{
//    XSetWindowAttributes sattr;
//    sattr.background_pixmap = ParentRelative;
//    XChangeWindowAttributes(dpy, w, CWBackPixmap, &sattr);
    XSetWindowBackgroundPixmap( dpy, w, None );
    XClearWindow( dpy, w );
}

Window WindowSystem::createMenu()
{
    XSetWindowAttributes pattr;

    pattr.override_redirect = True;
    //pattr.background_pixel = painter()->backgroundColor().pixel;
    pattr.border_pixel = BlackPixel( dpy, screen );
    pattr.event_mask = ChildMask|ButtonPressMask|ButtonMotionMask|
	ExposureMask|EnterWindowMask|PointerMotionMask;
 
    return XCreateWindow(dpy, root,
			 0, 0, 1, 1,
			 0, /* border width */ 
			 DefaultDepth(dpy, screen), 
			 CopyFromParent,
			 DefaultVisual(dpy, screen),
			 CWOverrideRedirect|/*CWBackPixel|*/
			 CWBorderPixel|CWEventMask, &pattr);
}


bool WindowSystem::grabPointer( Cursor c )
{
    return XGrabPointer(dpy, root, False, MouseMask,
			GrabModeAsync, GrabModeAsync, None, 
			c, CurrentTime) == GrabSuccess;
}

void WindowSystem::ungrabPointer() 
{ 
    XUngrabPointer(dpy, CurrentTime); 
}


void WindowSystem::waitForMouse( XEvent* ev )
{
    XMaskEvent(dpy, MouseMask, ev);
}


void WindowSystem::handleXerror( XErrorEvent *e)
{
    err("encountered X error, bailing");
    exit(1);
/*
    Client *c = findClient(e->resourceid, WINDOW);
    if (e->error_code == BadAccess && e->resourceid == root) {
	err("root window unavailible (maybe another wm is running?)");
	exit(1);
    } else {
	char msg[255];
	XGetErrorText(dpy, e->error_code, msg, sizeof msg);
	err("X error:" + std::string(msg));//e->resourceid
    }
    if (c) 
	remove(c, WITHDRAW);
*/
}

int WindowSystem::sendDeleteMessage( Window w)
{
    XEvent e;
    
    e.type = ClientMessage;
    e.xclient.window = w;
    e.xclient.message_type = wm_protos;
    e.xclient.format = 32;
    e.xclient.data.l[0] = wm_delete;
    e.xclient.data.l[1] = CurrentTime;
    
    return XSendEvent(dpy, w, False, NoEventMask, &e);
}

void WindowSystem::tryDelete( Window w )
{
    int i, n, found = 0;
    Atom *protocols;
    if ( getProtocols(w, &protocols, &n) ) {
	for (i=0; i<n; i++) if (protocols[i] == wm_delete) found++;
	xFree(protocols);
    }
    if (found)
	sendDeleteMessage(w);
    else 
	kill(w);
}


void WindowSystem::pointerPosition(int *x, int *y)
{
    Window mouse_root, mouse_win;
    int win_x, win_y;
    unsigned int mask;
    
    XQueryPointer(dpy, root, &mouse_root, &mouse_win,
		  x, y, &win_x, &win_y, &mask);
}

/* Attempt to follow the ICCCM by explicity specifying 32 bits for
 * this property. Does this goof up on 64 bit systems? */

void WindowSystem::setState(Window w, long state)
{
    CARD32 data[2];

    data[0] = state;
    data[1] = None;

    XChangeProperty(dpy, w, wm_state, wm_state,
		    32, PropModeReplace, (unsigned char *)data, 2);
}

void WindowSystem::iconize( Window w )
{
    setState( w, IconicState );
    XWindowAttributes attr;
    XGetWindowAttributes(dpy, w, &attr);
    if ( !attr.override_redirect )
	newIcon( w, (attr.map_state == IsViewable), 
		 attr.x, attr.y, attr.width, attr.height, attr.colormap);
}


/* If we can't find a WM_STATE we're going to have to assume
 * Withdrawn. This is not exactly optimal, since we can't really
 * distinguish between the case where no WM has run yet and when the
 * state was explicitly removed (Clients are allowed to either set the
 * atom to Withdrawn or just remove it... yuck.) */

long WindowSystem::getState(Window w)
{
    Atom real_type; int real_format;
    unsigned long items_read, bytes_left;
    long *data, state = WithdrawnState;

    if (XGetWindowProperty(dpy, w, wm_state, 0L, 2L, False,
			   wm_state, &real_type, &real_format, 
			   &items_read, &bytes_left,
			   (unsigned char **) &data) 
	== Success && items_read) {
	state = *data;
	XFree(data);
    }
    return state;
}


void WindowSystem::sendConfigureEvent( Window w, int x, int y,
				       unsigned int width, 
				       unsigned int height )
{
    XConfigureEvent ce;

    ce.type = ConfigureNotify;
    ce.event = w;
    ce.window = w;
    ce.x = x;
    ce.y = y;
    ce.width = width;
    ce.height = height;
    ce.border_width = 0;
    ce.above = None;
    ce.override_redirect = 0;

    XSendEvent(dpy, w, False, StructureNotifyMask, (XEvent *)&ce);
}

/* used below */
int ignore_xerror(Display */*dpy*/, XErrorEvent */*e*/)
{
    return 0;
}

void WindowSystem::ignoreErrors()
{
    XSetErrorHandler(ignore_xerror);
}

void WindowSystem::unignoreErrors()
{
    XSetErrorHandler(handle_xerror);
}

#ifdef DEBUG

/* Bleh, stupid macro names. I'm not feeling creative today. */

#define SHOW_EV(name, memb) \
case name: s = #name; w = e.memb.window; break;
#define SHOW(name) \
case name: return #name;

void WindowSystem::showEvent(XEvent e)
{
    const char* s;
//    char* buf[20];
    Window w;
    // db more
//    Client *c;

    switch (e.type) {
	SHOW_EV(ButtonPress, xbutton)
	    SHOW_EV(ButtonRelease, xbutton)
	    SHOW_EV(ClientMessage, xclient)
	    SHOW_EV(ColormapNotify, xcolormap)
	    SHOW_EV(ConfigureNotify, xconfigure)
	    SHOW_EV(ConfigureRequest, xconfigurerequest)
	    SHOW_EV(CreateNotify, xcreatewindow)
	    SHOW_EV(DestroyNotify, xdestroywindow)
	    SHOW_EV(EnterNotify, xcrossing)
	    SHOW_EV(Expose, xexpose)
	    SHOW_EV(KeyPress, xkey)
	    SHOW_EV(KeyRelease, xkey)
	    SHOW_EV(MapNotify, xmap)
	    SHOW_EV(MapRequest, xmaprequest)
	    SHOW_EV(MappingNotify, xmapping)
	    SHOW_EV(MotionNotify, xmotion)
	    SHOW_EV(PropertyNotify, xproperty)
	    SHOW_EV(ReparentNotify, xreparent)
	    SHOW_EV(ResizeRequest, xresizerequest)
	    SHOW_EV(UnmapNotify, xunmap)
	    default:
	s = "unknown event"; w = None;
	break;
    }

//    c = findClient(w, WINDOW);
//    snprintf(buf, sizeof buf, c ? c->name : "(none)");
//    err( convertToString(w) + std::string(":") + std::string( buf ) +
//	 std::string(":") + std::string( s ) );

    err( convertToString(w) + std::string(":") +
	 std::string(":") + std::string( s ) );
}
#endif

void WindowSystem::err( std::string e )
{
    std::cerr << e << std::endl;
}

void WindowSystem::eventLoop()
{
    scanWindows();

    XEvent ev;

    for (;;) {
	XNextEvent(dpy, &ev);
	if ( quit )
	    break;
#ifdef DEBUG
	showEvent(ev);
#endif
	switch (ev.type) {
	case ButtonPress:
	    handleButtonPress(&ev.xbutton); break;
	case ButtonRelease:
	    handleButtonRelease(&ev.xbutton); break;
	case ConfigureRequest:
	    handleConfigure(&ev.xconfigurerequest); break;
	case MapRequest:
	    handleMap(&ev.xmaprequest); break;
	case UnmapNotify:
	    handleUnmap(&ev.xunmap); break;
	case KeyPress:
	    handleKeyPress(&ev.xkey); break;
	case DestroyNotify:
	    handleDestroy(&ev.xdestroywindow); break;
	case ClientMessage:
	    handleClientMessage(&ev.xclient); break;
	case ColormapNotify:
	    handleColormapChange(&ev.xcolormap); break;
	case PropertyNotify:
	    handlePropertyChange(&ev.xproperty); break;
	case EnterNotify:
	    handleEnter(&ev.xcrossing); break;
	case Expose:
	    handleExpose(&ev.xexpose); break;
	case MotionNotify:
	    handleMotion(&ev.xmotion); break;
	}
    }
}

void WindowSystem::handleButtonPress(XButtonEvent *e)
{
    unsigned int button = 0;
    switch (e->button) {
    case Button1:
	button = 1;
	break;
    case Button2:
	button = 2;
	break;
    case Button3:
	button = 3;
	break;
    }
    if (e->window == root)
	takeRootButton( button, e->state & Mod1Mask, e->x, e->y );
    else
	takeButtonPress( e->window, button, 
			 e->state & Mod1Mask, e->x, e->y );
}

void WindowSystem::handleButtonRelease(XButtonEvent *e)
{
    unsigned int button = 0;
    switch (e->button) {
    case Button1:
	button = 1;
	break;
    case Button2:
	button = 2;
	break;
    case Button3:
	button = 3;
	break;
    }
    if (e->window != root)
	takeButtonRelease( e->window, button, 
			   e->state & Mod1Mask, e->x, e->y );
}

void WindowSystem::configure( Window w, int x, int y, 
			      int width, int height,
			      unsigned int valuemask,
			      Window sibling, int stack_mode)
{
    XWindowChanges wc;
    wc.x = x;
    wc.y = y;
    wc.width = width;
    wc.height = height;
    wc.border_width = 0;
    wc.sibling = sibling;
    wc.stack_mode = stack_mode;
    XConfigureWindow(dpy, w, valuemask, &wc);
}

void WindowSystem::handleConfigure(XConfigureRequestEvent *e)
{
    int x, y, width, height;
    x = y = width = height = 0;
    Window sibling;
    int stack_mode;

    if (e->value_mask & CWX) 
	x = e->x;
    if (e->value_mask & CWY) 
	y = e->y;
    if (e->value_mask & CWWidth) 
	width = e->width;
    if (e->value_mask & CWHeight) 
	height = e->height;
    sibling = e->above;
    stack_mode = e->detail;
    takeConfigureRequest( e->window, x, y, width, height,
			    e->value_mask,
			    sibling, stack_mode );
}

void WindowSystem::handleMap(XMapRequestEvent *e)
{
    XWindowAttributes attr;
    XGetWindowAttributes(dpy, e->window, &attr);
    //## db more fix this mess
    if (!attr.override_redirect && !managedWindow(e->window) ) {
	newWindow(e->window, (attr.map_state == IsViewable), 
		  attr.x, attr.y, attr.width, attr.height, attr.colormap);
    } else {
	takeMap( e->window );
    }
}

void WindowSystem::handleUnmap(XUnmapEvent *e)
{
    takeUnmap( e->window );
}


void WindowSystem::handleDestroy(XDestroyWindowEvent *e)
{
    takeDestroy( e->window );
}

void WindowSystem::handleClientMessage(XClientMessageEvent *e)
{
    if ( e->message_type == wm_change_state &&
	 e->format == 32 &&
	 e->data.l[0] == IconicState ) {
	iconize( e->window );
    }
}

void WindowSystem::handlePropertyChange(XPropertyEvent *e)
{
    switch (e->atom) {
    case XA_WM_NAME: {
	char* n;
	XFetchName(dpy, e->window, &n);
        takeFocus( e->window );
//	std::string newName( n );
	XFree( n );
//	takeNameChange( e->window, newName ); //possible buggy with gimp..
	break;
    }
    case XA_WM_NORMAL_HINTS:
	// db more
	err("XA_WM_NORMAL_HINTS not handled");
//	XGetWMNormalHints(dpy, c->window, c->size, &dummy);
    }
}

void WindowSystem::handleEnter(XCrossingEvent *e)
{
    takeFocus( e->window );
}

void WindowSystem::handleColormapChange(XColormapEvent *e)
{
    if ( e->c_new )
	takeNewColormap( e->window, e->colormap );
}

void WindowSystem::handleExpose(XExposeEvent *e)
{
    if (e->count == 0) 
	takeExpose( e->window );
}

void WindowSystem::handleMotion(XMotionEvent *e)
{
    takeMotion( e->window, e->x, e->y );
}

void WindowSystem::handleKeyPress(XKeyEvent *e)
{
    takeKeyPress( e->window, e->state, XKeycodeToKeysym( dpy, e->keycode, 0) );
}

