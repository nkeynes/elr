
#include "combvect.h"

CombVector::CombVector(int nStates, int nCols)
{
    /* Allocate the maximum possible space (equiv to 0 compression) */
    nextCheck = new int [(nStates+1)*nCols][2];
    base = new int [nStates+1];
    def = new int [nStates+1];
    numStates = nStates;
    numCols = nCols;
}

CombVector::~CombVector()
{
    delete [] nextCheck;
    delete [] base;
    delete [] def;
}

void CombVector::fromSource( CombSource &src )
{
    /* Quick non-compressing comb-vector build. Kinda useless really, but lets
     * the actual compression (non-trivial) be implemented later.
     */
    int pos = 0;
    int row[numStates];
    for( int i=0; i < numStates; i++ ) {
        src.getRow(i, row);
        base[i] = pos;
        def[i] = 0;
        for( int j=0; j < numCols; j++ ) {
            nextCheck[pos][0] = row[j];
            nextCheck[pos++][1] = i;
        }
    }
    nextCheckLen = pos;
    baseLen = numStates;
}
