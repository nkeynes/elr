
#ifndef elr_nfa_H
#define elr_nfa_H 1

#pragma interface

#include <stdio.h>
#include <vector>
#include <set>
#include "bitset.h"

class DFA;
class EquivClassSet;
class Symbol;

class NFAState {
  public:
    Symbol *accept; /* Accept sym (NULL = none) */
    Bitset on;
    int dest;       /* 0 = no move */
    vector<int> epsilonMoves;

    NFAState( int num ) { on = Bitset(num); dest=0; accept = NULL; }

    void addEmptyMove( int dest );
    void addCharMove( int ch, int dest );
    void addClassMove( string::iterator *s, int dest );
};

class NFA { 
  public:
    vector <NFAState> states;
    EquivClassSet *equivs;
    int numEquivs;
    
    NFA( int charsetsize = CHARSET_SIZE );
    
    static NFA *fromString( string &s, Symbol *sym );
    static NFA *fromRegexp( string &s, Symbol *sym );
    static NFA *fromEmpty( void );
    static NFA *fromEOF( Symbol *sym );
    void addAlternation( NFA *fsa );
    
    void epsilonClosure( int state, set<int> &in );
    void move( set<int> &from, int ch, set<int> &to );
    void computeEquivClasses( void );
    void print( FILE *out );
  protected:
    int addState( void );
    int parseRegexpString( string::iterator *s, int start );
    int parseRegexpChoice( string::iterator *s, int start );
    void genRegexpNFA( string &s, Symbol *sym );
    void genStringNFA( string &s, Symbol *sym );
};                               


#endif /* !elr_nfa_H */
