#include "stringset.h"

void printString( const TokenString &str )
{
    printf( "<" );
    for( TokenString::const_iterator i = str.begin(); i != str.end(); i++ ) {
        printf( " %d", *i );
    }
    printf( " >" );
}
void printSet( const StringSet &set )
{
    printf( "{" );
    for( Strings::iterator i = set.max.begin(); i != set.max.end(); i++ ) {
        printString( *i );
        printf( " " );
    }
    for( Strings::iterator i = set.rest.begin(); i != set.rest.end(); i++ ) {
        printString( *i );
        printf( " " );
    }
    printf( " }" );
}                                                                                

int main() {
  StringSet set = StringSet(4);
  StringSet set2= StringSet(4);
  
  set.addEmpty();
  set.append( 1 );
  set.append( 2 );
  set.addEmpty();
  set.append( 3 );

  set2.addEmpty();
  set2.append( 1 );
  set2.append( 2 );
  set2.addEmpty();
  set2.append( 3 );

  set.append( set2 );
  printSet( set );
}
