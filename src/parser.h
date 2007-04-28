/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

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
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

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
     EQUALS = 278
   };
#endif
/* Tokens.  */
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




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 20 "parser.y"
{
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
}
/* Line 1489 of yacc.c.  */
#line 109 "parser.hh"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

