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
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <string>
#include "config.h"
#include "grammar.h"
#include "equiv.h"
#include "codegen.h"

#define BUFSIZE 512 /* Maximum skel file line length */

CodeGen *CodeGen::getInstance( language_gen_t lang ) {
    switch( lang ) {
        case LANG_C:
            return new C_CodeGen();
        case LANG_CPP:
	    return new Cpp_CodeGen();
        case LANG_JAVA:
        case LANG_ADA:
            fprintf( stderr, "Language not yet supported\n" );
            return NULL;
        default:
            BUG("Unreachable branch in case statement");
    }
}

int CodeGen::getErrors()
{
    return errors;
}

#define MATCH(a,l,b) (!strncmp(a,b,l) && l == strlen(b))

void CodeGen::handleCommand( char *cmd, int len, FILE *out )
{
/* FIXME: Make the command matching a little more efficient. */

/* Generic */
    if( MATCH(cmd,len,"START_CODE") ) {
	FOR_EACH( act, Action, grammar->code ) {
	    fprintf( out, "%s", act->action->c_str() );
	}
    } else if( MATCH(cmd,len,"PARSER_CONTEXT") ) {
	FOR_EACH( act, Action, grammar->contextCode ) {
	    fprintf( out, "%s", act->action->c_str() );
	}
    } else if( MATCH(cmd,len,"PARSER_INTERFACE") ) {
	FOR_EACH( act, Action, grammar->interfaceCode ) {
	    fprintf( out, "%s", act->action->c_str() );
	}
    } else if( MATCH(cmd,len,"PARSER_IMPLEMENTATION") ) {
	FOR_EACH( act, Action, grammar->implementationCode ) {
	    fprintf( out, "%s", act->action->c_str() );
	}
    } else if( MATCH(cmd,len,"INTERFACE_FILENAME") ) {
	fprintf( out, "%s", getOutputHeaderFile().c_str() );
    } else if( MATCH(cmd,len,"END_CODE") ) {
    } else if( MATCH(cmd,len,"ATTRIBUTES") ) {
        writeAttributes( out );
    } else if( MATCH(cmd,len,"NUM_SYMBOLS") ) {
        writeIntegerConst( grammar->numSymbols+1, out );
    } else if( MATCH(cmd,len,"SYMBOL_NAME_ARRAY") ) {
        writeSymbolNameArray( out );
/* Parser */
    } else if( MATCH(cmd,len,"PARSER_NAME") ) {
        writeIdentifier( grammar->parserName->c_str(), out );
    } else if( MATCH(cmd,len,"PARSER_START_STATE") ) {
        writeIntegerConst( 1, out );
    } else if( MATCH(cmd,len,"PARSER_ACCEPT_STATE") ) {
        writeIntegerConst( lr->acceptState, out );
    } else if( MATCH(cmd,len,"PARSER_RETURN") ) {
	writeParserReturn( out );
    } else if( MATCH(cmd,len,"PARSER_NO_STATE") ) {
        writeIntegerConst( 0, out );
    } else if( MATCH(cmd,len,"PARSER_RETURN_TYPE") ) {
	if( grammar->startSymbol->type != NULL ) {
	    writeType( grammar->startSymbol->type->c_str(), out );
	} else {
	    writeType( NULL, out );
	}
    } else if( MATCH(cmd,len,"PARSER_RETURN_IF_TYPED") ) {
	if( grammar->startSymbol->type != NULL ) {
	    writeReturn( out );
	}
    } else if( MATCH(cmd,len,"PARSER_ERROR_TOKEN") ) {
        writeIntegerConst( grammar->errorTerm->symbolId, out );
    } else if( MATCH(cmd,len,"PARSER_NEXTCHECK_LEN") ) {
        writeIntegerConst( parser->nextCheckLen, out );
    } else if( MATCH(cmd,len,"PARSER_BASE_LEN") ) {
        writeIntegerConst( parser->baseLen, out );
    } else if( MATCH(cmd,len,"PARSER_NUM_STATES") ) {
        writeIntegerConst( lr->numTotalStates, out );
    } else if( MATCH(cmd,len,"PARSER_NUM_REAL_STATES") ) {
        writeIntegerConst( lr->states.size(), out );
    } else if( MATCH(cmd,len,"PARSER_NEXTCHECK_ARRAY") ) {
        writeNextCheckArray( parser, out );
    } else if( MATCH(cmd,len,"PARSER_BASE_ARRAY") ) {
        writeIntegerArray( parser->base, parser->baseLen, out );
    } else if( MATCH(cmd,len,"PARSER_DEFAULT_ARRAY") ) {
        writeIntegerArray( parser->def, parser->baseLen, out );
    } else if( MATCH(cmd,len,"PARSER_REDUCE_LEN_ARRAY") ) {
        writeIntegerArray( lr->reduceLengths, lr->numTotalStates, out );
    } else if( MATCH(cmd,len,"PARSER_REDUCE_TOK_ARRAY") ) {
        writeIntegerArray( lr->reduceTokens, lr->numTotalStates, out );
    } else if( MATCH(cmd,len,"PARSER_ACTION_CODE") ) {
        writeParserActions( out );
    } else if( MATCH(cmd,len,"PARSER_LAST_SHIFT_STATE") ) {
        writeIntegerConst( lr->states.size()-1, out );
    } else if( MATCH(cmd,len,"PARSER_FIRST_REDUCE_STATE") ) {
        writeIntegerConst( lr->states.size(), out );
/* Scanner */
    } else if( MATCH(cmd,len,"LEXER_NEXTCHECK_LEN") ) {
        writeIntegerConst( scanner->nextCheckLen, out );
    } else if( MATCH(cmd,len,"LEXER_BASE_LEN") ) {
        writeIntegerConst( scanner->baseLen, out );
    } else if( MATCH(cmd,len,"LEXER_NO_STATE") ) {
        writeIntegerConst( 0, out );
    } else if( MATCH(cmd,len,"LEXER_EOF_CHAR") ) {
        writeIntegerConst( EOF_CHAR, out );
    } else if( MATCH(cmd,len,"LEXER_EOF_TOKEN") ) {
        writeIntegerConst( grammar->eofSymbol->symbolId, out );
    } else if( MATCH(cmd,len,"LEXER_SPACE_TOKEN") ) {
        writeIntegerConst( grammar->spaceTerm?grammar->spaceTerm->symbolId:-2, out );
    } else if( MATCH(cmd,len,"LEXER_START_STATE_ARRAY") ) {
        int arr[lr->states.size()];
        for( int i=0; i<lr->states.size(); i++ ) {
	    if( conflict == NULL ) {
		arr[i] = 1;
	    } else {
		arr[i] = dfa->startStates[conflict->parserStateMap[i]];
	    }
	}
        writeIntegerArray( arr, lr->states.size(), out );
    } else if( MATCH(cmd,len,"LEXER_ACCEPT_ARRAY") ) {
        int arr[dfa->states.size()];
        for( int i=0; i<dfa->states.size(); i++ )
            arr[i] = ( dfa->states[i].accept != NULL ?
                       dfa->states[i].accept->symbolId : -1 );
        writeIntegerArray( arr, dfa->states.size(), out );
    } else if( MATCH(cmd,len,"LEXER_NUM_EQUIV") ) {
        writeIntegerConst( CHARSET_SIZE, out ); /* Number of chars */
    } else if( MATCH(cmd,len,"LEXER_EQUIV_ARRAY") ) {
        writeIntegerArray( dfa->equivs->group, CHARSET_SIZE, out );
    } else if( MATCH(cmd,len,"LEXER_NUM_STATES") ) {
        writeIntegerConst( dfa->states.size(), out );
    } else if( MATCH(cmd,len,"LEXER_NEXTCHECK_ARRAY") ) {
        writeNextCheckArray( scanner, out );
    } else if( MATCH(cmd,len,"LEXER_BASE_ARRAY") ) {
        writeIntegerArray( scanner->base, scanner->baseLen, out );
    } else if( MATCH(cmd,len,"LEXER_DEFAULT_ARRAY") ) {
        writeIntegerArray( scanner->def, scanner->baseLen, out );
    } else if( MATCH(cmd,len,"LEXER_ACTION_CODE") ) {
        writeLexerActions( out );        
    } else if( MATCH(cmd,len,"Id")  ) {
	fprintf(out, "$%.*s", len, cmd);
    } else if( len == 0 ) {
	fprintf(out, "$");
    } else {
        fprintf(stderr, "Unrecognized command in skeleton file: %.*s\n",len,cmd);
    }
}


/*
 * Read a skeleton file from inname and write it out to outname, with command
 * sequences (ie $START_CODE) replaced with the appropriate values.
 */
void CodeGen::processFile( const char *inname, const char *outname )
{
    FILE *in, *out;
    int len;
    char buf[BUFSIZE];

    /* Open files */
    if( (in = fopen( inname, "ro" )) == NULL ) {
        fprintf( stderr, "Unable to open skeleton source file: %s (%s)\n",
                 inname, strerror(errno) );
        return;
    }

    if( (out = fopen( outname, "wo" )) == NULL ) {
        fprintf( stderr, "Unable to open output source file: %s (%s)\n",
                 outname, strerror(errno) );
        return;
    }

    /* Process line-by-line (avoids breaking commands at end of buffer) */
    while( fgets( buf, BUFSIZE, in ) ) {
        char *s, *p;
        
        /* This looks worse than it is. Basically, extract out anything which
         * matches \$[A-za-z_]*, and pass it to handleCommand. everything else
         * gets written literally. ('tis a bit more flexible than ie rex/lalr)
         */
        for( s = p = buf; *s; s++ ) {
            if( *s == '$' ) { /* Command */
                if( s != p ) fwrite( p, s-p, 1, out );
                char *j;
                if( *(s+1) == '{' ) {
                    for( j = s+2; *j && *j !='}'; j++ );
                    handleCommand( s+2, j-s-2, out );
                    s = j; p=j+1;
                } else {
                    for( j = s+1; *j && (isalpha(*j) || *j == '_'); j++ );
                    handleCommand( s+1, j-s-1, out );
                    s = j - 1; p = j;
                }
            }
        }
        if( s != p ) fwrite( p, s-p, 1, out );
    }
    fclose( in );
    fclose( out );
}

void CodeGen::createSourceFile(void)
{
    processFile( getSkeletonFile( sourceSkel() ).c_str(),
		 getOutputSourceFile().c_str() );
}

void CodeGen::createHeaderFile(void)
{
    processFile( getSkeletonFile( headerSkel() ).c_str(),
		 getOutputHeaderFile().c_str() );
}

string CodeGen::getOutputSourceFile(void)
{
    if( config.outputSourceFile )
        return *config.outputSourceFile;
    else if( config.outputBase )
        return *config.outputBase + sourceExt();
    else BUG( "output base not set!" );
}
    

string CodeGen::getOutputHeaderFile(void)
{
    if( config.outputHeaderFile )
        return *config.outputHeaderFile;
    else if( config.outputBase )
        return *config.outputBase + headerExt();
    else BUG( "output base not set!" );
}
    


string CodeGen::getSkeletonFile( char *file )
{
    if( config.skeletonPath[config.skeletonPath.length()-1] != '/' ) {
	return config.skeletonPath + "/" + file;
    } else {
	return config.skeletonPath + file;
    }
}


void CodeGen::writeIntegerConst( int val, FILE *out )
{
    fprintf( out, "%d",val );
}

void CodeGen::writeIdentifier( const char *str, FILE *out )
{
    fprintf( out, "%s", str );
}

void CodeGen::writeType( const char *str, FILE *out )
{
    if( str != NULL ) {
	fprintf( out, "%s", str );
    }
}

/* Good for most languages, but virtual just in case */
void CodeGen::writeStringConst( const char *str, FILE *out )
{
    fprintf( out, "\"%s\"", str ); /* FIXME: Escaping? */
}

void CodeGen::writeEscapedString( const char *s, FILE *out )
{
    while( *s ) {
        if( *s == '\'' || *s == '\"' || *s == '\\' ) fputc( '\\', out );
        fputc( *s++, out );
    }
}

void CodeGen::writeAttributes( FILE *out )
{
    for( hash_map<string,string, hashString>::iterator p = typeNameMap.begin(); p != typeNameMap.end(); p++ ) {
	writeMemberVar( p->first.c_str(), p->second.c_str(), out );
    }
}

void CodeGen::writeIntegerArray( int *arr, int len, FILE *out )
{
    for( int i = 0; i < len; i++ ) {
        fprintf( out, "%d", arr[i] );
        if( i != len-1 )
            fprintf( out, ", " );
    }
}

void CodeGen::writeActionCode( const Rule *r, const char *s, FILE *out )
{
    while( *s ) {
        if( *s == '$' ) {
            if( *++s == '$' ) {
                writeSynthAttrCode( r->nonterm, out );
                s++;
            } else if( isdigit( *s ) ) {
                char *p;
                int v = strtol( s, &p, 10 );
                if( p == NULL ) /* Can't happen */
                    BUG( "strtol failed! Holy sh*t..." );
                if( r == NULL ){ /* Lexical rule */
                    if( v != 1 )
                        fprintf( stderr, "Attribute outside of rule: %d\n", v );
                    else
                        writeAttrCode( r, v, out );
                } else if( v <= 0 || v > r->length() )
                    fprintf( stderr, "Attribute outside of rule: %d\n", v );
                else
                    writeAttrCode( r, v, out );
                s = p;
            }
            else fputc( '$', out );
        }
        else {
            fputc( *s, out );
            s++;
        }
    }
}


void CodeGen::writeActionCode( const Terminal *t, const char *s, FILE *out )
{
    while( *s ) {
        if( *s == '$' ) {
            if( *++s == '$' ) {
                writeSynthAttrCode( t, out );
                s++;
            } else if( isdigit( *s ) ) {
                char *p;
                int v = strtol( s, &p, 10 );
                if( p == NULL ) /* Can't happen */
                    BUG( "strtol failed! Holy sh*t..." );
		if( v != 1 )
		    fprintf( stderr, "Attribute outside of rule: %d\n", v );
                else
		    writeAttrCode( t, out );
                s = p;
            }
            else fputc( '$', out );
        }
        else {
            fputc( *s, out );
            s++;
        }
    }
}

void CodeGen::computeSymbolUses( Rule *rule, const char *s ) 
{
    while( *s ) {
	if( *s == '$' ) {
	    if( isdigit( *++s ) ) {
		char *p;
		int v = strtol( s, &p, 10 );
		if( p == NULL ) /* Can't happen */
		    BUG( "strtol failed! Holy sh*t..." );
		if( v > 0 && v <= rule->length() ) {
		    rule->syms[v-1].sym->isResultUsed = true;
		    rule->syms[v-1].isResultUsed = true;
		}
		s = p;
	    } else if( *s == '$' ) {
	    }
	} else {
	    s++;
	}
    }
}

void CodeGen::computeTypes()
{
    map<int,int> seen;
    computeTypes( seen, grammar->startSymbol, NULL, 0 );
    FOR_EACH( t, TerminalPs, grammar->terms ) {
	if( (*t)->type != NULL && typeNameMap.find(*(*t)->type) == typeNameMap.end() &&
	    *(*t)->type != SCAN_TYPE ) {
	    typeNameMap[*(*t)->type] = *(*t)->name;
	}
    }
    FOR_EACH( nt, NonterminalPs, grammar->nonterms ) {
	if( (*nt)->type != NULL && typeNameMap.find(*(*nt)->type) == typeNameMap.end() ) {
	    typeNameMap[*(*nt)->type] = *(*nt)->name;
	}
    }
}

/**
 * Traverse the grammar, setting the types of all nonterminals that
 * lack both an explicit type and action. Reports conflicts and untyped
 * rules with actions.
 * Assumes that computeSymbolUses has already been run.
 */
string *CodeGen::computeTypes( map<int,int> &seen, Symbol *sym, string *type,
			       int resultUsed )
{
    if( seen.find(sym->symbolId) != seen.end() && seen[sym->symbolId] >= resultUsed ) {
	return sym->type;
    }
    seen[sym->symbolId] = resultUsed;
    if( resultUsed != 0 ) {
	sym->isResultUsed = true;
    }
    if( type != NULL ) {
	if( sym->type == NULL ) {
	    sym->type = new string(*type);
	}
    } 
    if( sym->isTerminal ) {
	if( sym->type == NULL && sym->isResultUsed ) {
	    sym->type = new string(SCAN_TYPE);
	}
    } else {
	Nonterminal *nt = (Nonterminal *)sym;
	FOR_EACH_RULE( r, nt ) {
	    if( (*r)->reduceAction == NULL ) {
		/* No reduction - assume default chain action */
		if( (*r)->length() > 0 ) {
		    Symbol *s = (*r)->syms[0].sym;
		    type = computeTypes( seen, s, type, resultUsed );
		    if( type != NULL ) {
			if( sym->type == NULL ) {
			    sym->type = new string(*type);
			} else if( *sym->type != *type ) {
			    fprintf(stderr, "Value of %s used in rule:\n ",
				    s->name->c_str());
			    (*r)->print(stderr);
			    fprintf(stderr, "\n was expected to have type %s, but was %s\n",
				    sym->type->c_str(), type->c_str());
			    errors++;
			}
		    }
		    for( RuleSymbols::iterator rs = ++(*r)->syms.begin(); 
			 rs != (*r)->syms.end(); rs++ ) {
			computeTypes( seen, rs->sym, NULL, rs->isResultUsed ? 1 : 0 );
		    }
		}
	    } else {
		FOR_EACH( act, Action, (*(*r)->reduceAction) ) {
		    const char *s = act->action->c_str();
		    computeSymbolUses(*r,s);
		}
		FOR_EACH_RULESYM( rs, *r ) {
		    if( computeTypes( seen, rs->sym, NULL, rs->isResultUsed ? 1 : 0 ) == NULL
			&& rs->isResultUsed ) {
			fprintf( stderr, "Value of %s is used in rule:\n ", 
				 rs->sym->name->c_str() );
			(*r)->print(stderr);
			fprintf( stderr, "\n  but has missing or unknown type.\n" );
			errors++;
		    }
		}
	    }
	}
    }
    return sym->type;
}
