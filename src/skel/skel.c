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
 *
 * As a special exception, you may create a larger work that contains
 * part or all of this parser skeleton and distribute that work
 * under terms of your choice, so long as that work isn't itself a
 * parser generator using the skeleton or a modified version thereof
 * as a parser skeleton.  Alternatively, if you modify or redistribute
 * the parser skeleton itself, you may (at your option) remove this
 * special exception, which will cause the skeleton and the resulting
 * Bison output files to be licensed under the GNU General Public
 * License without this special exception.                       
 */

$START_CODE

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef YYRETVAL
#define YYRETVAL int
#endif

typedef int YY_CHAR;

#define YYP_DEFAULT_STACK_LEN 128

#ifndef YYL_DEFAULT_BUFFER_LEN
#define YYL_DEFAULT_BUFFER_LEN 1024
#endif
#ifndef YYL_MIN_BUFFER_FILL
#define YYL_MIN_BUFFER_FILL 128
#endif

#define YYL_MAX_SNARF_LEN 4096*1024 /* Snarf up to a 4Mb file */
#define YYL_END_OF_LINE '\n'
#ifndef YY_ERROR
#define YY_ERROR(...) yyError(__VA_ARGS__)
#endif
#ifndef YY_DEBUG
#ifdef DEBUG
#define YY_DEBUG(...) printf(__VA_ARGS__)
#else
#define YY_DEBUG(...)
#endif
#endif

typedef union {
    $ATTRIBUTES
} yyattr_t;

typedef struct {
    int state;
    int line, col, pos;
    int strpos;
    yyattr_t attr;
} yystack_t;

struct yy_parseable {
    char *filename;
    char *buffer;
    int buflen;
    int bufend; /* end of current data */
    int fd;
    int ismybuffer; /* Whether to free buffer on completion */
    YY_CHAR yylchar;
    int yylline;
    int yylcol;
    int yylfirst;
    int yylpos;
    int yyllastpos;
    struct yy_parseable *next; /* For file stacking */
};

/* Function prototypes */
YYRETVAL $PARSER_NAME( struct yy_parseable yyf );
static int yyfInit( struct yy_parseable *yyf );
static int yyfRelease( struct yy_parseable *yyf );
static int yyfFill( struct yy_parseable *yyf );
static int yyError( char *s, ... );


/* Parser tables */
static const int yypNextCheck[$PARSER_NEXTCHECK_LEN][2] = { $PARSER_NEXTCHECK_ARRAY };
static const int yypBase[$PARSER_BASE_LEN] = { $PARSER_BASE_ARRAY };
static const int yypDefault[$PARSER_BASE_LEN] = { $PARSER_DEFAULT_ARRAY };
static const int yypReduceLength[$PARSER_NUM_STATES] = { $PARSER_REDUCE_LEN_ARRAY };
static const int yypReduceToken[$PARSER_NUM_STATES] = { $PARSER_REDUCE_TOK_ARRAY };
static const char *yySymbolName[$NUM_SYMBOLS] = { "ERR", $SYMBOL_NAME_ARRAY };

/* Lexer tables */
static const int yylNextCheck[$LEXER_NEXTCHECK_LEN][2] = { $LEXER_NEXTCHECK_ARRAY };
static const int yylBase[$LEXER_BASE_LEN] = { $LEXER_BASE_ARRAY };
static const int yylDefault[$LEXER_BASE_LEN] = { $LEXER_DEFAULT_ARRAY };
static const int yylAccept[$LEXER_NUM_STATES] = { $LEXER_ACCEPT_ARRAY };
static const int yylStart[$PARSER_NUM_REAL_STATES] = { $LEXER_START_STATE_ARRAY };
static const int yylEquivClasses[$LEXER_NUM_EQUIV] = { $LEXER_EQUIV_ARRAY };


YYRETVAL ${PARSER_NAME}_file( char *filename )
{
    struct yy_parseable yyf;
    
    yyf.filename = filename;
    yyf.next = NULL;
    yyf.fd = open( filename, O_RDONLY );
    if( yyf.fd == -1 ) return -1;
    yyf.buffer = NULL;
    return $PARSER_NAME( yyf );
}

YYRETVAL ${PARSER_NAME}_snarf_file( char *filename )
{
    struct yy_parseable yyf;
    struct stat st;
    
    yyf.filename = filename;
    yyf.next = NULL;
    yyf.fd = open( filename, O_RDONLY );
    if( yyf.fd == -1 ) return -1;

    if( fstat( yyf.fd, &st ) == -1 ) {
        YY_ERROR( "Unable to stat file (%s)\n", strerror(errno) );
        return -1;
    }
    if( S_ISREG(st.st_mode) && st.st_size < YYL_MAX_SNARF_LEN ) {
        yyf.buffer = (char *)malloc( st.st_size );
        if( yyf.buffer ) {
            yyf.buflen = st.st_size;
            yyf.ismybuffer = 1;
            yyf.bufend = read( yyf.fd, yyf.buffer, yyf.buflen );
        }
    }
    return $PARSER_NAME(yyf);    
}
    

YYRETVAL ${PARSER_NAME}_buffer( char *buf, int len )
{
    struct yy_parseable yyf;

    yyf.filename = NULL;
    yyf.next = NULL;
    yyf.buffer = buf;
    yyf.buflen = yyf.bufend = len;
    yyf.fd = -1;
    yyf.ismybuffer = 0;
    return $PARSER_NAME( yyf );
}

YYRETVAL ${PARSER_NAME}_stream( int fd )
{
    struct yy_parseable yyf;
    
    yyf.next = NULL;
    yyf.fd = fd;
    yyf.buffer = NULL;
    yyf.filename = NULL;
    return $PARSER_NAME( yyf );
}


/* Parser macros */
#define YYP_PUSH(st,a) { yypstack[yypstacktop].state = st; \
 	                       yypstack[yypstacktop].attr = a; \
                               yypstacktop++; }

int __inline__ YYP_GOTO( int state, int token ){
    int yypnextstate = state, yypidx;
    do {
        yypidx = yypBase[yypnextstate]+token;
        if( yypNextCheck[yypidx][1] == yypnextstate ) { 
            yypnextstate = yypNextCheck[yypidx][0]; 
            break; 
        } else { yypnextstate = yypDefault[yypnextstate]; } 
    } while( 1 );
    return yypnextstate;

}

/* Lexer macros */
int __inline__ YYL_GOTO( int state, int token ){
    int yylnextstate = state, yylidx;
    do {
        yylidx = yylBase[yylnextstate]+token;
        if( yylNextCheck[yylidx][1] == yylnextstate ) { 
            yylnextstate = yylNextCheck[yylidx][0]; 
            break; 
        } else { yylnextstate = yylDefault[yylnextstate]; } 
    } while( 1 );
    return yylnextstate;
}

#define YYL_PRE_ACTION() yyf.yylchar = yyf.buffer[yyf.yylpos]; yyf.buffer[yyf.yylpos] = '\0'
#define YYL_POST_ACTION() yyf.buffer[yyf.yylpos] = yyf.yylchar
#define YYL_TEXT() (yyf.buffer + yyf.yylfirst)

#define YYL_SAVE_TEXT() yylstrlen = yyf.yylpos - yyf.yylfirst + 1;

#define YYP_PUSH_TEXT( ) {						\
	if( yypstrstacklen - yypstrstacktop < yylstrlen ) {		\
	    if( (yypstrstacklen << 1) < (yypstrstacktop + yylstrlen) ) {	\
		yypstrstacklen = yypstrstacktop + yylstrlen;		\
	    } else {							\
		yypstrstacklen = yypstrstacklen << 1;			\
	    }								\
	    yypstrstack = realloc( yypstrstack, yypstrstacklen );	\
	}								\
	memcpy( yypstrstack + yypstrstacktop, yyf.buffer + yyf.yylfirst, yylstrlen-1 ); \
	yypstrstacktop += yylstrlen;					\
	yypstrstack[yypstrstacktop-1] = '\0';				\
    }

YYRETVAL $PARSER_NAME( struct yy_parseable yyf )
{
    int yytoken;
    int yypstate = $PARSER_START_STATE;
    int yypnextstate, yypnextidx, yyprecovering = 0;
    int yyperrorcount = 0;

    /* Lexer variables */
    int yylstate, yylnextstate;
    int yyllastaccept;
    int yylline, yylcol, yylpos, yylstrpos;
    int yylstrlen = 0;

    /* String stack */
    char *yypstrstack;
    int yypstrstacklen = YYL_DEFAULT_BUFFER_LEN;
    int yypstrstacktop = 0;
    
    yystack_t *yypstack;
    yyattr_t yylsynattr, yypsynattr;
    int yypstacklen = YYP_DEFAULT_STACK_LEN;
    int yypstacktop = 0;

    yyfInit( &yyf );
    
    yypstack = (yystack_t *)malloc( YYP_DEFAULT_STACK_LEN * sizeof(yystack_t) );
    yypstrstack = (char *)malloc( YYL_DEFAULT_BUFFER_LEN );
    
    while( yypstate != $PARSER_ACCEPT_STATE ) { /* While not accepting */
        /* Check stack size */ /* FIXME: Error checking */
        if( yypstacktop == yypstacklen ) {
            yypstacklen <<= 1;
            yypstack = (yystack_t *)realloc( yypstack, yypstacklen * sizeof(yystack_t) );
        }
        
        /* Read a token */
        
/************************** Lexical Analysis Section *************************/
      lexer:
        yylstate = yylStart[yypstate];
        yyllastaccept = -1;
	yylline = yyf.yylline;
	yylcol = yyf.yylcol;
	yylpos = yyf.yylfirst = yyf.yylpos;
	yylstrpos = yypstrstacktop;
	yylstrlen = 0;

        while( 1 ) {
	    /*************** Read next character ******************/
	    if( yyf.bufend <= yyf.yylpos && !yyfFill( &yyf ) ){
		yyf.yylchar = yylEquivClasses[$LEXER_EOF_CHAR]; 
		yyf.yylpos++;					
	    } else {						
		if( yyf.buffer[yyf.yylpos] == YYL_END_OF_LINE ) {	
		    yyf.yylline++;				
		    yyf.yylcol = 1;				
		} else {					
		    yyf.yylcol++;				
		}						  
		yyf.yylchar = (yylEquivClasses[yyf.buffer[yyf.yylpos++]]);
	    }

            /* Lookup transition table */
            yylnextstate = YYL_GOTO(yylstate, yyf.yylchar);
            
            if( yylnextstate == $LEXER_NO_STATE ) {
                if( yyllastaccept == $LEXER_EOF_TOKEN &&
                        yyfRelease( &yyf ) == 1 ) {
                    yyf.yylfirst = yyf.yylpos-1;
		    yyllastaccept = -1;
                    yylstate = yylStart[yypstate];
		    yylline = yylcol = 1;
                } else if( yyllastaccept == -1 ) {
		    /* We haven't encountered a valid string - ERROR
		     * Try to recover by dropping the first character and
		     * starting again
		     */
		    YY_ERROR( "Unscannable string\n" );
		    yyf.yylline = yylline;
		    yyf.yylcol = yylcol;
		    yyf.yylpos = yylpos = ++yyf.yylfirst;
		    yylstate = yylStart[yypstate];
		} else {
		    yyf.yylpos = yyf.yyllastpos;
		    switch( yyllastaccept ) {
$LEXER_ACTION_CODE;
                    default:
                        /* Have a valid accept state */
                        goto accept;
		    }
                }
            } else {
                yylstate = yylnextstate;
                if( yylAccept[yylstate] != -1 ) {
                    /* Potentially accepting state, save for backtracking */
                    yyf.yyllastpos = yyf.yylpos;
                    yyllastaccept = yylAccept[yylstate];
                }
            }
        }
      accept:
        yytoken = yyllastaccept;

	if( yyf.buffer ) {
	    YY_DEBUG( "Scanned: %s ('%.*s')\n", yySymbolName[yytoken], yyf.yylpos-yyf.yylfirst, yyf.buffer+yyf.yylfirst );
	} else {
	    YY_DEBUG( "Scanned: %s\n", yySymbolName[yytoken] );
	}

        if( yytoken == $LEXER_SPACE_TOKEN ) goto lexer;
/************************ End Lexical Analysis Section ***********************/

        do {
            /* Lookup state table (comb-vector) */
            yypnextstate = YYP_GOTO( yypstate, yytoken );
        
            if( yypnextstate == $PARSER_NO_STATE ) {
                /* Perform error recovery */
                if( yyprecovering ){ /* looking for a few good tokens... */
                    if( yytoken == $LEXER_EOF_TOKEN )
                        goto fini;
                    break; /* drop token on the floor */
                }
		YY_ERROR( "Unexpected token: %s\n", yySymbolName[yytoken] );
                yyperrorcount++;
                do {
                    yypnextstate = YYP_GOTO( yypstate, $PARSER_ERROR_TOKEN );
                    if( yypnextstate != $PARSER_NO_STATE ) break;
                    if( yypstacktop == 0 ) goto fini;
                    yypstate = yypstack[--yypstacktop].state;
		    YY_ERROR( "Pop\n" );
                } while( 1 );
                //yypstatestack++; /* no meaningful yylsynattr */
                //yypstate = yypnextstate;
                yyprecovering = 1;
            }
        
	    if( yypnextstate <= $PARSER_LAST_SHIFT_STATE ) {
		/* Shift action (terminal): push state, attrs */
                YY_DEBUG( "Shift %s => %d\n", yySymbolName[yytoken], yypnextstate );
		if( yylstrlen ) {
		    YYP_PUSH_TEXT();
		}
		yypstack[yypstacktop].strpos = yylstrpos;
		yypstack[yypstacktop].line = yylline;
		yypstack[yypstacktop].col = yylcol;
		yypstack[yypstacktop].pos = yylpos;
		YYP_PUSH( yypstate, yylsynattr );
		yytoken = -1;
		yyprecovering = 0;
	    }
                
            while ( yypnextstate >= $PARSER_FIRST_REDUCE_STATE ) {
		/* Reduce action -  pop stack */
		if( yypReduceLength[yypnextstate] ) {
		    yypstacktop -= yypReduceLength[yypnextstate];
                    
                    /* set state to old state */
		    yypstate = yypstack[yypstacktop].state;
		}
		yypstrstacktop = yypstack[yypstacktop].strpos;
                    
		/* Execute action (if any) */
		switch( yypnextstate ) {
$PARSER_ACTION_CODE;
		default:
		    yypsynattr = yypstack[yypstacktop].attr;
		} 
                    
		/* Push the result of the reduction onto the stack */
		YYP_PUSH( yypstate, yypsynattr );
                
		if( yypnextstate == $PARSER_ACCEPT_STATE ) {
		    goto fini;
		}
		YY_DEBUG( "Shift %s => %d\n", yySymbolName[yypReduceToken[yypnextstate]],
			  YYP_GOTO( yypstate, yypReduceToken[yypnextstate] ) );
		yypnextstate = YYP_GOTO( yypstate,
					 yypReduceToken[yypnextstate] );
		assert( yypstate != $PARSER_NO_STATE );
	    }
	    yypstate = yypnextstate;
        } while( yytoken != -1 );

/* Assert: scanned token is consumed by this point */
    }

  fini:
    yyfRelease( &yyf );
    free( yypstack );
    return yyperrorcount;
}

int yyfInit( struct yy_parseable *yyf )
{
    yyf->yylfirst = 0;
    yyf->yylpos = 0;
    yyf->yylline = 1;
    yyf->yylcol = 1;
    if( yyf->buffer == NULL ) {
        yyf->buffer = (char *)malloc( YYL_DEFAULT_BUFFER_LEN );
	yyf->bufend = 0;
        yyf->buflen = YYL_DEFAULT_BUFFER_LEN;
        yyf->ismybuffer = 1;
	
    }
}

/**
 * Move yylfirst .. yylpos to the front of the buffer, and fill the remainder
 * of the buffer with data. If the buffer is full (or nearly so), extend the
 * buffer and fill the extended region as well.
 *
 * @return 1 if at least some data was read, 0 if we've reached EOF
 */
static int yyfFill( struct yy_parseable *yyf )
{
    int len;
    if( yyf->buffer != NULL ) {
	if( yyf->yylfirst > 0 ) {
	    memmove( yyf->buffer, yyf->buffer + yyf->yylfirst, yyf->bufend - yyf->yylfirst );
	    yyf->bufend -= yyf->yylfirst;
	    yyf->yylpos -= yyf->yylfirst;
	    yyf->yyllastpos -= yyf->yylfirst;
	    yyf->yylfirst = 0;
	}
	if( yyf->buflen - yyf->bufend < YYL_MIN_BUFFER_FILL ) {
	    char *tmp = realloc( yyf->buffer, yyf->buflen << 1 );
	    yyf->buffer = tmp;
	    yyf->buflen = yyf->buflen << 1;
	}
	len = read( yyf->fd, yyf->buffer + yyf->bufend, yyf->buflen - yyf->bufend );
	if( len > 0 ) {
	    yyf->bufend += len;
	    return 1;
	}
    }
    return 0;
}

static int yyfRelease( struct yy_parseable *yyf )
{
    if( yyf->ismybuffer && yyf->buffer ) {
        free(yyf->buffer);
        yyf->buffer = NULL;
    }
    if( yyf->fd != -1 ) {
        close( yyf->fd );
        yyf->fd = -1;
    }
    if( yyf->next ) {
        struct yy_parseable *tmp = yyf->next;
        *yyf = *yyf->next;
        free( tmp );
        return 1;
    }
    return 0;
}

static int yyfPushFile( struct yy_parseable *yyf, char *filename )
{
    struct yy_parseable *next = (struct yy_parseable *)malloc( sizeof(struct yy_parseable) );
    *next = *yyf;
    yyf->next = next;
    yyf->fd = open( filename, O_RDONLY );
}

static int yyError( char *s, ... )
{
    va_list ap;

    va_start( ap, s );
    vfprintf( stderr, s, ap );
    va_end( ap );
}

$END_CODE
