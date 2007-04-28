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

#ifndef elr_bitset_H
#define elr_bitset_H

class Bitset {
  public:
    Bitset( );
    Bitset( unsigned int size );
    Bitset( const Bitset & );
    ~Bitset( );

    void set( unsigned int N );
    void clear( unsigned int N );
    void toggle( unsigned int N );
    bool get( unsigned int N );
    bool isDisjoint( const Bitset & );
    bool contains( const Bitset & );
    
    void set( void );
    void clear( void );
    void toggle( void );
    int size( void );
    int count( void );
    void print( void );
    bool isEmpty( void );

    bool operator []( const unsigned int N );
    Bitset &operator |=( const Bitset & );
    Bitset &operator &=( const Bitset & );
    Bitset &operator ^=( const Bitset & );
    Bitset &operator =( const Bitset & );
    bool operator ==( const Bitset &b ) const {
        if( length != b.length ) return false;
        for( int i=0; i<length; i++ ) if( data[i] != b.data[i] ) return false;
        return true;
    }
    bool operator !=( const Bitset &b ) const {
        if( length != b.length ) return true;
        for( int i=0; i<length; i++ ) if( data[i] != b.data[i] ) return true;
        return false;
    }

    Bitset operator &( const Bitset & );
  protected:
    unsigned int length;
    unsigned int *data;
};

#endif /* !elr_bitset_H */
