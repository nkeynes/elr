
#ifndef elr_dfa_H
#define elr_dfa_H 1

#pragma interface

#include <vector>
#include <set>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "bitset.h"
#include "combvect.h"

class Symbol;
class ConflictMap;
class DFA;
class NFA;
class EquivClassSet;
class Grammar;

class DFAState {
  public:
    unsigned int N, label, numEquivs;
    set<Symbol *> accepts;  /* Symbols acceptable in this state; */
    Symbol *accept; /* The one we're really accepting */
    int *moves;

    DFAState( int num );
//    ~DFAState() { if(moves) delete [] moves }

    bool isAccepting( void ) { return accepts.size() > 0; }
    bool isConflicting( void ) { return accepts.size() > 1; }
    bool canAccept( Symbol *sym ) { return accepts.find(sym) != accepts.end();}

  protected:
    Bitset postAccepts; /* Set of symbols accepted here or successor states*/
    Bitset predStates; /* Set of potential predecessor states */
    set<int> nfaStates;

    friend DFA;
};

class DFA : public CombSource { 
  public:
    vector <DFAState> states;
    EquivClassSet *equivs;
    int numEquivs, numBaseStates, numStartStates;
    int *startStates;
    
    DFA( int css = CHARSET_SIZE, EquivClassSet *ecs = NULL );
    ~DFA( );
    static DFA *fromNFA( NFA *nfa );
    
    void minimize( void );
    void checkDuplicateSymbols( Grammar &g );
    void checkConflicts( Grammar &g, ConflictMap &conflict );
    void resolveConflicts( Grammar &g, ConflictMap &conflict );
    void computeAccepts( Grammar &g );
    void computePredSets( );
    int getNumStartStates( );

    void print( FILE *f );
    virtual void getRow(int row, int *);  /* CombSource */

  protected:
    int addState( void );
    int addState( set<int> &nfaStates, NFA *nfa );
    
    void traverse( vector<int> &, int );
    void computePredSetsSub( int state );
    int copyStartState( Grammar &g, ConflictMap &conflict, int sstate );
    int copyStartStateMin( Grammar &g, ConflictMap &conflict, int sstate );
};



#endif /* !elr_dfa_H */
