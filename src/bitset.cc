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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "bitset.h"

#define INT_BITS (sizeof(unsigned int)*8)
#define SEG(n) data[n/INT_BITS]
#define OFF(n) (1<<(n%INT_BITS))
#define min(a,b) (a<b?a:b)

Bitset::Bitset( )
{
    length = 0;
    data = NULL;
}

Bitset::Bitset( unsigned int size )
{
    length = (size + INT_BITS-1 ) / INT_BITS;
    data = new unsigned int[length];
    clear();
}

Bitset::Bitset( const Bitset &src )
{
    length = src.length;
    data = new unsigned int[length];
    for( int i=0; i<length; i++ ) data[i] = src.data[i];
}

Bitset::~Bitset( )
{
    if( data ) delete []data;
}

void Bitset::set( unsigned int N )
{
    SEG(N) |= OFF(N);
}

void Bitset::clear( unsigned int N )
{
    SEG(N) &= ~OFF(N);
}

void Bitset::toggle( unsigned int N )
{
    SEG(N) ^= OFF(N);
}

void Bitset::set( void )
{
    memset( data, 0xFF, sizeof(int)*length );
}

void Bitset::clear( void )
{
    memset( data, 0, sizeof(int)*length );
}

void Bitset::toggle( void )
{
    for( unsigned int i=0;i<length;i++ ) data[i] ^= ~0;
}

int Bitset::size( void )
{
    return length * INT_BITS;
}

bool Bitset::get( unsigned int N )
{
    return !!(SEG(N) & OFF(N));
}

bool Bitset::isEmpty( void )
{
    for( unsigned int i=0; i<length; i++ )
        if( data[i] != 0 ) return false;
    return true;
}

bool Bitset::operator []( const unsigned int N )
{
    return ~~(SEG(N) & OFF(N));
}

Bitset &Bitset::operator |=( const Bitset &a )
{
    for( unsigned int i=0; i<min(length,a.length); i++ ) data[i] |= a.data[i];
    return *this;
}

Bitset &Bitset::operator &=( const Bitset &a )
{
    for( unsigned int i=0; i<min(length,a.length); i++ ) data[i] &= a.data[i];
    return *this;
}

Bitset Bitset::operator &( const Bitset &b )
{
    Bitset r = *this;
    r &= b;
    return r;
}

Bitset &Bitset::operator ^=( const Bitset &a )
{
    for( unsigned int i=0; i<min(length,a.length); i++ ) data[i] ^= a.data[i];
    return *this;
}

Bitset &Bitset::operator =( const Bitset &a )
{
    if( length == 0 ) {
        if( a.length == 0) return *this;
        data = new unsigned int [a.length];
        length = a.length;
    }
    for( unsigned int i=0; i<min(length,a.length); i++ ) data[i] = a.data[i];
    return *this;
}

bool Bitset::isDisjoint( const Bitset &a )
{
    for( unsigned int i=0; i<min(length,a.length); i++ ) {
        if( (data[i] & a.data[i]) != 0 ) return false;
    }
    return true;
}

bool Bitset::contains( const Bitset &a )
{
    for( unsigned int i=0; i<min(length,a.length); i++ ) {
        if( (a.data[i] & data[i]) != a.data[i] ) return false;
    }
    return true;
}

void Bitset::print()
{
    int end = length*INT_BITS-1;
    
    printf( "[" );
    for( unsigned int i=0; i<=end; i++ ) {
        if( get(i) ) {
            if( i == '-' || i == '\\' || i == '[' || i == ']') printf( "\\" );
            if( isprint(i) ) printf( "%c", i );
            else printf( "\\%03o", i );
            if( i<end && get(i+1) ) {
                while( i<end && get(i+1)) i++;
                printf( "-" );
                if( i == '-' || i == '\\' || i == '[' || i == ']') printf( "\\" );
                if( isprint(i) ) printf( "%c", i );
                else printf( "\\%03o", i );
            }
        }
    }
    printf( "]" );
}
