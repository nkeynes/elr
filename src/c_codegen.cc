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
#include "config.h"
#include "codegen.h"
#include "grammar.h"

void C_CodeGen::writeMemberVar( const char *type, const char *name, FILE *out )
{
    fprintf( out, "    %s yy%s;\n", type, name );
}

void C_CodeGen::writeSymbolNameArray( FILE *out )
{
    for( int i=1; i<=grammar->numSymbols; i++ ) {
        writeStringConst( grammar->symbol(i)->name->c_str(), out );
        if( i != grammar->numSymbols )
            fprintf( out, ", " );
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

void C_CodeGen::writeNextCheckArray( CombVector *vect, FILE *out )
{
    for( int i=0; i<vect->nextCheckLen; i++ ) {
        fprintf( out, "{%d,%d}", vect->nextCheck[i][0],vect->nextCheck[i][1] );
        if( i != vect->nextCheckLen -1 )
            fprintf( out, ", " );
    }
}

/* I couldn't think of a good way to put these in the template file, so... */
void C_CodeGen::writeParserActions( FILE *out )
{
    FOR_EACH( nt, NonterminalPs, grammar->nonterms ) {
        FOR_EACH( rule, RulePs, (*nt)->rules ) {
//            if( (*rule)->reduceAction && (*rule)->reduceAction->size() > 0 ) {
            fprintf( out, "                case %d:\n                    ",
                     (*rule)->ruleId + lr->states.size()-1 );
            if( config.genDebug ) {
                fprintf( out, "printf(\"Reducing %s ->",
                         (*rule)->nonterm->name->c_str() );
                FOR_EACH( sym, RuleSymbols, (*rule)->syms ) {
                    fputc( ' ', out );
                    writeEscapedString( sym->sym->name->c_str(), out );
                }
                fprintf( out, "\\n\");\n                    " );
            }
            if( (*rule)->reduceAction ) {
                FOR_EACH( act, Action, (*(*rule)->reduceAction) ) {
                    writeActionCode( *rule, act->action->c_str(), out );
                }
            }
            fprintf( out,"\n                    break;\n");
//            }
        }
    }
}

void C_CodeGen::writeLexerActions( FILE *out )
{
    FOR_EACH( term, TerminalPs, grammar->terms ) {
        if( (*term)->action ) {
            fprintf( out, "                    case %d: // %s \n",
                     (*term)->symbolId, (*term)->name->c_str() );
	    fprintf( out, "                        YYL_PRE_ACTION();\n" );
	    fprintf( out, "                        " );
            FOR_EACH( act, Action, (*(*term)->action) ) {
                writeActionCode( *term, act->action->c_str(), out );
            }
	    fprintf( out, "                        YYL_POST_ACTION();\n" );
            fprintf( out, "\n                        goto accept;\n" );
	}
    }


    int count = 0;
    FOR_EACH( term, TerminalPs, grammar->terms ) {
        if( (*term)->action == NULL && (*term)->isResultUsed ) {
	    fprintf( out, "                    case %d: // %s \n",		     
                     (*term)->symbolId, (*term)->name->c_str() );
	    count++;
	}
    }
    if( count > 0 ) {
	fprintf( out, "                        YYL_SAVE_TEXT();\n" );
	fprintf( out, "                        goto accept;\n" );
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

    if( sym->isTerminal && sym->type == NULL ) {
	fprintf( out, "(yypstrstack + yypstack[yypstacktop+%d].strpos)", n-1 );
    } else {
        fprintf( out, "yypstack[yypstacktop+%d].attr.yy%s", n-1, sym->name->c_str() );
    }
    
}

void C_CodeGen::writeSynthAttrCode( const Symbol *s, FILE *out )
{
    fprintf( out, "%s.yy%s", (s->isTerminal ? "yylsynattr" : "yypsynattr"), s->name->c_str() );
}
