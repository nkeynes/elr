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
