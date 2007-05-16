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
%{
#include <string>
#include <vector>
#include "grammar.h"
#include "lex.h"

#define YYERROR_VERBOSE 1

extern Grammar grammar;
assoc_t assoc;
Nonterminal *lhs; 
int prec = 0;

int yyerror( char *s );
 %}

%union {
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
}

%token IDENT REGEXP STRING ACTION TYPE CODE ERROR_IDENT
%token LEFTPREC RIGHTPREC NONASSOCPREC START CLASS INCLUDE
%token ERROR COLON OR PERIOD LPAREN RPAREN SEMICOLON EQUALS
%token CASE EXPECT DISAMBIGUATION NUMBER ON OFF

%type <scan> IDENT REGEXP STRING ACTION TYPE CODE ERROR_IDENT Type String NUMBER
%type <rules> Rhs
%type <nt> Lhs
%type <rule> Symbols Rule
%type <sym> Symbol
%type <term> Lexical
%type <action> Action
%type <boolean> Bool

%%

Spec : Decls                 { grammar.finalize(); }
Decls : Decls Decl |  ;
Decl : Directive | Rules ;
Directive : Lprec Precs SEMICOLON
          | Rprec Precs SEMICOLON
          | Nprec Precs SEMICOLON
          | START IDENT      { grammar.startSymbol = (Nonterminal *)
                                   grammar.lookupIdent( $2.str, $2.posn ); }
          | CLASS String     { grammar.setClass( $2.str ); }
          | CODE             { grammar.addCode( $1.str, $1.posn ); }
          | EXPECT NUMBER    { grammar.expectedParserConflicts = $2.number; }
          | CASE Bool        { grammar.caseSensitive = $2; }
          | DISAMBIGUATION Bool { grammar.autoLexDisambiguation = $2; }
Lprec : LEFTPREC             { assoc = assoc_left; prec++; }
Rprec : RIGHTPREC            { assoc = assoc_right; prec++; }
Nprec : NONASSOCPREC         { assoc = assoc_none; prec++; }

Rules : Type Lhs COLON Rhs SEMICOLON
        { $2->setType( $1.str );
          if( grammar.startSymbol == NULL ) grammar.startSymbol = $2; }
      | Type IDENT EQUALS Lexical Action SEMICOLON
        { grammar.makeLexicalSymbol( $1.str, $2.str, $4, $5, $2.posn ); }
      ;
Lhs : IDENT            { $$ = lhs = grammar.makeNonterminal( $1.str, $1.posn ); }
Type : TYPE                  { $$ = $1; }
     |                       { $$.str = NULL; $$.posn = NO_POSITION }
     ; 
Rhs : Rhs OR Rule            { lhs->rules.push_back($3); }
    | Rule                   { lhs->rules.push_back($1); }
    ;
Rule : Action Symbols        { $$ = $2; $$->preAction = $1; $$->fixReduce();
                               $$->nonterm = lhs; }
Symbols : Symbols Symbol Action { $$ = $1;
                                  $$->syms.push_back(RuleSymbol($2.sym, $3, $2.posn)); }
        |                    { $$ = new Rule(++grammar.numRules); }
        ; 
Symbol : IDENT           { $$.sym = grammar.lookupIdent( $1.str, $1.posn );
                           $$.posn = $1.posn; }
       | REGEXP          { $$.sym = grammar.lookupRegexpTerm($1.str, $1.posn);
                           $$.posn = $1.posn; }
       | STRING          { $$.sym = grammar.lookupStringTerm($1.str, $1.posn);
                           $$.posn = $1.posn; }
       | ERROR_IDENT     { $$.sym = grammar.errorTerm; $$.posn = $1.posn; }
       ;
Precs : Precs Symbol     { if( $2.sym->prec != 0 ) 
                                   yyerror("Precedence specified twice"); 
                           else {
                               $2.sym->prec = prec;
                               $2.sym->assoc = assoc;
                           }
                         }
        | ;
Bool: ON              { $$ = true; }
    | OFF             { $$ = false; }
String : IDENT
       | REGEXP
       | STRING
       ;
Lexical: REGEXP       { $$ = Terminal::fromRegexp($1.str, $1.posn, grammar.caseSensitive); }
       | STRING       { $$ = Terminal::fromString($1.str, $1.posn, grammar.caseSensitive); }
       ;
Action : Action ACTION       { $$ = $1;
                               $$->push_back( ActionItem( $2.str, $2.posn )); }
       |                     { $$ = new Action(); }
       ;

%%
int yyerror( char *s )
{
    fprintf( stderr, "%s\n", s );
    return 0;
}
