%{
#include <stdlib.h>
#include <string.h>
#include "keyd.h"
#include "parser.h"

#define yywrap() (1)
#define YY_NO_UNPUT
%}

%x cmd mode
HEX [0-9a-fA-F]
IDENT [A-Za-z0-9]
%%

"keycode"	return KW_KEYCODE;
"up"		return KW_UP;
"down"		return KW_DOWN;
"ctrl" |
"control"	return KW_CONTROL;
"alt"           return KW_ALT;  /* either alt key */
"shift"		return KW_SHIFT;
"altl" |
"lalt"		return KW_ALT_LEFT;
"altr" |
"ralt" |
"altgr"		return KW_ALT_RIGHT;
"ctrll" |
"controll" |
"lctrl" |
"lcontrol"	return KW_CONTROL_LEFT;
"ctrlr" |
"controlr" |
"rctrl" |
"rcontrol"	return KW_CONTROL_RIGHT;
"shiftl" |
"lshift"	return KW_SHIFT_LEFT;
"shiftr" |
"rshift"	return KW_SHIFT_RIGHT;

"entry"		return KW_ENTRY;
"exit"		return KW_EXIT;
"start"		return KW_START;

"mode"		{ BEGIN(mode); return KW_MODE; }
"exec"		BEGIN(cmd); return KW_EXEC;
"suid"		return KW_SUID;

"mixer"         return KW_MIXER;
"mute"          return KW_MUTE;

"con"		return KW_CONSOLE;
"console"	return KW_CONSOLE;
"mark"		return KW_MARK;
"restore"	return KW_RESTORE;
"new"		return KW_NEW;

"cd"		return KW_CD;
"stop"		return KW_STOP;
"pause"		return KW_PAUSE;
"next"		return KW_NEXT;
"prev" |
"back"		return KW_PREV;
"eject"		return KW_EJECT;
"lock"		return KW_LOCK;
"unlock"	return KW_UNLOCK;
"stopeject"	return KW_STOPEJECT;

"+"		return TOK_PLUS;
"-"		return TOK_MINUS;
"="		return TOK_EQUALS;
"{"		return TOK_LBRACE;
"}"		return TOK_RBRACE;
"\n"		return TOK_NEWLINE;

[0-9]+ |
"0x"{HEX}+	{ yylval.num = strtol(yytext,NULL,0);
		  return TOK_NUMBER;
		}

[+-][0-9]+ |
[+-]"0x"{HEX}+	{ yylval.num = strtol(yytext,NULL,0);
		  return TOK_SIGNNUMBER;
		}

{IDENT}+	{ yylval.str = strdup(yytext);
		  return TOK_IDENT;  /* anything else, pretty much */
		}
[#;][^\n]*	/* Full line comment */
[\r\t ]+        /* whitespace */

<cmd>[^\n]*	{ BEGIN(0); 
		  yylval.str = strdup(yytext); 
		  return TOK_CMD; /* eat up rest of line */
		}
<mode>[\r\t\n ]+ /* whitespace */
<mode>{IDENT}+	{ BEGIN(0);
		  yylval.str = strdup(yytext);
		  return TOK_IDENT;
		}
