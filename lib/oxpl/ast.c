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

#include "ast.h"

/**
 * Helper macro for initializing a node,
 * i.e. allocating the struct (which can be arbitrarily passed to
 * via strct), check whether the allocation was successful,
 * initialize the node common pointers to NULL and set the type
 * to the value ant.
 */
#define INIT_NODE(strct, ant) \
	struct strct *node = malloc(sizeof(struct strct)); \
	if (!node) { \
		fprintf(stderr, "Can't create "#strct": out of memory\n"); \
		return NULL;\
	} \
	node->child = NULL; \
	node->sibling = NULL; \
	node->type = ant;


/**
 * Set the type for a binary operation node in the switch-statement.
 */
#define SET_BINOP_TYPE(ch, ant_type) case ch: node->type = ant_type; break;

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

struct ast_node *ast_new_int(int value)
{
	INIT_NODE(ast_node_int, ANT_INT);

	node->value = value;

	return (struct ast_node *)node;
}

struct ast_node *ast_new_float(float value)
{
	INIT_NODE(ast_node_float, ANT_FLOAT);

	node->value = value;

	return (struct ast_node *)node;
}

struct ast_node *ast_new_str(char *value)
{
	INIT_NODE(ast_node_str, ANT_STR);

	node->value = malloc(strlen(value) + 1);
	strcpy(node->value, value);

	return (struct ast_node *)node;
}

struct ast_node *ast_new_scope(struct ast_node *value)
{
	INIT_NODE(ast_node, ANT_SCOPE);

	if (value == NULL) {
		fprintf(stderr, "Error: empty scope\n");
		free(node);
		return NULL;
	}

	if (value->type != ANT_STR) {
		fprintf(stderr, "Error: scope identifier has to "
				"be a string.\n");
		free(node);
		return NULL;
	}

	node->child = value;

	return (struct ast_node *)node;
}

struct ast_node *ast_new_call(struct ast_node *callee,
		struct ast_node *arglist)
{
	INIT_NODE(ast_node, ANT_CALL);

	node->child = callee;

	if (arglist != NULL)
		node->child->sibling = arglist;

	return (struct ast_node *)node;
}

struct ast_node *ast_new_binop(char oper, struct ast_node *operand1,
		struct ast_node *operand2)
{
	INIT_NODE(ast_node, ANT_UNDEFINED);

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

		SET_BINOP_TYPE('&', ANT_BAND);
		SET_BINOP_TYPE('|', ANT_BOR);
		SET_BINOP_TYPE('^', ANT_XOR);

		/* anything else */
		default:
			  fprintf(stderr, "Error: invalid operator\n");
			  free(node);
			  return NULL;
	}

	node->child = operand1;
	node->child->sibling = operand2;

	return (struct ast_node *)node;
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
	else {
		fprintf(stderr, "[A] Error: unknown binop '%s '\n", oper_s);
		return NULL;
	}

	return ast_new_binop(oper, operand1, operand2);
}

struct ast_node *ast_new_sig(struct ast_node *name)
{
	INIT_NODE(ast_node, ANT_SIG);

	node->child = name;

	return (struct ast_node *)node;
}

struct ast_node *ast_new_func(struct ast_node *sig,
		struct ast_node *block)
{
	INIT_NODE(ast_node, ANT_FUNC);

	node->child = sig;
	node->child->sibling = block;

	return (struct ast_node *)node;
}

struct ast_node *ast_new_address(struct ast_node *scope,
		struct ast_node *param)
{
	INIT_NODE(ast_node, ANT_ADDRESS);

	node->child = scope;

	if (param != NULL)
		node->child->sibling = param;

	return (struct ast_node *)node;
}

struct ast_node *ast_new_tfact(struct ast_node *subj,
		struct ast_node *rel, struct ast_node *obj)
{
	INIT_NODE(ast_node, ANT_TFACT);

	if (rel == NULL) {
		fprintf(stderr, "[A] Error: fact rel is NULL\n");
		return NULL;
	}

	node->child = rel;
	node->child->sibling = subj;
	node->child->sibling->sibling = obj;

	return (struct ast_node *)node;
}

struct ast_node *ast_new_transunit(struct ast_node *first)
{
	INIT_NODE(ast_node, ANT_TRANSUNIT);

	node->child = first;

	return (struct ast_node *)node;
}

struct ast_node *ast_add_seq(struct ast_node *node,
		struct ast_node *successor)
{
	if (node == NULL) {
		fprintf(stderr, "Can't add seq because node is NULL\n");
		return node;
	}

	struct ast_node **cursor = &node->sibling;

	while (*cursor != NULL)
		cursor = &(*cursor)->sibling;

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

	struct ast_node **cur = &node->child;

	while (*cur != NULL) {
		cur = &(*cur)->sibling;
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

	printf("Node type: %d\n", root->type);

	if (root->type == ANT_STR)
		printf("String: %s\n", ((struct ast_node_str *)root)->value);

	if (root->type == ANT_INT)
		printf("Int: %d\n", ((struct ast_node_int *)root)->value);

	printf("Child:\n");
	ast_print(root->child);
	printf("Sibling:\n");
	ast_print(root->sibling);
}

void ast_print_dot(struct ast_node *root)
{
	if (root == NULL)
		return;

	if (root->type == ANT_TRANSUNIT)
		printf("digraph ast\n{\n");

	struct ast_node *cur = root->child;

	while (cur != NULL) {
		printf("\tP%ld_T%d -> P%ld_T%d;\n",
				(long) root, root->type,
				(long) cur, cur->type);
		cur = cur->sibling;
	}

	ast_print_dot(root->sibling);
	ast_print_dot(root->child);

	if (root->type == ANT_TRANSUNIT)
		printf("}\n");
}

int ast_validate(struct ast_node *root)
{
	if (root == NULL) {
		fprintf(stderr, "Error: AST empty\n");
		return 1;
	}

	struct ast_node *cur = root->child;

	int main_found = 0;

	while (cur != NULL) {
		if (cur->type == ANT_FUNC) {
			/* if this is a function ... */
			struct ast_node *ident = cur->child->child;
			if (ident->type == ANT_STR) {
				/* ... and the identifier is a string ... */
				if (strcmp(
					((struct ast_node_str *)ident)->value,
					"main") == 0)
					main_found = 1;
			} else {
				fprintf(stderr, "[A] Error: invalid fn sig\n");
				return 1;
			}
		}

		cur = cur->sibling;
	}

	if (main_found != 1) {
		fprintf(stderr, "[A] Error: missing main function\n");
		return 1;
	}

	return 0;
}
