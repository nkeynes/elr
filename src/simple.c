

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
} yy_parseable;


/* Parser tables */
const int yypNextCheck[64][2] = { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,1}, {0,1}, {2,1}, {0,1}, {0,1}, {0,1}, {0,1}, {3,1}, {0,2}, {4,2}, {0,2}, {5,2}, {0,2}, {0,2}, {0,2}, {0,2}, {0,3}, {0,3}, {0,3}, {0,3}, {0,3}, {0,3}, {6,3}, {0,3}, {9,4}, {0,4}, {0,4}, {0,4}, {0,4}, {0,4}, {9,4}, {0,4}, {0,5}, {0,5}, {0,5}, {0,5}, {7,5}, {0,5}, {0,5}, {0,5}, {10,6}, {10,6}, {10,6}, {10,6}, {10,6}, {10,6}, {10,6}, {0,6}, {8,7}, {0,7}, {0,7}, {0,7}, {0,7}, {0,7}, {8,7}, {0,7} };
const int yypBase[8] = { 0, 8, 16, 24, 32, 40, 48, 56 };
const int yypDefault[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
const int yypReduceLength[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 3, 2, 2 };
const int yypReduceToken[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 8 };
const char *yySymbolName[9] = { "ERR", "error", "'b'", "\"[a-z]*[a-df-z]\"", "'e'", "WHITESPACE", "$", "grammar", "#START#" };

/* Lexer tables */
const int yylNextCheck[78][2] = { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,1}, {2,1}, {3,1}, {4,1}, {5,1}, {6,1}, {0,2}, {3,2}, {3,2}, {7,2}, {0,2}, {0,2}, {0,3}, {3,3}, {3,3}, {7,3}, {0,3}, {0,3}, {0,4}, {3,4}, {3,4}, {7,4}, {0,4}, {0,4}, {0,5}, {0,5}, {0,5}, {0,5}, {0,5}, {0,5}, {0,6}, {0,6}, {0,6}, {0,6}, {0,6}, {0,6}, {0,7}, {3,7}, {3,7}, {7,7}, {0,7}, {0,7}, {0,8}, {9,8}, {3,8}, {4,8}, {5,8}, {6,8}, {0,9}, {0,9}, {0,9}, {0,9}, {0,9}, {0,9}, {0,10}, {3,10}, {3,10}, {4,10}, {5,10}, {6,10}, {0,11}, {2,11}, {3,11}, {12,11}, {5,11}, {6,11}, {0,12}, {0,12}, {0,12}, {0,12}, {0,12}, {0,12} };
const int yylBase[13] = { 0, 6, 12, 18, 24, 30, 36, 42, 48, 54, 60, 66, 72 };
const int yylDefault[13] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
const int yylAccept[13] = { -1, -1, 2, 3, 4, 5, 6, -1, -1, 2, -1, -1, 4 };
const int yylStart[8] = { 1, 8, 10, 1, 1, 11, 1, 1 };
const int yylEquivClasses[257] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5 };


YYRETVAL yyparse_file( char *filename )
{
    struct yy_parseable yyf;
    
    yyf.filename = filename;
    yyf.fd = open( filename, O_RDONLY );
    if( yyf.fd == -1 ) return -1;
    yyf.buffer = NULL;
    return yyparse( &yyf );
}

YYRETVAL yyparse_snarf_file( char *filename )
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
        yyf.buffer = malloc( st.st_size );
        if( yyf.buffer ) {
            yyf.buflen = st.st_size;
            yyf.ismybuffer = 1;
            yyf.buflen = read( yyf.fd, yyf.buffer, yyf.buflen );
        }
    }
    return yyparse(&yyf);    
}
    

YYRETVAL yyparse_buffer( char *buf, int len )
{
    struct yy_parseable yyf;

    yyf.filename = NULL;
    yyf.buffer = buf;
    yyf.buflen = len;
    yyf.fd = -1;
    yyf.ismybuffer = 0;
    return yyparse( &yyf );
}

YYRETVAL yyparse_stream( int fd )
{
    struct yy_parseable yyf;
    
    yyf.fd = fd;
    yyf.buffer = NULL;
    yyf.filename = NULL;
    return yyparse( &yyf );
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
#define YYL_NEXT(yyf) if( yyf.buflen <= yyf.yylpos ){ yyf.yylchar = yylEquivClasses[256]; yyf.yylpos++; }else yyf.yylchar = (yylEquivClasses[yyf.buffer[yyf.yylpos++]])

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

YYRETVAL yyparse( yy_parseable yyf )
{
    int yytoken;
    int yypstate = 1;
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
    
    yypstatestack = malloc( YYP_DEFAULT_STACK_LEN * sizeof(int) );
    yypattrstack = malloc( YYP_DEFAULT_STACK_LEN * sizeof(yytype_t) );
    
    while( yypstate != 10 ) { /* While not accepting */
        /* Check stack size */ /* FIXME: Error checking */
        if( yypstacktop == yypstacklen ) {
            yypstacklen <<= 1;
            yypstatestack = realloc( yypstatestack, yypstacklen );
            yypattrstack = realloc( yypattrstack, yypstacklen );
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
            
            if( yylnextstate == 0 ) {
                if( yyllastaccept == 6 &&
                        yyfRelease( &yyf ) == 1 ) {
                    yylfirst = yyf.yylpos-1;
		    yyllastaccept = -1;
                    yylstate = yylStart[yypstate];
                } else switch( yyllastaccept ) {
                    case 5:
                                            goto accept;
;
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
        if( yytoken == 5 ) goto lexer;
        
	if( yyf.buffer )
	    printf( "Scanned: %s (%.*s)\n", yySymbolName[yytoken], yyf.yylpos-yylfirst-1, yyf.buffer+yylfirst );
	else
	    printf( "Scanned: %s\n", yySymbolName[yytoken] );
    
/************************ End Lexical Analysis Section ***********************/

        do {
            /* Lookup state table (comb-vector) */
            yypnextstate = YYP_GOTO( yypstate, yytoken );
        
            if( yypnextstate == 0 ) {
                /* Perform error recovery */
                if( yyprecovering ){ /* looking for a few good tokens... */
                    if( yytoken == 6 )
                        goto fini;
                    break; /* drop token on the floor */
                }
		YY_ERROR( "Unexpected token" );
                yyperrorcount++;
                do {
                    yypnextstate = YYP_GOTO( yypstate, 1 );
                    if( yypnextstate != 0 ) break;
                    if( yypstacktop == 0 ) goto fini;
                    yypstate = yypstatestack[--yypstacktop];
		    YY_ERROR( "Pop" );
                } while( 1 );
                //yypstatestack++; /* no meaningful yylsynattr */
                //yypstate = yypnextstate;
                yyprecovering = 1;
            }
        
            if ( yypnextstate >= 8 ) {
                if( yypnextstate <= 7 ) {
                    /* Shift action: push state, attrs */
                    YYP_PUSH( yypstate, yylsynattr );
                    yytoken = -1;
                    yyprecovering = 0;
                }
                
                do { /* Reduce */
                    /* pop stack */
                    yypstacktop -= yypReduceLength[yypnextstate];
                    
                    /* set state to old state */
                    yypstate = yypstatestack[yypstacktop];
                    
                    /* Execute action (if any) */
                    switch( yypnextstate ) {
                case 8:
                    printf("Reducing grammar -> \'b\' \"[a-z]*[a-df-z]\" \'e\'\n");
                    
                    break;
                case 9:
                    printf("Reducing grammar -> \'b\' error\n");
                    
                    break;
                case 10:
                    printf("Reducing #START# -> grammar $\n");
                    
                    break;
;
                        default:
                            yypsynattr = yypattrstack[yypstacktop];
                    } 
                    
                    /* Push the result of the reduction onto the stack */
                    YYP_PUSH( yypstate, yypsynattr );
                    
                    if( yypnextstate == 10 ) {
                        yytoken = -1;
                        break;
                    }
                    yypnextstate = YYP_GOTO( yypstate,
                                             yypReduceToken[yypnextstate] );
                    assert( yypstate != 0 );
                } while ( yypnextstate >= 8 );
                yypstate = yypnextstate;
            }
            else {
                /* Shift action: push state, attrs */
                YYP_PUSH( yypstate, yylsynattr );
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
        yyf->buffer = malloc( YYL_DEFAULT_BUFFER_LEN );
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
    struct yy_parseable *next = malloc( sizeof(struct yy_parseable) );
    *next = *yyf;
    yyf->next = next;
    yyf->fd = open( filename, O_RDONLY );
}

int yyError( char *s )
{
    fprintf( stderr, "Parse error: %s\n", s );
}


