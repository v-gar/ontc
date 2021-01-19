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
	if (fn_node == NULL || AST_NODE_TYPE(fn_node) != ANT_FUNC)
		return 1;

	/* walk through function child nodes */
	struct ast_node *cur;

	cur = AST_NODE_CHLD1(fn_node); /* signature (not implemented yet) */

	/* ontology proof of concept */
	AST_NODE_CAST(name_str_node_val, AST_NODE_CHLD1(cur), str);
	char *name = name_str_node_val->value;

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

	AST_NODE_NEXT_SIBL(cur); /* body */

	/* execute function body */
	while (cur != NULL) {
		if (ANT_CALL == AST_NODE_TYPE(cur)) {
			execute_call(cur);
		}

		/* next child of function */
		AST_NODE_NEXT_SIBL(cur);
	}

	return 0;
}

static int execute_call(struct ast_node *call_node)
{
	/* check whether node is call node */
	if (call_node == NULL || AST_NODE_TYPE(call_node) != ANT_CALL)
		return 1;

	struct ast_node *call_expr = AST_NODE_CHLD1(call_node);
	struct ast_node *args = AST_NODE_CHLD2(call_node);

	/* test for simple built-in function */
	if (AST_NODE_TYPE(call_expr) == ANT_SCOPE
			&& AST_NODE_CHLD1(call_expr) != NULL
			&& AST_NODE_TYPE(AST_NODE_CHLD1(call_expr)) == ANT_STR
			&& AST_NODE_CHLD2(call_expr) == NULL) /* one level */
	{
		AST_NODE_CAST(node_val, AST_NODE_CHLD1(call_expr), str);
		char *name = node_val->value;

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

	struct ast_node *cur = AST_NODE_CHLD1(ast);

	/* find main function */
	do {
		if (AST_NODE_TYPE(cur) != ANT_FUNC)
			continue;

		/* if this is a function ... */
		struct ast_node *sig = AST_NODE_CHLD1(cur);
		struct ast_node *ident = AST_NODE_CHLD1(sig);

		if (AST_NODE_TYPE(ident) != ANT_STR)
			continue;

		/* ... and the identifier is a string ... */
		AST_NODE_CAST(ident_str_node_val, ident, str);
		if (strcmp(ident_str_node_val->value, name) == 0)
			return cur;
	} while (NULL != AST_NODE_NEXT_SIBL(cur));

	return NULL;
}

static void collect_facts(struct ast_node *root, struct ontology_database *kb)
{
	if (root == NULL)
		return;

	struct ast_node *cur = AST_NODE_CHLD1(root);

	do {
		if (AST_NODE_TYPE(cur) != ANT_FUNC)
			continue;

		struct ast_node *sig = AST_NODE_CHLD1(cur);
		struct ast_node *str_node = AST_NODE_CHLD1(sig);
		AST_NODE_CAST(str_node_val, str_node, str);
		char *name = str_node_val->value;

		char *resname = malloc(strlen(name) + 1);
		strcpy(resname, name);

		struct ontology_resource *res = ontology_create_resource(
				resname);
		ontology_add_resource(kb, res);
	} while (NULL != AST_NODE_NEXT_SIBL(cur));

	cur = AST_NODE_CHLD1(root); /* reset */

	do {
		if (AST_NODE_TYPE(cur) != ANT_TFACT)
			continue;

		struct ast_node *rel = AST_NODE_CHLD1(cur); /* ANT_ADDR */
		struct ast_node *sbj = AST_NODE_CHLD2(cur); /* ANT_SCOPE */
		struct ast_node *obj = AST_NODE_CHLD3(cur); /* ANT_ADDR */

		char *relname, *sbjname, *objname = NULL;

		struct ast_node *sbj_scope = AST_NODE_CHLD1(sbj);
		AST_NODE_CAST(rel_str_node_val, AST_NODE_CHLD1(rel), str);
		AST_NODE_CAST(sbj_str_node_val, AST_NODE_CHLD1(sbj_scope),
				str);

		relname = rel_str_node_val->value;
		sbjname = sbj_str_node_val->value;

		if (obj != NULL) {
			struct ast_node *obj_scope = AST_NODE_CHLD1(obj);
			AST_NODE_CAST(obj_str_node_val,
					AST_NODE_CHLD1(obj_scope), str);
			objname = obj_str_node_val->value;
		}

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
	} while (NULL != AST_NODE_NEXT_SIBL(cur));
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
