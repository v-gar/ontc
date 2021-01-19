/*
 * lib/oxpl/ast.c
 *
 * Abstract Syntax Tree
 *
 * Copyright (c) 2020 Viktor Garske
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/**
 * \file ast.c
 * \brief Functions for the manipulation of the abstract syntax tree
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "ast.h"

static struct ast_node *init_node(enum ast_node_type const type);

/**
 * Set the type for a binary operation node in the switch-statement.
 */
#define SET_BINOP_TYPE(ch, ant_type) \
	case ch: AST_NODE_TYPE(node) = ant_type; break;

/**
 * Multi-character operators used in ast_new_binop_s.
 * Values start at 20 because it is an irrelevant ASCII range
 */
enum multicharacter_operator {
	/** Multi-char operator equals: == */
	MCOP_EQ     = 20,
	/** Multi-character operator not equals: != */
	MCOP_NEQ    = 21,
	/** Multi-character operator logical and: && */
	MCOP_LAND   = 22,
	/** Multi-character operator logical or: || */
	MCOP_LOR    = 23,
	/** Multi-character operator less equals: <= */
	MCOP_LEQ    = 24,
	/** Multi-character operator greater equals: >= */
	MCOP_GEQ    = 25,
	/** Multi-character operator shift left: << */
	MCOP_SHIFTL = 26,
	/** Multi-character operator shift right: >> */
	MCOP_SHIFTR = 27
};

static struct ast_node *init_node(enum ast_node_type const type)
{
	size_t size;

	switch (type) {
		case ANT_INT:
			size = sizeof(struct ast_node_int);
			break;

		case ANT_FLOAT:
			size = sizeof(struct ast_node_float);
			break;

		case ANT_STR:
			size = sizeof(struct ast_node_str);
			break;

		default:
			size = sizeof(struct ast_node);
	}

	struct ast_node *node = malloc(size);

	if (!node) {
		perror("Can't create node struct: out of memory");
		return NULL;
	}

	node->base.child = NULL;
	node->base.sibling = NULL;
	node->base.type = type;

	return node;
}

struct ast_node *ast_new_int(int value)
{
	struct ast_node_int *node = (struct ast_node_int *)init_node(ANT_INT);

	if (node == NULL)
		return NULL;

	node->value = value;

	return (struct ast_node *)node;
}

struct ast_node *ast_new_float(float value)
{
	struct ast_node_float *node =
		(struct ast_node_float *)init_node(ANT_FLOAT);

	if (node == NULL)
		return NULL;

	node->value = value;

	return (struct ast_node *)node;
}

struct ast_node *ast_new_str(char *value)
{
	struct ast_node_str *node = (struct ast_node_str *)init_node(ANT_STR);

	if (node == NULL)
		return NULL;

	node->value = value;

	return (struct ast_node *)node;
}

struct ast_node *ast_new_scope(struct ast_node *value)
{
	struct ast_node *node = init_node(ANT_SCOPE);

	if (node == NULL)
		return NULL;

	if (value == NULL) {
		fprintf(stderr, "Error: empty scope\n");
		free(node);
		return NULL;
	}

	if (AST_NODE_TYPE(value) != ANT_STR) {
		fprintf(stderr, "Error: scope identifier has to "
				"be a string.\n");
		free(node);
		return NULL;
	}

	AST_NODE_CHLD1(node) = value;

	return node;
}

struct ast_node *ast_new_cmpd(struct ast_node *head)
{
	struct ast_node *node = init_node(ANT_CMPD);

	if (node == NULL)
		return NULL;

	if (head == NULL) {
		fprintf(stderr, "Error: empty head\n");
		free(node);
		return NULL;
	}

	AST_NODE_CHLD1(node) = head;

	return node;
}

struct ast_node *ast_convert_cmpd_seq(struct ast_node *const cmpd_node)
{
	if (cmpd_node == NULL) {
		/*
		 * This case occurs if the compound is already empty,
		 * i.e. if the compound statement is empty.
		 */
		return NULL;
	}

	if (AST_NODE_TYPE(cmpd_node) != ANT_CMPD) {
		fprintf(stderr, "Error: cmpd_node has not type ANT_CMPD,"
				" it has %d\n", AST_NODE_TYPE(cmpd_node));
		return NULL;
		/*
		 * returning cmpd_node would be another option but this
		 * would have catastrophic influence on the following phases
		 * as other following stages need to handle this case.
		 *
		 * Now, they just need to cope with the NULL case.
		 */
	}

	AST_NODE_TYPE(cmpd_node) = ANT_SEQ;

	/**
	 * In order to improve performance, children of nodes are not
	 * checked (if NULL, etc.)
	 */

	return cmpd_node;
}

struct ast_node *ast_new_call(struct ast_node *callee,
		struct ast_node *arglist)
{
	struct ast_node *node = init_node(ANT_CALL);

	if (node == NULL)
		return NULL;

	AST_NODE_CHLD1(node) = callee;

	if (arglist != NULL)
		AST_NODE_CHLD2(node) = arglist;

	return node;
}

struct ast_node *ast_new_binop(char oper, struct ast_node *operand1,
		struct ast_node *operand2)
{
	struct ast_node *node = init_node(ANT_UNDEFINED);

	if (node == NULL)
		return NULL;

	switch (oper) {
		/* arithmetic operations */
		SET_BINOP_TYPE('+', ANT_BADD);
		SET_BINOP_TYPE('-', ANT_BSUB);
		SET_BINOP_TYPE('*', ANT_MUL);
		SET_BINOP_TYPE('/', ANT_DIV);
		SET_BINOP_TYPE('%', ANT_MOD);

		/* assignments */
		SET_BINOP_TYPE('=', ANT_ASSIGN);

		/* relational operators */
		SET_BINOP_TYPE(MCOP_EQ, ANT_EQ);
		SET_BINOP_TYPE(MCOP_NEQ, ANT_NEQ);
		SET_BINOP_TYPE(MCOP_LAND, ANT_LAND);
		SET_BINOP_TYPE(MCOP_LOR, ANT_LOR);
		SET_BINOP_TYPE(MCOP_LEQ, ANT_LEQ);
		SET_BINOP_TYPE(MCOP_GEQ, ANT_GEQ);
		SET_BINOP_TYPE(MCOP_SHIFTL, ANT_SHIFTL);
		SET_BINOP_TYPE(MCOP_SHIFTR, ANT_SHIFTR);

		SET_BINOP_TYPE('<', ANT_LT);
		SET_BINOP_TYPE('>', ANT_GT);

		SET_BINOP_TYPE('&', ANT_BAND);
		SET_BINOP_TYPE('|', ANT_BOR);
		SET_BINOP_TYPE('^', ANT_XOR);

		/* anything else */
		default:
			  fprintf(stderr, "Error: invalid operator\n");
			  free(node);
			  return NULL;
	}

	AST_NODE_CHLD1(node) = operand1;
	AST_NODE_CHLD2(node) = operand2;

	return node;
}

struct ast_node *ast_new_binop_s(char *oper_s, struct ast_node *operand1,
		struct ast_node *operand2)
{
	char oper;

	/*
	 * The basic idea is to use oper anyway and use an internal
	 * mapping that ast_new_binop() unterstands in order to exploit
	 * the value range of an char.
	 */

	if (!strcmp(oper_s, "=="))
		oper = MCOP_EQ;
	else if (!strcmp(oper_s, "!="))
		oper = MCOP_NEQ;
	else if (!strcmp(oper_s, "&&"))
		oper = MCOP_LAND;
	else if (!strcmp(oper_s, "||"))
		oper = MCOP_LOR;
	else if (!strcmp(oper_s, "<="))
		oper = MCOP_LEQ;
	else if (!strcmp(oper_s, ">="))
		oper = MCOP_GEQ;
	else if (!strcmp(oper_s, "<<"))
		oper = MCOP_SHIFTL;
	else if (!strcmp(oper_s, ">>"))
		oper = MCOP_SHIFTR;
	/*
	 * now some single-char ops follow which are passed to this
	 * function by the parser in order to keep the structure simple
	 * (rel_op is always a (char *) as it is the gcd between
	 * strings and chars)
	 *
	 * those single-char ops do not have a dedicated MCOP enumerator
	 * and use their own ASCII char value for oper
	 */
	else if (!strcmp(oper_s, "<"))
		oper = '<';
	else if (!strcmp(oper_s, ">"))
		oper = '>';
	/* if nothing matched */
	else {
		fprintf(stderr, "[A] Error: unknown binop '%s'\n", oper_s);
		return NULL;
	}

	return ast_new_binop(oper, operand1, operand2);
}

struct ast_node *ast_new_unop(char affix, char *oper,
		struct ast_node *operand)
{
	/*
	 * affix can be
	 * 0 = prefix operator
	 * 1 = postfix operator
	 * nothing else
	 */

	/* therefore check boundaries */
	if (!(affix == 0 || affix == 1))
		return NULL;

	struct ast_node *node = init_node(ANT_UNDEFINED);

	if (node == NULL)
		return NULL;

	if (!(strcmp(oper, "++"))) {
		AST_NODE_TYPE(node) = affix == 0 ? ANT_PREINC : ANT_POSTINC;
	} else if (!(strcmp(oper, "--"))) {
		AST_NODE_TYPE(node) = affix == 0 ? ANT_PREDEC : ANT_POSTDEC;
	} else if (!(strcmp(oper, "-"))) {
		AST_NODE_TYPE(node) = ANT_NEGSIGN;
	}

	if (AST_NODE_TYPE(node) == ANT_UNDEFINED) {
		fprintf(stderr, "[A] Warning: UNDEF UNOP '%s'\n", oper);
	}

	AST_NODE_CHLD1(node) = operand;

	return node;
}

struct ast_node *ast_new_sig(struct ast_node *name)
{
	struct ast_node *node = init_node(ANT_SIG);

	if (node == NULL)
		return NULL;

	AST_NODE_CHLD1(node) = name;

	return node;
}

struct ast_node *ast_new_sigvar(struct ast_node *identifier,
		struct ast_node *type)
{
	struct ast_node *node = init_node(ANT_SIGVAR);

	if (node == NULL)
		return NULL;

	AST_NODE_CHLD1(node) = identifier;

	if (type != NULL)
		AST_NODE_CHLD2(node) = type;

	return node;
}

struct ast_node *ast_new_func(struct ast_node *sig,
		struct ast_node *block)
{
	struct ast_node *node = init_node(ANT_FUNC);

	if (node == NULL)
		return NULL;

	AST_NODE_CHLD1(node) = sig;

	/* block is NULL if node is declaration only */
	AST_NODE_CHLD2(node) = block;

	return node;
}

struct ast_node *ast_new_address(struct ast_node *scope,
		struct ast_node *param)
{
	struct ast_node *node = init_node(ANT_ADDR);

	if (node == NULL)
		return NULL;

	AST_NODE_CHLD1(node) = scope;

	if (param != NULL)
		AST_NODE_CHLD2(node) = param;

	return node;
}

struct ast_node *ast_new_fact(struct ast_node *rel,
		struct ast_node *args)
{
	struct ast_node *node = init_node(ANT_FACT);

	if (node == NULL)
		return NULL;

	AST_NODE_CHLD1(node) = rel;
	AST_NODE_CHLD2(node) = args;

	return node;
}

struct ast_node *ast_new_tfact(struct ast_node *subj,
		struct ast_node *rel, struct ast_node *obj)
{
	struct ast_node *node = init_node(ANT_TFACT);

	if (node == NULL)
		return NULL;

	if (rel == NULL) {
		fprintf(stderr, "[A] Error: fact rel is NULL\n");
		return NULL;
	}

	AST_NODE_CHLD1(node) = rel;
	AST_NODE_CHLD2(node) = subj;
	AST_NODE_CHLD3(node) = obj;

	return node;
}

struct ast_node *ast_new_vardecl(struct ast_node *sigvar,
		struct ast_node *val)
{
	struct ast_node *node = init_node(ANT_VARDECL);

	if (node == NULL)
		return NULL;

	AST_NODE_CHLD1(node) = sigvar;

	if (val != NULL)
		AST_NODE_CHLD2(node) = val;

	return node;
}

struct ast_node *ast_new_class(struct ast_node *identifier,
		struct ast_node *spec)
{
	struct ast_node *node = init_node(ANT_CLASS);

	if (node == NULL)
		return NULL;

	assert(AST_NODE_TYPE(identifier) == ANT_STR);
	assert(spec == NULL || AST_NODE_TYPE(spec) == ANT_CSPEC);

	AST_NODE_CHLD1(node) = identifier;
	AST_NODE_CHLD2(node) = spec; /* NULL if no body */

	/* TODO: implement instance class type */

	return node;
}

struct ast_node *ast_new_cspec(struct ast_node *head)
{
	struct ast_node *node = init_node(ANT_CSPEC);

	if (node == NULL)
		return NULL;

	AST_NODE_CHLD1(node) = head;

	return node;
}

struct ast_node *ast_new_transunit(struct ast_node *first)
{
	struct ast_node *node = init_node(ANT_TRANSUNIT);

	if (node == NULL)
		return NULL;

	AST_NODE_CHLD1(node) = first;

	return node;
}

struct ast_node *ast_new_cond(struct ast_node *cond,
		struct ast_node *then, struct ast_node *else_)
{
	struct ast_node *node = init_node(ANT_COND);

	if (node == NULL)
		return NULL;

	AST_NODE_CHLD1(node) = cond;
	AST_NODE_CHLD2(node) = then; /* TODO: introduce ANT_SEQ */
	AST_NODE_CHLD3(node) = else_;

	return node;
}

struct ast_node *ast_new_ctern(struct ast_node *cond,
		struct ast_node *then, struct ast_node *else_)
{
	/*
	 * very similar to ast_new_cond
	 *
	 * Nevertheless separate functions will be helpful
	 * as ANT_COND has then/else-blocks and ANT_CTERN
	 * has then/else-expressions.
	 */
	struct ast_node *node = init_node(ANT_CTERN);

	if (node == NULL)
		return NULL;

	AST_NODE_CHLD1(node) = cond;
	AST_NODE_CHLD2(node) = then;
	AST_NODE_CHLD3(node) = else_;

	return node;
}

struct ast_node *ast_new_ret(struct ast_node *expr)
{
	struct ast_node *node = init_node(ANT_RET);

	if (node == NULL)
		return NULL;

	if (expr != NULL)
		AST_NODE_CHLD1(node) = expr;

	return node;
}

struct ast_node *ast_new_jump(char type)
{
	int ast_type;

	switch (type) {
		case 'b': ast_type = ANT_BREAK; break;
		case 'c': ast_type = ANT_CONT; break;
		default: return NULL;
	}

	return init_node(ast_type);
}

struct ast_node *ast_new_while(struct ast_node *condition,
		struct ast_node *block)
{
	struct ast_node *node = init_node(ANT_WHILE);

	if (node == NULL)
		return NULL;

	AST_NODE_CHLD1(node) = condition;
	AST_NODE_CHLD2(node) = block;

	return node;
}

struct ast_node *ast_new_for(struct ast_node *identifier,
		struct ast_node *iterable,
		struct ast_node *block)
{
	struct ast_node *node = init_node(ANT_FOR);

	if (node == NULL)
		return NULL;

	AST_NODE_CHLD1(node) = identifier;
	AST_NODE_CHLD2(node) = iterable;
	AST_NODE_CHLD3(node) = block;

	return node;
}

struct ast_node *ast_add_seq(struct ast_node *node,
		struct ast_node *successor)
{
	if (node == NULL) {
		fprintf(stderr, "Can't add seq because node is NULL\n");
		return node;
	}

	struct ast_node **cursor = &node->base.sibling;

	while (*cursor != NULL)
		cursor = &(*cursor)->base.sibling;

	*cursor = successor;
	return node;
}

struct ast_node *ast_scope_add(struct ast_node *node,
		struct ast_node *successor)
{
	if (node == NULL) {
		fprintf(stderr, "Can't add scope because node is NULL\n");
		return node;
	}

	struct ast_node **cur = &node->base.child;

	while (*cur != NULL) {
		cur = &(*cur)->base.sibling;
	}

	*cur = successor;

	return node;
}

void ast_print(struct ast_node *root)
{
	if (root == NULL) {
		printf("root is NULL\n");
		return;
	}

	printf("Node type: %d\n", AST_NODE_TYPE(root));

	if (AST_NODE_TYPE(root) == ANT_STR) {
		struct ast_node_str *node_val = (struct ast_node_str *)root;
		printf("String: %s\n", node_val->value);
	}

	if (AST_NODE_TYPE(root) == ANT_INT) {
		struct ast_node_int *node_val = (struct ast_node_int *)root;
		printf("Int: %d\n", node_val->value);
	}

	printf("Child:\n");
	ast_print(AST_NODE_CHLD(root));
	printf("Sibling:\n");
	ast_print(AST_NODE_SIBL(root));
}

void ast_print_dot(struct ast_node *root)
{
	if (root == NULL)
		return;

	if (AST_NODE_TYPE(root) == ANT_TRANSUNIT)
		printf("digraph ast\n{\n");

	struct ast_node *cur = AST_NODE_CHLD(root);

	while (cur != NULL) {
		printf("\tP%ld_T%d -> P%ld_T%d;\n",
				(long) root, AST_NODE_TYPE(root),
				(long) cur, AST_NODE_TYPE(cur));
		AST_NODE_NEXT_SIBL(cur);
	}

	ast_print_dot(AST_NODE_SIBL(root));
	ast_print_dot(AST_NODE_CHLD(root));

	if (AST_NODE_TYPE(root) == ANT_TRANSUNIT)
		printf("}\n");
}

int ast_validate(struct ast_node *root)
{
	if (root == NULL) {
		fprintf(stderr, "Error: AST empty\n");
		return 1;
	}

	struct ast_node *cur = AST_NODE_CHLD(root);

	int main_found = 0;

	while (cur != NULL) {
		if (AST_NODE_TYPE(cur) == ANT_FUNC) {
			/* if this is a function ... */
			struct ast_node *sig = AST_NODE_CHLD1(cur);
			struct ast_node *ident = AST_NODE_CHLD1(sig);
			if (AST_NODE_TYPE(ident) == ANT_STR) {
				/* ... and the identifier is a string ... */
				struct ast_node_str *node_val =
					(struct ast_node_str *)ident;
				if (strcmp(node_val->value, "main") == 0)
					main_found = 1;
			} else {
				fprintf(stderr, "[A] Error: invalid fn sig\n");
				return 1;
			}
		}

		AST_NODE_NEXT_SIBL(cur);
	}

	if (main_found != 1) {
		fprintf(stderr, "[A] Error: missing main function\n");
		return 1;
	}

	return 0;
}

void ast_free(struct ast_node *root)
{
	if (root == NULL)
		return;

	ast_free(AST_NODE_CHLD(root));
	ast_free(AST_NODE_SIBL(root));

	switch (AST_NODE_TYPE(root)) {
		case ANT_STR: {
			struct ast_node_str *node_val =
				(struct ast_node_str *)root;
			free(node_val->value);
			break;
			      }

		default:
			      break;
	}

	free(root);
}
