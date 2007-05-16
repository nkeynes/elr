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
#include <assert.h>
#include "config.h"
#include "dfa.h"
#include "equiv.h"
#include "conflict.h"

/****************************** Conflict handling ****************************/

/* Check the given DFA for
 *   - same match lexical conflicts ( syms.size() > 1 )
 *   - longest match lexical conflicts
 */
void DFA::checkConflicts( Grammar &g, ConflictMap &conflict )
{
    int count = 0, numConflicts = 0, numResolved = 0;
    int numLMCStates = 0, numLMCEdges = 0;
    int numLMCStatesResolved = 0, numLMCEdgesResolved = 0;
    
    for( unsigned int i = 1; i < states.size(); i++ ) {
        if( states[i].isConflicting() ) {
            numConflicts++;
            int status = conflict.addIdentityConflict( i, states[i].accepts );
            if( status >= 0 ) {
                /* This may arguably be impossible, however proving that..*/
                states[i].accepts.clear();
                states[i].accepts.insert( g.symbol(status) );
                states[i].accept = g.symbol(status);
            } else { /* Determine the default accept according to lex rules */
                int min = g.numSymbols+1;
                FOR_EACH( sym, set<Symbol *>, states[i].accepts ) {
                    if( (*sym)->symbolId < min ) min = (*sym)->symbolId;
                }
                states[i].accept = g.symbol(min);
            }
            /* we either resolved on parser state, or we didn't */
            if( status != -2 ) numResolved++;
        } else {
            /* Copy the accepted symbol into the accept field */
            if( states[i].accepts.empty() )
                states[i].accept = NULL;
            else states[i].accept = *states[i].accepts.begin();
        }
    }

    /* LM Conflicts: Defined as - in an accepting state p, there exists a valid
     * transition a such that a is also a valid transition from the start
     * state.
     */

    for( unsigned int i = 1, count = 0; i < states.size(); i++ ) {
        if( states[i].isAccepting() ) {
            bool hasConflicts = false, isResolved = true;
            for( int j = 0; j < numEquivs; j++ ) if( states[i].moves[j] != 0 ){
                numLMCEdges++;
                if( !hasConflicts ){
                    hasConflicts = true;
                    numLMCStates++;
                }
                int status = conflict.addLMConflict( i, j, states[i].accepts,
                                 states[states[1].moves[j]].postAccepts,
                                 states[states[i].moves[j]].postAccepts );
                if( status == C_DONTCARE || status == C_SHIFT ) {
                    /* If we don't care, then shift? */
                } else if( status == C_ACCEPT ) {
                    states[i].moves[j] = 0; /* delete this edge */
                }
                if( status != -2 ) numLMCEdgesResolved++;
                else isResolved = false;
            }
            if( isResolved && hasConflicts )
                numLMCStatesResolved++;
        }
    }
}

void DFA::resolveConflictsNoContext( Grammar &g )
{
    for( unsigned int i = 1; i < states.size(); i++ ) {
	if( !states[i].accepts.empty() ) {
	    int min = g.numSymbols+1;
	    FOR_EACH( sym, set<Symbol *>, states[i].accepts ) {
		if( (*sym)->symbolId < min ) min = (*sym)->symbolId;
	    }
	    states[i].accept = g.symbol(min);
	}
    }
}

void DFA::resolveConflicts( Grammar &g, ConflictMap &conflict )
{
    numStartStates = conflict.minimize();
    startStates = new int [numStartStates];
    for( int i=0; i<numStartStates; i++ ) {
        startStates[i] = copyStartStateMin( g, conflict, i );
    }
}

/*
 * For the given start state, copy the DFA for it and return the first state.
 */
int DFA::copyStartState( Grammar &g, ConflictMap &conflict, int sstate )
{
    int start = states.size() - 1;

    /* Copy and alter the necessary states */
    for( int i=1; i<numBaseStates; i++ ) {
        int state = addState();
        int sym = conflict.getIdentity(i,sstate);
        if( sym <= 0 ) states[state].accept = states[i].accept;
        else states[state].accept = g.symbol(sym);
        for( int move = 0; move < numEquivs; move++ ) {
            if( states[i].moves[move] == 0 ||
                conflict.getEdge( i, move, sstate ) == C_ACCEPT )
                states[state].moves[move] = 0;
            else
                states[state].moves[move] = states[i].moves[move] + start;
        }
    }
    return start+1;
}

int DFA::copyStartStateMin( Grammar &g, ConflictMap &conflict, int sstate )
{
    int map[numBaseStates];
    int start = states.size(), i,j;
    Bitset changed = Bitset(numBaseStates);

    for( i=1; i<numBaseStates; i++ ) {
        int sym = conflict.getIdentity(i,sstate);
        if( sym > 0 && sym != states[i].accept->symbolId ||
            conflict.hasEdgeActions(i,sstate) ) {
            changed |= states[i].predStates;
        }
    }
    
    for( i=1,j=states.size(); i<numBaseStates; i++ )
        map[i] = changed[i] ? j++ : i;

    if( j == states.size() ) /* no change */
        return 1;
    
    for( i=1; i<numBaseStates; i++ ) if( changed[i] ) {
        int state = addState();
        assert(map[i] == state);
        int sym = conflict.getIdentity(i,sstate);
        if( sym <= 0 ) states[state].accept = states[i].accept;
        else states[state].accept = g.symbol(sym);
        for( int move = 0; move < numEquivs; move++ ) {
            if( states[i].moves[move] == 0 ||
                conflict.getEdge( i, move, sstate ) == C_ACCEPT )
                states[state].moves[move] = 0;
            else
                states[state].moves[move] = map[states[i].moves[move]];
        }
    }
    return start;
}
