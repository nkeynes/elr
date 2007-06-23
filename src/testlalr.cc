/**
 * $Id$
 *
 * Unit test for the bit pattern support class
 *
 * Copyright (c) 2005 Nathan Keynes.
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

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include "config.h"
#include "bitset.h"
#include "grammar.h"
#include "lex.h"
#include "lr.h"

#define GRAMMAR_PATH "../test/"

Grammar grammar;

class LalrTest : public CppUnit::TestFixture {
public:
    CPPUNIT_TEST_SUITE( LalrTest );
    CPPUNIT_TEST( testIns );
    CPPUNIT_TEST_SUITE_END();

    void testIns( void ) {
	loadGrammar( GRAMMAR_PATH "insparse.g" );
	LRTable *lr = new LRTable( &grammar );
	lr->constructPDA();
	lr->computeLALRSets();

	LRState *state = lr->getState(18); // problematic reduce state
	LRReduceEdge *reduce = (LRReduceEdge *)state->edges[0];
	CPPUNIT_ASSERT( reduce != NULL );
	Bitset expect = Bitset( grammar.lastTerminal+1 );
	expect.set( grammar.eofSymbol->symbolId );
	CPPUNIT_ASSERT_EQUAL( expect, reduce->lookahead );
    }

private:
    int loadGrammar( char *file ) {
	if( lexOpenFile( file ) == -1 ) {
	    fprintf( stderr, "Unable to open grammar: %s\n", file );
	    return 1;
	}

	if( yyparse() != 0 ) {
	    printf( "Parse error: aborting\n" );
	    return 1;
	}
	
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( LalrTest );
