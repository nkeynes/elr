
#ifndef elr_lr_H
#define elr_lr_H 1

#include <vector>
#include <set>
#include <list>
#include "grammar.h"
#include "bitset.h"
#include "stringset2.h"
#include "combvect.h"

class LRItem {
  public:
    Rule *rule;
    int pos;
    StringSet2 *followsk; /* k-lookahead past end of item */
    StringSet2 *follows[3]; /* temporary for lalrk calculations */
    
    LRItem( Rule *r, int p ) { rule = r; pos = p; follows[1] = NULL; follows[2] = NULL; }
    bool operator <(const LRItem &a) const;
    bool operator ==(const LRItem &a) const;
};

typedef enum { SHIFT = 0, REDUCE = 1 } edge_type_t;

class LREdge {
  public:
    edge_type_t type;
    Action *action;
    LREdge *chain; /* Conflicts */
    Bitset read; /* 1-lookahead */
    StringSet2 *followsk; /* k-lookahead */
    unsigned int N; /* Used in LALR calculations */

    LREdge( unsigned int numTerms, edge_type_t t ) {
        read = Bitset(numTerms);
        type = t; }
};

class LRReduceEdge : public LREdge {
  public:
    Rule *rule;
    Bitset lookahead;
    LRReduceEdge( Rule *rule, Action *action, LREdge *chain, int numTerms );
};

class LRShiftEdge : public LREdge {
  public:
    int dest;
    LRShiftEdge( int dest, Action *action, LREdge *chain, int numTerms );
};

class LRState {
  public:
    set<LRItem> items;
    set<int> pred;  /* Immediate predecessor states */
    Symbol *accessSym; /* Accessing symbol */
    LREdge **edges;
    Bitset accepts;
    bool hasShifts;
    int numReductions;
    bool isAdequate;

    LRState( int numSymbols );
    ~LRState( );
  protected:
    int numSymbols;
};

typedef vector<LRState *> LRStatePs;

class LRTable : public CombSource {
  public:
    Grammar *grammar;
    LRStatePs states;
    int numLR0States, numLALR1States, numLR1States, numConflictStates;
    int numTotalStates; /* All real & pseudo states */
    int acceptState;
    
    int *reduceLengths, *reduceTokens;

    LRTable( Grammar *g );
    ~LRTable( );

    void constructPDA( void );
    void computeLALRSets( void );
    void computeLALRkSets( int k );
    void computeFollowSets( void ); /* Requires computeLALRSets() to be done */
    void dumpTable( void );
    void printState( int state );
    void printSummary( void );

    virtual void getRow(int row, int *out); /* CombSource */
  protected:
    int addState( set<LRItem> &items );
    void closure( Symbol *sym, set<LRItem> &to );
    void shift( set<LRItem> &from, Symbol *sym, set<LRItem> &to );

/* LALR subfunctions */
    typedef void (LRTable::*trav_func)(LRShiftEdge *,set<LRShiftEdge *> &vect);

    void computeDirectReadSets( void );
    void computeLookahead( int state, LRReduceEdge *edge );
    void resolveState( int state );
    void digraph( trav_func trav );
    void traverse( vector<LRShiftEdge *> &stack, LRShiftEdge *x, trav_func fn);
    void reads( LRShiftEdge *x, set<LRShiftEdge *> &vect );
    void includes( LRShiftEdge *x, set<LRShiftEdge *> &vect );
    void backtrack( set<int> &dest, set<int> &src );
    void printPredSets( void );
    void printItem( const LRItem &i );
    void printItemSet( set<LRItem> &items );
    void buildReduceArrays( void );

    void lalrk( const LRItem &item, int state, int k, StringSet2 &result );
    void lalrksub( const LRItem &item, int state, int k, StringSet2 &result,
                   set <const LRItem *> &done );
    void firstk( StringSet2 &result, const LRItem &item );
};

#endif /* !elr_lr_H */
