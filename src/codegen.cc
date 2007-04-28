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
        case LANG_JAVA:
        case LANG_ADA:
            fprintf( stderr, "Language not yet supported\n" );
            return NULL;
        default:
            BUG("Unreachable branch in case statement");
    }
}

#define MATCH(a,l,b) (!strncmp(a,b,l) && l == strlen(b))

void CodeGen::handleCommand( char *cmd, int len, FILE *out )
{
/* FIXME: Make the command matching a little more efficient. */

/* Generic */
    if( MATCH(cmd,len,"START_CODE") ) {
	FOR_EACH( act, Action, grammar->code ) {
                writeActionCode( NULL, act->action->c_str(), out );
	}
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
    } else if( MATCH(cmd,len,"PARSER_NO_STATE") ) {
        writeIntegerConst( 0, out );
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
        for( int i=0; i<lr->states.size(); i++ )
            arr[i] = dfa->startStates[conflict->parserStateMap[i]];
        writeIntegerArray( arr, lr->states.size(), out );
    } else if( MATCH(cmd,len,"LEXER_ACCEPT_ARRAY") ) {
        int arr[dfa->states.size()];
        for( int i=0; i<dfa->states.size(); i++ )
            arr[i] = ( dfa->states[i].accept ?
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
    string fname, skelname;
    FILE *f, *skel;
    int len;
    char buf[BUFSIZE];
    
    if( config.outputSourceFile )
        fname = *config.outputSourceFile;
    else if( config.outputBase )
        fname = *config.outputBase + sourceExt();
    else BUG( "output base not set!" );
    skelname = config.skeletonPath + sourceSkel();    
    processFile( skelname.c_str(), fname.c_str() );
}

void CodeGen::createHeaderFile(void)
{
    string fname, skelname;
    FILE *f, *skel;
    char buf[BUFSIZE];
    
    if( config.outputHeaderFile )
        fname = *config.outputHeaderFile;
    else if( config.outputBase )
        fname = *config.outputBase + headerExt();
    else BUG( "output base not set!" );
    skelname = config.skeletonPath + headerSkel();
    processFile( skelname.c_str(), fname.c_str() );
}

void CodeGen::writeIntegerConst( int val, FILE *out )
{
    fprintf( out, "%d",val );
}

void CodeGen::writeIdentifier( const char *str, FILE *out )
{
    fprintf( out, "%s", str );
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
    FOR_EACH( nt, NonterminalPs, grammar->nonterms ) {
        if ( (*nt)->type )
            writeMemberVar( (*nt)->type->c_str(), (*nt)->name->c_str(), out );
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
                writeSynthAttrCode( r, out );
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
