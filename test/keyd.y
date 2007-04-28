%{
#include <string.h>
#include "keyd.h"
#include "actions.h"
%}

%union {
  char *str;
  unsigned int modifier;
  int num;
  action_t action;
  keyent_t key;
  keymode_t mode;
  struct mix_struct { int op, l, r; } mixer;
}

%token KW_KEYCODE KW_UP KW_DOWN KW_SHIFT KW_ALT KW_CONTROL
%token KW_SHIFT_LEFT KW_SHIFT_RIGHT KW_ALT_LEFT KW_ALT_RIGHT
%token KW_CONTROL_LEFT KW_CONTROL_RIGHT
%token KW_MODE KW_EXEC KW_SUID KW_MIXER KW_CONSOLE KW_CD
%token KW_MUTE KW_NEW KW_MARK KW_RESTORE
%token KW_STOP KW_PAUSE KW_NEXT KW_PREV KW_EJECT KW_STOPEJECT KW_LOCK KW_UNLOCK
%token KW_START KW_ENTRY KW_EXIT
%token TOK_PLUS TOK_MINUS TOK_EQUALS TOK_LBRACE TOK_RBRACE
%token TOK_NEWLINE
%token <str> TOK_IDENT TOK_CMD
%token <num> TOK_NUMBER TOK_SIGNNUMBER

%type <modifier> modifier modifiers
%type <action> command modecmd execcmd suidcmd mixercmd consolecmd cdcmd
%type <action> consoleop
%type <str> mixnam mode device
%type <num> mixrec cdop
%type <mixer> mixset
%type <key> key
%%

keyd: block

block:	maybenls statements maybenls
	|
	;
	
statements: statement
	| statements newlines statement
	;
newlines: TOK_NEWLINE
	| newlines TOK_NEWLINE
	;
maybenls: newlines
	|
	;

statement: modeblock
	| keyline
	| enteract
	| exitact
	| startup
	| error TOK_NEWLINE
	;

modeblock: modestart block TOK_RBRACE	{ closeModeBlock(); }
	;

modestart: mode TOK_LBRACE	{ openModeBlock( addMode($1) ); }
	;

mode:	KW_MODE TOK_IDENT	{ $$ = $2 }
	;

enteract: KW_ENTRY TOK_EQUALS command	{ setEntryAction( $3 ); }
	;
exitact: KW_EXIT TOK_EQUALS command	{ setExitAction( $3 ); }
	;
startup: KW_START TOK_EQUALS command	{ setStartupAction( $3 ); }
	;

keyline: modifiers key command	{ $2->mod = $1; addKey($2, $3); }
	| key command		{ $1->mod = 0; addKey($1, $2); }

key:	KW_KEYCODE TOK_NUMBER TOK_EQUALS	{ $$ = makeKeyent($2); }
	;
modifiers: modifier
	| modifiers modifier	{ $$ = $1 | $2 }
	;
modifier: KW_SHIFT		{ $$ = MOD_SHIFT }
	| KW_ALT		{ $$ = MOD_ALT }
	| KW_CONTROL		{ $$ = MOD_CTRL }
	| KW_SHIFT_LEFT		{ $$ = MOD_SHIFTL }
	| KW_SHIFT_RIGHT	{ $$ = MOD_SHIFTR }
	| KW_ALT_LEFT		{ $$ = MOD_ALTL }
	| KW_ALT_RIGHT		{ $$ = MOD_ALTR }
	| KW_CONTROL_LEFT	{ $$ = MOD_CTRLL }
	| KW_CONTROL_RIGHT	{ $$ = MOD_CTRLR }
	| KW_UP			{ $$ = MOD_UP }
	| KW_DOWN		{ $$ = MOD_DOWN }
	;

command: modecmd | execcmd | suidcmd | mixercmd | cdcmd | consolecmd
	|			{ $$ = makeNullAction(); }
	;

modecmd: mode			{ $$ = makeModeAction($1); }
	;
execcmd: KW_EXEC TOK_CMD	{ $$ = makeExecAction(-1,-1,$2); }
	;
suidcmd: KW_SUID TOK_NUMBER TOK_NUMBER KW_EXEC TOK_CMD
				{ $$ = makeExecAction($2,$3,$5); }
	;
	
mixercmd: KW_MIXER mixrec mixnam mixset	{ $$ = makeMixerAction($2,$3,$4.op,$4.l,$4.r); }
	;
mixrec: TOK_PLUS	{ $$ = 1; }
	| TOK_MINUS	{ $$ = -1; }
	|		{ $$ = 0; }
	;
mixnam: TOK_IDENT
	|		{ $$ = strdup("vol"); }
	;
mixset: TOK_NUMBER TOK_NUMBER 		{ $$.l = $1; $$.r = $2; $$.op = MIX_SET; }
	| TOK_SIGNNUMBER TOK_SIGNNUMBER	{ $$.l = $1; $$.r = $2; $$.op = MIX_DELTA; }
	| TOK_NUMBER			{ $$.l = $$.r = $1; $$.op = MIX_SET; }
	| TOK_SIGNNUMBER		{ $$.l = $$.r = $1; $$.op = MIX_DELTA; }
	| KW_MUTE			{ $$.l = $$.r = -1; $$.op = MIX_MUTE; }
	|				{ $$.op = MIX_RECONLY; }
	;

cdcmd:	KW_CD device cdop		{ $$ = makeCDAction( $2, $3, 0, 0 ); }
	;
	
device:	TOK_IDENT
	|				{ $$ = NULL; }
	;
	
cdop:	KW_STOP				{ $$ = CD_STOP; }
	| KW_PAUSE			{ $$ = CD_PAUSE; }
	| KW_NEXT			{ $$ = CD_NEXT; }
	| KW_PREV			{ $$ = CD_PREV; }
	| KW_EJECT			{ $$ = CD_EJECT; }
	| KW_STOPEJECT			{ $$ = CD_STOPEJECT; }
	| KW_LOCK			{ $$ = CD_LOCK; }
	| KW_UNLOCK			{ $$ = CD_UNLOCK; }
	;


consolecmd: KW_CONSOLE consoleop	{ $$ = $2; }
	;
consoleop: KW_NEW			{ $$ = makeConsoleAction( CON_NEW, 0, 0 ); }
	| KW_NEXT 			{ $$ = makeConsoleAction( CON_NEXT, 0, 0 ); }
	| KW_PREV			{ $$ = makeConsoleAction( CON_PREV, 0, 0 ); }
	| KW_MARK TOK_NUMBER		{ $$ = makeConsoleAction( CON_MARK, -1, $2 ); }
	| KW_MARK TOK_NUMBER TOK_NUMBER	{ $$ = makeConsoleAction( CON_MARK, $3, $2 ); }
	| KW_RESTORE TOK_NUMBER		{ $$ = makeConsoleAction( CON_REST, -1, $2 ); }
	| TOK_NUMBER			{ $$ = makeConsoleAction( CON_SWITCH, $1, 0); }
	

%%

int yyerror( char *s )
{
	log( LOG_ERR, "Parser: %s\n", s );
	return 0;
}

keyent_t makeKeyent(int keycode)
{
	keyent_t key;
	
	key = s_malloc(sizeof(keyent_t));
	key->mod = 0;
	key->key = keycode;
	key->orig = 0;
	return key;
}

