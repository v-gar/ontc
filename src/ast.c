/*
 * src/ast.c
 *
 * Abstract Syntax Tree
 *
 * Copyright (c) 2020 Viktor Garske
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ast.h"

#define INIT_NODE(strct, ant) \
	struct strct *node = malloc(sizeof(struct strct)); \
	if (!node) { \
		fprintf(stderr, "Can't create "#strct": out of memory\n"); \
		return NULL;\
	} \
	node->child = NULL; \
	node->sibling = NULL; \
	node->type = ant;


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
		case '+': node->type = ANT_BADD; break;
		case '-': node->type = ANT_BSUB; break;
		case '*': node->type = ANT_MUL;  break;
		case '/': node->type = ANT_DIV;  break;
		case '%': node->type = ANT_MOD;  break;

		/* assignments */
		case '=': node->type = ANT_ASSIGN; break;

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
