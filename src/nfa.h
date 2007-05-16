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

#ifndef elr_nfa_H
#define elr_nfa_H 1

#pragma interface

#include <stdio.h>
#include <vector>
#include <set>
#include "bitset.h"

class DFA;
class EquivClassSet;
class Symbol;

class NFAState {
  public:
    Symbol *accept; /* Accept sym (NULL = none) */
    Bitset on;
    int dest;       /* 0 = no move */
    vector<int> epsilonMoves;

    NFAState( int num ) { on = Bitset(num); dest=0; accept = NULL; }

    void addEmptyMove( int dest );
    void addCharMove( int ch, int dest );
    void addCharMove( int ch, int dest, bool caseSensitive );
    void addClassMove( char **s, int dest, bool caseSensitive );

 protected:
    void set( int ch, bool caseSensitive );
};

class NFA { 
  public:
    vector <NFAState> states;
    EquivClassSet *equivs;
    int numEquivs;
    bool caseSensitive;
    
    NFA( int charsetsize = CHARSET_SIZE, bool caseSensitive = true );
    
    static NFA *fromString( string &s, Symbol *sym, bool caseSensitive );
    static NFA *fromRegexp( string &s, Symbol *sym, bool caseSensitive );
    static NFA *fromEmpty( void );
    static NFA *fromEOF( Symbol *sym );
    void addAlternation( NFA *fsa );
    
    void epsilonClosure( int state, set<int> &in );
    void move( set<int> &from, int ch, set<int> &to );
    void computeEquivClasses( void );
    void print( FILE *out );
  protected:
    int addState( void );
    int parseRegexpString( char **s, int start );
    int parseRegexpChoice( char **s, int start );
    void genRegexpNFA( string &s, Symbol *sym );
    void genStringNFA( string &s, Symbol *sym );
};                               


#endif /* !elr_nfa_H */
