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
#include <stdio.h>
#include "stringset2.h"

int main() {
  StringSet2 set = StringSet2(4);
  StringSet2 set2= StringSet2(4);
  
  set.addEmpty();
  set.append( 1 );
  set.addEmpty();
  set.append( 3 );
  set.addEmpty();
  set.print();
  printf("\n");

  set2.add(2);
  set2.addEmpty();
  set2.append( 3 );
  set2.print();
  printf("\n");

  set.append( set2 );
  set.print();
  printf("\n");
  
  set.clear();
  set2.clear();
  set.add(2);
  set.append(3);
  set2.add(2);
  set2.append(3);
  if( set == set2 ) printf( "Same\n" );
  else printf( "Diff\n" );
  set2.add(2);
  set2.append(2);
  if( set == set2 ) printf( "Same\n" );
  else printf( "Diff\n" );
  
}
