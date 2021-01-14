/**
 * src/exec.c
 *
 * Ontology Program executor
 *
 * Copyright (c) 2020 Viktor Garske
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/**
 * \file exec.c
 * \brief Executor implementation
 */

#include <stdlib.h>
#include <string.h>

#include "exec.h"
#include "parse.h"
#include "parse.tab.h"
#include "ast.h"
#include "onto.h"

#include "shell.h"

#include "builtin.h"

extern struct ast_node *parse_ast;
extern FILE *yyin;
extern int yyparse();
extern int yylex_destroy();

static int execute(struct ast_node *root, struct ontology_database *kb);
static int execute_function(struct ast_node *fn_node,
		struct ontology_database *kb,
		struct ast_node *root);
static int execute_call(struct ast_node *call_node);
static struct ast_node *get_fn(struct ast_node *ast, char *name);
static void collect_facts(struct ast_node *root, struct ontology_database *kb);
static void populate_kb(struct ontology_database *kb);
static void add_predef_fact(struct ontology_database *kb, char *name);

int exec_program(FILE *fp)
{
	yyin = fp;
	yyparse();
	ast_validate(parse_ast);

	/* build ontology */
	struct ontology_database *kb = ontology_create_database();
	populate_kb(kb);
	collect_facts(parse_ast, kb);

	/* execute program */
	execute(parse_ast, kb);

	/* clean up */
	ontology_free_database(kb);
	ast_free(parse_ast);
	yylex_destroy();

	return 0;
}

int debug_ontology(FILE *fp)
{
	yyin = fp;
	yyparse();
	ast_validate(parse_ast);

	/* build ontology */
	struct ontology_database *kb = ontology_create_database();
	populate_kb(kb);
	collect_facts(parse_ast, kb);

	/* start shell */
	start_repl_shell(kb); /* frees also kb! */

	return 0;
}

static int execute(struct ast_node *root, struct ontology_database *kb)
{
	struct ast_node *main_fn_node = get_fn(root, "main");

	if (main_fn_node == NULL) {
		fprintf(stderr, "[E] Error: main function not present\n");
		return 1;
	}

	execute_function(main_fn_node, kb, root);

	return 0;

}

static int execute_function(struct ast_node *fn_node,
		struct ontology_database *kb,
		struct ast_node *root)
{
	/* check whether node is function node */
	if (fn_node == NULL || fn_node->type != ANT_FUNC)
		return 1;

	/* walk through function child nodes */
	struct ast_node *cur;

	cur = fn_node->child; /* signature (no handler implemented yet) */

	/* ontology proof of concept */
	char *name = ((struct ast_node_str *)cur->child)->value;

	struct ontology_resource *rel = ontology_find_resource(kb,
			"printsATestMessageWhenCalled");
	struct ontology_resource *sbj = ontology_find_resource(kb, name);
	struct ontology_fact *fact = ontology_create_fact(kb, rel);
	ontology_add_argument_to_fact(kb, fact, sbj);

	if (ontology_check_fact(kb, fact) == 0) {
		printf("OXPL rocks!\n");
	}

	ontology_free_fact(fact);

	rel = ontology_find_resource(kb, "isPreceededBy");
	struct sl_list_node *qres = ontology_query_triple(kb,
			rel, sbj, NULL);

	while (qres != NULL) {
		struct ontology_resource *prec_fn = qres->data;

		execute_function(get_fn(root, prec_fn->name), kb, root);

		struct sl_list_node *tmp = qres;
		qres = qres->next;
		free(tmp);
	}
	/* end of ontology proof of concept */

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

static struct ast_node *get_fn(struct ast_node *ast, char *name)
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
		if (strcmp(((struct ast_node_str *)ident)->value, name) == 0)
			return cur;
	} while (NULL != (cur = cur->sibling));

	return NULL;
}

static void collect_facts(struct ast_node *root, struct ontology_database *kb)
{
	if (root == NULL)
		return;

	struct ast_node *cur = root->child;

	do {
		if (cur->type != ANT_FUNC)
			continue;

		char *name = ((struct ast_node_str *)cur->child->child)
			->value;

		char *resname = malloc(strlen(name) + 1);
		strcpy(resname, name);

		struct ontology_resource *res = ontology_create_resource(
				resname);
		ontology_add_resource(kb, res);
	} while (NULL != (cur = cur->sibling));

	cur = root->child;

	do {
		if (cur->type != ANT_TFACT)
			continue;

		struct ast_node *rel = cur->child;
		struct ast_node *sbj = rel->sibling;
		struct ast_node *obj = sbj->sibling;

		char *relname, *sbjname, *objname = NULL;

		relname = ((struct ast_node_str *)rel->child)->value;
		sbjname = ((struct ast_node_str *)sbj->child->child)
			->value;

		if (obj != NULL)
			objname = ((struct ast_node_str *)obj->child->child)
				->value;

		struct ontology_resource *relres = ontology_find_resource(
				kb, relname);
		struct ontology_resource *sbjres = ontology_find_resource(
				kb, sbjname);
		struct ontology_resource *objres = ontology_find_resource(
				kb, objname);

		if (relres == NULL || sbjres == NULL) {
			fprintf(stderr, "Warning: unknown sentence part\n");
			continue;
		}

		struct ontology_fact *fact = ontology_create_fact(kb, relres);
		ontology_add_argument_to_fact(kb, fact, sbjres);

		if (objres != NULL)
			ontology_add_argument_to_fact(kb, fact, objres);

		ontology_add_fact(kb, fact);
	} while (NULL != (cur = cur->sibling));
}

static void populate_kb(struct ontology_database *kb)
{
	add_predef_fact(kb, "isPreceededBy");
	add_predef_fact(kb, "printsATestMessageWhenCalled");
}

static void add_predef_fact(struct ontology_database *kb, char *name)
{
	char *resname = malloc(strlen(name) + 1);
	strcpy(resname, name);

	struct ontology_resource *res = ontology_create_resource(
			resname);
	ontology_add_resource(kb, res);
}
