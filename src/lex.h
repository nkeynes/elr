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
    int number;
    string *str;
};

int lexOpenFile( const char *file );
void lexCloseFile( void );
token_t yylex( void );
extern Position yyposn;

#endif /* ! elr_lex_H */
