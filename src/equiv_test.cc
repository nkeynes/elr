#include "equiv.h"
#include "bitset.h"

int main()
{
	EquivClassSet ecs;
	Bitset set1 = Bitset(256),set2 = Bitset(256),set3 = Bitset(256);
	
	for( int i='A'; i<='Z';i++ ) set1.set(i);
	for( int i='a'; i<='z';i++ ) set2.set(i);
	for( int i='T'; i<='g';i++ ) set3.set(i);

  ecs.addChar('V');	
	ecs.addCharClass(set1);
	ecs.addCharClass(set2);
	ecs.addCharClass(set3);
	ecs.printClassSet();
}