/*
 * src/ast.h
 *
 * Abstract Syntax Tree
 *
 * Copyright (c) 2020 Viktor Garske
 */

#ifndef H_AST
#define H_AST

enum ast_node_type {
	ANT_UNDEFINED
};

/* AST is implemented as a left-child, right-sibling tree */
struct ast_node {
	enum ast_node_type type;
	struct ast_node *child;
	struct ast_node *sibling;
};

#endif /* ifndef H_AST */
