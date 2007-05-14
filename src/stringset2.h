/**
 * $Id$
 *
 * Copyright (c) 2000-2007 Nathan Keynes.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef elr_stringset2_H
#define elr_stringset2_H

#include "bitset.h"
#include <vector>

using namespace std;

typedef char *(*string_format_fn)(int n);

class Grammar;

class StringSet2 {
  public:
    StringSet2( int numSym );
    void clear( void );
    void addMax( StringSet2 &with ); /* Add only strings of max length */
    void add( StringSet2 &with );
    void add( int token );
    void add( int *str, int len );
    void addEmpty( void ) { hasNull = true; }
    void deleteEmpty( void ) { hasNull = false; }
    void append( StringSet2 &with );
    void append( int token );
    void print( Grammar *g );

    Bitset *first( void );
    Bitset *follows( int token );
    int minLength( ){ return ((hasNull || isEmpty) ? 0 : (isFull ? 2 : 1)); }
    bool hasEmpty( ){ return hasNull; }

    bool operator == (const StringSet2 &b) const {
        return hasNull == b.hasNull && firstsym == b.firstsym &&
            firstonly == b.firstonly && second == b.second;
    }
    bool operator != (const StringSet2 &b) const {
        return hasNull != b.hasNull || firstsym != b.firstsym ||
            firstonly != b.firstonly || second != b.second;
    }
    
  protected:
    int numSymbols;
    bool hasNull;
    bool isFull; /* All elements of length 2 (excl. null) */
    bool isEmpty; /* No non-null elements */
    Bitset firstsym, firstonly;
    vector<Bitset> second;
    
};



#endif
