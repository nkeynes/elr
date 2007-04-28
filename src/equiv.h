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

    friend EquivClassIterator;
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
    
    friend EquivClassSet;
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
