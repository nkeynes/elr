/*
 * bootstrap parser
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
}

%token IDENT REGEXP STRING ACTION TYPE CODE ERROR_IDENT
%token LEFTPREC RIGHTPREC NONASSOCPREC START CLASS INCLUDE
%token ERROR COLON OR PERIOD LPAREN RPAREN SEMICOLON EQUALS

%type <scan> IDENT REGEXP STRING ACTION TYPE CODE ERROR_IDENT Type String
%type <rules> Rhs
%type <nt> Lhs
%type <rule> Symbols Rule
%type <sym> Symbol
%type <term> Lexical
%type <action> Action

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
Lprec : LEFTPREC             { assoc = left; prec++; }
Rprec : RIGHTPREC            { assoc = right; prec++; }
Nprec : NONASSOCPREC         { assoc = none; prec++; }

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
String : IDENT
       | REGEXP
       | STRING
       ;
Lexical: REGEXP       { $$ = Terminal::fromRegexp($1.str, $1.posn); }
       | STRING       { $$ = Terminal::fromString($1.str, $1.posn); }
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
