/**
 * src/exec.h
 *
 * Ontology Program executor interface
 *
 * Copyright (c) 2020 Viktor Garske
 */

#ifndef H_EXEC
#define H_EXEC

/**
 * \file exec.h
 * \brief Executor interface
 */

#include <stdio.h>

int exec_program(FILE *fp);
int debug_ontology(FILE *fp);

#endif /* ifndef H_EXEC */
