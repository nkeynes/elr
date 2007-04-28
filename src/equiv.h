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
#ifndef elr_equiv_H
#define elr_equiv_H 1

#include "config.h"
#include "bitset.h"

class EquivClassSet;
class EquivClassIterator;

class EquivClassSet {
  public:
    EquivClassSet( int sz = CHARSET_SIZE );
    ~EquivClassSet( );
    bool invariant( void );
    
    void addChar( int ch );
    void setChar( int ch, int cls );
    void addCharClass( Bitset &cls );
    void splitCharClass( int ec, Bitset &cls );
    void tidy( void );
    void sortClasses( void );

    void convertToEquivSet( Bitset &in );

    void printClassSet( );
    void printClass( Bitset &cs );
    void printClass( int ec );

    int getNumClasses( void ) { return nGroups; }
    int getClass( int ch ) { return group[ch]; }
    int getClassRep( int ec ) { return repelems[ec]; }
    int getClassSize( int ec );
    bool isSingletonClass( int ec );
    bool isEquivalentTo( int a, int b ) { return group[a] == group[b]; }

    EquivClassIterator beginClass( int ec );
    EquivClassIterator endClass( int ec ) const;
        
    int *group;
  protected:
    int size;
    int *felems;
    int *relems;
    int *repelems;
    int nGroups;

    friend class EquivClassIterator;
};

/*************** Equivalence class iteration *************/

class EquivClassIterator {
  public:
    bool operator ==( const EquivClassIterator &it ) const;
    bool operator !=( const EquivClassIterator &it ) const;
    void operator ++( ) { elem = set->relems[elem]; }
    int operator *( ) { return elem; }
  protected:
    int elem;
    const EquivClassSet *set;

    EquivClassIterator( const EquivClassSet *s, int el ) { set = s; elem = el;}
    
    friend class EquivClassSet;
};

inline bool EquivClassIterator::operator ==( const EquivClassIterator &it ) const {
    return it.elem == elem && it.set == set;
}
inline bool EquivClassIterator::operator !=( const EquivClassIterator &it ) const {
    return it.elem != elem || it.set != set;
}

inline EquivClassIterator EquivClassSet::beginClass( int ec ) {
    return EquivClassIterator( this, repelems[ec] );
}
inline EquivClassIterator EquivClassSet::endClass( int ec ) const {
    return EquivClassIterator(this,-1);
}


#endif /* !elr_equiv_H */
