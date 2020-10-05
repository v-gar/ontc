/*
 * src/shell.c
 *
 * Main shell for experimenting with the ontology database.
 *
 * Copyright (c) 2020 Viktor Garske
 */

/**
 * \file shell.c
 * \brief Implementation of the REPL shell
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "shell.h"
#include "onto.h"

enum shell_return_flag {
	SHELL_RETURN_FLAG_EXIT = 1 << 0
};

struct shell_return_value {
	/* Shell output */
	char *output;

	/* Flags for things like exit codes */
	enum shell_return_flag flags;
};

static struct shell_return_value prompt(struct ontology_database **db);
static struct shell_return_value evaluate(char *line,
		struct ontology_database **db);
static inline void free_shell_return_value(struct shell_return_value val);
static inline void print_out(char *text, char **output);

static void cmd_help(char **output);
static void cmd_create_db(struct ontology_database **db, char **output);
static void cmd_new_resource(struct ontology_database **db, char **output);
static void cmd_new_fact(struct ontology_database **db, char **output);
static void cmd_list_resources(struct ontology_database **db, char **output);
static void cmd_list_facts(struct ontology_database **db, char **output);

static struct ontology_resource *select_fact(struct ontology_database **db);
static inline void append(char **dst, char *src, int *i);
static inline void append_newline(char **dst, char *src, int *i);

/**
 * Start the REPL shell.
 */
void start_repl_shell(void)
{
	int active = 1; // 0 = exit

	struct ontology_database *db = NULL;
	printf("ontc interactive shell\n");
	printf("Enter \"help\" for a list of available commands.\n");

	while (active) {
		struct shell_return_value result = prompt(&db);

		if (result.flags & SHELL_RETURN_FLAG_EXIT) {
			active = 0;
			free_shell_return_value(result);
			continue;
		}

		if (result.output == NULL) {
			fprintf(stderr, "Error in output\n");
			free_shell_return_value(result);
			continue;
		}

		printf("%s", result.output);

		free_shell_return_value(result);
	}

	ontology_free_database(db);
}

/**
 * Prompt the user, evaluate the input and return the output.
 *
 * Returns a shell_return_value.
 */
static struct shell_return_value prompt(struct ontology_database **db)
{
	char line[20];

	printf("> ");
	fgets(line, sizeof(line), stdin);

	return evaluate(line, db);
}

/**
 * Evaluates a input
 *
 * Returns a shell_return_value.
 */
static struct shell_return_value evaluate(char *line,
		struct ontology_database **db)
{
	char return_flags = 0;
	char *output = NULL;

	size_t len = strlen(line);
	if (line[len - 1] == '\n')
		line[len - 1] = '\0';

	if (strcmp("exit", line) == 0
			|| strcmp("quit", line) == 0
			|| strcmp("q", line) == 0)
		return_flags |= SHELL_RETURN_FLAG_EXIT;
	else if (strcmp("help", line) == 0)
		cmd_help(&output);
	else if (strcmp("createdb", line) == 0)
		cmd_create_db(db, &output);
	else if (strcmp("newres", line) == 0)
		cmd_new_resource(db, &output);
	else if (strcmp("newfact", line) == 0)
		cmd_new_fact(db, &output);
	else if (strcmp("listres", line) == 0)
		cmd_list_resources(db, &output);
	else if (strcmp("listfacts", line) == 0)
		cmd_list_facts(db, &output);
	else
		print_out("Unknown command", &output);

	struct shell_return_value result = {
		output,
		return_flags
	};

	return result;
}

static void cmd_help(char **output)
{
	char *text = "Available commands:\n"
		"createdb\tCreate new database\n"
		"newres\t\tAdd new resource\n"
		"newfact\t\tAdd new fact\n"
		"listres\t\tList all resources\n"
		"listfacts\tList all facts\n"
		"quit\t\tQuit\n"
		"exit\t\tQuit";
	print_out(text, output);
}

/**
 * Create new ontology database, if it does not exist already.
 */
static void cmd_create_db(struct ontology_database **db, char **output)
{
	if (*db == NULL) {
		*db = ontology_create_database();
		print_out("Database created", output);
	} else
		print_out("Database exists already!", output);
}

/**
 * Add a new resource to the ontology database.
 */
static void cmd_new_resource(struct ontology_database **db, char **output)
{
	if (*db == NULL) {
		print_out("Error: no database available", output);
		return;
	}

	const short char_size = 32;

	char *name = malloc(sizeof(char) * (char_size + 1));

	printf("Name (%d chars): ", char_size);
	fgets(name, char_size, stdin);

	/* remove newline */
	int len = strlen(name);
	if (len > 0 && name[len - 1] == '\n') name[--len] = '\0';

	/* Create new resource and transfer ownership of name
	 * to the resource. */
	struct ontology_resource *res = ontology_create_resource(name);
	ontology_add_resource(*db, res);

	print_out("Resource created!", output);
}

static void cmd_new_fact(struct ontology_database **db, char **output)
{
	if (*db == NULL) {
		print_out("Error: no database available", output);
		return;
	}

	printf("Select predicate:\n");
	struct ontology_resource *pred = select_fact(db);

	if (NULL == pred) {
		print_out("Error while creating fact", output);
		return;
	}

	struct ontology_fact *fact = ontology_create_fact(*db, pred);
	printf("\n");

	struct ontology_resource *arg = NULL;

	do {
		printf("Select argument or press enter to finish:\n");
		arg = select_fact(db);
		if (NULL != arg) {
			ontology_add_argument_to_fact(*db, fact, arg);
			printf("\n");
		}
	} while (arg != NULL);

	ontology_add_fact(*db, fact);

	print_out("Fact created!", output);
}


static struct ontology_resource *select_fact(struct ontology_database **db)
{
	struct sl_list_node *node = (*db)->resource_head;

	int i = 0;
	while (node != NULL) {
		char *name = ((struct ontology_resource *)node->data)->name;
		printf("%i %s\n", ++i, name);
		node = node->next;
	}

	printf("Enter element to choose: ");
	char line[10];
	fgets(line, 10, stdin);

	if (strcmp(line, "\n") == 0)
		return NULL;

	long int selection = strtol(line, NULL, 10);

	if (!((selection > 0) && (selection <= i))) {
		fprintf(stderr, "Error: invalid selection\n");
		return NULL;
	}

	node = (*db)->resource_head;
	i = 0;

	while (node != NULL) {
		if (++i == selection)
			return node->data;
		else
			node = node->next;
	}

	return NULL;
}

/**
 * List all resources which are present in the ontology database.
 */
static void cmd_list_resources(struct ontology_database **db, char **output)
{
	if (*db == NULL) {
		print_out("Error: no database available", output);
		return;
	}

	struct sl_list_node *node = (*db)->resource_head;

	char *result = malloc(sizeof(char));
	result[0] = '\0';

	int i = 0; /* index of finish of last string */
	while (node != NULL) {
		char *to_add = ((struct ontology_resource *)node->data)->name;
		append_newline(&result, to_add, &i);
		node = node->next;
	}
	result[i - 1] = '\0';

	print_out(result, output);
	free(result);
}

/**
 * List all facts.
 */
static void cmd_list_facts(struct ontology_database **db, char **output)
{
	if (*db == NULL) {
		print_out("Error: no database available", output);
		return;
	}

	struct sl_list_node *node = (*db)->fact_head;

	char *result = malloc(sizeof(char));
	result[0] = '\0';

	int i = 0;
	while (node != NULL) {
		struct ontology_fact *fact = node->data;
		char *to_add = fact->predicate->name;
		append(&result, to_add, &i);
		append(&result, "(", &i);
		struct ontology_fact_argument_list_node *argcur =
			fact->argument_head;
		while (argcur != NULL) {
			to_add = argcur->argument->name;
			append(&result, to_add, &i);
			if (NULL != argcur->next)
				append(&result, ", ", &i);
			argcur = argcur->next;
		}
		append(&result, ").\n", &i);
		node = node->next;
	}
	result[i - 1] = '\0';

	print_out(result, output);
	free(result);
}

/**
 * Free the return output.
 */
static inline void free_shell_return_value(struct shell_return_value val) {
	free(val.output);
}

/**
 * Print string to the return output.
 */
static inline void print_out(char *text, char **output)
{
	size_t length = strlen(text) + 2;
	*output = malloc(sizeof(char) * length);
	strncpy(*output, text, strlen(text));
	(*output)[--length] = '\0';
	(*output)[--length] = '\n';
}

/**
 * Append to a string.
 */
static inline void append(char **dst, char *src, int *i)
{
	int len_added = strlen(src) + 1;

	*dst = realloc(*dst, *i + len_added);

	char *start = &(*dst)[*i];
	memset(start, '\0', len_added);
	strncpy(start, src, strlen(src));

	*i += --len_added;
	(*dst)[*i] = '\0';
}

/**
 * Append to a string.
 */
static inline void append_newline(char **dst, char *src, int *i)
{
	int len_added = strlen(src) + 1;

	*dst = realloc(*dst, *i + len_added);

	char *start = &(*dst)[*i];
	memset(start, '\0', len_added);
	strncpy(start, src, strlen(src));

	*i += len_added--;
	(*dst)[(*i) - 1] = '\n';
}
