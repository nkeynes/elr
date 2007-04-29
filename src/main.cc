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
#include <unistd.h>
#include <getopt.h>
#include <sys/time.h>
#include <string>
#include "config.h"
#include "grammar.h"
#include "lr.h"
#include "dfa.h"
#include "conflict.h"
#include "lex.h"
#include "codegen.h"

extern int yydebug;
Grammar grammar;
Config config;

typedef enum { OPT_YACC = 1, OPT_LALR = 2 } options_t;

static struct option long_options[] = {
    {"scan", 1, 0, 's'},
    {"parse",1, 0, 'p'},
    {"yacc",0,0,OPT_YACC},
    {"lalr",0,0,OPT_LALR},
    {"output",1,0,'o'},
    {"language",1,0,3},
        {0,0,0,0} };
static char short_options[] = "vo:";

int parseCommandLine( int argc, char *argv[] )
{
    config.skeletonPath = SKELETON_DIR;
    /* parse command line */
    while(1) {
        int this_option_optind = optind ? optind : 1;
        int option_index = 0;

        int c = getopt_long( argc, argv, short_options, long_options,
                         &option_index );
        if( c == EOF )
            break;
        switch(c) {
            case OPT_YACC: config.outputGen = OUT_YACC; break;
            case OPT_LALR: config.outputGen = OUT_LALR; break;
            case 'o': config.outputSourceFile = new string(optarg); break;
            case 'v': config.verbose = true;
        }
    }
    while( optind < argc ) {
        if( config.inputFile ) {
            printf( "Warning: only one input file expected (ignoring %s)\n",
                    argv[optind] );
        } else {
            config.inputFile = new string(argv[optind]);
        }
        optind++;
    }    
    if( config.inputFile == NULL ) {
        printf( "input filename required\n" );
        return 0;
    }
    if( config.outputSourceFile == NULL && config.outputBase == NULL ) {
        for( string::iterator l = config.inputFile->end()-1; 
	     l != config.inputFile->begin() && *l != '/'; l-- ){
	  if( *l == '.' ) {
	    config.outputBase = new string(config.inputFile->begin(), l);
	  }
	}
	if( config.outputBase == NULL ) 
	  config.outputBase = new string(*config.inputFile);
    }

    return 1;
}

void printTimeDiff( struct timeval *a, struct timeval *b )
{
    int ms, s;
    s = b->tv_sec - a->tv_sec;
    ms = b->tv_usec - a->tv_usec;
    if( ms < 0 ) {
        s--;
        ms += 1000000;
    }
    printf( "--- Time: %d.%06d\n",s,ms );
}

int main( int argc, char *argv[] )
{
    int lookahead = 2;
    struct timeval tva,tvb;

    if( !parseCommandLine(argc, argv) )
        return 1;
    
    if( lexOpenFile( config.inputFile->c_str() ) == -1 ) {
        fprintf( stderr, "Unable to open file: %s\n",
                 config.inputFile->c_str() );
        exit(1);
    }
    //    yydebug = 1;
    if( yyparse() != 0 ) {
        printf( "Parse error: aborting\n" );
        return 1;
    }

    gettimeofday( &tva, NULL );
    fprintf( stderr, "Computing DFA...");
    grammar.computeDFA();
    fprintf( stderr, "Done\nComputing FIRST(2) of grammar...\n" );
    grammar.computeFirst(lookahead);
    fprintf( stderr, "Done\n");
    grammar.dfa->computeAccepts(grammar);
    grammar.dfa->computePredSets();
//    grammar.dfa->print(stdout);
    LRTable *lr = new LRTable( &grammar );
    fprintf( stderr, "Computing LR(0)...\n");
    lr->constructPDA();
    fprintf( stderr, "Done\nComputing LALR(1)...\n");
    lr->computeLALRSets();
    gettimeofday( &tvb, NULL );
    printTimeDiff( &tva, &tvb );
    fprintf( stderr, "Done\nComputing LALR(2) lookahead sets...\n");
    lr->computeLALRkSets(lookahead);
    fprintf( stderr, "Done\n");
    gettimeofday( &tva, NULL );
    printTimeDiff( &tvb, &tva );
    lr->computeFollowSets();

    if( config.verbose ) {
        fprintf( stderr, "Base DFA:\n");
        grammar.dfa->print( stdout );
    }

    gettimeofday( &tvb, NULL );
    printTimeDiff( &tva, &tvb );
    fprintf( stderr, "Checking for lexical conflicts...");
    ConflictMap *conflict = new ConflictMap( lr, grammar.dfa, &grammar );
    grammar.dfa->checkConflicts( grammar, *conflict );
    fprintf( stderr, "done\nBuilding final DFA..." );
    gettimeofday( &tva, NULL );
    printTimeDiff( &tvb, &tva );
    grammar.dfa->resolveConflicts( grammar, *conflict );
    fprintf( stderr, "done\n" );
    gettimeofday( &tvb, NULL );
    printTimeDiff( &tva, &tvb );

    CodeGen *code = CodeGen::getInstance( config.languageGen );
    code->init(&grammar, lr, grammar.dfa, conflict);
    code->createSourceFile();

    if( config.verbose ) {
        grammar.dumpGrammar();
        grammar.dfa->print(stdout);
        lr->dumpTable();
        lr->printSummary();
    }
    printf( "Totals:\n" );
    printf( "  Terminals: %d\n  Nonterminals: %d\n  Rules: %d\n",
            grammar.terms.size(), grammar.nonterms.size(), grammar.numRules );
    printf( "  Parser states: %d\n  Base DFA states: %d\n",
            lr->states.size()-1, grammar.dfa->numBaseStates -1 );
    printf( "  Unique start states: %d\n  Final DFA states: %d\n",
            grammar.dfa->getNumStartStates(), grammar.dfa->states.size()-1 );

    return 0;
}
