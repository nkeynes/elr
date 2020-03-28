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
#include <strings.h>
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
    {"help", 0, 0, 'h'},
    {"debug", 0, 0, 'd' },
    {"interface", 0, 0, 'i'},
    {"yacc",0,0,OPT_YACC},
    {"lalr",0,0,OPT_LALR},
    {"output",1,0,'o'},
    {"language",1,0,'l'},
    {"skeleton-path",1,0,'S'},
    {"verbose",0,0,'v'},
    {"version",0,0,'V'},
    {0,0,0,0} };
static char short_options[] = "dhil:o:S:vV";

void usage( FILE *out )
{
    fprintf( out, "Usage: %s [OPTION]... <input-filename>\n\n", APP_NAME );
    fprintf( out, "Options:\n");
    fprintf( out, "  -d, --debug         Generate debugging output\n" );
    fprintf( out, "  -h, --help          Display this help text\n" );
    fprintf( out, "  -i, --interface     Generate header file in addition to source file\n" );
    fprintf( out, "  -l, --language      Target language [C]\n" );
    fprintf( out, "  -o, --output        Output filename\n" );
    fprintf( out, "  -S, --skeleton-path Path to skeleton files [%s]\n", 
	     config.skeletonPath.c_str() );
    fprintf( out, "  -v, --verbose       Output additional information and statistics\n" );
    fprintf( out, "  -V, --version       Print %s version\n", APP_NAME );
}

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
	case 'd': config.genDebug = true; break;
	case 'h': usage(stdout); exit(0);
	case 'i': config.genHeader = true; break;
	case 'l': 
	    if( strcasecmp(optarg, "c")==0 ) {
		config.languageGen = LANG_C;
	    } else if( strcasecmp(optarg, "c++")==0 ||
		       strcasecmp(optarg, "cpp")==0 ) {
		config.languageGen = LANG_CPP;
	    } else if( strcasecmp(optarg, "java")==0 ) {
		config.languageGen = LANG_JAVA;
	    } else if( strcasecmp(optarg, "ada")==0 ) {
		config.languageGen = LANG_ADA;
	    } else {
		fprintf( stderr, "Unrecognized target language '%s'\n", optarg );
		exit(2);
	    }
	    break;
	case OPT_YACC: config.outputGen = OUT_YACC; break;
	case OPT_LALR: config.outputGen = OUT_LALR; break;
	case 'o': config.outputSourceFile = new string(optarg); break;
	case 'S': config.skeletonPath = string(optarg); break;
	case 'v': config.verbose = true; break;
	case 'V': 
	    printf( "%s %s. Copyright (c) 2001-2007 Nathan Keynes.\n", APP_NAME, APP_VERSION );
	    exit(0);
        }
    }
    while( optind < argc ) {
        if( config.inputFile ) {
            printf( "Warning: only one input filename expected (ignoring %s)\n",
                    argv[optind] );
        } else {
            config.inputFile = new string(argv[optind]);
        }
        optind++;
    }    
    if( config.inputFile == NULL ) {
	usage(stdout);
        fprintf( stderr, "\ninput-filename required\n" );
        return 0;
    }
    if( config.outputBase == NULL ) {
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
    printf( "%d.%06d",s,ms );
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
    grammar.computeDFA();
    grammar.computeFirst(lookahead);
    grammar.dfa->computeAccepts(grammar);
    grammar.dfa->computePredSets();

    LRTable *lr = new LRTable( &grammar );
    lr->constructPDA();
    lr->computeLALRSets();
    lr->computeLALRkSets(lookahead);
    lr->computeFollowSets();

    ConflictMap *conflict = NULL;
    if( grammar.autoLexDisambiguation ) {
	conflict = new ConflictMap( lr, grammar.dfa, &grammar );
	grammar.dfa->checkConflicts( grammar, *conflict );
	grammar.dfa->resolveConflicts( grammar, *conflict );
    } else {
	grammar.dfa->resolveConflictsNoContext( grammar );
    }
    grammar.dfa->minimize();

    if( config.genDebug ) {
	printf( "Grammar:\n" );
        grammar.dumpGrammar();
	printf( "\nLexical Automata:\n" );
        grammar.dfa->print(stdout);
	printf( "\nParser Automata:\n" );
        lr->dumpTable();
        lr->printSummary();
    }
    if( config.verbose ) {
	printf( "Totals:\n" );
	printf( "  Terminals: %d\n  Nonterminals: %d\n  Rules: %d\n",
		grammar.terms.size(), grammar.nonterms.size(), grammar.numRules );
	printf( "  Parser states: %d\n  Base DFA states: %d\n",
		lr->states.size()-1, grammar.dfa->numBaseStates -1 );
	printf( "  Unique start states: %d\n  Final DFA states: %d\n",
		grammar.dfa->getNumStartStates(), grammar.dfa->states.size()-1 );
	printf( "  Unresolved parser conflicts: %d\n", lr->unresolvedConflicts );
    }

    if( lr->unresolvedConflicts != grammar.expectedParserConflicts ) {
	fprintf( stderr, "Error: Expected %d conflicts, but found %d.\n",
		 grammar.expectedParserConflicts, lr->unresolvedConflicts );
	return 1;
    }

    CodeGen *code = CodeGen::getInstance( config.languageGen );
    code->init(&grammar, lr, grammar.dfa, conflict);
    if( code->getErrors() == 0 ) {
	code->createSourceFile();
	if( config.genHeader ) {
	    code->createHeaderFile();
	}

	if( config.verbose ) {
	    gettimeofday(&tvb, NULL);
	    printf( "Parser generated in: " );
	    printTimeDiff( &tva, &tvb );
	    printf( "s\n" );
	}
	return 0;
    }

    return 2;
}
