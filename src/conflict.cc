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
 * conflict.cc - implementation of lexical conflict classes
 */

#include <assert.h>
#include "grammar.h"
#include "lr.h"
#include "stringset2.h"
#include "conflict.h"

ConflictMap::ConflictMap( LRTable *pda, DFA *dfsa, Grammar *gram )
{
    lr = pda;
    dfa = dfsa;
    grammar = gram;
    numParserStates = lr->states.size();
    numDFAStates = dfa->states.size();
    numDFASymbols = dfa->numEquivs;
    stateMap = new State[numDFAStates];
    parserStateMap = new int [numParserStates];
//    for( int i=0; i<dfa->states.size(); i++ ){ stateMap[i] = stateMap(); }
}

ConflictMap::~ConflictMap( )
{
    FOR_EACH(row, vector<int *>, table) {
        delete [] (*row);
    }
    for( int i=0; i<numDFAStates; i++ )
        if( stateMap[i].edges ) delete [] stateMap[i].edges;

    delete [] stateMap;
    delete [] parserStateMap;
}

int ConflictMap::getIdentity( int dfaState, int lrState )
{
    if( stateMap[dfaState].row == -1 ) return C_DONTCARE;
    return table[stateMap[dfaState].row][lrState];
}

int ConflictMap::getEdge( int dfaState, int dfaEdge, int lrState )
{
    if( stateMap[dfaState].edges == NULL ) return C_DONTCARE;
    if( stateMap[dfaState].edges[dfaEdge] == -1 ) return C_DONTCARE;
    return table[stateMap[dfaState].edges[dfaEdge]][lrState];
}

bool ConflictMap::hasStateAction( int dfaState, int lrState ){
    return ( stateMap[dfaState].row != -1 );
}

bool ConflictMap::hasEdgeActions( int dfaState, int lrState )
{
    if( stateMap[dfaState].edges == NULL ) return false;
    for( int i=0; i<numDFASymbols; i++ ) {
        if( stateMap[dfaState].edges[i] != -1 &&
            table[stateMap[dfaState].edges[i]][lrState] == C_ACCEPT )
            /* -1 = CONFLICT (use default), 0 = DONTCARE (also use default) */
            return true;
    }
    return false;
}

bool ConflictMap::stateHasActions( int dfaState, int lrState )
{
    return hasStateAction( dfaState, lrState ) ||
        hasEdgeActions( dfaState, lrState );
}

/*
 * Minimize the number of columns in the matrix (in-place). Looks reasonably
 * slow - could it be improved? (probably also has bad locality)
 * CHECK: Is this actually necessary/desireable? 
 */
int ConflictMap::minimize()
{
    int resultSize = 0;

#if 0
    memset( parserStateMap, 0, numParserStates );
    
    for( int x=0; x < numParserStates; x++ ) {
        if( parserStateMap[x] != 0 ) continue;
        if( x != resultSize )
            for( int i=0; i < table.size(); i++ )
                table[i][resultSize] = table[i][x];
        parserStateMap[x] = resultSize;
        
        for( int y=x+1; y < numParserStates; y++ ) {
            if( parserStateMap[y] != 0 ) continue;
            bool canMerge = true;
            for( int i=0; i < table.size(); i++ ) {
                if( table[i][resultSize] != table[i][y] &&
                    table[i][resultSize] != C_DONTCARE &&
                    table[i][y] != C_DONTCARE ) {
                    canMerge = false;
                    break;
                }
            }
            if( canMerge ) {
                for( int i=0; i < table.size(); i++ ) {
                    if( table[i][resultSize] == C_DONTCARE )
                        table[i][resultSize] = table[i][y];
                }
                parserStateMap[y] = resultSize;
            }
        }
        resultSize++;
    }
    numMinColumns = resultSize;

#endif
    for( int i=0; i<numParserStates; i++ ) parserStateMap[i] = i;
    numMinColumns = numParserStates;
    return numMinColumns;
}
    
    

int ConflictMap::addIdentityConflict( int state, set<Symbol *> &syms )
{
    int *row = new int [numParserStates];
    bool isResolved = true;

    row[0] = 0;
    for( int i=1; i<numParserStates; i++ ) {
        bool stateOk = true;
        Symbol *resSym;
        row[i] = C_DONTCARE;
        FOR_EACH( sym, set<Symbol *>, syms ) {
            if( lr->states[i]->accepts.get((*sym)->symbolId) ) {
                if( row[i] == C_DONTCARE ) {
                    row[i] = (*sym)->symbolId;
                    resSym = *sym;
                }else {
                    row[i] = C_CONFLICT;
                    stateOk = isResolved = false;
                }
            }
        }
    }
    int val = collapseRow( row );
    if( val != -1 ){ /* All states were the same or didn't care */
        delete [] row;
        return val;
    }
    stateMap[state].row = table.size();
    table.push_back( row );
    return (isResolved?-1:-2);
}

int ConflictMap::addLMConflict( int state, int edge, set<Symbol *> &syms,
                                Bitset &afterSyms, Bitset &alt )
{
    /* ie, a conflit in which we can accept syms, and then afterSyms, or else
     * just accept alt
     */

    int *row = new int [numParserStates];
    int thisSym;
    Bitset next = Bitset(lr->grammar->lastTerminal+1);
    bool isResolved = true;

    row[0] = 0;
    for( int i=1; i<numParserStates; i++ ) {
        row[i] = C_ACCEPT;
        if( stateMap[state].row != -1 ) {
            thisSym = table[stateMap[state].row][i];
            if( thisSym == -1 ) {
                /* We've already got an unresolved identity conflict. Argh */
                /* (consider whether we could usefully still do stuff?) */
                row[i] = -1;
                isResolved = false;
                continue;
            }
            if( thisSym == 0 ) {
                /* We aren't interested in anything in the current state. */
                if( alt.isDisjoint( lr->states[i]->accepts ) )
                    row[i] = C_DONTCARE; /* We're equally rooted at this point */
                else 
                    row[i] = C_SHIFT;
                continue;
            }
        } else {
            assert( syms.size() == 1 );
            thisSym = (*syms.begin())->symbolId;
            if( ! lr->states[i]->accepts.get(thisSym) ) {
                /* Nope, can't accept this, sorry */
                if( alt.isDisjoint( lr->states[i]->accepts ) )
                    row[i] = C_DONTCARE; /* We're equally rooted at this point */
                else 
                    row[i] = C_SHIFT;
                continue;
            }
        }
        if( alt.isDisjoint( lr->states[i]->accepts ) ) {
            /* The alternatives are useless to us - must accept now */
            row[i] = C_ACCEPT;
            continue;
        }
        follows( next, thisSym, i );
        if( afterSyms.isDisjoint( next ) ) {
            /* Accepting now would leave us in an untenable position -
             * therefore shift
             */
            row[i] = C_SHIFT;
        } else {
            /* We're stuffed - syms . afterSyms & alt both contain valid
             * symbols.
             */
            row[i] = C_OHFSCK;
            isResolved = false;
        }
    }

    int val = collapseRow( row );
    if( val != -1 ) {
        delete [] row;
        return val;
    }
    if( stateMap[state].edges == NULL ){
        stateMap[state].edges = new int [numDFASymbols];
        for( int i=0; i < numDFASymbols; i++ ) stateMap[state].edges[i] = -1;
    }
    stateMap[state].edges[edge] = table.size();
    table.push_back( row );
    return (isResolved?-1:-2);
}


int ConflictMap::collapseRow( int *row )
{
    int last = 0;

    for( int i=0; i<numParserStates; i++ ) {
        if( row[i] != C_DONTCARE ) {
            if( row[i] != last && last != 0 ) {
                /* Row has differences */
                return -1;
            }
            last = row[i];
        }
    }
    return last;
}

void ConflictMap::follows( Bitset &bits, int sym, int state )
{
    LREdge *edge = lr->states[state]->edges[sym];

    if( !edge && sym == grammar->spaceTerm->symbolId ) {
        bits = lr->states[state]->accepts;
    } else if( edge->type == SHIFT ) {
        bits = *edge->followsk->first();
    } else if( edge->type == REDUCE ) {
        bits = *edge->followsk->follows(sym);
    } else {
        assert( false );
    }
    if( grammar->spaceTerm )
        bits.set( grammar->spaceTerm->symbolId );
}
