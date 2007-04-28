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
        if( *s == '\"' ) fputc( '\\', out );
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
            fprintf( out, "                    case %d:\n                    ",
                     (*term)->symbolId );
            FOR_EACH( act, Action, (*(*term)->action) ) {
                writeActionCode( NULL, act->action->c_str(), out );
            }
            fprintf( out, "                        goto accept;\n" );
        }
    }
}

void C_CodeGen::writeAttrCode( const Rule *r, int n, FILE *out )
{
    if( !r ) {
        fprintf( out, "(yyf.buffer+yylfirst)" );
        return;
    }
    assert( n > 0 && n <= r->length() );
    
    Symbol *sym = r->syms[n-1].sym;

    fprintf( out, "yypattrstack[yypstacktop+%d]", n - 1 );
    if( sym->isTerminal )
        fprintf( out, ".yyscan" );
    else
        fprintf( out, ".yy%s", sym->name->c_str() );
    
}

void C_CodeGen::writeSynthAttrCode( const Rule *r, FILE *out )
{
    if( r )
        fprintf( out, "yypsynattr.yy%s", r->nonterm->name->c_str() );
    else
        fprintf( out, "yylsynattr.yyscan" );
}
