/*
 * equiv.cc - simple implementation of equivalence classes. Data structures
 * are derived from those used in flex.
 *
 * Copyright (c) 2000 Nathan Keynes. Distributed under the terms of the GNU
 * Public Licence version 2 or later.
 */
#include <ctype.h>
#include <stdio.h>
#include "equiv.h"

/* Based partly off the structures used in flex */

#define NIL -1


/************************** Constructor / Destructor *************************/

EquivClassSet::EquivClassSet( int sz )
{
    size = sz;
    felems = new int [sz]; relems = new int [sz];
    group = new int [sz]; repelems = new int [sz];
    
    nGroups = 1;
    felems[0] = NIL; repelems[0] = 0; group[0] = 0;
    relems[size-1] = NIL;
    for( int i=1; i<size; i++ ) {
        felems[i] = i-1;
        relems[i-1] = i;
        group[i] = 0;
        repelems[i] = NIL;
    }
}

EquivClassSet::~EquivClassSet( )
{
    delete [] felems; delete [] relems;
    delete [] group; delete [] repelems;
}

/*
 * Establishes whether the state invariants hold (and there's a few of them).
 */
bool EquivClassSet::invariant( void )
{
    bool holds = true;
    int count = 0;
    bool visit[size];

#define CHECK(x, s) if(!(x)){ holds = false; \
        fprintf(stderr, "EquivClassSet invariant failed: " #x " (" s ")\n"); }
    CHECK( nGroups > 0, "At least one class" );
    CHECK( nGroups <= size, "At most size groups" );

    for( int i=0; i<size; i++ )
        visit[i] = false;
    
    for( int i=0; i<size; i++ ) {
        if( repelems[i] == NIL ) continue;
        
        CHECK( felems[repelems[i]] == NIL, "Rep. element is at list head" );
        for( int j=repelems[i]; j != NIL; j=relems[j] ) {
            CHECK( !visit[j], "Elements reached only once" );
            CHECK( group[j] == i, "Group membership correct" );
            if( relems[j] != NIL )
                CHECK( felems[relems[j]] == j, "List is bidirectional" );
            count++;
            if( visit[j] )
                break; /* If this happens, we've probably got a loop, so we try
                        * to avoid going around indefinitely... */
            visit[j] = true;
        }
    }
    CHECK( count == size, "Correct count of items" );
    return holds;
}

/*********************** Basic Equivalence class stuff ***********************/

/*
 * Add a single character equivalence class. (ie ensures the given character
 * is in a class of its own)
 */
void EquivClassSet::addChar( int ch )
{
    if( felems[ch] == NIL && relems[ch] == NIL ) return;/* Already by itself */

    if( felems[ch] != NIL )
        relems[felems[ch]] = relems[ch];
    if( relems[ch] != NIL )
        felems[relems[ch]] = felems[ch];

    felems[ch] = relems[ch] = NIL;
    group[ch] = nGroups++;
    repelems[group[ch]] = ch;
}

/*
 * Given a bitset representing a set of characters, ensure that they can be
 * formed from a set of equivalence classes. Effectively, for each equivalence
 * class, either all members are inside the bitset, or none are.
 */
void EquivClassSet::addCharClass( Bitset &set )
{
    bool inout;
    int oldnGroups = nGroups;
    
    for(int i=0; i<oldnGroups;i++ ) {
        inout = set[repelems[i]];
        int out = NIL, newgroup;
        for( int j=relems[repelems[i]]; j != NIL; j=relems[j] ) {
            if( set[j] != inout ){
                relems[felems[j]] = relems[j];
                if( relems[j] != NIL )
                    felems[relems[j]] = felems[j];
                
                if( out == NIL ) {
                    newgroup = nGroups++;
                    repelems[newgroup] = j;
                } else relems[out] = j;
                felems[j] = out;
                group[j] = newgroup;
                out = j;
            }
        }
        if( out != NIL ) relems[out] = NIL;
    }
}

/*
 * Similar to addCharClass, but makes the simplifying assumption that all
 * members of the bitset are already in the same class (which is to say, we
 * split the class into (at most) two pieces.
 *
 * Rather faster if we can make this assumption. Otherwise kinda won't work too
 * well... (This is mainly intended for minimization stuff, but could have
 * other uses)
 */
void EquivClassSet::splitCharClass( int ec, Bitset &set )
{
    int j;
    bool inout;
    int out = NIL, newgroup;

    inout = set[repelems[ec]];
    
    for( j=relems[repelems[ec]]; j != NIL; j=relems[j] ) {
        if( set[j] != inout ){
            relems[felems[j]] = relems[j];
            if( relems[j] != NIL )
                felems[relems[j]] = felems[j];
            
            if( out == NIL ) {
                newgroup = nGroups++;
                repelems[newgroup] = j;
            } else relems[out] = j;
            felems[j] = out;
            group[j] = newgroup;
            out = j;
        }
    }
    if( out != NIL ) relems[out] = NIL;
}

/*
 * Given a bitset representing a set of characters, alters the bitset to
 * represent the minimum set of equivalence classes which contain those
 * characters
 */
void EquivClassSet::convertToEquivSet( Bitset &in )
{
    Bitset eq = Bitset(nGroups);

    for(int i=0; i<size;i++ ) {
        if( in.get(i) ) eq.set(group[i]);
    }
    in = eq;
}


/*************************** Informational functions *************************/

/*
 * Return the number of elements in the given class
 */
int EquivClassSet::getClassSize( int ecs )
{
    int count = 0;
    
    for( int i=repelems[ecs]; i != NIL; i = relems[i] ) count++;
    return count;
}

/*
 * Return whether the class is singleton (contains only one element). Also
 * returns true if the class is empty. (this tends to make more sense under the
 * circumstances where this is used)
 */
bool EquivClassSet::isSingletonClass( int ecs )
{
    int i = repelems[ecs];
    return i == NIL || relems[i] == NIL;
}

/**************************** Text output functions **************************/

/*
 * Output a single character class to stdout, in lex notation
 */
void EquivClassSet::printClass( int wh )
{
    printf( "[" );
    for( int i=repelems[wh]; i != NIL; i = relems[i] ) {
        if( i == '-' || i == '\\' || i == '[' || i == ']') printf( "\\" );
        if( isprint(i) ) printf( "%c", i );
        else printf( "\\%03o", i );
        if( relems[i] == i+1 ) {
            while(relems[i] == i+1) i++;
            printf( "-" );
            if( i == '-' || i == '\\' || i == '[' || i == ']') printf( "\\" );
            if( isprint(i) ) printf( "%c", i );
            else printf( "\\%03o", i );
        }
    }
    printf( "]" );
}

/*
 * Output all classes in the set
 */

void EquivClassSet::printClassSet( )
{
    printf( "{ " );
    for( int i=0; i<nGroups;i++ ) {
        printClass( i );
        if( i < nGroups-1 ) printf(", " );
    }
    printf( " }" );
}

/*
 * Print the character class corresponding to the given equivalence classes
 * NB: This could probably be done better
 */
void EquivClassSet::printClass( Bitset &cs )
{
    printf( "[" );
    
    for( int i=0; i<size; i++ ) {
        if( cs.get(group[i]) ) {
            if( i == '-' || i == '\\' || i == '[' || i == ']') printf( "\\" );
            if( isprint(i) ) printf( "%c", i );
            else printf( "\\%03o", i );
            if( i<size && cs.get(group[i+1]) ) {
                while( i<size && cs.get(group[i+1])) i++;
                printf( "-" );
                if( i == '-' || i == '\\' || i == '[' || i == ']') printf( "\\" );
                if( isprint(i) ) printf( "%c", i );
                else printf( "\\%03o", i );
            }
        }
    }
    printf( "]" );
}


/****************** Functions for state minimization support *****************/

/*
 * Explicitly specify the class of a given character. (Used by the dfa minimize
 * routines, actually). This is VERY different from the methods above. Assume that
 * each character will be set exactly once.
 */
void EquivClassSet::setChar( int ch, int cls )
{
    if( group[ch] == cls ) return; /* Already in the right class */
    
    /* Remove from the old class */
    if( felems[ch] != NIL )
        relems[felems[ch]] = relems[ch];
    if( relems[ch] != NIL )
        felems[relems[ch]] = felems[ch];
    if( relems[ch] == NIL && felems[ch] == NIL )
        repelems[group[ch]] = NIL;

    int prev = NIL, next = repelems[cls];

    relems[ch] = repelems[cls];
    felems[ch] = NIL;
    if( repelems[cls] != NIL ) felems[repelems[cls]] = ch;
    repelems[cls] = ch;

/*
    while( next < ch && next != NIL ){
        prev = next;
        next = relems[next];
    }
    if( prev != NIL ) relems[prev] = ch;
    else repelems[cls] = ch;
    if( next != NIL ) felems[next] = ch;
    felems[ch] = prev;
    relems[ch] = next;
*/
    group[ch] = cls;

    if( nGroups <= cls ) nGroups = cls+1;        
}

/*
 * Clean-up function to be called after use of setCharClass. Basically kills
 * off any classes with no elements. Does not preserve class ordering (as if
 * there was a meaningful order in the first place...)
 */
void EquivClassSet::tidy( void )
{
    int last = nGroups-1;

    while( last >= 0 && repelems[last] == NIL ) --last;
    
    for( int i=0; i<last; i++ ) {
        if( repelems[i] == NIL ) {
            /* move last to i */
            for( int j=repelems[last]; j != NIL; j = relems[j] )
                group[j] = i;
            repelems[i] = repelems[last];
            repelems[last] = NIL;
            /* update last */
            while( repelems[--last] == NIL );
        }
    }
    nGroups = last+1;
}

/*
 * Sort the classes by representative element (Implication being that we can
 * then more readily collapse a table with this).
 */
void EquivClassSet::sortClasses( void )
{
    int grp = 0;
    int map[nGroups];

    for( int i=0; i<nGroups; i++ ) map[i] = -1;
    for( int i=0; i<size; i++ ) {
        if( map[group[i]] == -1 ) {
            map[group[i]] = grp;
            repelems[grp++] = i;
        }
        group[i] = map[group[i]];
    }
}
