/*
 * src/lex.l
 *
 * Lexer for OXPL
 *
 * Copyright (c) 2020 Viktor Garske
 */
%{
#include "parse.h"
#include "parse.tab.h"

#define DEBUG 0
#if DEBUG == 1
#include <stdio.h>
#define DBGMSG(x) printf("%s\n", x);
#else
#define DBGMSG(x) ;
#endif
%}

%option noyywrap nodefault yylineno

%x COMMENT

%%
 /* keywords */
"fn"				{ return FN; }
"class"				{
	yylval.kwtype = K_CLS;
	return CLS_SEL_TYPE;
}
"instance"			{
	yylval.kwtype = K_INST;
	return CLS_SEL_TYPE;
}

"if"				{ return IF; }
"else"				{ return ELSE; }
"while"				{ return WHILE; }
"for"				{ return FOR; }
"in"				{ return IN; }
"return"			{ return RETURN; }
"break"				{ return BREAK; }
"continue"			{ return CONTINUE; }
"var"				{ return VAR; }

 /* == OPERATORS == */
 /* single char operators */
"(" |
")" |
"{" |
"}" |
"<" |				/* Also used as relation op */
">" |				/* Also used as relation op */
"=" |
":" |
"." |
";" |
"," |
"?"				{ return yytext[0]; }

 /* arrows */
"->"				{ return RARR; }
"<-"				{ return LARR; }
"::"				{ return SCOPE_OP; }

 /* classic binary ops */
"||"				{ return LOG_OR_OP; }
"&&"				{ return LOG_AND_OP; }
"|"				{ return BIT_OR_OP; }
"&"				{ return BIT_AND_OP; }
"^"				{ return BIT_XOR_OP; }

"==" |
"!="				{ return EQ_OP; }

"<=" |
">="				{ return REL_EQ_OP; }

"<<" |
">>"				{ return SHIFT_OP; }

"*" |
"/" |
"%"				{ return MUL_OP; }

 /* binary and unary ops */
"+" |
"-"				{ return ADD_OP; }

 /* unary ops */
"!"				{ return NEG_OP; }
"++"				{ return INC_OP; }
"--"				{ return DEC_OP; }

 /* identifiers */
[_a-zA-Z][_a-zA-Z0-9]*		{ DBGMSG("IDENT"); return IDENTIFIER; }

 /* integers */
[0-9]+				{ DBGMSG("INTCO"); return INT_CONST; }

 /* floats */
[0-9]+"."[0-9]+			{ DBGMSG("FLOCO"); return FLOAT_CONST; }

 /* strings */
\"([^"\\\n])*\"			{ return STRING_CONST; }

 /* comments */
"/*"				{ BEGIN(COMMENT); }
<COMMENT>{
	"*/"			{ BEGIN(INITIAL); }
	([^*]|\n)+|.
	<<EOF>>			{
		printf("[L] Error: unclosed comment at line %d\n", yylineno);
		return 1;
	}
}

 /* ignore whitespaces */
[ \t\n]+
 /* catch everything else which results in an error */
.				{
	printf("[L] Error: unknown symbol \"%s\" at line %d\n",
		yytext, yylineno);
	return 1;
}
%%
