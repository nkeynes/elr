/*
 * (dummy) Implementation of a comb-vector table
 */
#ifndef elr_combvect_h
#define elr_combvect_h

#include "config.h"

class CombSource {
  public:
    virtual void getRow(int row, int *out) = 0;
};

class CombVector {
  public:
    CombVector(int nStates, int nCols);
    ~CombVector();

    void fromSource( CombSource &src );
    
    int (*nextCheck)[2];
    int *def;
    int *base;
    int numStates, numCols;
    int nextCheckLen;
    int baseLen;
};

#endif /* !elr_combvect_h */
