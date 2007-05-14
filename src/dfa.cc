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
#include <stdlib.h>
#include <assert.h>
#include <vector>
#include <string>
#include "grammar.h"
#pragma implementation "dfa.h"
#include "dfa.h"
#include "nfa.h"
#include "equiv.h"
#include "conflict.h"

/****************************** Constructors *********************************/

DFA::DFA( int css, EquivClassSet *ecs )
{
    numEquivs = css;
    states.push_back( DFAState(css) ); /* Null state */
    equivs = ecs;
    startStates = NULL;
}

DFA::~DFA( )
{
    if( startStates ) delete [] startStates;
    
    FOR_EACH( state, vector<DFAState>, states ) { /* Yay */
        if( state->moves ) delete state->moves;
    }
}

namespace __gnu_cxx {

template <> struct hash< set<int> > {
    size_t operator()( set<int> __s ) const {
        size_t __h = 0;
        for( set<int>::iterator i = __s.begin(); i != __s.end(); i++ )
            __h = 5*__h + *i;
        return __h;
    }
};
}

/*
 * NFA => DFA Computation
 */
DFA *DFA::fromNFA( NFA *nfa )
{
    DFA *dfa = new DFA(nfa->numEquivs, nfa->equivs);
    hash_map<set<int>, int> nfaMap;
    set<int> dest;
    int next, state, count = 1;

    dest.insert(1);
    nfa->epsilonClosure( 1, dest );
    next = dfa->addState( dest, nfa );
    nfaMap[dest] = next;

    for( state = 1; state <= count; state ++ ) {
        for( int ch = 0; ch < nfa->numEquivs; ch++ ) {
            dest.clear();
            nfa->move( dfa->states[state].nfaStates, ch, dest );
            if( ! dest.empty() ) {
                next = nfaMap[dest];
                if( next == 0 ) {
                    next = dfa->addState( dest, nfa );
                    nfaMap[dest] = next;
                    count++;
                }
                dfa->states[state].moves[ch] = next;
            }
        }
    }

    dfa->numBaseStates = dfa->states.size();
    return dfa;
}

/******************************* Computational *******************************/

/*
 * More or less standard fsa state minimization algorithm.
 *
 * Note: Invalidates most of the computational data in the states, as it's
 * normally not needed after here anyway. (nfaStates, postAccepts, accepts)
 */

void DFA::minimize( void )
{
    int numStates = states.size();
    EquivClassSet ecs = EquivClassSet( numStates );

    /* Establish initial classes by accepted symbol */
    for( int i=0; i<numStates; i++ )
        ecs.setChar( i, (states[i].accept?states[i].accept->symbolId:0 ) );
    ecs.tidy(); // In the (unlikely) case of a symbol being accepted nowhere */
    assert(ecs.invariant()); /* paranoia yes */

    /* Find state equivalence classes (see Aho,Sethi,Ullman or similar for
     * explanations */
    bool changed;
    Bitset set = Bitset(numStates);
    do {
        changed = false;
        for( int ec=0; ec < ecs.getNumClasses(); ec++ ) {
            for( int edge=0; edge < numEquivs; edge++ ) {
                bool needSplit = false;
                set.clear();
                if( ecs.isSingletonClass( ec ) ) continue;
                EquivClassIterator mem = ecs.beginClass(ec);
                int rep = *mem;
                for( ++mem; mem != ecs.endClass(ec); ++mem ) {
                    if( ! ecs.isEquivalentTo( states[rep].moves[edge],
                                              states[*mem].moves[edge] ) ) {
                        changed = true; needSplit = true;
                        set.set(*mem);
                    }
                }
                if( needSplit )
                    ecs.splitCharClass(ec, set);
            }
        }
    } while( changed );

    /* Renumber and pack the state table (this is the fun bit) */
    int numMinStates = ecs.getNumClasses();
    if( numMinStates == numStates) return; /* Already minimal */
    ecs.sortClasses(); /* makes life a bit simpler */
    
    for( int i=1; i<numMinStates; i++ ) {
        states[i].accepts.clear();
        states[i].accept = states[ecs.getClassRep(i)].accept;
        int *srcedges = states[ecs.getClassRep(i)].moves;
        int *dstedges = states[i].moves;
        for( int j=0; j<numEquivs; j++ ) {
            dstedges[j] = ecs.getClass(srcedges[j]);
        }
    }

    /* Fixup the start state array */
    for( int i=0; i < numStartStates; i++ )
        startStates[i] = ecs.getClass(startStates[i]);

    /* Clear off the deceased states */
    for( int i=numStates; i > numMinStates; i-- ) states.pop_back();
}

/*
 * Digraph algorithm - PROOF: Show that this is sufficient
 * modified from lalr.cc. 
 */
void DFA::computeAccepts( Grammar &g )
{
    vector<int> stack;

    for( unsigned int state=1; state<states.size(); state++ ) {
        states[state].postAccepts = Bitset(g.lastTerminal+1);
        FOR_EACH( sym, set<Symbol *>, states[state].accepts ) {
            states[state].postAccepts.set((*sym)->symbolId);
        }
    }

    for( unsigned int state=1; state< states.size(); state++ ) {        
        if( states[state].N == 0 )
            traverse( stack, state );
    }
}

void DFA::traverse( vector<int> &stack, int x )
{
    stack.push_back( x );
    unsigned int depth = stack.size();
    states[x].N = depth;
    /* F(x) = F'(x) - skipped as F(x) pre-initialized */

    for( int i=0; i<numEquivs; i++ ) if( states[x].moves[i] != 0 ) {
        int y = states[x].moves[i];
        if( states[y].N == 0 ) traverse( stack, y );
        states[x].N = states[y].N < states[x].N ? states[y].N : states[x].N;
        states[x].postAccepts |= states[y].postAccepts;
    }
    if( states[x].N == depth ) {
        int top;
        do {
            top = stack.back();
            states[top].N = (unsigned)-1;
            if( top != x ) states[top].postAccepts = states[x].postAccepts;
            stack.pop_back();
        }while( top != x );
    }
}

void DFA::computePredSets( )
{
    for( unsigned int state=1; state<states.size(); state++ ) {
        states[state].predStates = Bitset(states.size());
        states[state].predStates.set(state);
    }
    computePredSetsSub( 1 );
}

void DFA::computePredSetsSub( int state )
{
    for( int move=0; move < numEquivs; move++ ) {
        int dest = states[state].moves[move];
        if( !dest ) continue;
        Bitset &destPred = states[dest].predStates;
        if( !(destPred.contains( states[state].predStates )) ) {
            destPred |= states[state].predStates;
            computePredSetsSub( dest );
        }
    }

}

    
int DFA::getNumStartStates( )
{
   int count = 0;
   if( startStates ) {
        bool seen[states.size()];
 
	for( int i=0; i<states.size(); i++ ) 
            seen[i]=false;
        for( int i=0; i<numStartStates; i++ ) {
            if( ! seen[startStates[i]] ) {
                seen[startStates[i]] = true;
                count++;
            }
        }
    }
    return count;
}

/****************************** State Functions ******************************/

int DFA::addState( void )
{
    int num = states.size();
    states.push_back( DFAState(numEquivs) );
    return num;
}

int DFA::addState( set<int> &nfaStates, NFA *nfa )
{
    int num = addState( );
    states[num].nfaStates = nfaStates;
    for( set<int>::iterator i = nfaStates.begin(); i != nfaStates.end(); i++ ){
        if( nfa->states[*i].accept != NULL )
            states[num].accepts.insert( nfa->states[*i].accept );
    }
    return num;
}

void DFA::getRow(int row, int *res)
{
    for( int i = 0; i < numEquivs; i++ ) {
        res[i] = states[row].moves[i];
    }
}


DFAState::DFAState( int num )
{
    moves = new int [num];
    memset( moves, 0, sizeof(int)*num);
    N = label = 0;
    accept = NULL;
    postAccepts=Bitset();
    predStates =Bitset();
}


/******************************** Text Output ********************************/

void DFA::print( FILE *f )
{
    unsigned int i,j;

    for( i = 1; i < states.size(); i++ ) {
        if( states[i].accept != NULL )
            fprintf( f, "[%d] {%s}  ", i, states[i].accept->name->c_str() );
        else if( ! states[i].accepts.empty() ) {
            fprintf( f, "[%d] {", i );
            FOR_EACH( n, set<Symbol *>, states[i].accepts ) {
                if( n != --states[i].accepts.end() )
                    fprintf( f, "%s,", (*n)->name->c_str() );
                else fprintf( f, "%s", (*n)->name->c_str() );
            }
            fprintf( f, "}  " );
        } else {
            fprintf( f, "(%d) ", i );
        }
//        if( i < numBaseStates )
//            states[i].postAccepts.print();
//        if( !states[i].nfaStates.empty() )
//            printIntSet( states[i].nfaStates );
        fprintf( f, ": " );
        for( j = 0; j < numEquivs; j++ ) {
            if( states[i].moves[j] ) {
                if( equivs )
                    equivs->printClass( j );
                else if( isprint(j) )
                    fprintf( f, "\'%c\'", j );
                else fprintf( f, "\\%03o", j );
                fprintf( f, " -> %d ", states[i].moves[j] );
            }
        }
        fprintf( f, "\n" );
    }

    if( equivs ) {
        fprintf( f, "Equivalence classes: %d\n", numEquivs );
        equivs->printClassSet();
        fprintf( f, "\n" );
    }
    if( startStates ) {
        fprintf( f, "Start states: %d\n   ",numStartStates );
        for( int i=0; i<numStartStates; i++ )
            fprintf( f, "%d, ", startStates[i] );
        fprintf( f, "\n" );
    }
}
