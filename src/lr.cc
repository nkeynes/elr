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
#include <stdio.h>
#include <assert.h>
#include <algorithm>
#include "grammar.h"
#include "lr.h"

namespace __gnu_cxx {
template <> struct hash< set<LRItem> > {
    size_t operator()( set<LRItem> __s ) const {
        size_t __h = 0;
        for( set<LRItem>::iterator i = __s.begin(); i != __s.end(); i++ )
            __h = 7*__h + 5*i->rule->ruleId + i->pos ;
        return __h;
    }
};
}

bool LRItem::operator <( const LRItem &item ) const
{
    /* Do not change this. Bad things will happen */
    return ( pos > item.pos ||
             (pos == item.pos && rule->ruleId < item.rule->ruleId ) );
}

bool LRItem::operator ==( const LRItem &item ) const
{
    return ( rule->ruleId == item.rule->ruleId &&
             pos == item.pos );
}

LRShiftEdge::LRShiftEdge( int d, Action *a, LREdge *c, int numTerms ) :
        LREdge( numTerms, SHIFT )
{
    dest = d; action = a; chain = c; N = 0; followsk = NULL;
}

LRReduceEdge::LRReduceEdge( Rule *r, Action *a, LREdge *c, int numTerms ) :
        LREdge( numTerms, REDUCE )
{
    rule = r; action = a; chain = c; followsk = NULL;
}


LRState::LRState( int num )
{
    numSymbols = num;
    edges = (LREdge **)calloc( sizeof(LREdge *), numSymbols );
    numReductions = 0;
    hasShifts = false;
    isAdequate = true;
    accessSym = NULL;
}

LRState::~LRState( )
{
    free( edges );
}

LRTable::LRTable( Grammar *gram )
{
    grammar = gram;
    numLR0States = numLALR1States = numLR1States = numConflictStates = 0;
    unresolvedConflicts = 0;
    states.push_back( new LRState( grammar->numSymbols+1 ) );
}

LRTable::~LRTable( )
{
    for( vector<LRState *>::iterator i=states.begin(); i!=states.end(); i++ ) {
        delete *i;
    }
    delete [] reduceLengths;
    delete [] reduceTokens;
}

void LRTable::getRow( int row, int *out )
{
    for( int i=0; i <= grammar->numSymbols; i++ ) {
        if( states[row]->edges[i] == NULL )
            out[i] = 0;
        else if( states[row]->edges[i]->type == SHIFT )
            out[i] = ((LRShiftEdge *)states[row]->edges[i])->dest;
        else
            out[i] = ((LRReduceEdge *)states[row]->edges[i])->rule->ruleId+
                states.size()-1;/* First reduce state follows last real state*/
    }
}

int LRTable::addState( set<LRItem> &items )
{
    int num = states.size();
    states.push_back( new LRState( grammar->numSymbols + 1 ) );
    states[num]->items = items;
    return num;
}

void LRTable::printItem( const LRItem &i )
{
    printf( "  (%d) %s -> ", i.rule->ruleId, i.rule->nonterm->name->c_str() );
    for( int j = 0; j < i.rule->length(); j ++ ) {
        if( j == i.pos ) printf( ". " );
        printf( "%s ", i.rule->syms[j].sym->name->c_str() );
    }
    if( i.pos == i.rule->length() ) printf( "." );
}    
void LRTable::printItemSet( set<LRItem> &items )
{
    for( set<LRItem>::iterator i = items.begin(); i != items.end(); i++ ) {
        printItem( *i );
//        if( i->followsk ) grammar->printStringSet( *i->followsk );

        printf("\n");
    }
}

void LRTable::constructPDA( void )
{
    hash_map<set<LRItem>,int> stateHash;
    set<LRItem> items;
    int next;

    /* Construct start state */
    items.insert( LRItem( grammar->startSymbol->rules[0], 0 ) );
    closure( grammar->startSymbol->rules[0]->syms[0].sym, items );
    unsigned int state = addState( items );
    stateHash[items] = state;

    /* And construct all the other states */
    for(; state < states.size(); state ++ ) {
        /* Check for shift moves */
        for( int i=1; i<=grammar->numSymbols; i++ ) {
            items.clear();
            Symbol *sym = grammar->symbol(i);
            shift( states[state]->items, sym, items );
            if( !items.empty() ) {
                next = stateHash[items];
                if( next == 0 ) {
                    next = addState( items );
                    stateHash[items] = next;
                    states[next]->accessSym = sym;
                } else { assert( states[next]->accessSym == sym ); }
                states[next]->pred.insert( state );
                states[state]->edges[i] =
                    new LRShiftEdge( next, NULL, NULL, grammar->lastTerminal );
                states[state]->hasShifts = true;
            }
        }
        /* Check for reduce moves */
        for( set<LRItem>::iterator i = states[state]->items.begin();
             i != states[state]->items.end(); i++ ) {
            if( i->pos == i->rule->length() ) {
                states[state]->edges[0] = new LRReduceEdge(
                    i->rule, i->rule->reduceAction, states[state]->edges[0],
                    grammar->lastTerminal);
                states[state]->numReductions++;
            }
        }
        if( states[state]->numReductions > 1 ||
            (states[state]->numReductions > 0 && states[state]->hasShifts ) ) {
            states[state]->isAdequate = false;
            numConflictStates++;
        } else numLR0States++;
    }
    
    buildReduceArrays();
}

void LRTable::closure( Symbol *sym, set<LRItem> &to )
{
    if( sym->isTerminal ) return; /* Terminal is already closed */
    Nonterminal *nt = (Nonterminal *)sym;
    
    for( RulePs::iterator i = nt->rules.begin(); i != nt->rules.end(); i++ ) {
        LRItem item = LRItem( *i, 0 );
        if( to.find( item ) == to.end() ) {
            to.insert( item );
            if( (*i)->length() > 0 )
                closure( (*i)->syms[0].sym, to );
        }
    }
}

void LRTable::shift( set<LRItem> &from, Symbol *sym, set<LRItem> &to )
{
    for( set<LRItem>::iterator i = from.begin(); i != from.end(); i++ ) {
        if( i->pos < i->rule->length() &&
            i->rule->syms[i->pos].sym == sym ) {
            to.insert( LRItem( i->rule, i->pos+1 ) );
            if( i->pos+1 < i->rule->length() )
                closure( i->rule->syms[i->pos+1].sym, to );
        }
    }
}

/* Compute the following terminals for each state and each symbol. This has
 * nothing to do with the LALR(1) generation - the results are used by the
 * lexical conflict resolver
 */
void LRTable::computeFollowSets( void )
{
    for( unsigned int state = 1; state < states.size(); state++ ) {
        states[state]->accepts = Bitset(grammar->lastTerminal+1);
        if( grammar->spaceTerm ) states[state]->accepts.set( grammar->spaceTerm->symbolId );
        for( int i=0; i<= grammar->lastTerminal; i++ ) {
            LREdge *edge = states[state]->edges[i];
            while( edge != NULL ) {
                if( states[state]->edges[i]->type == SHIFT ) {
                    states[state]->accepts.set(i);
                } else {
                    states[state]->accepts |=
                        ((LRReduceEdge *)states[state]->edges[i])->lookahead;
                }
                edge = edge->chain;
            }
        }
        Symbol *sym = states[state]->accessSym;
        if( sym != NULL ){ /* not the start state */
            if( sym->follows.size() == 0 )
                sym->follows = Bitset(grammar->lastTerminal+1);
            sym->follows |= states[state]->accepts;
        }
    }
}

/*
 * Construct the reduce length / reduce token arrays. Currently fairly
 * straightforward, but will need to change somewhat for ShiftReduce states
 * when/if added
 */
void LRTable::buildReduceArrays( void )
{
    numTotalStates = states.size() + grammar->numRules;
    reduceLengths = new int [numTotalStates];
    reduceTokens = new int [numTotalStates];
    memset( reduceLengths, 0, numTotalStates );
    memset( reduceTokens, 0, numTotalStates );
    FOR_EACH( nt, NonterminalPs, grammar->nonterms ) {
        FOR_EACH( rule, RulePs, (*nt)->rules ) {
            reduceLengths[(*rule)->ruleId+states.size()-1] = (*rule)->length();
            reduceTokens[(*rule)->ruleId+states.size()-1] = (*nt)->symbolId;
            if( *nt == grammar->startSymbol )
                acceptState = (*rule)->ruleId+states.size()-1;
        }
    }
}

void LRTable::dumpTable( void )
{
    for( unsigned int state = 1; state < states.size(); state++ ) {
        printState( state );
    }
}

void LRTable::printState( int state ) {
    bool hadShift = false;
    
    printf( "State %d:", state );
    if( ! states[state]->isAdequate )
        printf( " (inadequate)" );
    printf( "\n" );
    printItemSet( states[state]->items );
    printf( "\n" );
    
    int i = grammar->lastTerminal+1;
    do {
        if( states[state]->edges[i] != NULL &&
            states[state]->edges[i]->type == SHIFT ) {
            printf( "  shift %s -> %d  ",
                    grammar->symbol(i)->name->c_str(),
                    ((LRShiftEdge *)states[state]->edges[i])->dest );
            printf( "\n" );
            hadShift = true;
        }
        if( i == grammar->numSymbols ) {
            i = 0;
            if( hadShift == true ) {
                printf( "\n" );
                hadShift = false;
            }
        }
        i++;
    } while( i != grammar->lastTerminal+1 );
    if( hadShift ) printf( "\n" );
    for( LRReduceEdge *edge = (LRReduceEdge *)states[state]->edges[0];
         edge != NULL; edge = (LRReduceEdge *)edge->chain ) {
        printf( "  reduce rule %d  ", edge->rule->ruleId );
        grammar->printSymbolSet( edge->lookahead );
        printf( "\n" );
    }
    if( states[state]->edges[0] != NULL ) printf( "\n" );
    printf( "\n" );
}    

void LRTable::printSummary( void )
{
    printf("%d total states\n", states.size()-1 );
    if( states.size() > 1 ) {
        printf("%d (%.2f%%) states were LR(0)\n", numLR0States,
               numLR0States * 100.0 / (states.size()-1) );
        printf("%d (%.2f%%) states were LALR(1)\n", numLALR1States,
               numLALR1States * 100.0 / (states.size()-1) );
        printf("%d (%.2f%%) states were LR(1)\n", numLR1States,
               numLR1States * 100.0 / (states.size()-1) );
        printf("%d (%.2f%%) states contain conflicts\n", numConflictStates,
               numConflictStates * 100.0 / (states.size()-1) );
    }
}



