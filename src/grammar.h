
#ifndef elr_grammar_H
#define elr_grammar_H 1

#include <vector>
#include <list>
#include <string>
#include <hash_map>
#include "bitset.h"
#include "stringset2.h"

#pragma interface
class NFA; /* Defined fully in fsa.h */
class DFA;

#define FOR_EACH(i, T, V) for( T::iterator i = V.begin(); i != V.end(); i++ )

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

typedef enum { left, right, none } assoc_t;

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
};

typedef vector<Rule> Rules;
typedef vector<Rule *> RulePs;

class Terminal : public Symbol {
  public:
    NFA *nfa;
    Action *action; /* Lexical action */

    Terminal( string *name, Position &posn );
    static Terminal *fromString( string *name, Position &posn );
    static Terminal *fromRegexp( string *name, Position &posn );
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


struct hashString : hash<string> {
    hash<const char *> h;
    size_t operator()(string __s) const { return h(__s.c_str()); }
};

class Grammar : public Node {
  public:
    string *ooClass;
    string *parserName;
    Nonterminal *startSymbol;
    Terminal *eofSymbol, *errorTerm, *spaceTerm;
    DFA *dfa;
    int numRules;
    int numSymbols;
    int lastTerminal;
    TerminalPs terms;
    NonterminalPs nonterms;
    Action code;  /* Literal code chunks */

    Grammar( );
    ~Grammar( );
    
    void addCode( string *str, Position &posn );
    void setClass( string *str );
    void setStartSymbol( Nonterminal *sym );
    Terminal *add( Terminal *term );
    Nonterminal *add( Nonterminal *nonterm );
    Symbol *symbol( int symbolId );

    Symbol *lookupIdent( string *name, Position &posn );
    Symbol *getIdentByName( char *name ); /* Doesn't create */
    Nonterminal *makeNonterminal( string *name, Position &posn );
    Terminal *lookupStringTerm( string *name, Position &posn );
    Terminal *lookupRegexpTerm( string *name, Position &posn );
    Terminal *makeLexicalSymbol( string *type, string *name, Terminal *term,
				 Action *act, Position &posn );

    bool resolveForwardRefs( void );
    void finalize( void ); /* call after grammar reading complete */
    void computeDFA( void );
    void printSymbolSet( Bitset &b );
//    void printTokenString( const TokenString &str );
//    void printStringSet( const StringSet2 &set );
    void dumpGrammar( void );
    void computeFirst( int k );
    
  protected:
    hash_map<string,Symbol *,hashString> nontermHash;
    hash_map<string,Terminal *,hashString> stringTermHash;
    hash_map<string,Terminal *,hashString> regexpTermHash;

    /* Grammar computations run from finalize */
    void assignSymbolIds( void );
    void checkReachability( void );
    void computeNullable( void );
    void computeMinRuleLengths( void );
    void augment( void );
    void markReachable( Symbol *sym );
    void nullify( Nonterminal *nt, RulePs **used );
};

int yyerror( char *s );
int yyparse();

#endif /* !elr_grammar_H */
