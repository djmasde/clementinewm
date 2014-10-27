/* Clementine Window Manager
   Copyright 2002 Dave Berton <db@mosey.org>

   based on aewm Copyright 1998-2001 Decklin Foster <decklin@red-bean.com>

   This program is free software; see LICENSE for details. */

#ifndef KEYS_H
#define KEYS_H

#include "windowsystem.h"
#include <string>

class Keys
{
public:
    Keys();
    ~Keys();

//    WindowSystem::Action takeKey();

private:
    bool parse( const std::string& name );
};

#endif
