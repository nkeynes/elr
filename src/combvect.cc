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
