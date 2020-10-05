/*
 * src/parse.y
 *
 * Parser for OXPL
 *
 * Copyright (c) 2020 Viktor Garske
 */
%{
  #include <stdio.h>

  #include "parse.h"
  #include "ast.h"

  int yylex(void);
  void yyerror(char const *);
  extern int yylineno;

  struct ast_node *parse_ast;

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

%token EQ_OP
%token REL_EQ_OP
%token SHIFT_OP

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
		conditional_expr assign_expr expr
		expr_stmt imper_stmt imper_block imper_block_stmt
		sig func callarglist
		stmt tunit start

%%
start:
  %empty { parse_ast = NULL; }
| tunit { parse_ast = ast_new_transunit($1); }
;

tunit:
  stmt
| tunit stmt { $$ = ast_add_seq($1, $2); }

stmt:
  decl { /* TODO */ $$ = NULL; }
| func
;

/* Imperative block statement */
imper_stmt:
  imper_block
| expr_stmt
| iter_stmt { /* TODO */ $$ = NULL; }
| slct_stmt { /* TODO */ $$ = NULL; }
| jump_stmt { /* TODO */ $$ = NULL; }
;

/* Expression statement */
expr_stmt:
  expr ';' { $$ = $1; }
| ';' { $$ = NULL; }
;

/* Conditional / select statements */
slct_stmt:
  IF slct_conditional imper_block
| IF slct_conditional imper_block ELSE imper_block
| IF slct_conditional imper_block ELSE slct_stmt
;

slct_conditional:
  conditional_expr
;

/* Loops */
iter_stmt:
  WHILE conditional_expr imper_block
| FOR IDENTIFIER IN expr imper_block
;

/* Jumps */
jump_stmt:
  RETURN expr ';'
| RETURN ';'
| CONTINUE ';'
| BREAK ';'
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
  scope
  /* scope with property part */
| scope '.' addr_props
  /* scope with function arg addr */
| scope '#' IDENTIFIER
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
| class_sel class_decl_block
| class_sel ';'
;

/* imper_decl: VAR assign_expr; */
imper_decl: VAR sig_var '=' conditional_expr ';';

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
  fol_stmt '.'
| triple_stmt '.'
;

/*
 * First-order logic statement
 *
 * example: isSubclassOf(car, vehicle)
 *          ^ scope     ^ arglist
 */
fol_stmt:
  scope arglist
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
  /* Binary relation like "subclassOf <object>" */
| scope '<' addr '>'
  /* Binary relation with literal value / const */
| scope const
  /* Unary relation with an object */
| '<' addr '>' scope
  /* Binary relation between two objects */
| '<' addr '>' scope '<' addr '>'
  /* Binary relation between object and const */
| '<' addr '>' scope const
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
  IDENTIFIER
| IDENTIFIER ':' scope
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
  '(' arglist_args ')'
| '(' ')'
;

/* Function argument list element */
arglist_args:
  addr
| arglist_args ',' addr
;

/* == Blocks == */
imper_block:
  '{' imper_block_stmt '}' { $$ = $2; }
| '{' '}' { $$ = NULL; }
;

imper_block_stmt:
  imper_stmt
| imper_decl { /* TODO */ $$ = NULL; }
| imper_block_stmt imper_stmt { $$ = ast_add_seq($1, $2); }
| imper_block_stmt imper_decl { /* TODO */ }
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
| logical_or_expr '?' conditional_expr ':' conditional_expr
;

logical_or_expr:
  logical_and_expr
| logical_or_expr LOG_OR_OP logical_and_expr
;

logical_and_expr:
  inclusive_or_expr
| logical_and_expr LOG_AND_OP inclusive_or_expr
;

inclusive_or_expr:
  exclusive_or_expr
| inclusive_or_expr BIT_OR_OP exclusive_or_expr
;

exclusive_or_expr:
  and_expr
| exclusive_or_expr BIT_XOR_OP and_expr
;

and_expr:
  eq_expr
| and_expr BIT_AND_OP eq_expr
;

/* Equality expressions */
eq_expr:
  rel_expr
| eq_expr EQ_OP rel_expr
;

rel_expr:
  shift_expr
| rel_expr rel_op shift_expr
;

shift_expr:
  add_expr
| shift_expr SHIFT_OP add_expr
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
| unary_op postfix_expr { /* TODO */ }
;

postfix_expr:
  primary_expr
| postfix_expr '(' callarglist ')' { $$ = ast_new_call($1, $3); }
| postfix_expr '(' ')' { $$ = ast_new_call($1, NULL); }
| postfix_expr '.' IDENTIFIER
| postfix_expr INC_OP
| postfix_expr DEC_OP
;

callarglist:
  conditional_expr
| callarglist ',' conditional_expr
;

primary_expr:
  scope
| const
| '(' expr ')' { /* TODO */ }
;

const:
  INT_CONST { $$ = ast_new_int($1); }
| FLOAT_CONST { $$ = ast_new_float($1); }
| STRING_CONST { $$ = ast_new_str($1); }
;

unary_op:
  ADD_OP
| NEG_OP
| INC_OP
| DEC_OP
;

rel_op:
  '<'
| '>'
| REL_EQ_OP
;

%%
void yyerror(char const *s)
{
	fprintf(stderr, "[P] Error: %s at line %d\n", s, yylineno);
}
