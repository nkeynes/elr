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
 * lalr.cc - algorithms to compute the LALR(1) lookahead sets given the LR0
 *           automaton and grammar. Derived from the paper by DeRemer and
 *           Pennello (1982)
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "grammar.h"
#include "lr.h"

#define FOR_EACH_STATE( _state ) for( LRStatePs::iterator _state = states.begin()+1; _state != states.end(); _state++ ) /* For all states */

/*
 * Implement DeRemer & Pennello's digraph algorithm
 * (Modified from Eve & Kurki-Suonio if you want to be pedantic)
 */
void LRTable::digraph( trav_func trav )
{
    vector<LRShiftEdge *> stack;
    
    FOR_EACH_STATE( state ) {
        for( int i = grammar->lastTerminal+1; i <= grammar->numSymbols; i++ ){
            if( (*state)->edges[i] != NULL && (*state)->edges[i]->N == 0 )
                traverse( stack, (LRShiftEdge *)(*state)->edges[i], trav );
        }
    }
}

void LRTable::traverse( vector<LRShiftEdge *> &stack, LRShiftEdge *x,
                        trav_func trav )
{
    stack.push_back( x );
    unsigned int depth = stack.size();
    x->N = depth;
    /* F(x) = F'(x) - skipped as F(x) pre-initialized */

    set<LRShiftEdge *> rel;
    (this->*trav)( x, rel );
    FOR_EACH( y, set<LRShiftEdge *>, rel ) {
        if( (*y)->N == 0 ) traverse( stack, *y, trav );
        x->N = ((*y)->N < x->N ? (*y)->N : x->N );
        x->read |= (*y)->read;
    }
    if( x->N == depth ) {
        LRShiftEdge *top;
        do {
            top = stack.back();
            top->N = (unsigned)-1;
            if( top != x ) top->read = x->read;
            stack.pop_back();
        }while( top != x );
    }
}

/*
 * Compute the "reads" relation for a given shift transition:
 * (p,A) reads (q,B) iff p--A-->q--B--> and B is nullable.
 * (p shifts A to state q, which shifts B)
 */
void LRTable::reads( LRShiftEdge *x, set<LRShiftEdge *> &result )
{
    LRState *state = states[x->dest];

//    printf("read( %s -> %d )\n", state->accessSym->name->c_str(), x->dest );

    for( int i = grammar->lastTerminal; i <= grammar->numSymbols; i++ ) {
        if( state->edges[i] != NULL && grammar->symbol(i)->isNullable )
            result.insert( (LRShiftEdge *)state->edges[i] );
    }
/*    printf("Returning: {" );
    FOR_EACH( i, set<LRShiftEdge *>, result ) {
        printf( " %s -> %d ,", states[(*i)->dest]->accessSym->name->c_str(),
                (*i)->dest );
    }
    printf( " }\n" );
*/
}

/*
 * Compute the "includes" relation for a given shift transition:
 * (p,A) includes (q,B) iff B ::= cAg and q--a-->p where c and g are (possibly
 * null) strings and g is nullable
 */
void LRTable::includes( LRShiftEdge *x, set<LRShiftEdge *> &result )
{
    /* Not a brilliantly fast rendition of includes, but it shall suffice
     * for the moment
     */
    LRState *dest = states[x->dest];
    int pred = -1;
    set<LRItem> *items = &dest->items;
    set<int> pStates[2];

    /* Determine immediate predecessor state */
    FOR_EACH( i, set<int>, dest->pred ) {
        if( states[*i]->edges[dest->accessSym->symbolId] == x ) {
            /* Found pred state */
            pred = *i;
            break;
        }
    }
    assert( pred != -1 ); /* Implies break in graph. Which can't happen :) */
    /* (says he who saw this assert failing. repeatedly.) */

//    printf("includes( %d -- %s -> %d )\n", pred, dest->accessSym->name->c_str(), x->dest );

    
    /* Backtrack until we find the state at which the tail-nullable rules were
     * added.
     */
    pStates[1].insert(pred);
    set<LRItem>::reverse_iterator item = items->rbegin();
    while( item != items->rend() && item->pos == 0 ) item++;
    
    for( int i=1; item != items->rend(); i++ ) {
        while( item->pos == i ) {
            if( item->rule->minLength <= item->pos ) {
                /* Add the resultant transitions to result */
                int sym = item->rule->nonterm->symbolId;
            
                FOR_EACH( j, set<int>, pStates[i&1] ) {
                    assert( states[*j]->edges[sym] != NULL );
                    result.insert( (LRShiftEdge *)states[*j]->edges[sym] );
                }
            }
            item++;
            if( item == items->rend() )
                goto done;
        }
        /* Backup another state */
        backtrack( pStates[(~i)&1], pStates[i&1] );
    }
    
  done:
/*    printf("Returning: {" );
    FOR_EACH( i, set<LRShiftEdge *>, result ) {
        printf( " %s -> %d ,", states[(*i)->dest]->accessSym->name->c_str(),
                (*i)->dest );
    }
    printf( " }\n" );
*/
    return;
}

/*
 * given a set of state numbers, return the set containing the state numbers of
 * the predecessor states, ie all states which can directly shift into one of
 * the given states.
 */
void LRTable::backtrack( set<int> &dest, set<int> &src )
{
    dest.clear();
    FOR_EACH( i, set<int>, src )
        FOR_EACH( j, set<int>, states[*i]->pred )
            dest.insert( *j );
} 

/*
 * compute the direct read sets for each nonterminal transition. That is, the
 * set of all terminals which can be shifted from the destination state.
 */
void LRTable::computeDirectReadSets( void )
{
    FOR_EACH_STATE(state) {
        for( int i = grammar->lastTerminal+1; i <= grammar->numSymbols; i++ ){
            /* For all shift transitions */
            LRShiftEdge *edge = (LRShiftEdge *)(*state)->edges[i];
            if( edge != NULL ) {
                edge->read.clear();
                LRState *dest = states[ edge->dest ];
                for( int j = 1; j <= grammar->lastTerminal; j++ ) {
                    if( dest->edges[j] != NULL )
                        edge->read.set( j );
                }
            }
        }
    }
}    

void LRTable::computeLookahead( int state, LRReduceEdge *edge )
{
    int lhs = edge->rule->nonterm->symbolId;
    unsigned int i;
    set<int> pState[2];

    pState[0].insert(state);
    for( i=0; i < edge->rule->syms.size(); i++ )
        backtrack( pState[(~i)&1], pState[i&1] );

    edge->lookahead = Bitset(grammar->lastTerminal+1);
    FOR_EACH( j, set<int>, pState[i&1] ) {
        if( states[*j]->edges[lhs] != NULL )
            edge->lookahead |= states[*j]->edges[lhs]->read;
    }
}

void LRTable::resolveState( int st )
{
    LRState *state = states[st];
    LRReduceEdge *edge;
    set<LRReduceEdge *> conflicts;

    if( state->isAdequate ) { /* Already OK, just fill in the edges if necessary */
        edge = (LRReduceEdge *)state->edges[0];
        if( edge ) { /* Pure reduce state (optimize out later ) */
            for( int i=1; i <= grammar->lastTerminal; i++ ) {
                if( edge->lookahead.get(i) || edge->lookahead.isEmpty() )
                    state->edges[i] = edge;
            }
        }
        return;
    }

    state->accepts = Bitset(grammar->lastTerminal+1);
    for( int i=1; i <= grammar->lastTerminal; i++ )
        if( state->edges[i] != NULL )
            state->accepts.set(i);

    for( edge = (LRReduceEdge *)state->edges[0]; edge;
         edge = (LRReduceEdge *)edge->chain ) {
        LRReduceEdge *edge2 = (LRReduceEdge *)edge->chain;
        bool hasConflict = false;
        while( edge2 ) {
            if( ! edge2->lookahead.isDisjoint( edge->lookahead ) ) {
                conflicts.insert( edge2 );
                hasConflict = true;
            }
            edge2 = (LRReduceEdge *)edge2->chain;
        }
        if( ! hasConflict ) { /* No reduce/reduce conflicts */
            if( edge->lookahead.isDisjoint( state->accepts ) ) {
                /* Resolvable by LALR(1) */
                state->accepts |= edge->lookahead;
                for( int i = 1; i <= grammar->lastTerminal; i++ ) {
                    if( edge->lookahead.get(i) ) {
                        assert( state->edges[i] == NULL );
                        state->edges[i] = edge;
                    }
                }
            } else { /* Shift/reduce conflict. Check for precedence */
                int len = edge->rule->syms.size();
                if(len >= 2 && edge->rule->syms[len-2].sym->hasPrecedence()) {
                    Symbol *sym = edge->rule->syms[len-2].sym;
                    for( int i=1; i <= grammar->lastTerminal; i++ ) {
                        if( edge->lookahead.get(i) && state->accepts.get(i) &&
                            ! grammar->symbol(i)->hasPrecedence()){
                            hasConflict = true;
                            break;
                        }
                    }
                    if( !hasConflict ) { /* Resolvable by precedence */
                        for( int i=1; i <= grammar->lastTerminal; i++ ) {
                            if( edge->lookahead.get(i) ) {
                                if( state->edges[i] == NULL )
                                    state->edges[i] = edge;
                                else {
                                    Symbol *sym2 = grammar->symbol(i);
                                    if( sym->prec > sym2->prec ||
                                        (sym->prec == sym2->prec &&
                                         sym->assoc == assoc_left ) ) {
                                        delete state->edges[i];
                                        state->edges[i] = edge;
                                    } else if( sym->prec == sym2->prec &&
                                               sym->assoc == assoc_none ) {
                                        delete state->edges[i];
                                        state->edges[i] = NULL;
                                        edge->lookahead.clear(i);
                                        /* Nonassoc -> error */
                                    } else {
                                        /* otherwise, shift takes precedence */
                                        edge->lookahead.clear(i);
                                    }
                                }
                            }
                        }
                    } else conflicts.insert(edge);
                } else conflicts.insert(edge);
            }
        } else conflicts.insert(edge);
    }
    if( conflicts.size() == 0 ) {
        state->isAdequate = true;
        numLALR1States++;
        numConflictStates--;
    } else { 
	/* Unresolved conflicts - Report and fallback on "last resort resolution" */
        FOR_EACH( edge, set<LRReduceEdge *>, conflicts ) {
            for( int i=1; i <= grammar->lastTerminal; i++ ) {
                if( (*edge)->lookahead.get(i) ) {
		    if( state->edges[i] == NULL ) {
			state->edges[i] = *edge;
		    } else {
			LREdge *oldedge = state->edges[i];
			/* If we have a higher-precedence reduce edge than the
			 * existing one, replace it with the higher-prec one.
			 */
                        if( state->edges[i]->type == REDUCE &&
			    ((LRReduceEdge *)state->edges[i])->rule->ruleId >
			    (*edge)->rule->ruleId ) {
			    state->edges[i] = *edge;
			}
			reportConflict( st, oldedge, *edge, i, state->edges[i] );
                    }
                }
            }
        }
    }
} 

void LRTable::reportConflict( int st, const LREdge *edge1, const LREdge *edge2, int terminal,
			      const LREdge *resolvedEdge )
{
    if( edge1->type == SHIFT ) {
	fprintf( stderr, "Warning: Shift/reduce conflict in state %d on terminal %s:\n", 
		 st, grammar->symbol(terminal)->name->c_str() );
    } else {
	fprintf( stderr, "Warning: Reduce/reduce conflict in state %d on terminal %s:\n", 
		 st, grammar->symbol(terminal)->name->c_str() );
    }
    unresolvedConflicts++;
}
    

void LRTable::computeLALRSets( void )
{
    computeDirectReadSets( );

    /* Compute the full read sets using the digraph algorithm */
    /* (assume all N = 0) */
    digraph( &LRTable::reads );

    /* Clear the values of N for the second pass */
    FOR_EACH_STATE( state ) {
        for( int i = 1; i <= grammar->numSymbols; i++ ) {
            if( (*state)->edges[i] != NULL )
                ((LRShiftEdge *)(*state)->edges[i])->N = 0;
        }
    }

    /* Compute the includes sets in the second digraph pass */
    digraph( &LRTable::includes );

    /* Finally compute the lookahead sets themselves, and then see if we've
     * resolved anything */
    for( unsigned int state = 1; state < states.size(); state++ ) {
        LRReduceEdge *edge = (LRReduceEdge *)states[state]->edges[0];
        while( edge != NULL ) {
            computeLookahead( state, edge );
            edge = (LRReduceEdge *)edge->chain;
        }
        resolveState( state );
    }
}

