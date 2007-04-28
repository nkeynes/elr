/*
 * lex.h - input lexer header
 */

#ifndef elr_lex_H
#define elr_lex_H 1

#include "grammar.h"

/* Tokens */
typedef int token_t;
/*
typedef enum { IDENT, REGEXP, STRING, ACTION, TYPE, CODE,
               LEFTPREC, RIGHTPREC, NONASSOCPREC, START, CLASS, INCLUDE,
               ERROR, COLON, OR, PERIOD, LPAREN, RPAREN, SEMICOLON,
               ENDOFFILE } token_t;
*/

class Lexical {
  public:
    Position posn;
    string *str;
};

int lexOpenFile( const char *file );
void lexCloseFile( void );
token_t yylex( void );
extern Position yyposn;

#endif /* ! elr_lex_H */
