/* Clementine Window Manager
   Copyright 2002 Dave Berton <db@mosey.org>

   based on aewm Copyright 1998-2001 Decklin Foster <decklin@red-bean.com>

   This program is free software; see LICENSE for details. */

#ifndef LOOK_H
#define LOOK_H

#include "windowsystem.h"
#include <X11/Xlib.h>
#include <string>
#include <vector>

class Facet;

class RGBColor 
{
public:
    RGBColor( unsigned int r=0, unsigned int g=0, unsigned int b=0,
	   unsigned long p=0 );
    RGBColor& operator=( const RGBColor& other );
    RGBColor( const RGBColor& other );
    ~RGBColor();
	  
    void RGB( unsigned int& r, unsigned int& g, unsigned int& b ) const {
	r = red; g = green; b = blue;
    }
    bool operator==( const RGBColor& other ) const{
	return ( red == other.red && 
		 green == other.green && 
		 blue == other.blue );
    }
    unsigned long pixel() const;
    
private:
    unsigned int red, green, blue;
    unsigned long pix;
};

class ColorManager
{
public:
    ColorManager( Display* d, int s );
    ~ColorManager();

    RGBColor alloc(unsigned int r, unsigned int g, unsigned int b);
    RGBColor darken( const RGBColor& c );
    RGBColor lighten( const RGBColor& c );

private:
    ColorManager( const ColorManager& other );
    ColorManager& operator=(const ColorManager& other );

    Display*     dpy;
    int          screen;
    std::vector<RGBColor> colors;

};


class ColorSet 
{
public:
    ColorSet() {}
    ColorSet( const ColorSet& other ) 
	: light(other.light),
	  color(other.color),
	  dark(other.dark)
	{}
    ColorSet& operator=(const ColorSet& other ) {
	light = other.light;
	color = other.color;
	dark = other.dark;
	return *this;
    }
    RGBColor light;
    RGBColor color;
    RGBColor dark;
};

class Style 
{
public:
    Style() 
	: gradient(false),
	  flat(false),
	  bevel(false),
	  vertical(false),
	  descend(false) {}
    Style( const Style& other )
	: gradient( other.gradient ),
	  flat( other.flat ),
	  bevel( other.bevel ),
	  vertical( other.vertical ),
	  descend( other.descend )
	{}
    Style& operator=( const Style& other )
	{
	    gradient = other.gradient;
	    flat = other.flat;
	    bevel = other.bevel;
	    vertical = other.vertical;
	    descend = other.descend;
	    return *this;
	}

    ~Style() {}

    bool isGradient() const { return gradient; }
    bool isFlat() const { return flat; }
    bool isBeveled() const { return bevel; }
    bool isHorizontal() const { return !vertical; }
    bool isVertical() const { return vertical; }
    bool isAscending() const { return !descend; }
    bool isDescending() const { return descend; }

    void setIsGradient( bool b ) { gradient = b; }
    void setIsFlat( bool b ) { flat = b; }
    void setIsBevel( bool b ) { bevel = b; }
    void setIsHorizontal( bool b ) { vertical = !b; }
    void setIsVertical( bool b ) { vertical = b; }
    void setIsAscending( bool b ) { descend = !b; }
    void setIsDescending( bool b ) { descend = b; }

private:
    bool gradient;
    bool flat;
    bool bevel;
    bool vertical;
    bool descend;
};

class Facet
{
public:
    Facet(){}
    Facet( const Facet& other) 
	: styl( other.styl ),
	  colr( other.colr ),
	  back( other.back ),
	  bord( other.bord )
	{}
    Facet& operator=( const Facet& other ) {
	styl = other.styl;
	colr = other.colr;
	back = other.colr;
	bord = other.bord;
	return *this;
    }
	
    Style style() const { return styl; }
    ColorSet color() const { return colr; }
    ColorSet background() const { return back; }
    ColorSet border() const { return bord; }

protected:
    Style styl;
    ColorSet colr;
    ColorSet back;    
    ColorSet bord;
};

class FacetParser : public Facet
{
public:    
    FacetParser( Display* d, int screen, const std::string& name,
	   ColorManager* colormanager );
    FacetParser( const FacetParser& other );
    FacetParser& operator=( const FacetParser& other );
    ~FacetParser();

    void setStyle( Style s ) { styl = s; }
    void setColor( ColorSet c ) { colr = c; }
    void setBackground( ColorSet c ) { back = c; }
    void setBorder( ColorSet c ) { bord = c; }

    std::string name() const {return nm;}
    void parse( std::vector<std::string> setting );
private:
    Display* dpy;
    int screen;
    std::string nm;
    ColorManager* cm;

    std::string propertize( std::vector<std::string> in, 
			    std::vector<std::string>& out );
    void parseStyle( const std::vector<std::string>& s );
    void parseColor( const std::vector<std::string>& b );
    void parseBackground( const std::vector<std::string>& b );
    void parseBorder( const std::vector<std::string>& b );

    RGBColor parseColor( const std::string& c ) const;
    void parseColor( ColorSet& cs, const std::vector<std::string>& b );
    
};

class Look
{
public:
    Look( Display* d, int screen );
    ~Look();

    Facet* activeFacet() { return &active; }
    Facet* activeButtonFacet() { return &activeButton; }
    Facet* inactiveFacet() { return &inactive; }
    Facet* inactiveButtonFacet() { return &inactiveButton; }
    ColorManager* colorManager() { return &cm; } // db more const?
    std::string rootCommand() const { return rootCmd; }

private:
    bool parse( const std::string& name );
    void setDefaults();

    ColorManager cm;
    FacetParser  active;
    FacetParser  activeButton;
    FacetParser  inactive;
    FacetParser  inactiveButton;
    
    std::string  rootCmd;
};

#endif
