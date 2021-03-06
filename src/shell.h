/*
 * src/shell.h
 *
 * Shell interface
 *
 * Copyright (c) 2020 Viktor Garske
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef H_SHELL
#define H_SHELL

/**
 * \file shell.h
 * \brief Interface for the REPL shell
 */

#include "onto.h"

void start_repl_shell(struct ontology_database *kb);

#endif
