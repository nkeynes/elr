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
 * grammar.cc - Constructors, destructors, and various functions for the
 *              construction of the grammar data structures. Mostly referenced
 *              from the input reader.
 */
#include <string>
#include <vector>
#include <iterator>

#pragma implementation "grammar.h"
#include "grammar.h"
#include "dfa.h"
#include "nfa.h"

Position NO_POSITION = {0,0,NULL};

/********************* class Grammar *******************/

void Grammar::addCode( string *str, Position &pos )
{
    code.push_back( ActionItem(str, pos) );
}

void Grammar::addContextCode( string *str, Position &pos )
{
    contextCode.push_back( ActionItem(str, pos) );
}

void Grammar::addImplementationCode( string *str, Position &pos )
{
    implementationCode.push_back( ActionItem(str, pos) );
}

void Grammar::addInterfaceCode( string *str, Position &pos )
{
    interfaceCode.push_back( ActionItem(str, pos) );
}

void Grammar::setClass( string *str )
{
    if( ooClass ) delete ooClass;
    ooClass = str;
}

void Grammar::setName( string *str )
{
    if( parserName ) delete parserName;
    parserName = str;
}

void Grammar::setStartSymbol( Nonterminal *sym )
{
    startSymbol = sym;
}

Terminal *Grammar::add( Terminal *term )
{
    terms.push_back( term );
    return term;
}

Nonterminal *Grammar::add( Nonterminal *nonterm )
{
    nonterms.push_back( nonterm );
    return nonterm;
}

Symbol *Grammar::symbol( int symbolId )
{
    if( symbolId <= lastTerminal ) return terms[symbolId-1];
    else return nonterms[symbolId-lastTerminal-1];
}

void Grammar::computeDFA( void )
{
    NFA *fsa = NFA::fromEmpty();
    for( TerminalPs::iterator i = terms.begin(); i != terms.end(); i++ ) {
        if( (*i)->nfa != NULL )
            fsa->addAlternation( (*i)->nfa );
    }
//    printf("NFA:\n");
//    fsa->print(stdout);
    fsa->computeEquivClasses();
    dfa = DFA::fromNFA( fsa );
//    printf("\nDFA:\n");
//    dfa->print(stdout);
//    printf("\nDFA Checks\n");
//    dfa->checkDuplicateSymbols( *this );
//    printf("\n");
    delete fsa;
}


/****************** Lookup routines ****************/
Symbol *Grammar::getIdentByName( const char *name )
{
    string *str= new string(name);
    Symbol *sym = nontermHash[*str];
    delete str;
    return sym;
}

Terminal *Grammar::lookupStringTerm( string *name, Position &posn )
{
    Terminal *sym = stringTermHash[*name];
    if( sym == NULL ) {
        sym = Terminal::fromString( name, posn, caseSensitive );
        stringTermHash[*name] = sym;
        add( sym );
    } else delete name;
    return sym;
}
Terminal *Grammar::lookupRegexpTerm( string *name, Position &posn )
{
    Terminal *sym = regexpTermHash[*name];
    if( sym == NULL ) {
        sym = Terminal::fromRegexp( name, posn, caseSensitive );
        regexpTermHash[*name] = sym;
        add( sym );
    } else delete name;
    return sym;
}

Symbol *Grammar::lookupIdent( string *name, Position &posn )
{
    Symbol *sym = nontermHash[*name];
    if( sym ) {
        delete name;
        return sym;
    }
    return new Symbol( name, posn );
}    

Nonterminal *Grammar::makeNonterminal( string *name, Position &posn )
{
    Symbol *sym = nontermHash[*name];
    if( sym == NULL || sym->isTerminal ) {
        if( sym )
            printf( "Error: Lexical identifier redefined as nonterminal\n" );
        sym = new Nonterminal( name, posn );
        nontermHash[*name] = sym;
        add( (Nonterminal *)sym );
    } else delete name;
    return (Nonterminal *)sym;
}


Terminal *Grammar::makeLexicalSymbol( string *type, string *name, 
                           Terminal *term, Action *action, Position &posn )
{
    Symbol *sym = nontermHash[*name];
    if( sym != NULL ) {
        printf( "Error: Attempt to redefine nonterminal\n ");
        delete type;
        delete name;
        delete action;
    }
    if( action != NULL && action->size() == 0 ) {
	delete action;
	action = NULL;
    }
    nontermHash[*name] = term;
    term->action = action;
    term->type = type;
    delete term->name;
    term->name = name;
    add( term );
}

/******************** Constructors *********************/

Grammar::Grammar( )
{
    ooClass = parserName = NULL;
    startSymbol = NULL;
    eofSymbol = NULL;
    errorTerm = new Terminal(new string("error"), NO_POSITION);
    add(errorTerm);
    dfa = NULL;
    numRules = numSymbols = lastTerminal = 0;
    caseSensitive = true;
    autoLexDisambiguation = true;
    expectedParserConflicts = 0;
}

Grammar::~Grammar( )
{
    if( ooClass ) delete ooClass;
    if( parserName ) delete parserName;
    if( dfa ) delete dfa;
    FOR_EACH( t, TerminalPs, terms ) { delete *t; }
    FOR_EACH( nt, NonterminalPs, nonterms ) { delete *nt; }
}
    

Symbol::Symbol( string *str, Position &pos )
{
    name = str;
    posn = pos;
    refs = prec = 0;
    assoc = assoc_none;
    type = NULL;
    symbolId = 0;
    follows = Bitset();
    isNullable = false;
    isReachable = false; /* Set to true when we see the symbol used */
    isPlaceholder = true;
    isResultUsed = false;
}    

Terminal::Terminal( string *str, Position &pos ) : Symbol(str, pos)
{
    nfa = NULL;
    isTerminal = true;
    action = NULL;
    isPlaceholder = false;
}

Terminal *Terminal::fromString( string *str, Position &pos, bool caseSensitive ) {
    Terminal *term = new Terminal( str, pos );
    string s = str->substr(1,str->length()-2);    
    term->nfa = NFA::fromString( s, term, caseSensitive );
    return term;
}

Terminal *Terminal::fromRegexp( string *str, Position &pos, bool caseSensitive ) {
    Terminal *term = new Terminal( str, pos );
    string s = str->substr(1,str->length()-2);
    term->nfa = NFA::fromRegexp( s, term, caseSensitive );
    return term;
}

Nonterminal::Nonterminal( string *str, Position &pos ) : Symbol( str, pos )
{
    isTerminal = false;
    isPlaceholder = false;
}

Nonterminal::~Nonterminal( )
{
    FOR_EACH_RULE( i, this ) {
        delete *i;
    }
    if( name ) {
	delete name;
	name = NULL;
    }
    if( type ) {
	delete type;
	type = NULL;
    }
}

ActionItem::ActionItem(  string *act, Position &pos )
{
    action = act;
    posn = pos;
}

RuleSymbol::RuleSymbol( Symbol *s, Action *a, Position &pos )
{
    sym = s;
    if( a != NULL && a->size() == 0 ) {
	delete a;
	a = NULL;
    }
    action = a;
    posn = pos;
    isResultUsed = false;
    sym->refs++;
}

Rule::Rule( int id )
{
    reduceAction = preAction = NULL;
    ruleId = id;
    nonterm = NULL;
}

Rule::~Rule( )
{
    FOR_EACH_RULESYM( i, this ) {
        if( i->action ) delete i->action;
        i->sym->refs--;
    }
    if( preAction ) delete preAction;
    if( reduceAction ) delete reduceAction;
}
    

void Rule::fixReduce( void )
{
    if( syms.size() > 0 ) {
        reduceAction = syms.back().action;
        syms.back().action = NULL;
    } else {
        reduceAction = preAction;
        preAction = NULL;
    }
}

int Rule::length( ) const
{
    return syms.size();
}

void Rule::print( FILE *out ) 
{
    if( this->nonterm->type != NULL ) {
	fprintf( out, "(%s) ", this->nonterm->type->c_str() );
    }
    fprintf( out, "%s ::= ", this->nonterm->name->c_str());
    FOR_EACH( k, RuleSymbols, this->syms ) {
	fprintf( out, "%s ", k->sym->name->c_str() );
    }
}

void Nonterminal::setType( string *str )
{
    if( type == NULL ) type = str;
    else if( *type == *str ) delete str;
    else yyerror( "Multiple types declared on nonterminal\n" );
}

