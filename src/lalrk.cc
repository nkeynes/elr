/*
 * lalr(k) implementation. Fairly straightforward version of Kristensen &
 * Madsen's algorithm.
 */

#include <stdio.h>
#include "lr.h"
#include "grammar.h"

void LRTable::computeLALRkSets( int k )
{
    for( int state = 1; state < states.size(); state++ ) {
        fprintf( stderr, "%d", state );
        FOR_EACH( item, set<LRItem>, states[state]->items ) {
            fprintf( stderr, "." );
            ((LRItem *)(&*item))->followsk =
                new StringSet2(grammar->lastTerminal); /* Ugh */
            lalrk( *item, state, k, *item->followsk );

            /* Apply the item to the actual automata */
            if( item->pos < item->rule->length() ) {
                Symbol *sym = item->rule->syms[item->pos].sym;
                assert( states[state]->edges[sym->symbolId] );
                if( !states[state]->edges[sym->symbolId]->followsk )
                    states[state]->edges[sym->symbolId]->followsk =
                        new StringSet2(grammar->lastTerminal);
                StringSet2 seta = StringSet2(grammar->lastTerminal);
                firstk(seta, *item);
                seta.append( *item->followsk );
                states[state]->edges[sym->symbolId]->followsk->add( seta );
            } else {
                LRReduceEdge *edge = (LRReduceEdge *)states[state]->edges[0];
                while( edge ) {
                    if( edge->rule == item->rule ) {
                        edge->followsk = item->followsk;
                        break;
                    }
                    edge = (LRReduceEdge *)edge->chain;
                }
            }
        }
    }

    for( int state = 1; state < states.size(); state++ ) {
        FOR_EACH( item, set<LRItem>, states[state]->items ) {
            delete item->follows[1];
            delete item->follows[2];
        }
    }
}

void LRTable::lalrk( const LRItem &item, int state, int k, StringSet2 &result )
{
    set<const LRItem *> done;

    if( item.rule->nonterm == grammar->startSymbol )
        result.addEmpty();
    else
        lalrksub( item, state, k, result, done );
    return;
}

void LRTable::lalrksub( const LRItem &item, int state, int k,
                        StringSet2 &result, set<const LRItem *> &done )
{
    StringSet2 set1 = StringSet2(grammar->lastTerminal);
    set<int> pred[2];
    int d;

    if( item.follows[k] ) {
        result = *item.follows[k];
        return;
    }
    done.insert( &item );
    /* Compute PRED(state,item) */
    pred[0].insert(state);
    for( d=0; d<item.pos; d++ ) {
        backtrack( pred[(~d)&1], pred[d&1] );
    }
    /* pred[i&1] now contains all predecessor states */
    FOR_EACH( p, set<int>, pred[d&1] ) {
        FOR_EACH( i, set<LRItem>, states[*p]->items ) {
            if( i->rule->length() > 0 &&
                i->rule->syms[i->pos].sym == item.rule->nonterm ) {
                firstk( set1, *i );
                result.addMax( set1 );
                int j = k - set1.minLength();
                bool hasEmpty = set1.hasEmpty();
                if( j > 0 && j < k ) {
                    StringSet2 set2 = StringSet2(grammar->lastTerminal);
                    lalrk( *i, *p, j, set2 );
                    set1.deleteEmpty();
                    set1.append( set2 );
                    result.add( set1 );
                }
                if( hasEmpty && done.find(&(*i)) == done.end() ) {
                    lalrksub( *i, *p, k, result, done );
                }
            }
        }
    }
    /* So sue me... */
    ((LRItem &)item).follows[k] = new StringSet2(result);
}

void LRTable::firstk( StringSet2 &result, const LRItem &item )
{
    result.clear();
    result.addEmpty();
    for( int i=item.pos+1; i<item.rule->length();i++ ) {
        Symbol *sym = item.rule->syms[i].sym;
        if( sym->isTerminal ) result.append( sym->symbolId );
        else result.append( *((Nonterminal *)sym)->first );
    }
}        

