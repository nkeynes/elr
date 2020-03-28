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

#ifndef elr_grammar_H
#define elr_grammar_H 1

#include <vector>
#include <list>
#include <string>
#include <unordered_map>

using namespace std;

#include "bitset.h"
#include "stringset2.h"


#pragma interface
class NFA; /* Defined fully in fsa.h */
class DFA;

#define FOR_EACH(i, T, V) for( T::iterator i = V.begin(); i != V.end(); i++ )

#define FOR_EACH_NONTERM( i ) FOR_EACH( i, NonterminalPs, nonterms )
#define FOR_EACH_TERM( i ) FOR_EACH( i, TerminalPs, terms )
#define FOR_EACH_RULE( i, nt ) FOR_EACH( i, RulePs, (nt)->rules )
#define FOR_EACH_RULESYM( i, r ) FOR_EACH( i, RuleSymbols, (r)->syms )

class Position {
  public:
    int line, column;
    const char *filename;
};
extern Position NO_POSITION;

class Node {
  public:
    Position posn;

};

typedef enum { assoc_left, assoc_right, assoc_none } assoc_t;

class ActionItem : public Node {
  public:
    string *action;

    ActionItem( string *act, Position &pos );
    ActionItem( ) { action = NULL; }
};

typedef vector<ActionItem> Action;

class Symbol : public Node {
  public:
    int symbolId;
    string *name;
    string *type;
    bool isTerminal;
    bool isReachable;
    bool isNullable;
    bool isPlaceholder;
    bool isResultUsed;
    int refs;
    int prec;
    assoc_t assoc;
    Bitset follows; /* Terminals that can follow this symbol in the input stream */

    Symbol( string *name, Position &posn );
    ~Symbol( ) { delete name; }
    bool hasPrecedence() { return isTerminal && prec != 0; }
};

typedef vector<Symbol> Symbols;

class RuleSymbol : public Node {
  public:
    Symbol *sym;
    Action *action;
    bool isResultUsed;

    RuleSymbol( Symbol *sym, Action *action, Position &pos );
};

typedef vector<RuleSymbol> RuleSymbols;

class Nonterminal;
class Rule : public Node {
  public:
    int ruleId;
    RuleSymbols syms;
    Nonterminal *nonterm;
    Action *preAction, *reduceAction;
    int minLength;

    Rule( int id );
    ~Rule( );
    void fixReduce( void );
    int length( ) const;
    void print( FILE *out );
};

typedef vector<Rule> Rules;
typedef vector<Rule *> RulePs;

class Terminal : public Symbol {
  public:
    NFA *nfa;
    Action *action; /* Lexical action */

    Terminal( string *name, Position &posn );
    static Terminal *fromString( string *name, Position &posn, bool caseSensitive );
    static Terminal *fromRegexp( string *name, Position &posn, bool caseSensitive );
};

typedef vector<Terminal> Terminals;
typedef vector<Terminal *> TerminalPs;

class Nonterminal : public Symbol {
  public:
    RulePs rules;
    StringSet2 *first;  /* FIRST-k for some k */

    Nonterminal( string *name, Position &posn );
    ~Nonterminal( );
    void setType( string *str );
};

typedef vector<Nonterminal> Nonterminals;
typedef vector<Nonterminal *> NonterminalPs;

class Grammar : public Node {
  public:
    string *ooClass;
    string *parserName;
    bool caseSensitive;
    bool autoLexDisambiguation;
    int expectedParserConflicts;
    Nonterminal *startSymbol;
    Terminal *eofSymbol, *errorTerm, *spaceTerm;
    DFA *dfa;
    int numRules;
    int numSymbols;
    int lastTerminal;
    TerminalPs terms;
    NonterminalPs nonterms;
    Action code;  /* Literal code chunk for both interface+implementation */
    Action interfaceCode;
    Action implementationCode;
    Action contextCode;
    Grammar( );
    ~Grammar( );
    
    void addCode( string *str, Position &posn );
    void addContextCode( string *str, Position &posn );
    void addImplementationCode( string *str, Position &posn );
    void addInterfaceCode( string *str, Position &posn );
    void setClass( string *str );
    void setName( string *str );
    void setStartSymbol( Nonterminal *sym );
    Terminal *add( Terminal *term );
    Nonterminal *add( Nonterminal *nonterm );
    Symbol *symbol( int symbolId );

    Symbol *lookupIdent( string *name, Position &posn );
    Symbol *getIdentByName( const char *name ); /* Doesn't create */
    Nonterminal *makeNonterminal( string *name, Position &posn );
    Terminal *lookupStringTerm( string *name, Position &posn );
    Terminal *lookupRegexpTerm( string *name, Position &posn );
    Terminal *makeLexicalSymbol( string *type, string *name, Terminal *term,
				 Action *act, Position &posn );

    bool resolveForwardRefs( void );
    void finalize( void ); /* call after grammar reading complete */
    void computeDFA( void );
    void printSymbolSet( Bitset &b, FILE *out );
    void printSymbolSet( StringSet2 &s );
//    void printTokenString( const TokenString &str );
//    void printStringSet( const StringSet2 &set );
    void dumpGrammar( void );
    void computeFirst( int k );
    
  protected:
    unordered_map<string,Symbol *> nontermHash;
    unordered_map<string,Terminal *> stringTermHash;
    unordered_map<string,Terminal *> regexpTermHash;

    /* Grammar computations run from finalize */
    void assignSymbolIds( void );
    void checkReachability( void );
    void computeNullable( void );
    void computeMinRuleLengths( void );
    void augment( void );
    void markReachable( Symbol *sym );
    void nullify( Nonterminal *nt, RulePs **used );
};

int yyerror( const char *s );
int yyparse();

#endif /* !elr_grammar_H */
