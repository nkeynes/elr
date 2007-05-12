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
/*
 * lex.c - input lexical analyser
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string>
#include "lex.h"
#include "parser.h"

/* Module */
int yyf;
unsigned int yylength;
Position yychposn;
char *yydata, *yych, *yyend;
bool yyStart = true;

/* Recognizes:
   %left = LEFTPREC
   %right = RIGHTPREC
   %nonassoc = NONASSOCPREC
   %start = START
   %class = CLASS
   %include = INCLUDE
   %{ .*? %} = CODE
   error = ERROR
   : = COLON
   | = OR
   . = PERIOD
   { * } = ACTION (w/ nested parens)
   [A-Za-z_-][A-Za-z0-9_-]* = IDENTIFIER
   ".*?" = REGEXP
   '.*?' = STRING
   .*? = TYPE (at start of rule only)
 */

int lexOpenFile( const char *filename )
{
    yyf = open( filename, O_RDONLY );
    if( yyf == -1 ) return -1;

    yylength = lseek( yyf, 0, SEEK_END );

    yychposn.line = 1;
    yychposn.column = 1;
    yychposn.filename = filename;
    if( yylength == 0 ) { /* Empty file - hack around mmap strangeness */
        close( yyf );
        yydata = yyend = NULL;
        yych = yyend+1;
        return 0;
    }
    /* Anyone with a > 4Gb grammar file deserves to lose... */
    yydata = (char *)mmap( NULL, yylength, PROT_READ, MAP_PRIVATE, yyf, 0 );
    if( yydata == MAP_FAILED ) {
        close(yyf);
        return -1;
    }
    yych = yydata;
    yyend = yydata + yylength - 1;
    yyStart = true;
    return 0;
}

void lexCloseFile( )
{
    munmap( yydata, yylength );
    close( yyf );
}

/*
char *yystrdup( char *s, int length )
{
    char *str = (char *)malloc( length + 1 );
    if( str == NULL ) {
        fprintf( stderr, "Memory allocation error\n");
        exit(1);
    }
    memcpy( str, s, length );
    str[length] = '\0';
    return str;
}
*/
string *yystrdup( char *s, int length )
{
    return new string(s, length);
}

#define ISEOF() (yych > yyend )
#define NEXT() nextCh()

void nextCh( void )
{
    if( *yych == '\n' ){
        yychposn.line++;
        yychposn.column = 1;
    } else yychposn.column++;
    yych++;
}

token_t yylex( void )
{
    int c;
    char *yyhead;
    bool atStart = yyStart;
    yyStart = false;
    
    while( 1 ) {
        while( !ISEOF() && isspace( *yych ) ) NEXT();
        yylval.scan.posn = yychposn;
        
        if( ISEOF() ) return 0;
        if( isalpha( *yych ) || *yych == '_' || *yych == '-' ){
            /* Identifier */
            yyhead = yych;
            while(!ISEOF() && (isalnum(*yych) || *yych == '_' || *yych == '-'))
                NEXT();
            if( yych - yyhead == 5 &&
                !strncmp( yyhead, "error", 5 ) )
                return ERROR_IDENT;
            yylval.scan.str = yystrdup( yyhead, yych - yyhead );
            return IDENT;
        }
        switch( *yych ) {
            case ':': NEXT(); return COLON;
            case ';': NEXT(); yyStart = true; return SEMICOLON;
            case '|': NEXT(); return OR;
            case '.': NEXT(); return PERIOD;
            case '(':
                NEXT();
                if( atStart ) {
                    yyhead = yych;
                    c = 1;
                    while( c > 0 && yych < yyend ) {
                        NEXT();
                        if( *yych == '(' ) c++;
                        else if( *yych == ')' ) c--;
                    }
                    NEXT();
                    yylval.scan.str = yystrdup( yyhead, yych - yyhead - 1 );
                    return TYPE;
                } else return LPAREN;
            case ')': NEXT(); return RPAREN;
            case '=': NEXT(); return EQUALS;
            case '%': /* Directive */
                NEXT();
                if( *yych == '{' ) {
                    c = 1;
                    yyhead = yych;
                    while( c > 0 && yych < yyend ) {
                        NEXT();
                        if( *yych == '{' ) c++;
                        else if( *yych == '}' ) c--;
                    }
                    NEXT();
                    yylval.scan.str = yystrdup( yyhead+1, yych - yyhead-2 );
                    return CODE;
                }
                yyhead = yych;
                while( !ISEOF() && isalpha(*yych) ) NEXT();
                c = yych - yyhead;
                if( !strncasecmp(yyhead, "left", c ) ) return LEFTPREC;
                else if( !strncasecmp(yyhead, "right", c ) ) return RIGHTPREC;
                else if( !strncasecmp(yyhead, "nonassoc", c ) )
                    return NONASSOCPREC;
                else if( !strncasecmp(yyhead, "start", c ) ) return START;
                else if( !strncasecmp(yyhead, "class", c ) ) return CLASS;
                else if( !strncasecmp(yyhead, "include", c ) ) return INCLUDE;
                break;
            case '\"': /* Regexp */
                yyhead = yych;
                NEXT();
                while( !ISEOF() && (*yych != '\"' || *(yych-1) == '\\' ) )
                    NEXT();
                yylval.scan.str = yystrdup( yyhead, yych - yyhead + 1 );
                if( *yych != '\"' ) yylval.scan.str->append("\"");
                NEXT();
                return REGEXP;
                break;
            case '\'': /* String */
                yyhead = yych;
                NEXT();
                while( !ISEOF() && (*yych != '\'' || *(yych-1) == '\\' ) )
                    NEXT();
                yylval.scan.str = yystrdup( yyhead, yych - yyhead + 1 );
                if( *yych != '\'' ) yylval.scan.str->append("\'");
                NEXT();
                return STRING;
                break;
            case '{': /* Action */
                c = 1;
                yyhead = yych;
                while( c > 0 && yych < yyend ) {
                    NEXT();
                    if( *yych == '{' ) c++;
                    else if( *yych == '}' ) c--;
                }
                NEXT();
                yylval.scan.str = yystrdup( yyhead+1, yych - yyhead-2 );
                return ACTION;
                break;
	    case '/': 
		NEXT(); 
		if( *yych == '*' ) { /* comment */
		    NEXT();
		    if( !ISEOF() ) {
			NEXT();
		    }
		    while (!ISEOF() && !(*yych == '/' && *(yych-1) == '*') ) {
			NEXT();
		    }
		    NEXT();
		    continue;
		} else if( *yych == '/' ) { /* line comment */
		    while(!ISEOF() && *yych != '\n') {
			NEXT();
		    }
		    NEXT();
		    continue;
		}
		/* fallthrough */
            default:
                printf( "Illegal character: '%c'\n", *yych );
                NEXT();
        }
    }
}
