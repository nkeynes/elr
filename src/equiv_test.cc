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