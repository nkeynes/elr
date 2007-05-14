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
 * grammar2.cc - algorithms which transform and compute properties of the
 *               grammar itself, as opposed to on the lr automaton.
 *               (generally in preparation for generation of the parser)
 */

#include <vector>
#include <set>
#include <stdio.h>
#include "config.h"
#include "grammar.h"
#include "nfa.h"

#define FOR_EACH_NONTERM( i ) FOR_EACH( i, NonterminalPs, nonterms )
#define FOR_EACH_TERM( i ) FOR_EACH( i, TerminalPs, terms )
#define FOR_EACH_RULE( i, nt ) FOR_EACH( i, RulePs, (nt)->rules )
#define FOR_EACH_RULESYM( i, r ) FOR_EACH( i, RuleSymbols, (r)->syms )

/******** Public Functions *********/

void Grammar::finalize( void )
{
    if( parserName == NULL ) parserName = new string(DEFAULT_PARSER_NAME);
    if( !resolveForwardRefs() ) abort();
    Symbol *sym = getIdentByName( WHITESPACE_TERM );
    if( sym && sym->isTerminal ) spaceTerm = (Terminal *)sym;

    checkReachability();
    augment();
    assignSymbolIds();
    computeNullable();
    computeMinRuleLengths();
}


/********* Protected Functions **********/

/*
 * Resolve forward identifier references - replace every placeholder symbol
 * with the real one, or raise an error (undefined identifier)
 */
bool Grammar::resolveForwardRefs( void )
{
    bool error = false;
    
    FOR_EACH_NONTERM( nt ) {
        FOR_EACH_RULE( i, *nt ) {
            FOR_EACH_RULESYM( rs, *i ) {
                if( rs->sym->isPlaceholder ) {
                    Symbol *sym = nontermHash[*rs->sym->name];
                    if( sym == NULL ) {
                        error = true;
                        printf( "Undefined identifier %s\n", rs->sym->name->c_str() );
                    } else {
                        delete rs->sym;
                        rs->sym = sym;
                    }       
                }
            }
        }
    }
    return !error;
}

/*
 * Check that all symbols are reachable from the start symnol. Warn about and
 * remove any symbols (and rules) that we can't get to.
 */
void Grammar::checkReachability( void )
{
    markReachable( startSymbol );
    if( spaceTerm ) spaceTerm->isReachable = true;
    errorTerm->isReachable = true;
    int count;

    /* Now check if any symbols weren't seen */
    count = 0;
    FOR_EACH_NONTERM(i) {
        if( ! (*i)->isReachable ) {
            if( count == 0 ) printf( "Nonterminal(s) " );
            printf("%s ", (*i)->name->c_str());
            delete *i;
            i = nonterms.erase(i);
            if( i == nonterms.end() ) break;
            i--;
            count++;
        }
    }
    if( count != 0 ) printf( "unreachable - deleted (%d)\n", count );
    count = 0;
    FOR_EACH_TERM(i) {
        if( ! (*i)->isReachable ) {
            if( count == 0 ) printf( "Terminal(s) " );
            printf("%s ", (*i)->name->c_str());
            delete *i;
            i = terms.erase(i);
            if( i == terms.end() ) break;
            i--;
            count++;
        }
    }
    if( count != 0 ) printf( "unreachable - deleted (%d)\n", count );
}

/*
 * Recursively traverse the grammar, marking all observed symbols as reachable
 */
void Grammar::markReachable( Symbol *sym )
{
    sym->isReachable = true;
    if( sym->isTerminal ) return;

    Nonterminal *nt = (Nonterminal *)sym;
    FOR_EACH_RULE( i, nt ) {
        FOR_EACH_RULESYM( j, *i ) {
            if( ! j->sym->isReachable )
                markReachable( j->sym );
        }
    }
}

/*
 * Add the production #START# -> <startSym> $ to the grammar, along with the
 * new start & eof symbols.
 */
void Grammar::augment( void )
{
    Terminal *eof = new Terminal( new string("$"), NO_POSITION );
    eof->nfa = NFA::fromEOF(eof);
    add( eof );
    eofSymbol = eof;
    Nonterminal *start = makeNonterminal( new string("#START#"), NO_POSITION );
    Rule *startRule = new Rule( ++numRules );
    startRule->syms.push_back( RuleSymbol( startSymbol, NULL, NO_POSITION ) );
    startRule->syms.push_back( RuleSymbol( eof, NULL, NO_POSITION ) );
    startRule->nonterm = start;
    start->rules.push_back( startRule );
    startSymbol = start;
}


/*
 * Set each symbol's symbolId to whatever it should be. Basically number
 * terminals in array order from 1, and then nonterminals immediately after.
 * Also set numSymbols, just in case.
 * ( sym == symbol( sym->symbolId ) )
 */
void Grammar::assignSymbolIds( void )
{
    int num = 0;
    for( vector<Terminal *>::iterator i=terms.begin(); i != terms.end(); i++ )
        (*i)->symbolId = ++num;
    lastTerminal = num;
    for( vector<Nonterminal *>::iterator i = nonterms.begin();
         i != nonterms.end(); i++ )
        (*i)->symbolId = ++num;
    numSymbols = num;
}

/*
 * Compute the nullable nonterminals (those that can produce nothing)
 * TODO: need to prove that this procedure covers all cases
 * Note: modifies minLength in all rules. symbol IDs must have been set.
 */
void Grammar::computeNullable( void )
{
    RulePs *used = new RulePs[numSymbols+1];

    /* For each symbol, determine the rules in which it occurs */
    FOR_EACH_NONTERM( i ) {
        FOR_EACH_RULE( j, *i ) {
            (*j)->minLength = (*j)->syms.size();
            FOR_EACH_RULESYM( k, *j )
                if( k->sym->isTerminal == false )
                    used[ k->sym->symbolId ].push_back( *j );
        }
    }

    /* Now, for each symbol with a null rule, set isNullable to true and
     * propagate the fact to all rules referencing that symbol
     */
    FOR_EACH_NONTERM( i ) {
        FOR_EACH_RULE( j, *i ) {
            if( (*j)->minLength == 0 ) {
                nullify(*i, &used);
                break;
            }
        }
    }

    delete []used;
}

/*
 * Compute FIRSTk() of each nonterminal in the grammar. Algorithm used is based
 * on that given in Aho & Ullman. This is probably painfully slow at present.
 */

# define TABLE(i) table[(i)&1]

typedef vector<StringSet2> table_row;

void Grammar::computeFirst( int k )
{
    int numNonTerms = numSymbols - lastTerminal;
    table_row table[2] = { table_row(numNonTerms, StringSet2(lastTerminal)),
                           table_row(numNonTerms, StringSet2(lastTerminal)) };
    StringSet2 set = StringSet2(lastTerminal);
    int i;
    bool done = false;

    /* Base case - Initialize first row such that
     *   F(0,A) = { x : Terminals* | A -> xa and |x| = k || a.isNullable() }
     */
    FOR_EACH_NONTERM( nt ) {
        FOR_EACH_RULE( rule, *nt ) {
            int str[k];
            int l = 0;
            FOR_EACH_RULESYM( rulesym, *rule ) {
                if( ! rulesym->sym->isTerminal ) {
                    do { /* Check if tail is nullable */
                        if( ! rulesym->sym->isNullable ) {
                            l = -1;
                            break;
                        }
                    } while( ++rulesym != (*rule)->syms.end() );
                    break;
                }
                str[l] = rulesym->sym->symbolId;
                if( ++l == k ) break;
            }
            if( l != -1 )
                table[0][(*nt)->symbolId-lastTerminal-1].add( str, l );
        }
    }

    /* Recursive case. Compute F(i,A) until F(i,A) = F(i-1,A) for all A */
    for( i=1; !done; i++ ) {
        done = true;
        for( int nt = 0; nt < numNonTerms; nt++ ) {
            TABLE(i)[nt].clear();
            FOR_EACH_RULE( rule, nonterms[nt] ) {
                set.clear();
                set.addEmpty( );
                FOR_EACH_RULESYM( rulesym, *rule ) {
                    Symbol *sym = rulesym->sym;
                    if( sym->isTerminal ) set.append( sym->symbolId );
                    else set.append( TABLE(i-1)[sym->symbolId-lastTerminal-1]);
                    if( set.minLength() == k )
                        break; /* Can't possibly append any more */
                }
                TABLE(i)[nt].add( set );
            }
            TABLE(i)[nt].add( TABLE(i-1)[nt] );

            if( TABLE(i)[nt] != TABLE(i-1)[nt] )
                done = false;
        }
    }
    for( int nt = 0; nt < numNonTerms; nt++ )
        nonterms[nt]->first = new StringSet2(TABLE(i)[nt]);
    
//    delete []table;
}

/*
 * Set the given nonterminal to nullable, and mark each rule in which it occurs
 * as having 1 less non-nullable symbol. For any rules which are found to have
 * no non-nullable symbols, recursively mark their non-terminals as nullable.
 */
void Grammar::nullify( Nonterminal *nt, RulePs **used )
{
    nt->isNullable = true;
    FOR_EACH( i, RulePs, (*used)[nt->symbolId] ) {
       (*i)->minLength--;
       if( (*i)->minLength == 0 && (*i)->nonterm->isNullable == false )
           nullify( (*i)->nonterm, used );
    }
}

/*
 * compute the minimum length of each rule, ie the position in the rule after
 * which all further symbols are nullable. (Used to compute includes)
 */
void Grammar::computeMinRuleLengths( void )
{
    FOR_EACH_NONTERM( i ) {
        FOR_EACH_RULE( j, *i ) {
            (*j)->minLength = (*j)->syms.size();
            for( RuleSymbols::reverse_iterator k = (*j)->syms.rbegin();
                 k != (*j)->syms.rend() && k->sym->isNullable; k++ ) {
                (*j)->minLength--;
            }
        }
    }
}

void Grammar::printSymbolSet( Bitset &b )
{
    printf( "{" );
    if( b.size() != 0 ) {
        for( int i=0; i <= lastTerminal; i++ ) {
            if( b[i] )
                printf( " %s", symbol(i)->name->c_str() );
        }
    }
    printf( " }" );
}

void Grammar::printSymbolSet( StringSet2 &s )
{
    s.print(this);
}    

void Grammar::dumpGrammar( void )
{
    printf( "Symbols: %d\n", numSymbols );
    printf( "Terminals: %d\n", lastTerminal );
    for( int i=1; i <= lastTerminal; i++ ) {
        Symbol *sym = terms[i-1];
        printf("  (%d) %s ", i, sym->name->c_str() );
//        if( sym->follows )
//            printSymbolSet( sym->follows );
        printf( "\n" );
    }
    
    printf( "Nonterminals: %d\n", numSymbols - lastTerminal );
    FOR_EACH( i, NonterminalPs, nonterms ) {
        if( (*i)->isNullable )
            printf( "  (%d) [%s] ", (*i)->symbolId,
                    (*i)->name->c_str() );
        else printf( "  (%d)  %s ", (*i)->symbolId,
                     (*i)->name->c_str() );
//        if( (*i)->follows )
//            printStringSet( (*i)->first );
        if( (*i)->first ) (*i)->first->print(this);
        printf( ":\n" );
        FOR_EACH( j, RulePs, (*i)->rules ) {
            printf( "     (Rule %d) -> ", (*j)->ruleId );
//                    (*j)->nonterm->name->c_str());
            FOR_EACH( k, RuleSymbols, (*j)->syms ) {
                printf( "%s ", k->sym->name->c_str() );
            }
            printf("\n");
        }
    }
}
