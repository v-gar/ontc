/*
 * lib/ontg/onto.c
 *
 * Main ontology functions for constructing and maintaining
 * ontology databases.
 *
 * Copyright (c) 2020 Viktor Garske
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/**
 * \file onto.c
 * \brief Functions for the manipulation of ontology databases
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "onto.h"
#include "util.h"

/**
 * Create an ontology database and sets up the linked lists.
 *
 * Returns a pointer to the database or NULL if error happend.
 */
struct ontology_database *ontology_create_database(void)
{
	/* General database */
	struct ontology_database *db;
	db = malloc(sizeof (struct ontology_database));

	if (NULL == db) {
		fprintf(stderr, "Error: malloc failed while creating DB\n");
		return NULL;
	}

	/* Set up lists */
	db->resource_head = NULL;
	db->fact_head = NULL;

	return db;
}

/**
 * Free the database and ALL of their resources and facts.
 */
void ontology_free_database(struct ontology_database *db)
{
	if (db == NULL) {
		return;
	}

	/* Facts */
	void *cur = db->fact_head, *next = db->fact_head;

	while (NULL != cur) {
		next = ((struct sl_list_node *) cur)->next;
		struct ontology_fact *fact = 
			((struct sl_list_node *) cur)->data;
		ontology_free_fact(fact);
		free(cur);
		cur = next;
	}

	/* Resources */
	cur = db->resource_head, next = db->resource_head;

	while (NULL != cur) {
		next = ((struct sl_list_node *) cur)->next;
		struct ontology_resource *res =
			((struct sl_list_node *) cur)->data;
		ontology_free_resource(res);
		free(cur);
		cur = next;
	}

	free(db);
}


void ontology_free_resource(struct ontology_resource *res)
{
	free(res->name);
	free(res);
}

void ontology_free_fact(struct ontology_fact *fact)
{
	struct ontology_fact_argument_list_node
		*cur = fact->argument_head,
		*next = fact->argument_head;

	while (NULL != cur) {
		next = cur->next;
		/* We do NOT free the resource here as
		it is being freed by ontology_free_resource. */
		free(cur);
		cur = next;
	}

	free(fact);
}

/**
 * Create a resource.
 *
 * Note that you transfer the char ownership to this struct!
 */
struct ontology_resource *ontology_create_resource(char *name)
{
	struct ontology_resource *res;
	res = malloc(sizeof(struct ontology_resource));

	if (NULL == res) {
		fprintf(stderr, "Error: malloc failed while "
				"creating resource\n");
		return NULL;
	}

	/* Set up resource */
	res->name = name;

	return res;
}


/**
 * Create a new fact.
 *
 * This method that that all resources originate from the same database
 * so that the database stays consistent.
 */
struct ontology_fact *ontology_create_fact(struct ontology_database *db,
		struct ontology_resource *predicate)
{
	struct ontology_fact *fact;
	fact = malloc(sizeof(struct ontology_fact));

	if (NULL == fact) {
		fprintf(stderr, "Error: malloc failed while "
				"creating fact\n");
		return NULL;
	}

	/* Consistency check */
	int found = 0;
	struct sl_list_node *cur = db->resource_head;

	while (NULL != cur) {
		if (predicate == cur->data) {
			found = 1;
			break;
		} else {
			cur = cur->next;
		}
	}

	if (!found) {
		fprintf(stderr, "Error: predicate being added to fact is "
				"not present in resource list\n");
		free(fact);
		return NULL;
	}
	/* Consistency check finished */

	fact->predicate = predicate;
	fact->argument_head = NULL;

	return fact;
}

/**
 * Add argument to fact.
 *
 * This method that that all resources originate from the same database
 * so that the database stays consistent.
 */
void ontology_add_argument_to_fact(struct ontology_database *db,
		struct ontology_fact *fact,
		struct ontology_resource *argument)
{
	/* Consistency check */
	int found = 0;
	struct sl_list_node *cur = db->resource_head;

	while (NULL != cur) {
		if (argument == cur->data) {
			found = 1;
			break;
		} else {
			cur = cur->next;
		}
	}

	if (!found) {
		fprintf(stderr, "Error: predicate being added to fact is "
				"not present in resource list\n");
		return;
	}
	/* Consistency check finished */

	/* Build argument list node */
	struct ontology_fact_argument_list_node *node;
	node = calloc(sizeof(struct ontology_fact_argument_list_node), 1);

	if (NULL == node) {
		fprintf(stderr, "Error: malloc failed for new argument "
				"list node\n");
		return;
	}
	node->argument = argument;

	struct ontology_fact_argument_list_node **cursor =
		&fact->argument_head;

	while (NULL != *cursor) {
		cursor = &(*cursor)->next;
	}

	*cursor = node;
}

/**
 * Add resource to ontology database.
 */
void ontology_add_resource(struct ontology_database *db,
		struct ontology_resource *res)
{
	if (NULL == db || NULL == res) {
		fprintf(stderr, "Error: missing DB or resource");
		return;
	}

	/* Build list node */
	struct sl_list_node *node;
	node = calloc(sizeof(struct sl_list_node), 1);

	if (NULL == node) {
		fprintf(stderr, "Error: malloc failed for new resource "
				"list node\n");
		return;
	}

	node->data = res;
	node->next = NULL;

	struct sl_list_node **cursor = &db->resource_head;

	while (NULL != *cursor) {
		cursor = &(*cursor)->next;
	}

	*cursor = node;
}

/**
 * Adds new fact to the ontology database.
 */
void ontology_add_fact(struct ontology_database *db,
		struct ontology_fact *fact)
{
	if (NULL == db || NULL == fact) {
		fprintf(stderr, "Error: missing DB or fact");
		return;
	}

	/* Build list node */
	struct sl_list_node *node;
	node = malloc(sizeof(struct sl_list_node));

	if (NULL == node) {
		fprintf(stderr, "Error: malloc failed for new fact "
				"list node\n");
		return;
	}

	node->data = fact;
	node->next = NULL;

	struct sl_list_node **cursor = &db->fact_head;

	while (NULL != *cursor) {
		cursor = &(*cursor)->next;
	}

	*cursor = node;
}

struct ontology_resource *ontology_find_resource(struct ontology_database *db,
		char *name)
{
	if (db == NULL || name == 0)
		return NULL;

	struct sl_list_node *cursor = db->resource_head;

	while (NULL != cursor) {
		struct ontology_resource *res = cursor->data;

		if (strcmp(name, res->name) == 0)
			return res;

		cursor = cursor->next;
	}

	return NULL;
}

static int ontology_check_fact_args(
		struct ontology_fact_argument_list_node *argcur,
		struct ontology_fact_argument_list_node *kbargcur)
{
	while (argcur != NULL && kbargcur != NULL) {
		if (argcur->argument == kbargcur->argument) {
			argcur = argcur->next;
			kbargcur = kbargcur->next;
		} else
			break;
	}

	return argcur == kbargcur ? 0 : 1;
}

int ontology_check_fact(struct ontology_database *db,
		struct ontology_fact *fact)
{
	struct sl_list_node *cursor = db->fact_head;

	while (NULL != cursor) {
		/*
		 * TODO: replace with something better as rudimentary ptr
		 * check
		 */
		struct ontology_fact *kbfact = cursor->data;

		if (kbfact->predicate != fact->predicate) {
			/* continue searching */
			cursor = cursor->next;
			continue;
		}

		int result = ontology_check_fact_args(fact->argument_head,
				kbfact->argument_head);

		if (result == 0) {
			return 0;
		}

		cursor = cursor->next;
	}

	/* fact missing? */
	return 1;
}

static void add_to_query_result(struct sl_list_node **node,
		struct ontology_resource *res)
{
	while (*node != NULL) {
		node = &(*node)->next;
	}

	*node = malloc(sizeof(struct sl_list_node));
	if (!*node) {
		fprintf(stderr, "Error: OOM!\n");
		return;
	}

	(*node)->data = res;
	(*node)->next = NULL;
}

struct sl_list_node *ontology_query_triple(struct ontology_database *db,
		struct ontology_resource *rel,
		struct ontology_resource *sbj,
		struct ontology_resource *obj)
{
	if (db == NULL)
		return NULL;

	if (sbj != NULL && obj != NULL) {
		fprintf(stderr, "Error: no query goal\n");
		return NULL;
	}

	struct sl_list_node *result = NULL;

	struct sl_list_node *cursor = db->fact_head;

	while (NULL != cursor) {
		/*
		 * TODO: replace with something better as rudimentary ptr
		 * check
		 */
		struct ontology_fact *kbfact = cursor->data;

		if (kbfact->predicate == rel) {
			struct ontology_fact_argument_list_node *kbsbj =
				kbfact->argument_head;
			if (sbj == NULL && kbsbj->next != NULL) {
				if (kbsbj->next->argument == obj) {
					add_to_query_result(
						&result,
						kbsbj->argument
					);
				}
			} else if (obj == NULL && kbsbj->next != NULL) {
				if (kbsbj->argument == sbj) {
					add_to_query_result(
						&result,
						kbsbj->next->argument
					);
				}
			}
		}

		cursor = cursor->next;
	}

	return result;
}
