
#ifndef elr_stringset2_H
#define elr_stringset2_H

#include "bitset.h"
#include <vector>

class StringSet2 {
  public:
    StringSet2( int numSym );
    void clear( void );
    void addMax( StringSet2 &with ); /* Add only strings of max length */
    void add( StringSet2 &with );
    void add( int token );
    void add( int *str, int len );
    void addEmpty( void ) { hasNull = true; }
    void deleteEmpty( void ) { hasNull = false; }
    void append( StringSet2 &with );
    void append( int token );
    void print( void );

    Bitset *first( void );
    Bitset *follows( int token );
    int minLength( ){ return ((hasNull || isEmpty) ? 0 : (isFull ? 2 : 1)); }
    bool hasEmpty( ){ return hasNull; }

    bool operator == (const StringSet2 &b) const {
        return hasNull == b.hasNull && firstsym == b.firstsym &&
            firstonly == b.firstonly && second == b.second;
    }
    
  protected:
    int numSymbols;
    bool hasNull;
    bool isFull; /* All elements of length 2 (excl. null) */
    bool isEmpty; /* No non-null elements */
    Bitset firstsym, firstonly;
    vector<Bitset> second;
    
};



#endif
