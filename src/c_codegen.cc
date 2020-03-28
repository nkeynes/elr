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
#include <assert.h>
#include <string.h>
#include "config.h"
#include "codegen.h"
#include "grammar.h"

void C_CodeGen::init( Grammar *g, LRTable *lrt, DFA *fsa, ConflictMap *conf )
{
    CodeGen::init(g,lrt,fsa,conf);
    computeExpectedTokens();
}

void C_CodeGen::computeExpectedTokens( void )
{
    expectedTokens = vector<int>();
    expectedTokenIndex = vector<int>();

    for( LRStatePs::iterator it = lr->states.begin(); it != lr->states.end(); it++ ) {
	LRState *state = *it;
	expectedTokenIndex.push_back(expectedTokens.size());
	for( int i=1; i<=grammar->lastTerminal; i++ ) {
	    if( state->edges[i] != NULL ) {
		expectedTokens.push_back(i);
	    }
	}
	expectedTokens.push_back(-1);
    }
}


void C_CodeGen::handleCommand( char *cmd, int len, FILE *out )
{ 
    if( MATCH(cmd,len,"PARSER_EXPECT_ARRAY") ) {
	writeIntegerArray( &(*expectedTokens.begin()), expectedTokens.size(), out );
    } else if( MATCH(cmd,len,"PARSER_EXPECT_INDEX_ARRAY") ) {
	for( int i=0; i<expectedTokenIndex.size(); i++ ) {
	    if( i != 0 ) {
		write( ", ", out );
	    }
	    fprintf( out, "&yypExpectTokenData[%d]", expectedTokenIndex[i] );
	}
    } else {
	CodeGen::handleCommand( cmd, len, out );
    }
}

void C_CodeGen::writeMemberVar( const char *type, const char *name, FILE *out )
{
    fprintf( out, "    %s yy%s;\n", type, name );
    outputLine++;
}

void C_CodeGen::writeSymbolNameArray( FILE *out )
{
    for( int i=1; i<=grammar->numSymbols; i++ ) {
        writeStringConst( grammar->symbol(i)->name->c_str(), out );
        if( i != grammar->numSymbols )
            write( ", ", out );
    }
}

void C_CodeGen::writeStringConst( const char *s, FILE * out )
{
    fputc( '\"', out );
    while( *s ) {
        if( *s == '\"' || *s == '\\' ) fputc( '\\', out );
        fputc( *s++, out );
    }
    fputc( '\"', out );
}

void C_CodeGen::writeType( const char *s, FILE *out )
{
    if( s == NULL ) {
	fputs( "void", out );
    } else {
	fputs( s, out );
    }
}

void C_CodeGen::writeNextCheckArray( CombVector *vect, FILE *out )
{
    for( int i=0; i<vect->nextCheckLen; i++ ) {
        fprintf( out, "{%d,%d}", vect->nextCheck[i][0],vect->nextCheck[i][1] );
        if( i != vect->nextCheckLen -1 )
            fprintf( out, ", " );
    }
}

void C_CodeGen::writeParserReturn( FILE *out )
{
    if( grammar->startSymbol->type == NULL ) {
	fputs( "return", out );
    } else {
	fprintf( out, "return yypsynattr.yy%s", 
		 typeNameMap[*grammar->startSymbol->type].c_str() );
    }
}

void C_CodeGen::writeReturn( FILE *out )
{
    fputs( "return", out );
}

void C_CodeGen::writeCode( const Action &action, FILE *out )
{
    for( Action::const_iterator i = action.begin(); i != action.end(); i++ ) {
	const ActionItem &act = *i;
	fprintf( out, "#line %d \"%s\"\n", act.posn.line, act.posn.filename );
	outputLine++;
	write( act.action->c_str(), out );
	outputLine++;
	fprintf( out, "\n#line %d \"%s\"\n", outputLine+1, outputFile.c_str() );
	outputLine++;
    }
}

/* I couldn't think of a good way to put these in the template file, so... */
void C_CodeGen::writeParserActions( FILE *out )
{
    FOR_EACH( nt, NonterminalPs, grammar->nonterms ) {
        FOR_EACH( rule, RulePs, (*nt)->rules ) {
//            if( (*rule)->reduceAction && (*rule)->reduceAction->size() > 0 ) {
            fprintf( out, "                case %d:\n", 
		     (*rule)->ruleId + lr->states.size()-1 );
	    outputLine++;
            if( config.genDebug ) {
                fprintf( out, "YY_DEBUG(\"Reducing %s ->",
                         (*rule)->nonterm->name->c_str() );
                FOR_EACH( sym, RuleSymbols, (*rule)->syms ) {
                    fputc( ' ', out );
                    writeEscapedString( sym->sym->name->c_str(), out );
                }
                fprintf( out, "\\n\");\n" );
		outputLine++;
            }
                     
            if( (*rule)->reduceAction ) {
                FOR_EACH( act, Action, (*(*rule)->reduceAction) ) {
		    fprintf( out, "#line %d \"%s\"\n", act->posn.line, act->posn.filename );
		    outputLine++;
		    fprintf( out, "                    " );
                    writeActionCode( *rule, *act, out );
		    outputLine++;
		    fprintf( out, "\n#line %d \"%s\"\n", outputLine+1, outputFile.c_str() );
		    outputLine++;
                }
            }
            write( "\n                    break;\n", out );
        }
    }
}

void C_CodeGen::writeLexerActions( FILE *out )
{
    FOR_EACH( term, TerminalPs, grammar->terms ) {
        if( (*term)->action ) {
            fprintf( out, "                    case %d: // %s \n",
                     (*term)->symbolId, (*term)->name->c_str() );
	    outputLine++;
	    write( "                        YYL_PRE_ACTION();\n", out );
            FOR_EACH( act, Action, (*(*term)->action) ) {
		fprintf( out, "#line %d \"%s\"\n", act->posn.line, act->posn.filename );
		outputLine++;
		fprintf( out, "                        " );
                writeActionCode( *term, *act, out );
		outputLine++;
		fprintf( out, "\n#line %d \"%s\"\n", outputLine+1, outputFile.c_str() );
		outputLine++;
            }
	    write( "                        YYL_POST_ACTION();\n", out );
            write( "\n                        goto accept;\n", out );
	}
    }


    int count = 0;
    FOR_EACH( term, TerminalPs, grammar->terms ) {
        if( (*term)->action == NULL && (*term)->isResultUsed ) {
	    fprintf( out, "                    case %d: // %s \n",		     
                     (*term)->symbolId, (*term)->name->c_str() );
	    outputLine++;
	    count++;
	}
    }
    if( count > 0 ) {
	write( "                        YYL_SAVE_TEXT();\n", out );
        write( "                        goto accept;\n", out );
    }	    
}

void C_CodeGen::writeAttrCode( const Terminal *t, FILE *out )
{
    fprintf( out, "YYL_TEXT()" );
}

void C_CodeGen::writeAttrCode( const Rule *r, int n, FILE *out )
{
    assert( n > 0 && n <= r->length() );
    
    Symbol *sym = r->syms[n-1].sym;

    if( sym->type == NULL || *sym->type == SCAN_TYPE ) {
	fprintf( out, "(yypstrstack + yypstack[yypstacktop+%d].strpos)", n-1 );
    } else {
        fprintf( out, "yypstack[yypstacktop+%d].attr.yy%s", n-1,
	     typeNameMap[*sym->type].c_str() );
    }
    
}

void C_CodeGen::writeSynthAttrCode( const Symbol *s, FILE *out )
{
    fprintf( out, "%s.yy%s", (s->isTerminal ? "yylsynattr" : "yypsynattr"), 
	     typeNameMap[*s->type].c_str() );
}

string *C_CodeGen::defaultSymType()
{
    return new string("char *");
}
