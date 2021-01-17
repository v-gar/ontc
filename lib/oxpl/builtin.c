/*
 * lib/oxpl/builtin.c
 *
 * Built-in language features
 *
 * Copyright (c) 2020 Viktor Garske
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/**
 * \file builtin.c
 * \brief Implementation of built-in language features
 */

#include "builtin.h"
#include "stdio.h"

void lang_builtin_fn_print(struct ast_node *args)
{
	if (args == NULL) {
		fprintf(stderr, "Error: print: argument missing\n");
		return;
	}

	if (args->type != ANT_STR) {
		fprintf(stderr, "Error: print: wrong type of argument\n");
		return;
	}

	if (args->sibling != NULL) {
		fprintf(stderr, "Error: print: too many arguments\n");
		return;
	}

	struct ast_node_str *str_node_val = args->value;
	char *print_str = str_node_val->value;

	printf("%s", print_str);
}

void lang_builtin_fn_println(struct ast_node *args)
{
	if (args == NULL) {
		fprintf(stderr, "Error: print: argument missing\n");
		return;
	}

	if (args->type != ANT_STR) {
		fprintf(stderr, "Error: print: wrong type of argument\n");
		return;
	}

	if (args->sibling != NULL) {
		fprintf(stderr, "Error: print: too many arguments\n");
		return;
	}

	struct ast_node_str *str_node_val = args->value;
	char *print_str = str_node_val->value;

	printf("%s\n", print_str);
}
