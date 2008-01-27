/* A Bison parser, made by GNU Bison 1.875a.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     IDENT = 258,
     REGEXP = 259,
     STRING = 260,
     ACTION = 261,
     TYPE = 262,
     CODE = 263,
     ERROR_IDENT = 264,
     LEFTPREC = 265,
     RIGHTPREC = 266,
     NONASSOCPREC = 267,
     START = 268,
     CLASS = 269,
     INCLUDE = 270,
     ERROR = 271,
     COLON = 272,
     OR = 273,
     PERIOD = 274,
     LPAREN = 275,
     RPAREN = 276,
     SEMICOLON = 277,
     EQUALS = 278,
     CASE = 279,
     EXPECT = 280,
     DISAMBIGUATION = 281,
     NUMBER = 282,
     ON = 283,
     OFF = 284,
     NAME = 285,
     LANGUAGE = 286,
     INTERFACE = 287,
     IMPLEMENTATION = 288,
     CONTEXT = 289
   };
#endif
#define IDENT 258
#define REGEXP 259
#define STRING 260
#define ACTION 261
#define TYPE 262
#define CODE 263
#define ERROR_IDENT 264
#define LEFTPREC 265
#define RIGHTPREC 266
#define NONASSOCPREC 267
#define START 268
#define CLASS 269
#define INCLUDE 270
#define ERROR 271
#define COLON 272
#define OR 273
#define PERIOD 274
#define LPAREN 275
#define RPAREN 276
#define SEMICOLON 277
#define EQUALS 278
#define CASE 279
#define EXPECT 280
#define DISAMBIGUATION 281
#define NUMBER 282
#define ON 283
#define OFF 284
#define NAME 285
#define LANGUAGE 286
#define INTERFACE 287
#define IMPLEMENTATION 288
#define CONTEXT 289




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 32 "parser.y"
typedef union YYSTYPE {
    Lexical scan;
    Action *action;
    struct {
        Symbol *sym;
        Position posn;
    } sym;
    Nonterminal *nt;
    Terminal *term;
    RuleSymbols *ruleSyms;
    RulePs *rules;
    Rule *rule;
    bool boolean;
} YYSTYPE;
/* Line 1196 of yacc.c.  */
#line 120 "parser.hh"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



