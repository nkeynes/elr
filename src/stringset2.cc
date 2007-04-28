/*
 * Stringset optimized for 2 element strings. (replacement for stringset.cc)
 */
#include <stdlib.h>
#include <stdio.h>
#include "stringset2.h"

StringSet2::StringSet2( int numSym )
{
    numSymbols = ++numSym;
    firstsym = Bitset(numSym);
    firstonly = Bitset(numSym);
    second = vector<Bitset>( numSym, Bitset(numSym) );
    hasNull = false;
    isFull = false;
    isEmpty = true;
}

void StringSet2::clear( void )
{
    firstonly.clear();
    firstsym.clear();
    for( int i=0; i<numSymbols; i++ ) second[i].clear();
    hasNull = isFull = false;
    isEmpty = true;
}

void StringSet2::add( int token )
{
    firstonly.set(token);
    firstsym.set(token);
    isFull = isEmpty = false;
}

void StringSet2::add( int *str, int len )
{
    if( len == 0 ) addEmpty();
    else if( len == 1 ) add( str[0] );
    else {
        firstsym.set(str[0]);
        second[str[0]].set(str[1]);
        if( isEmpty ) {
            isEmpty = false;
            isFull = true;
        }
    }
}

void StringSet2::add( StringSet2 &with )
{
    if( with.hasNull ) hasNull = true;
    if( with.isEmpty ) return;
    firstsym |= with.firstsym;
    firstonly |= with.firstonly;
    for( int i=0; i<numSymbols; i++ ) second[i] |= with.second[i];
    if( isEmpty && with.isFull ){ isFull = true; }
    isEmpty = false;
}

void StringSet2::addMax( StringSet2 &with )
{
    if( with.isEmpty ) return;
    
    for( int i=0; i<numSymbols; i++ ) {
        if( !second[i].isEmpty() ) firstsym.set(i);
        second[i] |= with.second[i];
    }
    if( isEmpty ){ isEmpty = false; isFull = true; }
}

void StringSet2::append( int token )
{
    for( int i=0; i<numSymbols; i++ )
        if( firstonly.get(i) ) second[i].set(token);
    firstonly.clear();
    if( hasNull ){
        hasNull = false;
        firstsym.set(token);
        firstonly.set(token);
        isFull = isEmpty = false;
    } else if( ! isEmpty ) isFull = true;
}

void StringSet2::append( StringSet2 &with )
{
    if( with.isEmpty ) return;
    if( ! isEmpty ) {
        for( int i=0; i<numSymbols; i++ )
            if( firstonly.get(i) ) second[i] |= with.firstsym;
        if( ! with.hasNull ) {
            firstonly.clear();
            if( ! hasNull ) isFull = true;
        }
    }
    if( hasNull ) {
        hasNull = false;
        add( with );
    }
}

Bitset *StringSet2::first( void )
{
    return &firstsym;
}

Bitset *StringSet2::follows( int token )
{
    return &second[token];
}

void StringSet2::print( void )
{
    printf( "{" );
    if( hasNull ) printf( " <>," );
    for( int i=0; i<numSymbols; i++ ) {
        if(firstonly.get(i))
            printf( " <%d>",i );
        for( int j=0; j<numSymbols; j++ ) {
            if( second[i].get(j) )
                printf( " <%d,%d>",i,j );
        }
    }
    printf( " }" );
}
