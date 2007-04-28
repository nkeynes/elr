$START_CODE

#include <stdlib.h>
#include <stdio.h>
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
#define YYL_DEFAULT_BUFFER_LEN 1024

#define YYL_MAX_SNARF_LEN 4096*1024 /* Snarf up to a 4Mb file */

#ifndef YY_ERROR
#define YY_ERROR(s) yyError(s)
#endif



typedef struct {
    char *source;
    char *str;
    int line, col, length;
} yyscan_t;

typedef union {
$ATTRIBUTES
    yyscan_t yyscan;
} yytype_t;

typedef struct yy_parseable {
    char *filename;
    char *buffer;
    int buflen;
    int bufpos;
    int fd;
    int ismybuffer; /* Whether to free buffer on completion */
    YY_CHAR yylchar;
    int yylpos;
    struct yy_parseable *next; /* For file stacking */
};

/* Function prototypes */
YYRETVAL $PARSER_NAME( struct yy_parseable yyf );
int yyfInit( struct yy_parseable *yyf );
int yyfRelease( struct yy_parseable *yyf );
int yyError( char *s );


/* Parser tables */
const int yypNextCheck[$PARSER_NEXTCHECK_LEN][2] = { $PARSER_NEXTCHECK_ARRAY };
const int yypBase[$PARSER_BASE_LEN] = { $PARSER_BASE_ARRAY };
const int yypDefault[$PARSER_BASE_LEN] = { $PARSER_DEFAULT_ARRAY };
const int yypReduceLength[$PARSER_NUM_STATES] = { $PARSER_REDUCE_LEN_ARRAY };
const int yypReduceToken[$PARSER_NUM_STATES] = { $PARSER_REDUCE_TOK_ARRAY };
const char *yySymbolName[$NUM_SYMBOLS] = { "ERR", $SYMBOL_NAME_ARRAY };

/* Lexer tables */
const int yylNextCheck[$LEXER_NEXTCHECK_LEN][2] = { $LEXER_NEXTCHECK_ARRAY };
const int yylBase[$LEXER_BASE_LEN] = { $LEXER_BASE_ARRAY };
const int yylDefault[$LEXER_BASE_LEN] = { $LEXER_DEFAULT_ARRAY };
const int yylAccept[$LEXER_NUM_STATES] = { $LEXER_ACCEPT_ARRAY };
const int yylStart[$PARSER_NUM_REAL_STATES] = { $LEXER_START_STATE_ARRAY };
const int yylEquivClasses[$LEXER_NUM_EQUIV] = { $LEXER_EQUIV_ARRAY };


YYRETVAL ${PARSER_NAME}_file( char *filename )
{
    struct yy_parseable yyf;
    
    yyf.filename = filename;
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
    yyf.fd = open( filename, O_RDONLY );
    if( yyf.fd == -1 ) return -1;

    if( fstat( yyf.fd, &st ) == -1 ) {
        fprintf( stderr, "Unable to stat file (%s)\n", strerror(errno) );
        return -1;
    }
    if( S_ISREG(st.st_mode) && st.st_size < YYL_MAX_SNARF_LEN ) {
        yyf.buffer = (char *)malloc( st.st_size );
        if( yyf.buffer ) {
            yyf.buflen = st.st_size;
            yyf.ismybuffer = 1;
            yyf.buflen = read( yyf.fd, yyf.buffer, yyf.buflen );
        }
    }
    return $PARSER_NAME(yyf);    
}
    

YYRETVAL ${PARSER_NAME}_buffer( char *buf, int len )
{
    struct yy_parseable yyf;

    yyf.filename = NULL;
    yyf.buffer = buf;
    yyf.buflen = len;
    yyf.fd = -1;
    yyf.ismybuffer = 0;
    return $PARSER_NAME( yyf );
}

YYRETVAL ${PARSER_NAME}_stream( int fd )
{
    struct yy_parseable yyf;
    
    yyf.fd = fd;
    yyf.buffer = NULL;
    yyf.filename = NULL;
    return $PARSER_NAME( yyf );
}


/* Parser macros */
#define YYP_PUSH(state,attr) { yypstatestack[yypstacktop] = state; \
                            yypattrstack[yypstacktop] = attr; yypstacktop++; }

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
#define YYL_NEXT(yyf) if( yyf.buflen <= yyf.yylpos ){ yyf.yylchar = yylEquivClasses[$LEXER_EOF_CHAR]; yyf.yylpos++; }else yyf.yylchar = (yylEquivClasses[yyf.buffer[yyf.yylpos++]])

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

YYRETVAL $PARSER_NAME( struct yy_parseable yyf )
{
    int yytoken;
    int yypstate = $PARSER_START_STATE;
    int yypnextstate, yypnextidx, yyprecovering = 0;
    int yyperrorcount = 0;

    /* Lexer variables */
    int yylstate, yylnextstate, yylfirst;
    int yyllastpos, yyllastaccept, yyllastchar;
    
    int *yypstatestack;
    yytype_t *yypattrstack;
    yytype_t yypsynattr, yylsynattr;
    int yypstacklen = YYP_DEFAULT_STACK_LEN;
    int yypstacktop = 0;

    yyfInit( &yyf );
    YYL_NEXT(yyf);
    
    yypstatestack = (int *)malloc( YYP_DEFAULT_STACK_LEN * sizeof(int) );
    yypattrstack = (yytype_t *)malloc( YYP_DEFAULT_STACK_LEN * sizeof(yytype_t) );
    
    while( yypstate != $PARSER_ACCEPT_STATE ) { /* While not accepting */
        /* Check stack size */ /* FIXME: Error checking */
        if( yypstacktop == yypstacklen ) {
            yypstacklen <<= 1;
            yypstatestack = (int *)realloc( yypstatestack, yypstacklen );
            yypattrstack = (yytype_t *)realloc( yypattrstack, yypstacklen );
        }
        
        /* Read a token */
        
/************************** Lexical Analysis Section *************************/
      lexer:
        yylstate = yylStart[yypstate];
        yylfirst = yyf.yylpos-1;
        yyllastaccept = -1;

        while( 1 ) {

            /* Lookup transition table */
            yylnextstate = YYL_GOTO(yylstate, yyf.yylchar);
            
            if( yylnextstate == $LEXER_NO_STATE ) {
                if( yyllastaccept == $LEXER_EOF_TOKEN &&
                        yyfRelease( &yyf ) == 1 ) {
                    yylfirst = yyf.yylpos-1;
		    yyllastaccept = -1;
                    yylstate = yylStart[yypstate];
                } else switch( yyllastaccept ) {
$LEXER_ACTION_CODE;
                    default:
                        /* Have a valid accept state */
                        goto accept;
                    case -1:
                        /* We haven't encountered a valid string - ERROR
                         * Try to recover by dropping the first character and
                         * starting again
                         */
                        YY_ERROR( "Unscannable string" );
                        if( yyf.yylpos == yylfirst+1 ) {
                            YYL_NEXT(yyf);
                            yylfirst = yyf.yylpos-1;
                        } else {
                            yyf.yylpos = ++yylfirst+1;
                            yyf.yylchar = yylEquivClasses[yyf.buffer[yylfirst]];
                        }
                        yylstate = yylStart[yypstate];
                        break;
                        
                }
            } else {
                YYL_NEXT(yyf);
                yylstate = yylnextstate;
                if( yylAccept[yylstate] != -1 ) {
                    /* Potentially accepting state, save for backtracking */
                    yyllastpos = yyf.yylpos;
                    yyllastaccept = yylAccept[yylstate];
		    yyllastchar = yyf.yylchar;
                }
            }
        }
      accept:
        yyf.yylchar = yyllastchar;
        yyf.yylpos = yyllastpos;
        yytoken = yyllastaccept;

#ifdef DEBUG
	if( yyf.buffer )
	    printf( "Scanned: %s (%.*s)\n", yySymbolName[yytoken], yyf.yylpos-yylfirst-1, yyf.buffer+yylfirst );
	else
	    printf( "Scanned: %s\n", yySymbolName[yytoken] );
#endif /* DEBUG */

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
		YY_ERROR( "Unexpected token" );
                yyperrorcount++;
                do {
                    yypnextstate = YYP_GOTO( yypstate, $PARSER_ERROR_TOKEN );
                    if( yypnextstate != $PARSER_NO_STATE ) break;
                    if( yypstacktop == 0 ) goto fini;
                    yypstate = yypstatestack[--yypstacktop];
		    YY_ERROR( "Pop" );
                } while( 1 );
                //yypstatestack++; /* no meaningful yylsynattr */
                //yypstate = yypnextstate;
                yyprecovering = 1;
            }
        
            if ( yypnextstate >= $PARSER_FIRST_REDUCE_STATE ) {
                if( yypnextstate <= $PARSER_LAST_SHIFT_STATE ) {
                    /* Shift action: push state, attrs */
                    YYP_PUSH( yypstate, yylsynattr );
                    yytoken = -1;
                    yyprecovering = 0;
                }
                
                do { /* Reduce */
                    /* pop stack */
                    if( yypReduceLength[yypnextstate] ) {
                        yypstacktop -= yypReduceLength[yypnextstate];
                    
                    /* set state to old state */
                        yypstate = yypstatestack[yypstacktop];
                    }
                    
                    /* Execute action (if any) */
                    switch( yypnextstate ) {
$PARSER_ACTION_CODE;
                        default:
                            yypsynattr = yypattrstack[yypstacktop];
                    } 
                    
                    /* Push the result of the reduction onto the stack */
                    YYP_PUSH( yypstate, yypsynattr );
                    
                    if( yypnextstate == $PARSER_ACCEPT_STATE ) {
                        yytoken = -1;
                        break;
                    }
#ifdef DEBUG
                    printf( "Shift %s => ", yySymbolName[yypReduceToken[yypnextstate]] );
#endif /* DEBUG */
                    yypnextstate = YYP_GOTO( yypstate,
                                             yypReduceToken[yypnextstate] );
#ifdef DEBUG
                    printf( "%d\n", yypnextstate );
#endif /* DEBUG */
                    assert( yypstate != $PARSER_NO_STATE );
                } while ( yypnextstate >= $PARSER_FIRST_REDUCE_STATE );
                yypstate = yypnextstate;
            }
            else {
                /* Shift action: push state, attrs */
                YYP_PUSH( yypstate, yylsynattr );
#ifdef DEBUG
                printf( "Shift %s => %d\n", yySymbolName[yytoken], yypnextstate );
#endif /* DEBUG */
                yypstate = yypnextstate;
                yyprecovering = 0;
                break;
            }
        } while( yytoken != -1 );

/* Assert: scanned token is consumed by this point */
    }

  fini:
    return yyperrorcount;
}

int yyfInit( struct yy_parseable *yyf )
{
    yyf->yylpos = 0;
    if( yyf->buffer == NULL ) {
        yyf->buffer = (char *)malloc( YYL_DEFAULT_BUFFER_LEN );
        yyf->buflen = YYL_DEFAULT_BUFFER_LEN;
        yyf->ismybuffer = 1;
    }
}

int yyfRelease( struct yy_parseable *yyf )
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

int yyfPushFile( struct yy_parseable *yyf, char *filename )
{
    struct yy_parseable *next = (struct yy_parseable *)malloc( sizeof(struct yy_parseable) );
    *next = *yyf;
    yyf->next = next;
    yyf->fd = open( filename, O_RDONLY );
}

int yyError( char *s )
{
    fprintf( stderr, "Parse error: %s\n", s );
}

$END_CODE
