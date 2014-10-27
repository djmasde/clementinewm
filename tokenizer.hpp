/* Clementine Window Manager
   Copyright 2002 Dave Berton <db@mosey.org>

   based on aewm Copyright 1998-2001 Decklin Foster <decklin@red-bean.com>

   This program is free software; see LICENSE for details. 

   Adapted from http://www.kuix.de/snippets/tokenizer.h */

#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <string>
#include <vector>

template < class Char, class Cont >
void tokenize( const Char& str, 
	       const Char& delims, 
	       Cont& found_tokens )
{
    typename Char::size_type walk_pos = 0;
    while( walk_pos != Char::npos && 
	   walk_pos < str.length() ) {
	typename Char::size_type token_pos = 0, token_len = 0;
	typename Char::size_type delim_pos = str.find_first_of( delims, walk_pos );

	if( delim_pos == Char::npos ) {
	    // no more delims, a token starts at walk_pos
	    token_pos = walk_pos;
	    token_len = str.length() - token_pos;
	    walk_pos += token_len;
	} else if( delim_pos > walk_pos ) {
	    // more tokens / delims left, but a token starts at walk_pos
	    token_pos = walk_pos;
	    token_len = delim_pos - token_pos;
	    walk_pos = delim_pos;
	} else if( delim_pos == walk_pos ) {
	    // delimiters start at walk_pos
	    walk_pos = str.find_first_not_of( delims, walk_pos );
	    if( walk_pos == Char::npos ) {
		// only delims left in str, no more tokens
		break;
	    } else {
		// more tokens left
		continue;
	    }
	}
	found_tokens.push_back( str.substr( token_pos, token_len ) );
    }
}

#endif
