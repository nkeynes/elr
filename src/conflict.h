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
/*
 * conflict.h - classes for dealing with lexical conflicts
 */

#ifndef elr_conflict_H
#define elr_conflict_H

#include <vector>
#include <set>
#include "grammar.h"
#include "lr.h"
#include "dfa.h"

#define C_DONTCARE 0
#define C_SHIFT 1
#define C_ACCEPT 2
#define C_CONFLICT -1
#define C_OHFSCK C_CONFLICT

class ConflictMap {
  public:
    class State {
      public:
        int row;
        int *edges;
        State() { row = -1; edges = NULL; }
    };
    State *stateMap;  /* Mapping from DFA state & edges to ConflictMap row */

    vector<int *> table;
    int *parserStateMap;
    int numMinColumns; /* Number of columns after minimization */
    int numParserStates, numDFAStates, numDFASymbols;
    LRTable *lr;
    DFA *dfa;
    Grammar *grammar;
    
    ConflictMap( LRTable *pda, DFA *dfsa, Grammar *gram );
    ~ConflictMap( );

    int addIdentityConflict( int state, set<Symbol *> &syms);
    int addLMConflict( int state, int edge, set<Symbol *> &syms,
                       Bitset &afterSyms, Bitset &altSyms );
    int collapseRow( int *row );
    int minimize( void );

    int getIdentity( int dfaState, int lrState );
    int getEdge( int dfaState, int dfaEdge, int lrState );
    bool hasStateAction( int dfaState, int lrState );
    bool hasEdgeActions( int dfaState, int lrState );
    bool stateHasActions( int dfaState, int lrState ); /* Either? */

  protected:
    void follows( Bitset &bits, int sym, int state );
};

#endif /* elr_conflict_H */
