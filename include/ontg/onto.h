/*
 * include/ontg/onto.h
 *
 * Contains structs and functions for the ontology system.
 *
 * Copyright (c) 2020 Viktor Garske
 */

#ifndef H_ONTOLOGY
#define H_ONTOLOGY
#include "util.h"

/**
 * \file onto.h
 * \brief Interface (with fundamental structs) for the ontology system
 */

struct ontology_database;
struct ontology_resource;
struct ontology_fact;

/**
 * An ontology database contains all information about the ontology.
 *
 * Allocated by: ontology_create_database
 * Deallocated by: ontology_free_database
 */
struct ontology_database {
	/* List of resources. Can be NULL if no resources present. */
	struct sl_list_node *resource_head;

	/* List of facts. Can be NULL if no resources facts. */
	struct sl_list_node *fact_head;
};

/**
 * Ontology resources are the individual constants.
 *
 * Allocated by: ontology_create_resource
 * Deallocated by: ontology_free_resource
 */
struct ontology_resource {
	/* Name of the resource */
	char *name;
};

/**
 * Ontology facts are atomic sentences.
 *
 * Allocated by: ontology_create_fact
 * Deallocated by: ontology_free_fact
 */
struct ontology_fact {
	/* Resource acting as a predicate */
	struct ontology_resource *predicate;

	/* List of resources acting as the individual constants. */
	struct ontology_fact_argument_list_node *argument_head;
};

/**
 * Singly-linked list for all arguments of an ontology fact.
 *
 * Managed by: ontology_fact
 */
struct ontology_fact_argument_list_node {
	struct ontology_resource *argument;
	struct ontology_fact_argument_list_node *next;
};

/* == MAIN FUNCTIONS == */

/* Database management */
struct ontology_database *ontology_create_database(void);

/* Memory management */
void ontology_free_database(struct ontology_database *db);
void ontology_free_resource(struct ontology_resource *res);
void ontology_free_fact(struct ontology_fact *fact);

/* Resource and fact management */
struct ontology_resource *ontology_create_resource(char *name);
struct ontology_fact *ontology_create_fact(struct ontology_database *db,
		struct ontology_resource *predicate);
void ontology_add_argument_to_fact(struct ontology_database *db,
		struct ontology_fact *fact,
		struct ontology_resource *argument);

void ontology_add_resource(struct ontology_database *db,
		struct ontology_resource *res);
void ontology_add_fact(struct ontology_database *db,
		struct ontology_fact *fact);

struct ontology_resource *ontology_find_resource(struct ontology_database *db,
		char *name);

int ontology_check_fact(struct ontology_database *db,
		struct ontology_fact *fact);
struct sl_list_node *ontology_query_triple(struct ontology_database *db,
		struct ontology_resource *rel,
		struct ontology_resource *sbj,
		struct ontology_resource *obj);

#endif
