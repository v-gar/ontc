/*
 * lib/oxpl/parse.y
 *
 * Parser for OXPL
 *
 * Copyright (c) 2020 Viktor Garske
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
%{
  #include <stdio.h>
  #include <string.h>

  #include "parse.h"
  #include "ast.h"

  int yylex(void);
  void yyerror(char const *);
  extern int yylineno;

  struct ast_node *parse_ast;

  #define OPERSCPY(op, len, dest) \
	if (0 == (dest = malloc(len + 1))) { \
		fprintf(stderr, "[P] OOM!\n"); \
	} else { \
		strcpy(dest, op); \
	}

#define DEBUG 1
#if DEBUG == 1
#include <stdio.h>
#define DBGMSG(x) printf("%s\n", x);
#else
#define DBGMSG(x) ;
#endif
%}

%union {
	/* keyword type */
	enum keywords kwtype;

	struct ast_node *node;

	int intval;
	float fpnval;
	char *strval;

	char oper; /* operator */
	char *oper_s;
};

/* Basic tokens */
%token <strval> IDENTIFIER
%token <intval> INT_CONST
%token <fpnval> FLOAT_CONST
%token <strval> STRING_CONST

%token LOG_OR_OP
%token LOG_AND_OP
%token BIT_OR_OP
%token BIT_AND_OP
%token BIT_XOR_OP

%token <oper_s> EQ_OP
%token <oper_s> REL_EQ_OP
%token <oper_s> SHIFT_OP

%token <oper> ADD_OP
%token <oper> MUL_OP
%token NEG_OP

%token INC_OP
%token DEC_OP

/* Arrows */
%token RARR
%token LARR

/* Scope operator (::) */
%token SCOPE_OP

/* Keywords */
%token IF
%token ELSE
%token WHILE
%token FOR
%token IN
%token RETURN
%token BREAK
%token CONTINUE
%token VAR

%token <kwtype> CLS_SEL_TYPE
%token <kwtype> FN

%type <node> 	const scope
		primary_expr postfix_expr unary_expr
		mul_expr add_expr shift_expr
		rel_expr eq_expr
		and_expr exclusive_or_expr inclusive_or_expr
		logical_and_expr logical_or_expr
		conditional_expr assign_expr expr expr_stmt
		imper_stmt imper_block imper_block_stmt imper_decl
		fol_stmt triple_stmt triple_subj triple_obj
		slct_stmt slct_conditional jump_stmt iter_stmt
		arglist arglist_args sig_var
		fact decl
		sig func callarglist addr
		stmt tunit start

%type <oper_s>	rel_op unary_op
%%
start:
  %empty { parse_ast = NULL; }
| tunit { parse_ast = ast_new_transunit($1); }
;

tunit:
  stmt
| tunit stmt { $$ = ast_add_seq($1, $2); }

stmt:
  decl
| func
;

/* Imperative block statement */
imper_stmt:
  imper_block
| expr_stmt
| iter_stmt
| slct_stmt
| jump_stmt
;

/* Expression statement */
expr_stmt:
  expr ';' { $$ = $1; }
| ';' { $$ = NULL; }
;

/* Conditional / select statements */
slct_stmt:
  IF slct_conditional imper_block { $$ = ast_new_cond($2, $3, NULL); }
| IF slct_conditional imper_block ELSE imper_block {
	$$ = ast_new_cond($2, $3, $5);
}
| IF slct_conditional imper_block ELSE slct_stmt {
	$$ = ast_new_cond($2, $3, $5);
}
;

slct_conditional:
  conditional_expr
;

/* Loops */
iter_stmt:
  WHILE conditional_expr imper_block { $$ = ast_new_while($2, $3); }
| FOR IDENTIFIER IN expr imper_block {
	$$ = ast_new_for(ast_new_str($2), $4, $5);
}
;

/* Jumps */
jump_stmt:
  RETURN expr ';' { $$ = ast_new_ret($2); }
| RETURN ';' { $$ = ast_new_ret(NULL); }
| CONTINUE ';' { $$ = ast_new_jump('c'); }
| BREAK ';' { $$ = ast_new_jump('b'); }
;

/* == Basic addresses == */

/* Scope, like std::printf */
scope:
  IDENTIFIER { $$ = ast_new_scope(ast_new_str($1)); }
| scope SCOPE_OP IDENTIFIER { $$ = ast_scope_add($1, ast_new_str($3)); }
;

/*
 * Addresses, like std::net::server.port
 * This one can be used to address properties.
 */
addr:
  /* just a scope */
  scope	{ $$ = ast_new_address($1, NULL); }
  /* scope with property part */
| scope '.' addr_props
  /* scope with function arg addr */
| scope '#' IDENTIFIER { $$ = ast_new_address($1, ast_new_str($3)); }
  /* combined: scope with prop and ONE function arg addr */
| scope '.' addr_props '#' IDENTIFIER
;

addr_props:
  IDENTIFIER
| addr_props '.' IDENTIFIER
;

/* == Declarations == */

decl:
  fact
| class_sel class_decl_block { /* TODO */ $$ = NULL; }
| class_sel ';' { /* TODO */ $$ = NULL; }
;

imper_decl:
  VAR sig_var ';' { $$ = ast_new_vardecl($2, NULL); }
| VAR sig_var '=' conditional_expr ';' { $$ = ast_new_vardecl($2, $4); }
;

/* Selectors, similar to CSS */

/* Class selector */
class_sel:
  CLS_SEL_TYPE IDENTIFIER
;

class_decl_block:
  '{' class_decl_block_stmt '}'
| '{' '}'
;

class_decl_block_stmt:
  func
| fact
| class_decl_block_stmt func
| class_decl_block_stmt fact
;

/* == Ontology == */
fact:
  fol_stmt '.' { $$ = $1; }
| triple_stmt '.' { $$ = $1; }
;

/*
 * First-order logic statement
 *
 * example: isSubclassOf(car, vehicle)
 *          ^ scope     ^ arglist
 */
fol_stmt:
  scope arglist { $$ = ast_new_fact($1, $2); }
;

/*
 * Triple statement
 * shorthand for fol_stmts
 *
 * example: <car>    isSubclassOf <vehicle>
 *           ^ scope ^ scope       ^ addr
 */
triple_stmt:
  /* Unary relation like "isReadOnly" */
  scope
  /* Binary relation like "subclassOf <object>" or const */
| scope triple_obj
  /* Unary relation with an object */
| triple_subj scope { $$ = ast_new_tfact($1, $2, NULL); }
  /* Binary relation between two objects or one object and const */
| triple_subj scope triple_obj { $$ = ast_new_tfact($1, $2, $3); }
;

triple_subj:
  '<' addr '>' { $$ = $2; }
;

triple_obj:
  '<' addr '>' { $$ = $2; }
| const
;

/* == Functions == */

/* Basic function rule */
func:
  sig imper_block { $$ = ast_new_func($1, $2); }
| sig ';'
;

/* Function signature */
sig:
  FN IDENTIFIER fn_arglist sig_ret { $$ = ast_new_sig(ast_new_str($2)); }
;

/* Function signature return value */
sig_ret:
  %empty
| RARR sig_var
;

/* Function siganture variable (with optional type) */
sig_var:
  IDENTIFIER { $$ = ast_new_sigvar(ast_new_str($1), NULL); }
| IDENTIFIER ':' scope { $$ = ast_new_sigvar(ast_new_str($1), $3); }
;

/* Function argument list */
fn_arglist:
  '(' fn_arglist_args ')'
| '(' ')'
;

/* Function argument list element */
fn_arglist_args:
  sig_var
| fn_arglist_args ',' sig_var
;

/* Function argument list */
arglist:
  '(' arglist_args ')' { $$ = $2; }
| '(' ')' { $$ = NULL; }
;

/* Function argument list element */
arglist_args:
  addr
| arglist_args ',' addr { $$ = ast_add_seq($1, $3); }
;

/* == Blocks == */
imper_block:
  '{' imper_block_stmt '}' { $$ = $2; }
| '{' '}' { $$ = NULL; }
;

imper_block_stmt:
  imper_stmt
| imper_decl
| imper_block_stmt imper_stmt { $$ = ast_add_seq($1, $2); }
| imper_block_stmt imper_decl { $$ = ast_add_seq($1, $2); }
;

/*
 * == Expressions ==
 * The syntax of expressions is inspired by C
 * See "The C Programming Language" 2nd edition, pp. 234 ff.
 */
expr:
  assign_expr
| expr ',' assign_expr
;

assign_expr:
  conditional_expr
| unary_expr '=' assign_expr { $$ = ast_new_binop('=', $1, $3); }
;

conditional_expr:
  logical_or_expr
| logical_or_expr '?' conditional_expr ':' conditional_expr {
	$$ = ast_new_ctern($1, $3, $5);
}
;

logical_or_expr:
  logical_and_expr
| logical_or_expr LOG_OR_OP logical_and_expr {
	$$ = ast_new_binop_s("||", $1, $3);
}
;

logical_and_expr:
  inclusive_or_expr
| logical_and_expr LOG_AND_OP inclusive_or_expr {
	$$ = ast_new_binop_s("&&", $1, $3);
}
;

inclusive_or_expr:
  exclusive_or_expr
| inclusive_or_expr BIT_OR_OP exclusive_or_expr {
	$$ = ast_new_binop('|', $1, $3);
}
;

exclusive_or_expr:
  and_expr
| exclusive_or_expr BIT_XOR_OP and_expr { $$ = ast_new_binop('^', $1, $3); }
;

and_expr:
  eq_expr
| and_expr BIT_AND_OP eq_expr { $$ = ast_new_binop('&', $1, $3); }
;

/* Equality expressions */
eq_expr:
  rel_expr
| eq_expr EQ_OP rel_expr { $$ = ast_new_binop_s($2, $1, $3); }
;

rel_expr:
  shift_expr
| rel_expr rel_op shift_expr { $$ = ast_new_binop_s($2, $1, $3); }
;

shift_expr:
  add_expr
| shift_expr SHIFT_OP add_expr { $$ = ast_new_binop_s($2, $1, $3); }
;

/* Arithmetic */
add_expr:
  mul_expr
| add_expr ADD_OP mul_expr { $$ = ast_new_binop($2, $1, $3); }
;

mul_expr:
  unary_expr
| mul_expr MUL_OP unary_expr { $$ = ast_new_binop($2, $1, $3); }
;

unary_expr:
  postfix_expr
| unary_op postfix_expr { $$ = ast_new_unop(0, $1, $2); }
;

postfix_expr:
  primary_expr
| postfix_expr '(' callarglist ')' { $$ = ast_new_call($1, $3); }
| postfix_expr '(' ')' { $$ = ast_new_call($1, NULL); }
| postfix_expr '.' IDENTIFIER
| postfix_expr INC_OP { $$ = ast_new_unop(1, "++", $1); }
| postfix_expr DEC_OP { $$ = ast_new_unop(1, "--", $1); }
;

callarglist:
  conditional_expr
| callarglist ',' conditional_expr
;

primary_expr:
  scope
| const
| '(' expr ')' { $$ = $2; }
;

const:
  INT_CONST { $$ = ast_new_int($1); }
| FLOAT_CONST { $$ = ast_new_float($1); }
| STRING_CONST { $$ = ast_new_str($1); }
;

unary_op:
  ADD_OP { /* ADD_OP means + and - */
	OPERSCPY("X", 1, $$); /* placeholder oper */
	$$[0] = yylval.oper; /* here is the oper */
	/*
	 * this hack is neccessary as ADD_OP is a char and
	 * not a (char *)
	 */
}
| NEG_OP { OPERSCPY("!", 1, $$); }
| INC_OP { OPERSCPY("++", 2, $$); }
| DEC_OP { OPERSCPY("--", 2, $$); }
;

rel_op:
  '<' { OPERSCPY("<", 1, $$); }
| '>' { OPERSCPY(">", 1, $$); }
| REL_EQ_OP
;

%%
void yyerror(char const *s)
{
	fprintf(stderr, "[P] Error: %s at line %d\n", s, yylineno);
}
