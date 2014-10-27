/* Clementine Window Manager
   Copyright 2002 Dave Berton <db@mosey.org>

   based on aewm Copyright 1998-2001 Decklin Foster <decklin@red-bean.com>

   This program is free software; see LICENSE for details. */

#include "keys.h"
#include <iostream>
#include <fstream>

Keys::Keys()
{
// keybindings more buggy ?
//    (void)parse( "keys" );
//    (void)parse( ".config/keys" ); //db moreadd other search paths
}

Keys::~Keys()
{
}

//    WindowSystem::Action takeKey();

bool Keys::parse( const std::string& name )
{
    std::ifstream file( name.c_str() );
    if ( !file.is_open() ) { 
	std::cerr << "Error opening file " << name << std::endl; 
	return false;
    }

    return true;
}
