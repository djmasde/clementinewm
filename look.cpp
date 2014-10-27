/* Clementine Window Manager
   Copyright 2002 Dave Berton <db@mosey.org>

   based on aewm Copyright 1998-2001 Decklin Foster <decklin@red-bean.com>

   This program is free software; see LICENSE for details. */

#include "look.h"
#include "tokenizer.hpp"
#include <iostream>
#include <fstream>
#include <cassert>
#include <memory>
#include <vector>
#include <algorithm>


RGBColor::RGBColor( unsigned int r, unsigned int g, unsigned int b,
	      unsigned long p )
    : red(r), green(g), blue(b), pix(p)
{
}

RGBColor::~RGBColor()
{
}

RGBColor::RGBColor( const RGBColor& other )
{
    *this = other;
}

RGBColor& RGBColor::operator=( const RGBColor& other )
{
    red = other.red;
    green = other.green;
    blue = other.blue;
    pix = other.pix;
    return *this;
}

	    
unsigned long RGBColor::pixel() const 
{ 
    return pix; 
}

///////////

ColorManager::ColorManager( Display* d, int s )
    : dpy(d), screen(s)
{
    assert( dpy );
}

ColorManager::~ColorManager()
{
    if ( colors.size() ) {
	unsigned long *pixels = new unsigned long [colors.size()];
	unsigned int i;
	for ( i = 0; i < colors.size(); ++i )
	    *(pixels + i) = colors[i].pixel();
	XFreeColors(dpy, DefaultColormap(dpy, screen), 
		    pixels, colors.size(), 0);
	// can't check for error here, although an X error will occur
	// if there is a problem.
	delete [] pixels;
    }
}

RGBColor ColorManager::alloc(unsigned int r, 
			     unsigned int g, 
			     unsigned int b)
{
    /* find it */
    std::vector<RGBColor>::const_iterator it = colors.begin();
    for ( ; it != colors.end(); ++it ) {
	unsigned int red, green, blue;
	(*it).RGB( red, green, blue );
	if ( red == r && green == g && blue == b ) {
	    return *it;
	}
    }
    /* otherwise, allocate it */
    XColor c;
    c.red = r;
    c.green = g;
    c.blue = b;
    if (!XAllocColor( dpy, DefaultColormap(dpy, screen), &c ) )
	std::cerr << "Could not allocate r:" << 
	    (unsigned int)r << " g:" << (unsigned int)g << 
	" b:" << (unsigned int)b << std::endl;

    // note! save the -originally- requested RGD values so that
    // subsequent lookups will return the same color.  XAllocColor can
    // possible change these values to a similar matching color
    // depending on the server we are running under.
    RGBColor rgbc( r, g, b, c.pixel ); 
    colors.push_back( rgbc ); 
    
    return rgbc;
}

RGBColor ColorManager::darken( const RGBColor& c )
{
    unsigned int r, g, b;
    c.RGB( r, g, b );
    r = (r*2/3);
    g = (g*2/3);
    b = (b*2/3);
    RGBColor d = alloc( r, g, b );
    return d;
}

RGBColor ColorManager::lighten( const RGBColor& c )
{
    unsigned int r, g, b;
    c.RGB( r, g, b );
    if ( r + g + b == 0 ) /* special case, black */
	return alloc( 180*256, 180*256, 180*256 );
    r = r + (r/3);
    if ( r > 65535 )
	r = 65535;
    g = g + (g/3);
    if ( g > 65535 )
	g = 65535;
    b = b + (b/3);
    if ( b > 65535 )
	b = 65535;
    RGBColor l = alloc( r, g, b );
    return l;
}

///////////

FacetParser::FacetParser( Display* d, int s, const std::string& name,
	      ColorManager* colormanager )
    : dpy(d), screen(s), nm(name), cm(colormanager)
{
    assert(dpy);
}

FacetParser::FacetParser( const FacetParser& other )
    : Facet( other ),
      dpy( other.dpy ), 
      screen( other.screen ),
      nm( other.nm )
{
}

FacetParser& FacetParser::operator=( const FacetParser& other )
{
    Facet::operator=(other);
    dpy = other.dpy;
    screen =  other.screen;
    nm = other.nm;
    styl = other.styl;
    back = other.back;
    return *this;
}

FacetParser::~FacetParser()
{
}

void FacetParser::parse( std::vector<std::string> setting )
{
    if ( setting.size() == 0 )
	return;
    std::vector<std::string> property;
    std::string propName = propertize( setting, property );
    if ( !propName.length() ) 
	return;
    if ( propName == "style" ) 
	parseStyle( property );
    if ( propName == "color" )
	parseColor( property );
    if ( propName == "background" )
	parseBackground( property );
    if ( propName == "border" )
	parseBorder( property );
}

std::string FacetParser::propertize( std::vector<std::string> in, 
			       std::vector<std::string>& out )
{
    /* 'in' is of the form:
       name.property[.subproperty...] setting [setting...] 
    */
    if ( ! in.size() >= 2 ) {
	std::cerr << "Bad property" << std::endl;
	return "";
    }
    /* parse the main property we are interested in */
    std::vector<std::string> prop;
    tokenize( in[0], std::string("."), prop );
    if ( !prop.size() > 2 ) {
	std::cerr << "Unable to parse property:" << in[0] << std::endl;
	return "";
    }
    if ( prop[0] != nm ) /* must be my property */
	return "";
    std::string mainProperty = prop[1];
    if ( prop.size() > 2 )
	out.push_back( prop[2] );
    std::vector<std::string>::const_iterator it = in.begin();
    for ( ++it; it != in.end(); ++it ) 
	out.push_back( *it );
    return mainProperty;
 }

void FacetParser::parseStyle( const std::vector<std::string>& s )
{
    std::vector<std::string>::const_iterator it;
    for ( it=s.begin(); it!=s.end();++it) {
	if ( *it == "gradient" )
	    styl.setIsGradient( true );
	if ( *it == "flat" )
	    styl.setIsFlat( true );
	if ( *it == "bevel" )
	    styl.setIsBevel( true );
	if ( *it == "horizontal" )
	    styl.setIsHorizontal( true );
	if ( *it == "vertical" )
	    styl.setIsVertical( true );
	if ( *it == "ascend" )
	    styl.setIsAscending( true );
	if ( *it == "descend" )
	    styl.setIsDescending( true );
    }
}

void FacetParser::parseColor( const std::vector<std::string>& b )
{
    parseColor( colr, b );
}

void FacetParser::parseBackground( const std::vector<std::string>& b )
{
    parseColor( back, b );
}

void FacetParser::parseBorder( const std::vector<std::string>& b )
{
    parseColor( bord, b );
}

void FacetParser::parseColor( ColorSet& cs, 
			      const std::vector<std::string>& b )
{
    if ( b.size() == 1 ) { /* just the color */
	cs.color = parseColor( b[0] );
	cs.light = cm->lighten( back.color );
	cs.dark = cm->darken( back.color );
	return;
    }
    std::vector<std::string>::const_iterator it;
    for ( it=b.begin(); it!=b.end();++it) {
	if ( *it == "from" ) 
	    cs.light = parseColor( b[b.size()-1] );
	if ( *it == "to" )
	    cs.dark = parseColor( b[b.size()-1] );
    }
}

RGBColor FacetParser::parseColor( const std::string& name ) const
{
    RGBColor c;
    if ( !name.length() ) {
	std::cerr << "Cannot parse empty color" << std::endl;
	return c;
    }
    XColor color;
    // db more error check
    if ( XParseColor( dpy, DefaultColormap( dpy, screen ), (char*)name.c_str(),
		      &color) ) {
	c = cm->alloc( color.red, color.green, color.blue );
    } else {
	std::cerr << "Could not parse color:" << name << std::endl;
    }
    return c;
}

///////////

Look::Look( Display* d, int screen )
    : cm(d, screen),
      active( d, screen, "active", &cm ),
      activeButton( d, screen, "button", &cm ),
      inactive( d, screen, "inactive", &cm ),
      inactiveButton( d, screen, "button", &cm )
{
    if ( !parse( "look" ) ) //db more add other search paths
    if ( !parse( ".config/look" ) ) //db more add other search paths
	setDefaults();
}

Look::~Look()
{
}

bool Look::parse( const std::string& name )
{
    std::ifstream file( name.c_str() );
    if ( !file.is_open() ) { 
	std::cerr << "Error opening file '" << name << "'" << std::endl; 
	return false;
    }

    std::auto_ptr<char> linebuffer(new char[1024]);
	
//  int line=0;

    while (!file.eof()) {	
	file.getline(linebuffer.get(), 1024);		
	std::string line( linebuffer.get() );
	if ( !line.length() )
	    continue;
	std::vector<std::string> property;
	tokenize( line, std::string(" "), property );
	if ( !property.size() || property[0][0] == '#' )
	    continue; // skip comments and blank lines
	if ( property.size() > 0 ) {
	    if ( property[0] == "background" ) {
		rootCmd = "";
		std::vector<std::string>::const_iterator it = 
		    property.begin();
		for ( ++it; it != property.end(); ++it ) 
		    rootCmd += " " + *it;
		rootCmd = rootCmd.substr( 1, rootCmd.length()-1 );
	    } else if ( property[0].substr(0,13) == "active.button" ) {
		property[0] = property[0].substr( 7, property[0].length() );
		activeButton.parse( property );
	    } else if ( property[0].substr(0,15) == "inactive.button" ) {
		property[0] = property[0].substr( 9, property[0].length() );
		inactiveButton.parse( property );
	    } else if ( property[0].substr(0,6) == "active" )
		active.parse( property );
	    else if ( property[0].substr(0,8) == "inactive" )
		inactive.parse( property );
	}
    }
    
    return true;
}

void Look::setDefaults()
{
    /* simple grey theme */
    Style s;
    s.setIsGradient( true );
    s.setIsVertical( true );
    //s.setIsBevel( true );
    active.setStyle( s );
    inactive.setStyle( s );

    ColorSet c;
    c.color = cm.alloc( 0, 0, 0 );
    c.light = cm.alloc( 206*256, 206*256, 206*256 );
    c.dark = cm.alloc( 169*256, 169*256, 169*265 ); 
    active.setColor( c );
    inactive.setColor( c );
    ColorSet b;
    b.color = cm.alloc( 0, 0, 0 );
    b.light = cm.lighten( b.color );
    b.dark = cm.darken( b.color );

    ColorSet o;
    o.color = cm.alloc( 230*256, 230*256, 230*256 );
    o.light = cm.lighten( o.color );
    o.dark = cm.darken( o.color );
    active.setBackground( o );
    active.setColor( o );
    inactive.setColor( o );
    inactive.setBackground( o );
    rootCmd = "xsetroot -solid grey30";

}
