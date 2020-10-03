/*
 * src/ast.h
 *
 * Abstract Syntax Tree
 *
 * Copyright (c) 2020 Viktor Garske
 */

#ifndef H_AST
#define H_AST

#include <stddef.h>

enum ast_node_type {
	ANT_UNDEFINED,
	ANT_INT,
	ANT_FLOAT,
	ANT_STR,
	ANT_SCOPE,
	ANT_CALL,
	ANT_INC,

	/* Operations */
	ANT_BADD, /* binary */
	ANT_BSUB, /* binary */
	ANT_MUL,
	ANT_DIV,
	ANT_MOD,
	ANT_ASSIGN,

	ANT_FUNC,
	ANT_SIG
};

enum ast_unary_op {
	UOP_MINUS,
	UOP_PLUS
};

enum ast_binary_op {
	BOP_ADD,
	BOP_SUB,
	BOP_MUL,
	BOP_DIV,
	BOP_MOD
};

/* AST is implemented as a left-child, right-sibling tree */
struct ast_node {
	enum ast_node_type type;
	struct ast_node *child;
	struct ast_node *sibling;
};

struct ast_node_int {
	enum ast_node_type type;
	struct ast_node *child;
	struct ast_node *sibling;

	int value;
};

struct ast_node_float {
	enum ast_node_type type;
	struct ast_node *child;
	struct ast_node *sibling;

	float value;
};

struct ast_node_str {
	enum ast_node_type type;
	struct ast_node *child;
	struct ast_node *sibling;

	char *value;
};

struct ast_node *ast_new_int(int value);
struct ast_node *ast_new_float(float value);
struct ast_node *ast_new_str(char *value);
struct ast_node *ast_new_scope(struct ast_node *value);
struct ast_node *ast_new_call(struct ast_node *callee,
		struct ast_node *arglist);
struct ast_node *ast_new_binop(char oper, struct ast_node *operand1,
		struct ast_node *operand2);
struct ast_node *ast_new_sig(struct ast_node *name);
struct ast_node *ast_new_func(struct ast_node *sig,
		struct ast_node *block);

struct ast_node *ast_add_seq(struct ast_node *node,
		struct ast_node *successor);
struct ast_node *ast_scope_add(struct ast_node *node,
		struct ast_node *successor);

/* debugging tools */
void ast_print(struct ast_node *root);

#endif /* ifndef H_AST */
