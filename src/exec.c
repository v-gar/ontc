/**
 * src/exec.c
 *
 * Ontology Program executor
 *
 * Copyright (c) 2020 Viktor Garske
 */

/**
 * \file exec.c
 * \brief Executor implementation
 */

#include <string.h>

#include "exec.h"
#include "parse.h"
#include "parse.tab.h"
#include "ast.h"

#include "builtin.h"

extern struct ast_node *parse_ast;
extern FILE *yyin;
extern int yyparse();

static int execute(struct ast_node *root);
static int execute_function(struct ast_node *fn_node);
static int execute_call(struct ast_node *call_node);
static struct ast_node *get_main(struct ast_node *ast);

int exec_program(FILE *fp)
{
	yyin = fp;
	yyparse();
	ast_validate(parse_ast);
	execute(parse_ast);
	return 0;
}

static int execute(struct ast_node *root)
{
	struct ast_node *main_fn_node = get_main(root);

	if (main_fn_node == NULL) {
		fprintf(stderr, "[E] Error: main function not present\n");
		return 1;
	}

	execute_function(main_fn_node);

	return 0;

}

static int execute_function(struct ast_node *fn_node)
{
	/* check whether node is function node */
	if (fn_node == NULL || fn_node->type != ANT_FUNC)
		return 1;

	/* walk through function child nodes */
	struct ast_node *cur;

	cur = fn_node->child; /* signature (no handler implemented yet) */
	cur = cur->sibling; /* body */

	/* execute function body */
	while (cur != NULL) {
		if (ANT_CALL == cur->type) {
			execute_call(cur);
		}

		/* next child of function */
		cur = cur->sibling;
	}

	return 0;
}

static int execute_call(struct ast_node *call_node)
{
	/* check whether node is call node */
	if (call_node == NULL || call_node->type != ANT_CALL)
		return 1;

	struct ast_node *call_expr = call_node->child;
	struct ast_node *args = call_expr->sibling;

	/* test for simple built-in function */
	if (call_expr->type == ANT_SCOPE
			&& call_expr->child != NULL
			&& call_expr->child->type == ANT_STR
			&& call_expr->child->sibling == NULL) /* one level */
	{
		char *name = ((struct ast_node_str *)call_expr->child)->value;

		if (strcmp("print", name) == 0) {
			lang_builtin_fn_print(args);
		} else if (strcmp("println", name) == 0) {
			lang_builtin_fn_println(args);
		} else {
			printf("[E] unknown function...\n");
			return 1;
		}
	}

	return 0;
}

static struct ast_node *get_main(struct ast_node *ast)
{
	if (ast == NULL)
		return NULL;

	struct ast_node *cur = ast->child;

	/* find main function */
	do {
		if (cur->type != ANT_FUNC)
			continue;

		/* if this is a function ... */
		struct ast_node *ident = cur->child->child;

		if (ident->type != ANT_STR)
			continue;

		/* ... and the identifier is a string ... */
		if (strcmp(((struct ast_node_str *)ident)->value, "main") == 0)
			return cur;
	} while (NULL != (cur = cur->sibling));

	return NULL;
}
